/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020-2023 Alejandro Exojo Piqueras
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

#include "resourcemanager.h"

#include "bifffile.h"
#include "keyfile.h"

#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log, "resourcemanager", QtWarningMsg);

namespace {

// Helper types ////////////////////////////////////////////////////////////////

struct Resource {
    QString name;
    ResourceType type;
};

bool operator==(const Resource& a, const Resource& b)
{ return a.name == b.name && a.type == b.type; }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
uint qHash(const Resource& resource, uint seed)
#else
size_t qHash(const Resource& resource, size_t seed)
#endif
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, resource.name);
    seed = hash(seed, resource.type);
    return seed;
}

struct Biff { QString name; BiffFile meta; };

}

// Main class //////////////////////////////////////////////////////////////////

struct ResourceManager::Private
{
    QString root; // The root location where the game is stored

    KeyFile chitinKey;
    QVector<Biff> biffs;
    QHash<Resource, quint32> resourceIndex;
};

ResourceManager::ResourceManager(QObject* parentObject)
    : QObject(parentObject)
    , d(*(new Private))
{
}

ResourceManager::~ResourceManager()
{
    delete &d;
}

const KeyFile& ResourceManager::chitinKey() const
{
    return d.chitinKey;
}

void ResourceManager::load(const QString& path)
{
    QElapsedTimer timer;
    timer.start();

    d.root = path.section(QLatin1Char('/'), 0, -2, QString::SectionIncludeTrailingSep);

    // Make idempotent.
    d.chitinKey = KeyFile();
    d.biffs.clear();
    d.resourceIndex.clear();

    QFile chitinFile(path);
    chitinFile.open(QIODevice::ReadOnly);
    QDataStream chitinStream(&chitinFile);
    chitinStream >> d.chitinKey;

    if (!d.chitinKey.isValid()) {
        qWarning("CHITIN.KEY file is not valid");
        emit failureLoading();
        return;
    }

    // Loop over the BIFF entries declared on the chitin.key file, and index
    // them (just read the metadata, not the contents), linking them with their
    // name so can be looked up on disk easily.
    // FIXME: I don't like it, but we add a dummy, empty BIFF even if we fail to
    // find or parse the file. This is because in the unit test we have a KEY
    // file that lists all the real BIFFs of the game, but we don't have all of
    // those on the tests. And if we skip them, the indexes are out of sync.
    // This also makes us print warnings on the tests, so, yeah, FIXME.
    for (const KeyFile::BiffEntry& biff : d.chitinKey.biffEntries) {
        QFile file(d.root + biff.name);
        if (!file.open(QIODevice::ReadOnly)) {
            file.setFileName(d.root + biff.name.toLower());
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << "Could not open" << file.fileName();
                d.biffs.append(Biff{});
                continue;
            }
        }
        qCDebug(log) << "Reading BIFF" << file.fileName();
        BiffFile biffFile;
        QDataStream stream(&file);
        stream >> biffFile;
        if (!biffFile.isValid()) {
            qWarning() << "Could not parse" << file.fileName();
        }
        d.biffs.append(Biff{biff.name, biffFile});
    }

    // Now build a hash to quikly map from resource name+type to its locator.
    for (const KeyFile::ResourceEntry& resource : d.chitinKey.resourceEntries)
        // Lower the case on the name, as no resource should differ from other
        // only in the case of the name. We find them lowering the name as well.
        d.resourceIndex.insert(Resource{resource.name.toLower(), resource.type},
                               resource.locator);

    emit loaded();
    qCDebug(log) << "Loaded:" << timer.elapsed() << "ms";
}

// TODO: Probably sanitize a bit the name argument, and look for ".." or "/" in it.
QByteArray ResourceManager::resource(const QString& name) const
{
    // Try to find the resource on the overridden directory first.
    // TODO: I feel tempted to use a container to store which files have been
    // found in the overridden directory, so we could avoid some disk activity.
    // Additionally, sometime in the game browser UI we might want to highlight
    // files which are overridden. We would need to have a function to clear the
    // container for a "refresh override folder" option.
    QFile overridden(d.root + QLatin1String("override/") + name);
    if (!overridden.exists())
        overridden.setFileName(d.root + QLatin1String("override/") + name.toLower());
    if (overridden.exists()) {
        if (!overridden.open(QIODevice::ReadOnly))
            qWarning() << "Could not open overridden file"
                       << overridden.fileName() << overridden.errorString();
        else {
            qCDebug(log) << "overridden:" << overridden.fileName();
            return overridden.readAll();
        }
    }

    auto baseName = [](const QString& string) {
        return string.section(QLatin1Char('.'), 0, 0);
    };

    return defaultResource(baseName(name), resourceType(name));
}

