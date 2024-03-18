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

#pragma once

#include <QFileInfo>
#include <QObject>
#include <QRunnable>

class GameSearcher : public QObject, public QRunnable
{
    Q_OBJECT

public:
    struct Result { QString path; QString name; };

    GameSearcher();
    void run() override;

    void setStart(const QString& start);
    QList<Result> results() const;

    /*!
     * \brief Return a potential game name from a QFileInfo of a chitin.key
     * \param fileInfo
     * \return
     */
    QString gameName(const QFileInfo& info) const;

signals:
    void progress(const QString& name);
    void finished();

private:
    QString m_start;
    QList<Result> m_results;
};


QDebug operator<<(QDebug debug, const GameSearcher::Result& roll);