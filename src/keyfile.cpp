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

#include "keyfile.h"

#include <QDebug>
#include <QScopeGuard>

bool KeyFile::isValid() const
{
    return version != 0;
}

// https://gibberlings3.github.io/iesdp/file_formats/ie_formats/key_v1.htm
// Structure:
// - HEADER
// - BIF ENTRIES: Sequence of BifIndex, which refer to strings after the entries.
// - STRINGS (we read it as a large string, then we can index into sections).
// - RESOURCE ENTRIES: Sequence of ResourceIndex.

QDataStream& operator>>(QDataStream& stream, KeyFile& file)
{
    stream.setByteOrder(QDataStream::LittleEndian);

    auto onFailure = qScopeGuard([&file] {
        file.version = 0;
        file.bifIndexes.clear();
        file.rawBifNames.clear();
        file.resourceIndexes.clear();

        file.bifDetails.clear();
    });

    char header[8];
    stream.readRawData(header, sizeof(header));
    constexpr auto start = "KEY V1  ";
    if (qstrncmp(header, start, sizeof(start)) == 0)
        file.version = 1;
    else
        return stream;

#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(&file.bifCount), 4*sizeof(quint32));
#else
    stream >> file.bifCount >> file.resourceCount >> file.bifStart >> file.resourceStart;
#endif

    if (stream.status() != QDataStream::Ok)
        return stream;

    // NOTE (semi-TODO): Seems like we should use the bifStart value to find
    // where the BIF entries start, but I don't see the point. It should always
    // be 24 (if I understand it correctly): the size of the header, because the
    // BIF entries follow immediately the header (which has a fixed size).
    // It is also the current position
    Q_ASSERT(file.bifStart == stream.device()->pos());

    file.bifIndexes.clear();
    file.bifIndexes.resize(file.bifCount);
#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(file.bifIndexes.data()),
                       sizeof(KeyFile::BifIndex) * file.bifCount);
#else
    for (KeyFile::BifIndex& entry : file.bifIndexes) {
        stream >> entry.size >> entry.nameStart >> entry.nameLength >> entry.location;
    }
#endif

    if (stream.status() != QDataStream::Ok)
        return stream;

    constexpr auto headerSize = sizeof(header) * sizeof(char)
            + sizeof(file.bifCount) + sizeof(file.resourceCount)
            + sizeof(file.bifStart) + sizeof(file.resourceStart);


    /// Location where the sequence of bif strings begin. Substract this to the "start of file name".
    const auto stringsStart = headerSize + sizeof(KeyFile::BifIndex) * file.bifCount;
    const auto stringsLength = file.resourceStart - stringsStart;
    Q_ASSERT(qint64(stringsStart) == stream.device()->pos());

    file.rawBifNames.clear();
    file.rawBifNames.resize(stringsLength);
    stream.readRawData(file.rawBifNames.data(), stringsLength);

    // Now that bif indexes and the strings of the bif names are read, get the
    // sequence of actual useful bif structs, with more convenient format.
    file.bifDetails.reserve(file.bifIndexes.size());
    for (const KeyFile::BifIndex& entry : file.bifIndexes) {
        KeyFile::BifDetails bif;
        bif.size = entry.size;
        const auto realStart = file.rawBifNames.constData() + entry.nameStart - stringsStart;
        bif.name = QString::fromLatin1(realStart);
        file.bifDetails << bif;
    }

    if (stream.status() != QDataStream::Ok)
        return stream;

    file.resourceIndexes.clear();
    file.resourceIndexes.resize(file.resourceCount);

#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(file.resourceIndexes.data()),
                       sizeof(KeyFile::ResourceIndex) * file.resourceCount);
#else
    for (KeyFile::ResourceIndex& entry : file.resourceIndexes) {
        stream.readRawData(entry.name, sizeof(entry.name));
        stream >> entry.type >> entry.locator;
    }
#endif

    if (stream.status() == QDataStream::Ok)
        onFailure.dismiss();

    return stream;
}
