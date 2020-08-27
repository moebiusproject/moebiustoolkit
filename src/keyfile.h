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
#include "resourcetype.h"

#include <QByteArray>
#include <QDataStream>
#include <QMetaType>
#include <QVector>

struct KeyFile
{
    struct BiffEntry {
        QString name;
        qint64 size = 0;
        // TODO: Make this an enum? Or remove, given that we don't make any use of it.
        quint16 location = 0; ///< Always 1 in all the files that I've found (=> data directory).
    };

    struct ResourceEntry {
        QString name;
        ResourceType type = NoType;
        quint16 source = 0;  ///< Index of which BIFF file from the list of BIFF files.
        quint16 index = 0;   ///< Index of which file from the list of files inside a BIFF.
        quint32 locator = 0; ///< The "locator" which bundles BIFF number and file number.
    };

    bool isValid() const;

    QVector<BiffEntry> biffEntries;
    QVector<ResourceEntry> resourceEntries;

private:
    friend class tst_KeyFile;
    friend QDataStream& operator>>(QDataStream& stream, KeyFile& file);

    quint8 version = 0;
};
Q_DECLARE_METATYPE(KeyFile::ResourceEntry)

QDataStream& operator>>(QDataStream& stream, KeyFile& file);

