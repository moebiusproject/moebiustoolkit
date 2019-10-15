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
