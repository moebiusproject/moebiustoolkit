/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2023 Alejandro Exojo Piqueras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gamesearcher.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>

GameSearcher::GameSearcher()
{
    setAutoDelete(false);
}

void GameSearcher::run()
{
    m_results.clear();

    QDirIterator iterator(m_start, QDir::NoFilter, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        const QFileInfo info = iterator.fileInfo();
        if (info.isDir()) {
            emit progress(info.absolutePath());
        } else {
            const QString name = info.fileName();
            if (name.compare(QLatin1String("chitin.key"), Qt::CaseInsensitive) == 0) {
                m_results.append({.path = info.absoluteFilePath(), .name = gameName(info)});
            }
        }
    }
    emit progress(tr("Search finished"));
    emit finished();
}

void GameSearcher::setStart(const QString& start)
{
    m_start = start;
}

QList<GameSearcher::Result> GameSearcher::results() const
{
    return m_results;
}

// We try to find a line like the following in engine.lua:
//     engine_name = "Baldur's Gate II - Enhanced Edition"
// But hopefully skipping comments, just in case. This is by no means proper
// Lua parsing. Not even close. Only looks for doubly quoted text because it's
// likely that people will want to write "Baldur's" instead of 'Baldur\'s'.
QString GameSearcher::gameName(const QFileInfo& info) const
{
    const QDir dir = info.absoluteDir();
    if (dir.exists(QLatin1String("engine.lua"))) {
        QFile engine(dir.absoluteFilePath(QLatin1String("engine.lua")));
        if (engine.open(QIODevice::ReadOnly)) {
            const QByteArray contents = engine.readAll();
            for (const QByteArray& bytes : contents.split('\n')) {
                const QString line = QString::fromLatin1(bytes);
                const QRegularExpression exp(QLatin1String(R"===(
                        ^(?!.*--) # Negative lookahead to prevent Lua comments.
                        .*engine_name\s*=\s* # Allow whitespace around the equals.
                        # Match between single quotes only (why? because "Baldur's")
                        "([^"]*)"
                    )==="),
                    QRegularExpression::ExtendedPatternSyntaxOption);
                auto match = exp.match(line);
                if (match.hasMatch()) {
                    return match.captured(1);
                }
            }
        }
    }
    return QString();
}

QDebug operator<<(QDebug debug, const GameSearcher::Result& result)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "(name=" << result.name << ", path=" << result.path << ")";
    return debug;
}
