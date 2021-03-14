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

class tst_KeyFile : public QObject
{
    Q_OBJECT

private:
    KeyFile loadFile(const QString& fileName);

private slots:
    void testGeneral_data();
    void testGeneral();
    void testDetails_data();
    void testDetails();

    void testSanity_data();
    void testSanity();
};

KeyFile tst_KeyFile::loadFile(const QString &fileName)
{
    qDebug() << QFileInfo(fileName).absoluteFilePath();
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    KeyFile key;
    stream >> key;
    return key;
}

void tst_KeyFile::testGeneral_data()
{
    const auto bifNamesBG1EE = QStringList()
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
            << QLatin1String("data/PATCH25.BIF");

    QTest::addColumn<QString>("fileName");

    QTest::addColumn<quint32>("biffCount");
    QTest::addColumn<quint32>("resourceCount");

    QTest::addColumn<quint32>("bifSize24");
    QTest::addColumn<quint16>("bifLocation24");

    QTest::addColumn<quint32>("bifSize42");
    QTest::addColumn<quint16>("bifLocation42");

    QTest::addColumn<QStringList>("bifNames");

    // TODO: we only have a test row for BG1 EE, but we have a file for BG1 classic.
    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/key/bg1ee/chitin.key")
               // biffCount, resourceCount.
            << quint32(80)      << quint32(36993)
               // BIF entries 24 and 42 (size and location).
            << quint32(3012)    << quint16(1)
            << quint32(2727956) << quint16(1)
            << (bifNamesBG1EE);
}

void tst_KeyFile::testGeneral()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");

    KeyFile key;

    QCOMPARE(key.version, 0);
    QVERIFY(!key.isValid());

    key = loadFile(fileName);

    QCOMPARE(key.version, 1);
    QVERIFY(key.isValid());

    QFETCH(quint32, resourceCount);

    QFETCH(quint32, bifSize24);
    QFETCH(quint32, bifSize42);

    QFETCH(QStringList, bifNames);

    QCOMPARE(quint32(key.resourceEntries.count()), resourceCount);

    QCOMPARE(key.biffEntries.count(), bifNames.count());
    QStringList bifDetailsNames;
    for (const KeyFile::BiffEntry& bif : qAsConst(key.biffEntries)) {
        bifDetailsNames << bif.name;
        QCOMPARE(bif.location, 1);
    }
    QCOMPARE(bifDetailsNames, bifNames);
    QCOMPARE(key.biffEntries.at(24).size, bifSize24);
    QCOMPARE(key.biffEntries.at(42).size, bifSize42);

}

void tst_KeyFile::testDetails_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addColumn<QHash<int, KeyFile::ResourceEntry>>("resources");

    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/key/bg1ee/chitin.key")
            << QHash<int, KeyFile::ResourceEntry>({
                    {0,
                     {QLatin1String("A020000"),
                      PvrzType,
                      quint16(0),
                      quint16(0),
                      quint32(0)} },
                    {42,
                     {QLatin1String("A022412"),
                      PvrzType,
                      quint16(0),
                      quint16(42),
                      quint32(42)} },
                    {8442,
                     {QLatin1String("SCRL3B"),
                      ItmType,
                      quint16(25),
                      quint16(26215289),
                      quint32(26215289)} },
                    {14242,
                     {QLatin1String("SPPR309"),
                      SplType,
                      quint16(42),
                      quint16(44040860),
                      quint32(44040860)} },
                });
}

void tst_KeyFile::testDetails()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");

    const KeyFile key = loadFile(fileName);

    using Resources = QHash<int, KeyFile::ResourceEntry>;
    QFETCH(Resources, resources);

    for (auto entry = resources.constBegin(), last = resources.constEnd();
         entry != last; ++entry) {
        const auto& received = key.resourceEntries.at(entry.key());
        const auto& expected = entry.value();
        QCOMPARE(received.name, expected.name);
        QCOMPARE(received.type, expected.type);
        QCOMPARE(received.index, expected.index);
        QCOMPARE(received.source, expected.source);
        QCOMPARE(received.locator, expected.locator);
    }
}

void tst_KeyFile::testSanity_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/key/bg1ee/chitin.key");
    QTest::newRow("BG1")
            << QFINDTESTDATA("../../data/key/bg1/chitin.key");
}

void tst_KeyFile::testSanity()
{
    QFETCH(QString, filePath);
    if (filePath.isEmpty())
        QSKIP("File not found, skipping test.");
    const KeyFile key = loadFile(filePath);

    // Confirm our suspicion that the resources are unique if we consider name
    // and type, even if we lowercase the name.
    QHash<QPair<QString, quint16>, KeyFile::ResourceEntry> seenResourceNames;
    for (const KeyFile::ResourceEntry& entry : key.resourceEntries) {
        const auto seenKey = qMakePair(entry.name.toLower(), entry.type);
        if (seenResourceNames.contains(seenKey)) {
            const KeyFile::ResourceEntry& seen = seenResourceNames.value(seenKey);
            qDebug() << seen.name << int(seen.type) << seen.index << seen.source;
            qDebug() << entry.name << int(entry.type) << entry.index << entry.source;
        }
        QVERIFY2(!seenResourceNames.contains(seenKey), qPrintable(entry.name));
        seenResourceNames.insert(seenKey, entry);
    }
}

QTEST_MAIN(tst_KeyFile)

#include "tst_keyfile.moc"
