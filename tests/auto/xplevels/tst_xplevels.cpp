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

#include <QtTest>

#include "xplevels.h"

class tst_XpLevels: public QObject
{
    Q_OBJECT

private slots:
    void test();
};

#include "tst_xplevels.moc"

void tst_XpLevels::test()
{
    // Empty string == no path == use built ins == synchronous
    const XpLevels levels((QString()));

    const QVector<quint32> thief = levels.thresholds(QLatin1String("THIEF"));
    QVERIFY(!thief.isEmpty());
    QCOMPARE(thief.at(0),  0u);
    QCOMPARE(thief.at(1),  1'250u);
    QCOMPARE(thief.at(39), 8'000'000u);

    const QVector<quint32> fighter = levels.thresholds(QLatin1String("FIGHTER"));
    QVERIFY(!fighter.isEmpty());
    QCOMPARE(fighter.at(0),  0u);
    QCOMPARE(fighter.at(1),  2'000u);
    QCOMPARE(fighter.at(39), 8'000'000u);

    const QVector<quint32> mage = levels.thresholds(QLatin1String("MAGE"));
    QVERIFY(!mage.isEmpty());
    QCOMPARE(mage.at(0),  0u);
    QCOMPARE(mage.at(1),  2'500u);
    QCOMPARE(mage.at(30), 7'875'000u);
}

QTEST_MAIN(tst_XpLevels)

