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

#include <QtTest>

#include "itmfile.h"
#include "resourcetype.h"

class tst_ItmFile : public QObject
{
    Q_OBJECT

private:
    ItmFile loadFile(const QString &fileName)
    {
        qDebug() << QFileInfo(fileName).absoluteFilePath();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QDataStream stream(&file);
        ItmFile itm;
        stream >> itm;
        return itm;
    }

private slots:
    void testGeneral_data();
    void testGeneral();
};

void tst_ItmFile::testGeneral_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<StrRef>("unidentifiedName");
    QTest::addColumn<StrRef>("identifiedName");
    QTest::addColumn<quint8>("animation0");
    QTest::addColumn<quint8>("animation1");
    QTest::addColumn<quint16>("requiredStrength");
    QTest::addColumn<quint32>("price");
    QTest::addColumn<QString>("inventoryIcon");

    QTest::addColumn<quint16>("abilitiesCount");
    QTest::addColumn<quint16>("equippedEffectsFirst");
    QTest::addColumn<quint16>("equippedEffectsCount");

    QTest::newRow("Albruin in vanilla BG1 EE")
            << QFINDTESTDATA("../../data/itm/sw1h34.itm")
            << 6646u
            << 31707u
            << quint8('S')
            << quint8('0')
            << quint16(11)
            << quint32(10000)
            << QString::fromLatin1("ISW1H34")
            << quint16(2) // Melee damage and Detect Invisibility
            << quint16(0)
            << quint16(10);
}

void tst_ItmFile::testGeneral()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");

    ItmFile itm;

    QVERIFY(!itm.isValid());

    itm = loadFile(fileName);

    QVERIFY(itm.isValid());
}


QTEST_MAIN(tst_ItmFile)

#include "tst_itmfile.moc"
