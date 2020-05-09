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
    struct BifDetails {
        QString name;
        qint64 size = 0;
    };
    bool isValid() const;

    QVector<BifDetails> bifDetails;

private:
    friend class tst_KeyFile;
    friend QDataStream& operator>>(QDataStream& stream, KeyFile& file);

    // TODO: Once the API has settled for good, move this struct fully to the
    // internals, and remove the duplication of API.
    struct BifIndex {
        quint32 size = 0;
        quint32 nameStart = 0;
        quint16 nameLength = 0;
        quint16 location = 0; ///< Always 1 in all the files that I've found (=> data directory).
    } PACKED_ATTRIBUTE;

    // TODO: Make a proper struct with name being a QString, and public API.
    struct ResourceIndex {
        char name[8] = {0}; ///< Null terminated if shorter than 8, but not otherwise.
        quint16 type = 0;
        quint32 locator = 0;
    } PACKED_ATTRIBUTE;

    // Header.
    quint8 version = 0;
    quint32 bifCount = 0;
    quint32 resourceCount = 0;
    quint32 bifStart = 0;
    quint32 resourceStart = 0;

    // Body.
    QVector<BifIndex> bifIndexes;
    QByteArray rawBifNames; // FIXME: Get rid of this and just get the strings from the file.
    QVector<ResourceIndex> resourceIndexes;
};

QDataStream& operator>>(QDataStream& stream, KeyFile& file);

