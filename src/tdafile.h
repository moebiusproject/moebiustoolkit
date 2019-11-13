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

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

class QIODevice;

struct TdaFile
{
    static TdaFile from(QIODevice& device);

    QStringList rowNames() const;

    QStringList row(int index) const;
    QStringList row(const QString& name) const;

    QString cell(int row, int col) const;
    QString cell(const QString& rowName, int col) const;
    QString cell(int row, const QString& columnName) const;
    QString cell(const QString& rowName, const QString& columnName) const;

    int rowCount() const;
    int columnCount() const;

    QString defaultValue;
    QStringList headers;
    // NB: First element in each entry is the entry/row name, followed by the values.
    // This is due to how the headers are descriptions that align with the entries
    // to make it look like a formatted table.
    QVector<QStringList> entries;
};
