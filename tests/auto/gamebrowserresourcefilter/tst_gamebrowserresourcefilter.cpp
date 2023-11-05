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

#include "gamebrowserresourcefilter.h"

class tst_GameBrowserResourceFilter : public QObject
{
    Q_OBJECT

private slots:
    void testEmpty();

    void testName();

    void testType_data();
    void testType();

    void testLocation_data();
    void testLocation();
};

void tst_GameBrowserResourceFilter::testEmpty()
{
    // Without filter string, everything is accepted.
    GameBrowserResourceFilter filter;
    QCOMPARE(filter.accept({}), true);

    GameBrowserResourceFilter::Row row;
    QCOMPARE(filter.accept({
        QLatin1String("example"),
        QLatin1String("foo"),
        QLatin1String("bar"),
    }), true);
    QCOMPARE(filter.accept({
        QLatin1String("this"),
        QLatin1String("that"),
        QLatin1String("here"),
    }), true);
    QCOMPARE(filter.accept({
        QLatin1String(""),
        QLatin1String("that"),
        QLatin1String("here"),
    }), true);
    QCOMPARE(filter.accept({
        QLatin1String("this"),
        QLatin1String(""),
        QLatin1String(""),
    }), true);
}

void tst_GameBrowserResourceFilter::testName()
{
    GameBrowserResourceFilter::Row row;
    // Some input defaults.
    row.type = QLatin1String("foo");
    row.location = QLatin1String("bar");

    GameBrowserResourceFilter filter;

    filter.setFilter(QLatin1String("foo"));

    // Lower case.
    row.name = QLatin1String("foobar");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("foobaz");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("baz");
    QCOMPARE(filter.accept(row), false);

    // Upper case.
    row.name = QLatin1String("FOOBAR");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("FOOBAZ");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("BAZ");
    QCOMPARE(filter.accept(row), false);

    // Mixed case.
    row.name = QLatin1String("fOObar");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("fooBAZ");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("Baz");
    QCOMPARE(filter.accept(row), false);

    // Now different case in the filter.
    filter.setFilter(QLatin1String("Foo"));

    row.name = QLatin1String("foobar");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("foobaz");
    QCOMPARE(filter.accept(row), true);
    row.name = QLatin1String("baz");
    QCOMPARE(filter.accept(row), false);
}

void tst_GameBrowserResourceFilter::testType_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("type");
    QTest::addColumn<bool>("result");

    QTest::addRow("2da filter, 2da row")
        << "is:2da" << "2da" << true;
    QTest::addRow("2da filter, ids row")
        << "is:2da" << "ids" << false;
    QTest::addRow("2da and ids filter, 2da row")
        << "is:2da is:ids" << "2da" << true;
    QTest::addRow("2da and ids filter, ids row")
        << "is:2da is:ids" << "ids" << true;
    QTest::addRow("2da and ids filter, bcs row")
        << "is:2da is:ids" << "bcs" << false;

    QTest::addRow("2da and ids and bcs filter, bcs row")
        << "is:2da is:ids is:bcs" << "bcs" << true;

    QTest::addRow("name plus 2da filter, 2da row")
        << "example is:2da" << "2da" << true;
    QTest::addRow("name plus 2da filter, ids row")
        << "example is:2da" << "ids" << false;
    QTest::addRow("name plus 2da and ids filter, 2da row")
        << "example is:2da is:ids" << "2da" << true;
    // Name at the end or middle now.
    QTest::addRow("name (end) plus 2da filter, 2da row")
        << "is:2da example" << "2da" << true;
    QTest::addRow("name (middle) plus 2da and ids filter, 2da row")
        << "is:2da example is:ids" << "2da" << true;

    QTest::addRow("wrong name plus 2da filter, 2da row")
        << "wrong is:2da" << "2da" << false;
    QTest::addRow("wrong name plus 2da filter, ids row")
        << "wrong is:2da" << "ids" << false;
    QTest::addRow("wrong name plus 2da and ids filter, 2da row")
        << "wrong is:2da is:ids" << "2da" << false;
}

void tst_GameBrowserResourceFilter::testType()
{
    GameBrowserResourceFilter::Row row;
    row.name = QLatin1String("example");
    row.location = QLatin1String("biff");

    QFETCH(QString, input);
    QFETCH(QString, type);
    QFETCH(bool, result);

    GameBrowserResourceFilter filter;
    filter.setFilter(input);
    row.type = type;

    QCOMPARE(filter.accept(row), result);
}

void tst_GameBrowserResourceFilter::testLocation_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("location");
    QTest::addColumn<bool>("result");

    QTest::addRow("here filter, here row")
        << "in:here" << "here" << true;
    QTest::addRow("here filter, over row")
        << "in:here" << "over" << false;
    QTest::addRow("here and over filter, here row")
        << "in:here in:over" << "here" << true;
    QTest::addRow("here and over filter, over row")
        << "in:here in:over" << "over" << true;
    QTest::addRow("here and over filter, other row")
        << "in:here in:over" << "other" << false;

    // Mix in:x and is:y
    QTest::addRow("here and over plus is:2da filter, here row")
        << "in:here in:over is:2da" << "here" << true;
    QTest::addRow("here and over plus is:2da filter, nope row")
        << "in:here in:over is:2da" << "nope" << false;

    QTest::addRow("name plus here filter, here row")
        << "example in:here" << "here" << true;
    QTest::addRow("name plus here filter, over row")
        << "example in:here" << "over" << false;
    QTest::addRow("name plus here and over filter, here row")
        << "example in:here in:over" << "here" << true;
    // Name at the end or middle now.
    QTest::addRow("name (end) plus here filter, here row")
        << "in:here example" << "here" << true;
    QTest::addRow("name (middle) plus here and over filter, here row")
        << "in:here example in:over" << "here" << true;

    QTest::addRow("wrong name plus here filter, here row")
        << "wrong in:here" << "here" << false;
    QTest::addRow("wrong name plus here filter, over row")
        << "wrong in:here" << "over" << false;
    QTest::addRow("wrong name plus here and over filter, here row")
        << "wrong in:here in:over" << "here" << false;
}

void tst_GameBrowserResourceFilter::testLocation()
{
    GameBrowserResourceFilter::Row row;
    row.name = QLatin1String("example");
    row.type = QLatin1String("2da");

    QFETCH(QString, input);
    QFETCH(QString, location);
    QFETCH(bool, result);

    GameBrowserResourceFilter filter;
    filter.setFilter(input);
    row.location = location;

    QCOMPARE(filter.accept(row), result);
}

QTEST_MAIN(tst_GameBrowserResourceFilter)

#include "tst_gamebrowserresourcefilter.moc"
