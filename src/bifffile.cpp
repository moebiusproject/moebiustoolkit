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

#include "bifffile.h"

#include <QDebug>
#include <QScopeGuard>

bool BiffFile::isValid() const
{
    return type != Invalid;
}

// https://gibberlings3.github.io/iesdp/file_formats/ie_formats/bif_v1.htm
// Structure:
// - HEADER
// - CONTENT (files bundled toghether)
// - FILE ENTRIES (indexes into the content)
// - TILE ENTRIES (indexes into the content)
// NOTE: The order might not be the same for all files, but I've not found one
// yet. See: https://github.com/Gibberlings3/iesdp/pull/90

QDataStream& operator>>(QDataStream& stream, BiffFile& file)
{
    stream.setByteOrder(QDataStream::LittleEndian);

    auto onFailure = qScopeGuard([&file] {
        file.type = BiffFile::Invalid;
        file.fileEntries.clear();
        file.tilesetEntries.clear();
    });

    QByteArray signature(8, '\0');
    stream.readRawData(signature.data(), signature.size());

    if (qstrncmp(signature.data(), "BIFFV1  ", signature.size()) == 0)
        file.type = BiffFile::BiffV1;
    else
    if (qstrncmp(signature.data(), "BIF V1.0", signature.size()) == 0)
        file.type = BiffFile::BifcV1;
    else
    if (qstrncmp(signature.data(), "BIFCV1.0", signature.size()) == 0)
        file.type = BiffFile::BifcV1x0;
    else
        return stream;

    // TODO: support something else, and do it nicely to support all, if possible.
    // According to Gibberlings3: "some older classic games used compressed bif format"
    // "nowadays they dont use it, just the v1 bif which is sufficient for storage"
    Q_ASSERT(file.type == BiffFile::BiffV1);

    quint32 fileEntriesCount, tilesetEntriesCount, fileEntriesOffset;
    stream >> fileEntriesCount >> tilesetEntriesCount >> fileEntriesOffset;

    stream.device()->seek(fileEntriesOffset);

    if (stream.status() != QDataStream::Ok)
        return stream;

    file.fileEntries.clear();
    file.fileEntries.resize(fileEntriesCount);
#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(fileEntries.data()),
                       sizeof(FileEntriesIndex) * fileEntriesCount);
#else
    for (BiffFile::FileEntriesIndex& entry : file.fileEntries) {
        stream >> entry.locator >> entry.offset >> entry.size >> entry.type >> entry.unknown;
    }
#endif

    file.tilesetEntries.clear();
    file.tilesetEntries.resize(tilesetEntriesCount);
#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(tileset.data()),
                       sizeof(TilesetEntriesIndex) * tilesetEntriesCount);
#else
    for (BiffFile::TilesetEntriesIndex& entry : file.tilesetEntries) {
        stream >> entry.locator >> entry.offset >> entry.count >> entry.size
               >> entry.type >> entry.unknown;
    }
#endif

    Q_ASSERT(stream.atEnd());

    if (stream.status() == QDataStream::Ok)
        onFailure.dismiss();
    return stream;
}
