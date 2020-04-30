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

#include <QtTest>

#include "keyfile.h"
#include "resourcetype.h"

class tst_KeyFile: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_KeyFile::test_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addColumn<quint32>("bifCount");
    QTest::addColumn<quint32>("resourceCount");
    QTest::addColumn<quint32>("bifStart");
    QTest::addColumn<quint32>("resourceStart");

    QTest::addColumn<quint32>("bifSize24");
    QTest::addColumn<quint32>("bifNameStart24");
    QTest::addColumn<quint16>("bifNameLength24");
    QTest::addColumn<quint16>("bifLocation24");

    QTest::addColumn<quint32>("bifSize42");
    QTest::addColumn<quint32>("bifNameStart42");
    QTest::addColumn<quint16>("bifNameLength42");
    QTest::addColumn<quint16>("bifLocation42");

    QTest::addColumn<QStringList>("bifNames");

    QTest::addColumn<QByteArray>("resource0Name");
    QTest::addColumn<ResourceType>("resource0Type");
    QTest::addColumn<quint32>("resource0Locator");

    QTest::addColumn<QByteArray>("resource42Name");
    QTest::addColumn<ResourceType>("resource42Type");
    QTest::addColumn<quint32>("resource42Locator");

    QTest::addColumn<QByteArray>("resource8442Name");
    QTest::addColumn<ResourceType>("resource8442Type");
    QTest::addColumn<quint32>("resource8442Locator");

    QTest::addColumn<QByteArray>("resource14242Name");
    QTest::addColumn<ResourceType>("resource14242Type");
    QTest::addColumn<quint32>("resource14242Locator");


    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/key/bg1ee/chitin.key")
               // Header.
            << quint32(80)      << quint32(36993) << quint32(24) << quint32(2320)
               // BIF entries 24 and 42.
            << quint32(3012)    << quint32(1370)  << quint16(16) << quint16(1)
            << quint32(2727956) << quint32(1676)  << quint16(16) << quint16(1)
            << (QStringList()
                << QLatin1String("data/AR02XX.BIF")
                << QLatin1String("data/AR10XX.BIF")
                << QLatin1String("data/AR1XXX.BIF")
                << QLatin1String("data/AR01XX.BIF")
                << QLatin1String("data/AR07XX.BIF")
                << QLatin1String("data/AR08XX.BIF")
                << QLatin1String("data/AR03XX.BIF")
                << QLatin1String("data/AR0002.BIF")
                << QLatin1String("data/AR04XX.BIF")
                << QLatin1String("data/AR05XX.BIF")
                << QLatin1String("data/AR09XX.BIF")
                << QLatin1String("data/AR06XX.BIF")
                << QLatin1String("data/AR20XX.BIF")
                << QLatin1String("data/AR2XXX.BIF")
                << QLatin1String("data/AR30XX.BIF")
                << QLatin1String("data/AR3XXX.BIF")
                << QLatin1String("data/AR40XX.BIF")
                << QLatin1String("data/AR4XXX.BIF")
                << QLatin1String("data/AR50XX.BIF")
                << QLatin1String("data/AR5XXX.BIF")
                << QLatin1String("data/MPGUI.BIF")
                << QLatin1String("data/LocCha.bif")
                << QLatin1String("data/CHASOUND.BIF")
                << QLatin1String("data/DEFAULT.BIF")
                << QLatin1String("data/AR7XXX.BIF")
                << QLatin1String("data/ITEMS.BIF")
                << QLatin1String("data/Effects.bif")
                << QLatin1String("data/25EFFECT.BIF")
                << QLatin1String("data/Gui.bif")
                << QLatin1String("data/OHAREAS.BIF")
                << QLatin1String("data/SCRIPTS.BIF")
                << QLatin1String("data/SFXSOUND.BIF")
                << QLatin1String("data/LocSFX.bif")
                << QLatin1String("data/MISSOUND.BIF")
                << QLatin1String("data/25GUIBAM.BIF")
                << QLatin1String("data/25GUIDES.BIF")
                << QLatin1String("data/25ITEMS.BIF")
                << QLatin1String("data/25CREANI.BIF")
                << QLatin1String("data/CD4CREA2.BIF")
                << QLatin1String("data/GUIBGEE.bif")
                << QLatin1String("data/25GUIMOS.BIF")
                << QLatin1String("data/CREANIM.BIF")
                << QLatin1String("data/Spells.bif")
                << QLatin1String("data/Misc.bif")
                << QLatin1String("data/Creature.bif")
                << QLatin1String("data/GUIIcon.bif")
                << QLatin1String("data/CHAAnim.bif")
                << QLatin1String("data/OBJANIM.BIF")
                << QLatin1String("data/Project.bif")
                << QLatin1String("data/25DEFLT.BIF")
                << QLatin1String("data/SPELANIM.BIF")
                << QLatin1String("data/PORTRAIT.BIF")
                << QLatin1String("data/GUIFont.bif")
                << QLatin1String("data/GUIMosc.bif")
                << QLatin1String("data/Hd0GMosc.bif")
                << QLatin1String("data/DIALOG.BIF")
                << QLatin1String("data/AR60XX.BIF")
                << QLatin1String("data/AR6XXX.BIF")
                << QLatin1String("data/CREANIM1.BIF")
                << QLatin1String("data/PAPERDOL.BIF")
                << QLatin1String("data/GUIDesc.bif")
                << QLatin1String("data/CRESound.Bif")
                << QLatin1String("data/LocCre.bif")
                << QLatin1String("data/CRIWANIM.BIF")
                << QLatin1String("data/LocMP.bif")
                << QLatin1String("data/MPSOUNDS.BIF")
                << QLatin1String("data/25STORE.BIF")
                << QLatin1String("data/LocNpc.bif")
                << QLatin1String("data/NPCSOUND.BIF")
                << QLatin1String("data/AREAS.BIF")
                << QLatin1String("data/ARMISC.BIF")
                << QLatin1String("data/GUIBAM.BIF")
                << QLatin1String("data/NPCANIM.BIF")
                << QLatin1String("data/GUICHUI.BIF")
                << QLatin1String("data/Tutorial.Bif")
                << QLatin1String("data/LocTut.bif")
                << QLatin1String("data/v2017.bif")
                << QLatin1String("data/PATCH20.BIF")
                << QLatin1String("data/AREA0900.Bif")
                << QLatin1String("data/PATCH25.BIF"))

            << QByteArray("A020000") << PvrzType << quint32(0)
            << QByteArray("A022412") << PvrzType << quint32(42)
            << QByteArray("SCRL3B")  << ItmType  << quint32(26215289)
            << QByteArray("SPPR309") << SplType  << quint32(44040860);
}

