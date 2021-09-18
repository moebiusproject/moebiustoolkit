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

#include "calculators.h"

using namespace Calculators;

class tst_Calculators : public QObject
{
    Q_OBJECT

private slots:
    void testHitRatio_data();
    void testHitRatio();
    void testToHit_data();
    void testToHit();
    void testDamage_data();
    void testDamage();
};

void tst_Calculators::testHitRatio_data()
{
    // User input data
    QTest::addColumn<Damage::Common>("common");
    QTest::addColumn<WeaponArrangement>("weapon1");
    QTest::addColumn<WeaponArrangement>("weapon2");
    // Variables
    QTest::addColumn<int>("ac");
    QTest::addColumn<int>("roll");
    QTest::addColumn<int>("result");

    const Damage::Common defaultCommon;
    const DiceRoll defaultDamage = DiceRoll().sides(8);
    WeaponArrangement defaultWeapon;
    defaultWeapon.damage.insert(DamageType::Crushing, defaultDamage);

    const auto armors = {10, 8, 5, 1, -1, -10};
    const auto rolls = {1, 2, 9, 10, 11, 15, 18, 19, 20};

    // To avoid testing the implementation by doing again the implementation in
    // the unit test, I sort of flip the numbers a bit by using an ascending
    // armor class system that I call "hit 11". I choose 11 because is the start
    // of the upper half of a 1-20 range, so you more or less get the 50%/50%
    // feeling of first level.
    // At base THAC0 = 20, your Attack Bonus is +1. So, for fighters the Attack
    // Bonus is their level, as it improves at one per level.
    // Defense Bonus would be just how much an armor/shield improves
    // AC: armor class.  TH: to hit
    // AB: attack bonus  DB: defense bonus

    for (int ac : armors) {
        for (int roll : rolls) {
            const int ab = 1;
            const int db = 10 - ac;
            const int th = 11 - ab + db;
            const int result = roll == 20 ? 2
                             : roll ==  1 ? 0
                             : roll >= th ? 1 : 0;
            QTest::addRow("defaults vs. AC %d, roll %d", ac, roll)
                    << defaultCommon << defaultWeapon << defaultWeapon
                    << ac << roll << result;
        }
    }
}

void tst_Calculators::testHitRatio()
{
    QFETCH(Damage::Common, common);
    QFETCH(WeaponArrangement, weapon1);
    QFETCH(WeaponArrangement, weapon2);
    QFETCH(int, ac);
    QFETCH(int, roll);
    QFETCH(int, result);

    const Damage calculator(weapon1, weapon2, common);
    QCOMPARE(calculator.hit(Damage::Main, ac, roll), result);
}

void tst_Calculators::testToHit_data()
{
    QTest::addColumn<int>("toHit");
    QTest::addColumn<double>("criticalChance");
    QTest::addColumn<double>("result");

    QTest::addRow("Fifty-fifty")                      << 11 << 0.05 << 0.50;
    QTest::addRow("Only criticals, on the edge")      << 20 << 0.05 << 0.05;
    QTest::addRow("Only criticals, past the edge")    << 24 << 0.05 << 0.05;

    for (int toHit = -10; toHit <= 30; ++toHit) {
        const double result = toHit >= 20 ? 0.05 :
                              toHit <= 2  ? 0.95 :
                              qBound(0.05, 0.05 + (20 - toHit) * 0.05, 0.95);
        QTest::addRow("Critical on 20; to hit: %d", toHit) << toHit << 0.05 << result;
    }

    for (int toHit = -10; toHit <= 30; ++toHit) {
        const double result = toHit >= 20 ? 0.10 :
                              toHit <= 2  ? 0.95 :
                              qBound(0.10, 0.05 + (20 - toHit) * 0.05, 0.95);
        QTest::addRow("Critical on 19; to hit: %d", toHit) << toHit << 0.10 << result;
    }

    for (int toHit = -10; toHit <= 30; ++toHit) {
        const double result = toHit >= 20 ? 0.15 :
                              toHit <= 2  ? 0.95 :
                              qBound(0.15, 0.05 + (20 - toHit) * 0.05, 0.95);
        QTest::addRow("Critical on 18; to hit: %d", toHit) << toHit << 0.15 << result;
    }

    for (int toHit = -10; toHit <= 30; ++toHit) {
        QTest::addRow("Critical Strike; to hit: %d", toHit) << toHit << 1.00 << 1.00;
    }
}

void tst_Calculators::testToHit()
{
    QFETCH(int, toHit);
    QFETCH(double, criticalChance);
    QFETCH(double, result);

    QCOMPARE(Calculators::chanceToHit(toHit, criticalChance), result);
}

void tst_Calculators::testDamage_data()
{
    QTest::addColumn<Damage::Common>("common");
    QTest::addColumn<WeaponArrangement>("weapon1");
    QTest::addColumn<WeaponArrangement>("weapon2");

    // TODO: Many more output values!
    QTest::addColumn<double>("average");

    const Damage::Common defaultCommon;

    QTest::addRow("defaults")
            << defaultCommon << defaultWeapon() << defaultWeapon()
            << 3.5;

    auto common = defaultCommon;
    common.statDamage = +7;
    QTest::addRow("19 STR")
            << common << defaultWeapon() << defaultWeapon()
            << 10.5;

    common.otherDamage = +2;
    QTest::addRow("19 STR with +2 bonus")
            << common << defaultWeapon() << defaultWeapon()
            << 12.5;

    WeaponArrangement weapon = defaultWeapon();
    weapon.damage.find(DamageType::Crushing).value().resistance(0.5);
    QTest::addRow("19 STR with +2 bonus at 50%% resistance")
            << common << weapon << weapon
            << 6.5;
}

void tst_Calculators::testDamage()
{
    QFETCH(Damage::Common, common);
    QFETCH(WeaponArrangement, weapon1);
    QFETCH(WeaponArrangement, weapon2);

    QFETCH(double, average);

    const Damage calculator(weapon1, weapon2, common);
    auto averageDamages = calculator.onHitDamages(Damage::Main, Damage::Average);
    QCOMPARE(averageDamages.count(), 1);
    QCOMPARE(averageDamages.value(DamageType::Crushing), average);
}

QTEST_MAIN(tst_Calculators)

#include "tst_calculators.moc"
