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
    void testDamage_data();
    void testDamage();
private:
    WeaponArrangement defaultWeapon() // Quarterstaff at Wintrhop's
    {
        WeaponArrangement result;
        result.damage.insert(DamageType::Crushing, DiceRoll().sides(6));
        return result;
    }

    WeaponArrangement ashideena()
    {
        WeaponArrangement result;
        result.damage.insert(DamageType::Crushing, DiceRoll().sides(4).bonus(3));
        result.damage.insert(DamageType::Electricity, DiceRoll().number(0).bonus(1));
        return result;
    }

    WeaponArrangement varscona()
    {
        WeaponArrangement result;
        result.damage.insert(DamageType::Slashing, DiceRoll().sides(8).bonus(2));
        result.damage.insert(DamageType::Cold, DiceRoll().number(0).bonus(1));
        return result;
    }
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

    const Damage::Common defaultCommon;
    Damage::Common common = defaultCommon;
    WeaponArrangement weapon = defaultWeapon();

    auto ab = [&common, &weapon]() {
        return 1 + (20 - common.thac0) + common.toHitBonuses() + weapon.toHitBonuses();
    };

    int ch = 20; // critical hit
    int cm = 1; // critical miss

    const auto armors = {10, 8, 5, 1, -1, -10};

    auto addRows = [&](const char* name) {
        for (int ac : armors) {
            for (int roll = 1; roll <= 20; ++roll) {
                const int db = 10 - ac;
                const int th = 11 + db - ab();
                const int result = roll >= ch ? 2
                                 : roll <= cm ? 0
                                 : roll >= th ? 1 : 0;
                QTest::addRow("%s vs. AC %d, roll %d", name, ac, roll)
                        << common << weapon << weapon
                        << ac << roll << result;
            }
        }
    };

    addRows("defaults");

    common.thac0 = 18;
    addRows("defaults, THAC0 18");

    common.thac0 = 8;
    addRows("defaults, THAC0 8");

    common.thac0 = 0;
    addRows("defaults, THAC0 0");

    common = defaultCommon;
    weapon.proficiencyToHit = 1;
    weapon.proficiencyDamage = 2; // Irrelevant, but for consistency.
    weapon.attacks = 1.5;         // Irrelevant, but for consistency.
    addRows("defaults, specialization");

    weapon.weaponToHit = 1;
    addRows("defaults, specialization, +1 weapon");

    weapon.criticalHit = 10;
    ch = 19;
    addRows("defaults, specialization, +1 weapon, 10% criticals");

    weapon.criticalMiss = 0;
    cm = 0;
    addRows("defaults, specialization, +1 weapon, 10% criticals, 0% critical miss");

    weapon.criticalHit = 0;
    ch = 21;
    weapon.criticalMiss = 15;
    cm = 3;
    addRows("defaults, specialization, +1 weapon, 0% criticals, 15% critical miss");
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
    QCOMPARE(calculator.hit(Damage::One, ac, roll), result);
}

// Note to self. This maybe "over tests" a bit. We do well in testing the
// DiceRoll extensively, which does some heavy lifting on the tricky math for
// the damage resistance, etc. But doing some complex tests here again maybe
// it's not strictly necessary. The calculator is adding up the values of the
// dice and the other sources of bonuses into the same bonus, and the DiceRoll
// class returns the average damage with all the bonuses merged. It's ignorant
// if a +7 comes from a +7 weapon or a +2 weapon with +5 from proficiency. The
// math for the average with resistance would be the same. Maybe we should be
// fine just testing that the calculator adds bonuses, and call it a day. But
// maybe not... Maybe we actually do well in ensuring that bonuses of
// proficiency are taken into account in the resistance, for example. So here I
// am, talking with myself because coding alone is lonely and I don't have with
// who to bounce ideas and problems with.

void tst_Calculators::testDamage_data()
{
    QTest::addColumn<Damage::Common>("common");
    QTest::addColumn<WeaponArrangement>("weapon1");
    QTest::addColumn<WeaponArrangement>("weapon2");

    QTest::addColumn<double>("physical");
    QTest::addColumn<double>("elemental");

    const Damage::Common defaultCommon;

    QTest::addRow("defaults")
            << defaultCommon << defaultWeapon() << defaultWeapon()
            << 3.5 << 0.0;

    auto common = defaultCommon;
    common.statDamage = +7;
    QTest::addRow("19 STR")
            << common << defaultWeapon() << defaultWeapon()
            << 10.5 << 0.0;

    common.otherDamage = +2;
    QTest::addRow("19 STR with +2 bonus")
            << common << defaultWeapon() << defaultWeapon()
            << 12.5 << 0.0;

    WeaponArrangement weapon = defaultWeapon();
    weapon.damage.find(DamageType::Crushing).value().resistance(0.5);
    QTest::addRow("19 STR with +2 bonus at 50%% resistance")
            << common << weapon << weapon
            << 6.5 << 0.0;

    weapon = ashideena();
    QTest::addRow("Ashideena, 19 STR with +2 bonus")
            << common << weapon << weapon
            << 14.5 << 1.0;

    weapon.damage.find(DamageType::Electricity).value().resistance(0.75);
    QTest::addRow("Ashideena, 19 STR with +2 bonus, 75%% resistance to element")
            << common << weapon << weapon
            << 14.5 << 1.0;

    weapon.damage.find(DamageType::Electricity).value().resistance(1.0);
    QTest::addRow("Ashideena, 19 STR with +2 bonus, 100%% resistance to element")
            << common << weapon << weapon
            << 14.5 << 0.0;

    common = defaultCommon;
    common.statDamage = 1;
    weapon = varscona();
    weapon.proficiencyDamage = weapon.proficiencyToHit = 3; // to hit irrelevant, but for consistency
    QTest::addRow("Varscona, 17 STR, mastery")
            << common << weapon << weapon
            << 10.5 << 1.0;

    weapon.damage.find(DamageType::Slashing).value().resistance(0.5);
    weapon.damage.find(DamageType::Cold).value().resistance(1.0);
    QTest::addRow("Varscona, 17 STR, mastery, 50%% slashing, 100%% cold")
            << common << weapon << weapon
            << 5.5 << 0.0;
}

void tst_Calculators::testDamage()
{
    QFETCH(Damage::Common, common);
    QFETCH(WeaponArrangement, weapon1);
    QFETCH(WeaponArrangement, weapon2);

    QFETCH(double, physical);
    QFETCH(double, elemental);

    const Damage calculator(weapon1, weapon2, common);
    const auto averageDamages = calculator.onHitDamages(Damage::One, Damage::Regular);
    QCOMPARE(averageDamages.value(weapon1.physicalDamageType()), physical);

    QHash<DamageType, double>::const_key_value_iterator result = averageDamages.keyValueBegin();
    for (; result != averageDamages.keyValueEnd(); ++result) {
        if (result.base().key() & DamageType::ElementalBit) {
            break;
        }
    }
    QCOMPARE(result.base().value(), elemental);
}

QTEST_MAIN(tst_Calculators)

#include "tst_calculators.moc"
