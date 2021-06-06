/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019 Alejandro Exojo Piqueras
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

#include "tdafile.h"

#include <QDebug>
#include <QIODevice>
#include <QRegularExpression>

TdaFile TdaFile::from(QIODevice& device)
{
    Q_ASSERT(device.isOpen());
    Q_ASSERT(device.isReadable());
    TdaFile table;

    // TODO: handle encrypted files.
    const QByteArray signature = device.readLine();
    // TODO: some files I've found have a V1.1 signature, but that is not
    // covered in the IESDP at all. Same as file without signature or defaults.
    //if (!signature.contains("2DA") || !signature.contains("V1.0"))
    if (!signature.contains("2DA"))
    {
        return table;
    }

    QRegularExpression space(QStringLiteral("\\s+"));
    const QByteArray defaultValueLine = device.readLine();
    table.defaultValue = QString::fromLatin1(defaultValueLine.trimmed());

    const QByteArray headersLine = device.readLine();
    const QString& headers = QString::fromLatin1(headersLine.trimmed());
    for (const QString& header : headers.split(space, Qt::SkipEmptyParts))
        table.headers.append(header);

    while (!device.atEnd()) {
        const QString& row = QString::fromLatin1(device.readLine().trimmed());
        if (row.isEmpty())
            continue;
        table.entries.append(row.split(space, Qt::SkipEmptyParts));
    }

    return table;
}

QStringList TdaFile::rowNames() const
{
    QStringList result;
    for (const QStringList& row : entries)
        result.append(row.first());
    return result;
}

QStringList TdaFile::row(int index) const
{
    QStringList entry = entries.at(index);
    entry.removeFirst(); // take out the row name/stringified index
    return entry;
}

QStringList TdaFile::row(const QString& name) const
{
    for (int index = 0, last = entries.size(); index != last; ++index) {
        const QStringList& entry = entries.at(index);
        if (entry.first() == name) {
            QStringList result = entry;
            result.removeFirst();
            return result;
        }
    }
    // TODO: maybe remove the assert, just leave the empty result as indication
    // of whether there is a row by that name.
    Q_ASSERT_X(false, Q_FUNC_INFO, "No row by that name");
    return QStringList();
}

// Reminder: in 2DA files, rows can have less entries than the columns. Then the
// default value is used when a missing value is asked for. Since the headers
// are not passed, the caller has to check for `col < colCount()`.
static QString cellFromRow(const QStringList& row, int col, const QString& byDefault)
{
    if (row.isEmpty()) // Should not happen, but it could when the API is misused.
        return QString();
    if (col >= row.size())
        return byDefault;
    else
        return row.at(col);
}

QString TdaFile::cell(int row, int col) const
{
    Q_ASSERT(col < columnCount());
    return cellFromRow(this->row(row), col, defaultValue);
}

QString TdaFile::cell(const QString& rowName, int col) const
{
    Q_ASSERT(col < columnCount());
    Q_ASSERT(rowNames().contains(rowName));
    return cellFromRow(this->row(rowName), col, defaultValue);
}

QString TdaFile::cell(int row, const QString& columnName) const
{
    const int column = headers.indexOf(columnName);
    Q_ASSERT_X(column != -1, Q_FUNC_INFO, "Given column name not found in the headers");
    return cell(row, column);
}

QString TdaFile::cell(const QString& rowName, const QString& columnName) const
{
    const int column = headers.indexOf(columnName);
    Q_ASSERT_X(column != -1, Q_FUNC_INFO, "Given column name not found in the headers");
    return cell(rowName, column);
}

int TdaFile::rowCount() const
{
    return entries.size();
}

int TdaFile::columnCount() const
{
    return headers.size();
}
