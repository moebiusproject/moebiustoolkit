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

#include "packed.h"

#include <QByteArray>
#include <QDataStream>
#include <QVector>

struct KeyFile
{
    struct BifEntry {
        quint32 size = 0;
        quint32 nameStart = 0;
        quint16 nameLenght = 0;
        quint16 location = 0; ///< Always 1 in all the files that I've found.
    } PACKED_ATTRIBUTE;

    struct ResourceEntry {
        char name[8] = {0};
        quint16 type = 0;
        quint32 locator = 0;
    } PACKED_ATTRIBUTE;

    bool isValid() const;

    // Header.
    quint8 version = 0;
    quint32 bifCount = 0;
    quint32 resourceCount = 0;
    quint32 bifStart = 0;
    quint32 resourceStart = 0;

    // Body.
    QVector<BifEntry> bifEntries;
    QByteArray bifNames;
    QVector<ResourceEntry> resourceEntries;
};

QDataStream& operator>>(QDataStream& stream, KeyFile& file);

