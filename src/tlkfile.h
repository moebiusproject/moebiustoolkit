#pragma once

#include "packed.h"

#include <QByteArray>
#include <QDataStream>
#include <QVector>

struct TlkFile
{
    struct Index {
        quint16 type = 0;
        char soundFile[8] = {0};
        quint32 volume = 0;
        quint32 pitch = 0;
        quint32 start; ///< Offset of the entry from the start of the strings.
        quint32 length;
    } PACKED_ATTRIBUTE;

    bool isValid() const;
    QString text(quint32 id) const;

    // Header.
    quint8 version = 0;
    quint16 languageId = 0;
    quint32 stringsCount = 0;
    quint32 stringsStart = 0;

    // Index table.
    QVector<Index> index;

    // Strings section.
    QByteArray strings;
};

QDataStream& operator>>(QDataStream& stream, TlkFile& file);