QByteArray ResourceManager::resource(const QString& name, ResourceType type)
{
    // TODO: to refactor once the support for files in override gets better.
    // It's pretty silly that we have the name and the type separated, and we
    // merge them to check on override, but then split again to look in the
    // default game files.
    return resource(resourceFileName(name, type));
}

QByteArray ResourceManager::defaultResource(const QString& name, ResourceType type) const
{
    QByteArray result;

    const Resource resource = {name.toLower(), type};
    // Fall back to look it up case insensitive. This approach is discarded, for now.
#if 0
    if (!d.resourceIndex.contains(resource)) {
        for (auto key = d.resourceIndex.keyBegin(),
             last = d.resourceIndex.keyEnd(); key != last; ++key)
        {
            if (key->name.compare(name, Qt::CaseInsensitive) == 0 && key->type == type)
                resource = *key;
        }
    }
#endif

    // Resource metadata
    const quint32 locator = d.resourceIndex.value(resource);
    const quint16 index  = (locator & 0x00003FFF); // which file in the list inside the biff
    const quint16 source = (locator & 0xFFF00000) >> 20; // biff number

    const Biff biff = d.biffs.at(source);
    const BiffFile::FileEntriesIndex& fileIndex = biff.meta.fileEntries.at(index);

    QFile file(d.root + biff.name);
    auto warn = [&file](const char* text) {
        qWarning() << text << file.fileName() << file.errorString();
    };
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName(d.root + biff.name.toLower());
        if (!file.open(QIODevice::ReadOnly))
            return warn("Could not open"), result;
    }

    if (!file.seek(fileIndex.offset))
        return warn("Could not seek"), result;

    result.resize(fileIndex.size);
    qint64 read = file.peek(result.data(), fileIndex.size);
    if (read != fileIndex.size)
        warn("Read incomplete");
    return result;
}

QString ResourceManager::resourceTermination(ResourceType type)
{
    switch (type) {
    case NoType:       break;
    case BmpType:      return QStringLiteral("bmp");
    case WavType:      return QStringLiteral("wav");
    case BamType:      return QStringLiteral("bam");
    case MosType:      return QStringLiteral("mos");
    case ItmType:      return QStringLiteral("itm");
    case SplType:      return QStringLiteral("spl");
    case BcsType:      return QStringLiteral("bcs");
    case IdsType:      return QStringLiteral("ids");
    case CreType:      return QStringLiteral("cre");
    case TdaType:      return QStringLiteral("2da");
    case BsType:       return QStringLiteral("bs");
    case PvrzType:     return QStringLiteral("pvrz");
    case MenuType:     return QStringLiteral("menu");
    }
    return QString();
}

QString ResourceManager::resourceFileName(const QString& name, ResourceType type)
{
    return name + QLatin1Char('.') + resourceTermination(type);
}

// FIXME: This is one of the first cases where we find the case sensitivity
// problem. It is very easy to fix here, but maybe consider something else.
ResourceType ResourceManager::resourceType(const QString& name)
{
    static QHash<QString, ResourceType> table = {
        {QLatin1String("bmp"),      BmpType},
        {QLatin1String("wav"),      WavType},
        {QLatin1String("bam"),      BamType},
        {QLatin1String("mos"),      MosType},
        {QLatin1String("itm"),      ItmType},
        {QLatin1String("spl"),      SplType},
        {QLatin1String("bcs"),      BcsType},
        {QLatin1String("ids"),      IdsType},
        {QLatin1String("cre"),      CreType},
        {QLatin1String("2da"),      TdaType},
        {QLatin1String("bs"),       BsType},
        {QLatin1String("pvrz"),     PvrzType},
        {QLatin1String("menu"),     PvrzType},
    };
    auto termination = [](const QString& string) {
        return string.section(QLatin1Char('.'), -1);
    };
    Q_ASSERT(table.contains(termination(name.toLower())));
    return table.value(termination(name.toLower()));
}
