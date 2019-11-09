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


QDataStream& operator>>(QDataStream& stream, KeyFile& file)
{
    stream.setByteOrder(QDataStream::LittleEndian);

    auto onFailure = qScopeGuard([&file] {
        file.version = 0;
        file.bifEntries.clear();
        file.bifNames.clear();
        file.resourceEntries.clear();
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

    file.bifEntries.clear();
    file.bifEntries.resize(file.bifCount);
#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(file.bifEntries.data()),
                       sizeof(KeyFile::BifEntry) * file.bifCount);
#else
    for (quint32 count = 0; count < file.bifCount; ++count) {
        KeyFile::BifEntry& entry = file.bifEntries[count];
        stream >> entry.size >> entry.nameStart >> entry.nameLenght >> entry.location;
    }
#endif

    if (stream.status() != QDataStream::Ok)
        return stream;

    constexpr auto headerSize = 4 * 6;
    // TODO: this stringsStart is probably worth keeping, as the indexes of the
    // bif entries refer to the start of the file, but the QByteArray starts at
    // zero instead. So this is the number to substract to get the proper start.
    const auto stringsStart  = headerSize + (4+4+2+2) * file.bifCount;
    const auto stringsLength = file.resourceStart - stringsStart;
    Q_ASSERT(stringsStart == stream.device()->pos());
    file.bifNames.clear();
    file.bifNames.resize(stringsLength);

    stream.readRawData(file.bifNames.data(), stringsLength);

    if (stream.status() != QDataStream::Ok)
        return stream;

    file.resourceEntries.clear();
    file.resourceEntries.resize(file.resourceCount);

    if (stream.status() == QDataStream::Ok)
        onFailure.dismiss();

    return stream;
}