void tst_KeyFile::test()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");
    qDebug() << QTest::currentDataTag() << fileName;

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QDataStream stream(&file);
    KeyFile key;

    QCOMPARE(key.version, 0);
    QVERIFY(!key.isValid());

    stream >> key;

    QCOMPARE(key.version, 1);
    QVERIFY(key.isValid());

    QFETCH(quint32, bifCount);
    QFETCH(quint32, resourceCount);
    QFETCH(quint32, bifStart);
    QFETCH(quint32, resourceStart);
    QCOMPARE(key.bifCount, bifCount);
    QCOMPARE(key.resourceCount, resourceCount);
    QCOMPARE(key.bifStart, bifStart);
    QCOMPARE(key.resourceStart, resourceStart);

    QFETCH(quint32, bifSize24);
    QFETCH(quint32, bifNameStart24);
    QFETCH(quint16, bifNameLength24);
    QFETCH(quint16, bifLocation24);
    QFETCH(quint32, bifSize42);
    QFETCH(quint32, bifNameStart42);
    QFETCH(quint16, bifNameLength42);
    QFETCH(quint16, bifLocation42);

    QFETCH(QStringList, bifNames);

    QCOMPARE(key.bifIndexes.at(24).size, bifSize24);
    QCOMPARE(key.bifIndexes.at(24).nameStart, bifNameStart24);
    QCOMPARE(key.bifIndexes.at(24).nameLength, bifNameLength24);
    QCOMPARE(key.bifIndexes.at(24).location, bifLocation24);

    QCOMPARE(key.bifIndexes.at(42).size, bifSize42);
    QCOMPARE(key.bifIndexes.at(42).nameStart, bifNameStart42);
    QCOMPARE(key.bifIndexes.at(42).nameLength, bifNameLength42);
    QCOMPARE(key.bifIndexes.at(42).location, bifLocation42);

    QCOMPARE(key.bifDetails.count(), int(bifCount));
    QStringList bifDetailsNames;
    for (const KeyFile::BifDetails& bif : key.bifDetails)
        bifDetailsNames << bif.name;
    QCOMPARE(bifDetailsNames, bifNames);

    QFETCH(QByteArray, resource0Name);
    QFETCH(ResourceType, resource0Type);
    QFETCH(quint32, resource0Locator);
    QFETCH(QByteArray, resource42Name);
    QFETCH(ResourceType, resource42Type);
    QFETCH(quint32, resource42Locator);
    QFETCH(QByteArray, resource8442Name);
    QFETCH(ResourceType, resource8442Type);
    QFETCH(quint32, resource8442Locator);
    QFETCH(QByteArray, resource14242Name);
    QFETCH(ResourceType, resource14242Type);
    QFETCH(quint32, resource14242Locator);

    QCOMPARE(QByteArray::fromRawData(key.resourceIndexes[0].name,
             qMin(uint(8), qstrlen(key.resourceIndexes[0].name))), resource0Name);
    QCOMPARE(key.resourceIndexes[0].type, resource0Type);
    QCOMPARE(key.resourceIndexes[0].locator, resource0Locator);

    QCOMPARE(QByteArray::fromRawData(key.resourceIndexes[42].name,
             qMin(uint(8), qstrlen(key.resourceIndexes[42].name))), resource42Name);
    QCOMPARE(key.resourceIndexes[42].type, resource42Type);
    QCOMPARE(key.resourceIndexes[42].locator, resource42Locator);

    QCOMPARE(QByteArray::fromRawData(key.resourceIndexes[8442].name,
             qMin(uint(8), qstrlen(key.resourceIndexes[8442].name))), resource8442Name);
    QCOMPARE(key.resourceIndexes[8442].type, resource8442Type);
    QCOMPARE(key.resourceIndexes[8442].locator, resource8442Locator);

    QCOMPARE(QByteArray::fromRawData(key.resourceIndexes[14242].name,
             qMin(uint(8), qstrlen(key.resourceIndexes[14242].name))), resource14242Name);
    QCOMPARE(key.resourceIndexes[14242].type, resource14242Type);
    QCOMPARE(key.resourceIndexes[14242].locator, resource14242Locator);
}

QTEST_MAIN(tst_KeyFile)

#include "tst_keyfile.moc"
