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

#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <QScopeGuard>

bool KeyFile::isValid() const
{
    return version != 0;
}

// https://gibberlings3.github.io/iesdp/file_formats/ie_formats/key_v1.htm
// Structure:
// - HEADER
// - BIFF ENTRIES: Sequence of BiffIndex, which refer to strings after the entries.
// - STRINGS (we read it as a large string, then we can index into sections).
// - RESOURCE ENTRIES: Sequence of ResourceIndex.

QDataStream& operator>>(QDataStream& stream, KeyFile& file)
{
    stream.setByteOrder(QDataStream::LittleEndian);

    auto onFailure = qScopeGuard([&file] {
        file.version = 0;

        file.biffEntries.clear();
        file.resourceEntries.clear();
    });

    char signature[8];
    stream.readRawData(signature, sizeof(signature));
    constexpr auto start = "KEY V1  ";
    if (qstrncmp(signature, start, sizeof(start)) == 0)
        file.version = 1;
    else
        return stream;

    quint32 biffCount, resourceCount, biffStart, resourceStart;
    stream >> biffCount >> resourceCount >> biffStart >> resourceStart;

    if (stream.status() != QDataStream::Ok)
        return stream;

    // NOTE (semi-TODO): Seems like we should use the biffStart value to find
    // where the BIFF entries start, but I don't see the point. It should always
    // be 24 (if I understand it correctly): the size of the header, because the
    // BIFF entries follow immediately the header (which has a fixed size).
    // It is also the current position
    Q_ASSERT(biffStart == stream.device()->pos());

    struct BiffIndex {
        quint32 size = 0;
        quint32 nameStart = 0;
        quint16 nameLength = 0;
        quint16 location = 0; ///< Always 1 in all the files that I've found (=> data directory).
    } PACKED_ATTRIBUTE;

    QVector<BiffIndex> biffIndexes;
    biffIndexes.resize(biffCount);
#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(biffIndexes.data()),
                       sizeof(BiffIndex) * biffCount);
#else
    for (BiffIndex& entry : biffIndexes) {
        stream >> entry.size >> entry.nameStart >> entry.nameLength >> entry.location;
    }
#endif

    if (stream.status() != QDataStream::Ok)
        return stream;

    // The total size of the header. This is just for making a sanity check, and verify our
    // assumptions. The header should be 0x18 (24) bytes.
    constexpr auto headerSize = sizeof(signature) * sizeof(char)
            + sizeof(biffCount) + sizeof(resourceCount)
            + sizeof(biffStart) + sizeof(resourceStart);
    static_assert(headerSize == 24);

    // Location where the sequence of biff strings begin. It should be the current state
    // of the file cursor. Substract this value to the "start of resources" to get the size
    // of the large string to read.
    const auto stringsStart = headerSize + sizeof(BiffIndex) * biffCount;
    const auto stringsLength = resourceStart - stringsStart;
    Q_ASSERT(qint64(stringsStart) == stream.device()->pos());

    QByteArray rawBiffNames;
    rawBiffNames.resize(stringsLength);
    stream.readRawData(rawBiffNames.data(), stringsLength);

    if (stream.status() != QDataStream::Ok)
        return stream;

    // Now that biff indexes and the strings of the biff names are read, get the
    // sequence of actual useful biff structs, with more convenient format.
    file.biffEntries.reserve(biffIndexes.size());
    for (const BiffIndex& entry : biffIndexes) {
        KeyFile::BiffEntry biff;
        biff.size = entry.size;
        const auto realStart = rawBiffNames.constData() + entry.nameStart - stringsStart;
        // TODO: Confirm if all BIFFs are ASCII only, otherwise, change it.
        biff.name = QString::fromLatin1(realStart);
        biff.location = entry.location;
        file.biffEntries << biff;
    }

    file.resourceEntries.resize(resourceCount);

#if defined(PACKED_STRUCTS)
    struct ResourceIndex {
        // The names are null terminated only if shorter than 8. Fill with 0s.
        char name[8] = {0};
        ResourceType type = NoType;
        quint32 locator = 0;
    } PACKED_ATTRIBUTE;
    QVector<ResourceIndex> resourceIndexes;
    resourceIndexes.resize(resourceCount);

    stream.readRawData(reinterpret_cast<char*>(resourceIndexes.data()),
                       sizeof(ResourceIndex) * resourceCount);

    int count = 0;
    for (ResourceIndex& index : resourceIndexes) {
        KeyFile::ResourceEntry& entry = file.resourceEntries[count];

        const uint nameLength = qMin(uint(8), qstrlen(index.name));
        entry.name = QString::fromLatin1(index.name, nameLength);
        entry.type = index.type;
        entry.index  = (index.locator & 0x00003FFF);
        entry.source = (index.locator & 0xFFF00000) >> 20;
        entry.locator = index.locator;

        ++count;
    }
#else
    char rawName[8];
    for (KeyFile::ResourceEntry& entry : file.resourceEntries) {
        stream.readRawData(rawName, sizeof(rawName));
        stream >> entry.type >> entry.locator;

        const uint nameLength = qstrnlen(rawName, 8);
        entry.name = QString::fromLatin1(rawName, nameLength);

        entry.index  = (entry.locator & 0x00003FFF);
        entry.source = (entry.locator & 0xFFF00000) >> 20;
        // We don't support tile index for now.
//        entry.tile  = (entry.locator & 0x000FC000) >> 14;
    }
#endif

    if (stream.status() == QDataStream::Ok)
        onFailure.dismiss();

    return stream;
}
