#pragma once

#include "packed.h"

#include <QByteArray>
#include <QDataStream>
#include <QVector>

struct KeyFile
{
    struct BifEntry {
        quint32 size = 0;
        quint32 nameStart = 0;
        quint16 nameLenght = 0;
        quint16 location = 0; ///< Always 1 in all the files that I've found.
    } PACKED_ATTRIBUTE;

    struct ResourceEntry {
        char name[8] = {0};
        quint16 type = 0;
        quint32 locator = 0;
    } PACKED_ATTRIBUTE;

    bool isValid() const;

    // Header.
    quint8 version = 0;
    quint32 bifCount = 0;
    quint32 resourceCount = 0;
    quint32 bifStart = 0;
    quint32 resourceStart = 0;

    // Body.
    QVector<BifEntry> bifEntries;
    QByteArray bifNames;
    QVector<ResourceEntry> resourceEntries;
};

QDataStream& operator>>(QDataStream& stream, KeyFile& file);

