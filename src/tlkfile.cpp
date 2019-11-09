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

#include "tlkfile.h"

#include <QDebug>
#include <QScopeGuard>

bool TlkFile::isValid() const
{
    return version != 0;
}

QString TlkFile::text(quint32 id) const
{
    const Index entry = index.at(id);
    return QString::fromUtf8(strings.constData() + entry.start, entry.length);
}

QDataStream& operator>>(QDataStream& stream, TlkFile& file)
{
    auto onFailure = qScopeGuard([&file] {
        file.version = 0;
        file.index.clear();
        file.strings.clear();
    });

    stream.setByteOrder(QDataStream::LittleEndian);

    char header[8];
    stream.readRawData(header, sizeof(header));
    constexpr auto start = "TLK V1  ";
    if (qstrncmp(header, start, sizeof(start)) == 0)
        file.version = 1;
    else
        return stream;

#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(&file.languageId),
                       sizeof(quint16) + sizeof(quint32) + sizeof(quint32));
#else
    stream >> file.languageId >> file.stringsCount >> file.stringsStart;
#endif

    if (stream.status() != QDataStream::Ok)
        return stream;

    file.index.clear();
    file.index.resize(file.stringsCount);

#if defined(PACKED_STRUCTS)
    stream.readRawData(reinterpret_cast<char*>(file.index.data()),
                       sizeof(TlkFile::Index) * file.stringsCount);
#else
    for (quint32 count = 0; count < file.stringsCount; ++count) {
        TlkFile::Index& entry = file.index[count];
        stream >> entry.type;
        stream.readRawData(entry.soundFile, sizeof(entry.soundFile));
        stream >> entry.volume >> entry.pitch >> entry.start >> entry.length;
    }
#endif

    if (stream.status() != QDataStream::Ok)
        return stream;

    file.strings.clear();
    const auto stringSize = stream.device()->size() - file.stringsStart;
    file.strings.resize(stringSize);
    stream.readRawData(file.strings.data(), stringSize);

    if (stream.status() == QDataStream::Ok)
        onFailure.dismiss();

    return stream;
}
