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

#pragma once

#include <QHash>
#include <QMetaType>

#include "diceroll.h"

namespace Calculators
{

enum DamageType
{
    /// If this bit is set, it's elemental. Physical otherwise.
    ElementalBit    = 0b1000,

    Crushing        = 0,
    Missile         = 1,
    Piercing        = 2,
    Slashing        = 3,

    Acid            = ElementalBit + 0,
    Cold            = ElementalBit + 1,
    Electricity     = ElementalBit + 2,
    Fire            = ElementalBit + 3,

    MagicDamage     = ElementalBit + 4,
    PoisonDamage    = ElementalBit + 5,
    // TODO: About poison/bleed damage, consider that it might be a different
    // effect in many weapons ("get poisoned" vs "direct poison damage").
};

struct WeaponArrangement {
    int proficiencyToHit = 0;
    int styleToHit = 0;
    int weaponToHit = 0;

    int proficiencyDamage = 0;

    QHash<DamageType, DiceRoll> damage;
    double attacks = 1.0;
    double criticalHit = 0.05;
    double criticalMiss = 0.05;

    /// Returns the physical damage roll with the arrangment modifiers applied.
    /// Doesn't include the "global" modifiers like Strength, class.
    DiceRoll physicalDamage() const;
    DamageType physicalDamageType() const;
};

struct Backstab {
    struct Other {
        int multiplier = 2;
        int kit = 0;
        int bonus = 0;
        int strength = 0;
    };

    explicit Backstab(WeaponArrangement weapon_, Backstab::Other other_)
        : weapon(weapon_)
        , other(other_)
    {}

    WeaponArrangement weapon;
    Other other;
};

class Damage {
public:
    struct Common {
        int thac0 = 20;

        int statToHit = 0;
        int otherToHit = 0;

        int statDamage = 0;
        int otherDamage = 0;
    };

    explicit Damage(const WeaponArrangement& one, const WeaponArrangement& two,
                    const Damage::Common& common)
        : m_1(one)
        , m_2(two)
        , m_common(common)
    {
    }

    enum Hand {Main, OffHand};
    // TODO: This probably should be expanded to cover more use cases. One, for
    // example, might be "average but with critical strike" which makes the
    // physical damage be maximum, but the elemental damages still be average.
    enum Stat {Average, Maximum};

    int thac0(Hand hand) const;
    QHash<DamageType, double> onHitDamages(Hand hand, Stat stat) const;

private:
    WeaponArrangement m_1, m_2;
    Damage::Common m_common;
};

/*!
 * \brief Return the chance of hitting for a certain to-hit number in a d20.
 * \param toHit The number to achieve in a d20 to hit a certain AC.
 * \param criticalChance The chance of critical hit. Typically 0.05 or 0.10 (or 1.00 during Critical Strike).
 * \return Chance to hit, from 0.05 to 1.00.
 */
double chanceToHit(int toHit, double criticalChance);

};

Q_DECLARE_METATYPE(Calculators::DamageType)
Q_DECLARE_METATYPE(Calculators::WeaponArrangement)
Q_DECLARE_METATYPE(Calculators::Damage::Common)
