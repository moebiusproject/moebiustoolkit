/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2021 Alejandro Exojo Piqueras
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

#include "xplevels.h"

#include "resourcemanager.h"
#include "tdafile.h"

#include <QBuffer>
#include <QDebug>

#ifndef Q_OS_WASM
#include <QThreadPool>
#endif

struct XpLevels::Private
{
    Private(XpLevels& p) : parent(p) {}

    XpLevels& parent;
    ResourceManager manager;
    QString path;
    QVector<XpLevels::Level> levels;
    QStringList classes;

    void readData();
    QByteArray fileData() const;

};

void XpLevels::Private::readData()
{
    QByteArray bytes = fileData();
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);
    const TdaFile file = TdaFile::from(buffer);
    buffer.close();

    classes = file.rowNames();

    for (const QStringList& entry : file.entries) {
        // entry: "CLASS 0 1250 2500 ..."
        const QString key = entry.first();
        QVector<quint32> values;

        bool ok = false;
        for (int level = 1; level < entry.count() && level <= 41; ++level) {
            const quint32 value = entry.at(level).toULong(&ok);
            if (!ok)
                break;
            values.append(value);
        }

        levels.append(XpLevels::Level{key, values});
    }
    // This lines sometimes run, directly or indirectly, from the constructor.
    // We need to defer emiting the signal to allow for it to be connected to
    // (and we can only connect to an object after it's been constructed).
    QMetaObject::invokeMethod(&parent, &XpLevels::loaded, Qt::QueuedConnection);
}

XpLevels::XpLevels(const QString& path, QObject* parentObject)
    : QObject(parentObject)
    , d(*(new Private(*this)))
{
    d.path = path;

    if (!path.isEmpty()) {
        // NB: use a context object in the connection for the signal in the right thread!
        connect(&d.manager, &ResourceManager::loaded, this, [&]() { d.readData(); });
#ifndef Q_OS_WASM
        QThreadPool::globalInstance()->start([this, path]() {d.manager.load(path);});
#else
        d.manager.load(path);
#endif
    } else {
        d.readData();
    }
}

XpLevels::~XpLevels()
{
    delete &d;
}

QStringList XpLevels::classes() const
{
    return d.classes;
}

const QVector<XpLevels::Level>& XpLevels::levels() const
{
    return d.levels;
}

QVector<quint32> XpLevels::thresholds(const QString& name) const
{
    for (const Level& level : qAsConst(d.levels)) {
        if (level.name == name)
            return level.thresholds;
    }
    return QVector<quint32>();
}

QByteArray XpLevels::Private::fileData() const
{
    if (!path.isEmpty())
        return manager.resource(QLatin1String("XPLEVEL.2DA"));

    // From BG2EE 2.5.16.6, and AFAIK fully equivalent to pen and paper.
    static const char rawData[] = R"(2DA V1.0
-1
                1           2           3           4           5           6           7           8           9           10          11          12          13          14          15          16          17          18          19          20        21         22         23	        24	        25         26         27         28         29         30         31         32         33         34	        35         36        37         38         39         40         41
MAGE            0           2500        5000        10000       20000       40000       60000       90000       135000      250000      375000      750000      1125000     1500000     1875000     2250000     2625000     3000000     3375000     3750000   4125000    4500000    4875000    5250000    5625000    6000000    6375000    6750000    7125000    7500000    7875000    8250000    8625000    9000000    9375000    9750000   10125000   10500000   10875000   11250000   13000000
FIGHTER         0           2000        4000        8000        16000       32000       64000       125000      250000      500000      750000      1000000     1250000     1500000     1750000     2000000     2250000     2500000     2750000     3000000   3250000    3500000    3750000    4000000    4250000    4500000    4750000    5000000    5250000    5500000    5750000    6000000    6250000    6500000    6750000    7000000   7250000    7500000    7750000    8000000    8250000
PALADIN         0           2250        4500        9000        18000       36000       75000       150000      300000      600000      900000      1200000     1500000     1800000     2100000     2400000     2700000     3000000     3300000     3600000   3900000    4200000    4500000    4800000    5100000    5400000    5700000    6000000    6300000    6600000    6900000    7200000    7500000    7800000    8100000    8400000   8700000    9000000    9300000    9600000    9900000
RANGER          0           2250        4500        9000        18000       36000       75000       150000      300000      600000      900000      1200000     1500000     1800000     2100000     2400000     2700000     3000000     3300000     3600000   3900000    4200000    4500000    4800000    5100000    5400000    5700000    6000000    6300000    6600000    6900000    7200000    7500000    7800000    8100000    8400000   8700000    9000000    9300000    9600000    9900000
CLERIC          0           1500        3000        6000        13000       27500       55000       110000      225000      450000      675000      900000      1125000     1350000     1575000     1800000     2025000     2250000     2475000     2700000   2925000    3150000    3375000    3600000    3825000    4050000    4275000    4500000    4725000    4950000    5175000    5400000    5625000    5850000    6075000    6300000   6525000    6750000    6975000    8000000    9000000
DRUID           0           2000        4000        7500        12500       20000       35000       60000       90000       125000      200000      300000      750000      1500000     3000000     3150000     3300000     3450000     3600000     3750000   3900000    4150000    4400000    4700000    5000000    5500000    6000000    6500000    7000000    7500000    8000000    8500000    9000000    9500000    10000000   10500000  11000000   11500000   12000000   12500000   13000000
THIEF           0           1250        2500        5000        10000       20000       40000       70000       110000      160000      220000      440000      660000      880000      1100000     1320000     1540000     1760000     1980000     2200000   2420000    2640000    2860000    3080000    3300000    3520000    3740000    3960000    4180000    4400000    4620000    4840000    5060000    5280000    5500000    5720000   5940000    6160000    6380000    8000000    9000000
BARD            0           1250        2500        5000        10000       20000       40000       70000       110000      160000      220000      440000      660000      880000      1100000     1320000     1540000     1760000     1980000     2200000   2420000    2640000    2860000    3080000    3300000    3520000    3740000    3960000    4180000    4400000    4620000    4840000    5060000    5280000    5500000    5720000   5940000    6160000    6380000    8000000    9000000
SORCERER        0           2500        5000        10000       20000       40000       60000       90000       135000      250000      375000      750000      1125000     1500000     1875000     2250000     2625000     3000000     3375000     3750000   4125000    4500000    4875000    5250000    5625000    6000000    6375000    6750000    7125000    7500000    7875000    8250000    8625000    9000000    9375000    9750000   10125000   10500000   10875000   11250000   13000000
MONK            0           1500        3000        6000        13000       27500       55000       110000      225000      450000      675000      900000      1125000     1350000     1575000     1800000     2025000     2250000     2475000     2700000   2925000    3150000    3375000    3600000    3825000    4050000    4275000    4500000    4725000    4950000    5175000    5400000    5625000    5850000    6075000    6300000   6525000    6750000    6975000    8000000    9000000
SHAMAN          0           2500        5000        10000       20000       40000       60000       90000       135000      250000      375000      750000      1125000     1500000     1875000     2250000     2625000     3000000     3375000     3750000   4125000    4500000    4875000    5250000    5625000    6000000    6375000    6750000    7125000    7500000    7875000    8250000    8625000    9000000    9375000    9750000   10125000   10500000   10875000   11250000   13000000
)";

    return QByteArray::fromRawData(rawData, sizeof(rawData));
}
