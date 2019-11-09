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

    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/key/bg1ee/chitin.key")
               // Header.
            << quint32(80)      << quint32(36993) << quint32(24) << quint32(2320)
               // BIF entries 24 and 42.
            << quint32(3012)    << quint32(1370)  << quint16(16) << quint16(1)
            << quint32(2727956) << quint32(1676)  << quint16(16) << quint16(1);
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

    QCOMPARE(key.bifEntries.at(24).size, bifSize24);
    QCOMPARE(key.bifEntries.at(24).nameStart, bifNameStart24);
    QCOMPARE(key.bifEntries.at(24).nameLenght, bifNameLength24);
    QCOMPARE(key.bifEntries.at(24).location, bifLocation24);

    QCOMPARE(key.bifEntries.at(42).size, bifSize42);
    QCOMPARE(key.bifEntries.at(42).nameStart, bifNameStart42);
    QCOMPARE(key.bifEntries.at(42).nameLenght, bifNameLength42);
    QCOMPARE(key.bifEntries.at(42).location, bifLocation42);
}

QTEST_MAIN(tst_KeyFile)

#include "tst_keyfile.moc"
