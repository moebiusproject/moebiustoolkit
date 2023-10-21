/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020 Alejandro Exojo Piqueras
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
#include "resourcetype.h"

#include <QByteArray>
#include <QVector>

class QDataStream;

struct BiffFile
{
    /*!
     * \brief Type of BIFF file
     *
     * The names are based on the title given in IESDP, more than the "magic
     * bytes" at the start of the file. So "BIF V1.0" (bytes) becomes "BIFC V1"
     * (IESDP title).
     */
    enum Type {
        Invalid = 0,
        BiffV1,
        BifcV1,
        BifcV1x0,
    } type = Invalid;

    struct FileEntriesIndex {
        quint32 locator; ///< Might be used to make a check against what the CHITIN.KEY says
        quint32 offset;
        quint32 size;
        ResourceType type;
        quint16 unknown;
    } PACKED_ATTRIBUTE;

    struct TilesetEntriesIndex {
        quint32 locator;
        quint32 offset;
        quint32 count;
        quint32 size;
        ResourceType type;
        quint16 unknown;
    } PACKED_ATTRIBUTE;

    bool isValid() const;

    QVector<FileEntriesIndex> fileEntries;
    QVector<TilesetEntriesIndex> tilesetEntries;

private:
    friend QDataStream& operator>>(QDataStream& stream, BiffFile& file);

};

QDataStream& operator>>(QDataStream& stream, BiffFile& file);

