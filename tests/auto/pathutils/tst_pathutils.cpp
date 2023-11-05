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

#include "pathutils.h"

class tst_PathUtils : public QObject
{
    Q_OBJECT

private slots:
    void testBaseName_data();
    void testBaseName();
    void testSuffix();
};

void tst_PathUtils::testBaseName_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("baseName");
    QTest::addColumn<QString>("completeBaseName");

    QTest::newRow("absolute path to a simple file") << "/home/user/file.txt"
        << "file" << "file";
    QTest::newRow("absolute path with redundant slashes to a simple file") << "/home//user///file.txt"
        << "file" << "file";
    QTest::newRow("relative path to a simple file") << "./file.txt"
        << "file" << "file";
    QTest::newRow("just a simple file") << "file.txt"
        << "file" << "file";

    QTest::newRow("absolute path to a compressed file") << "/home/user/file.txt.gz"
        << "file" << "file.txt";
    QTest::newRow("absolute path with redundant slashes to a compressed file") << "/home//user///file.txt.gz"
        << "file" << "file.txt";
    QTest::newRow("relative path to a compressed file") << "./file.txt.gz"
        << "file" << "file.txt";
    QTest::newRow("just a compressed file") << "file.txt.gz"
        << "file" << "file.txt";
}

void tst_PathUtils::testBaseName()
{
    QFETCH(QString, input);
    QFETCH(QString, baseName);
    QFETCH(QString, completeBaseName);

    QFileInfo info(input);
    QCOMPARE(info.baseName(), baseName);
    QCOMPARE(info.completeBaseName(), completeBaseName);

    QCOMPARE(path::baseName(input), baseName);
    QCOMPARE(path::completeBaseName(input), completeBaseName);
}

void tst_PathUtils::testSuffix()
{
    QCOMPARE(path::suffix(QLatin1String("/home/user/file.txt")),
             QLatin1String("txt"));
    QCOMPARE(path::suffix(QLatin1String("/home/user//file.txt")),
             QLatin1String("txt"));
    QCOMPARE(path::suffix(QLatin1String("file.txt")),
             QLatin1String("txt"));
    QCOMPARE(path::suffix(QLatin1String("txt")),
             QLatin1String("txt"));

    QCOMPARE(path::suffix(QLatin1String("file.txt.gz")),
             QLatin1String("gz"));
}

QTEST_MAIN(tst_PathUtils)

#include "tst_pathutils.moc"
