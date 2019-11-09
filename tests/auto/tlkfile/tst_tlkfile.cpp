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

#include "tlkfile.h"

class tst_TlkFile: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_TlkFile::test_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<quint16>("languageId");
    QTest::addColumn<quint32>("stringsCount");
    QTest::addColumn<quint32>("stringsStart");

    QTest::addColumn<QString>("text42");
    QTest::addColumn<QString>("text242");

    QTest::newRow("BG1 classic")
            << QFINDTESTDATA("../../data/tlk/bg1/dialog.tlk")
            << quint16(0) << quint32(24124) << quint32(627242)
            << QString("Back so soon.  How did your battle fare?")
            << QString("I prefer to manage on my own.");
    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/tlk/bg1ee/dialog.tlk")
            << quint16(0) << quint32(71375) << quint32(1855768)
            << QString("Back so soon! How did your battle fare?")
            << QString("I prefer to manage on my own.");
}

void tst_TlkFile::test()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");
    qDebug() << QTest::currentDataTag() << fileName;

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QDataStream stream(&file);
    TlkFile tlk;

    QCOMPARE(tlk.version, 0);
    QVERIFY(!tlk.isValid());

    stream >> tlk;

    QCOMPARE(tlk.version, 1);
    QVERIFY(tlk.isValid());

    QFETCH(quint16, languageId);
    QFETCH(quint32, stringsCount);
    QFETCH(quint32, stringsStart);
    QCOMPARE(tlk.languageId, languageId);
    QCOMPARE(tlk.stringsCount, stringsCount);
    QCOMPARE(tlk.stringsStart, stringsStart);

    QFETCH(QString, text42);
    QFETCH(QString, text242);

    QCOMPARE(tlk.text(42), text42);
    QCOMPARE(tlk.text(242), text242);
}

QTEST_MAIN(tst_TlkFile)

#include "tst_tlkfile.moc"
