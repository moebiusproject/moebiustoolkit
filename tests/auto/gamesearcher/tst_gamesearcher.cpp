/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2023 Alejandro Exojo Piqueras
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

#include "gamesearcher.h"

class tst_GameSearcher : public QObject
{
    Q_OBJECT

private slots:
    void testGameName_data();
    void testGameName();
};

void tst_GameSearcher::testGameName_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("result");

    QTest::newRow("BG:EE")
        << QFINDTESTDATA("01-baldursgateee/chitin.key")
        << QString::fromLatin1("Baldur's Gate - Enhanced Edition");

    QTest::newRow("BG2:EE")
        << QFINDTESTDATA("02-baldursgate2ee/chitin.key")
        << QString::fromLatin1("Baldur's Gate II - Enhanced Edition");

    QTest::newRow("IWD:EE")
        << QFINDTESTDATA("03-icewinddaleee/chitin.key")
        << QString::fromLatin1("Icewind Dale - Enhanced Edition");

    QTest::newRow("PST:EE")
        << QFINDTESTDATA("04-planescapetormentee/chitin.key")
        << QString::fromLatin1("Planescape Torment - Enhanced Edition");

    QTest::newRow("BG:EE edited")
        << QFINDTESTDATA("05-bgee-edited/chitin.key")
        << QString::fromLatin1("BGEE vanilla");
}

void tst_GameSearcher::testGameName()
{
    QFETCH(QString, path);
    QFETCH(QString, result);

    QFileInfo info(path);
    QVERIFY(info.exists());

    GameSearcher searcher;
    QCOMPARE(searcher.gameName(info), result);
}


QTEST_MAIN(tst_GameSearcher)

#include "tst_gamesearcher.moc"
