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

/*!
 * \brief Struct mapping the inputs in WeaponArrangementWidget
 */
struct WeaponArrangement {
    int proficiencyToHit = 0;
    int styleToHit = 0;
    int weaponToHit = 0;

    int proficiencyDamage = 0;

    // Each damage roll is purely from the weapon.
    QHash<DamageType, DiceRoll> damage;
    double attacks = 1.0;
    int criticalHit = 5;
    int criticalMiss = 5;

    // This silly helpers just serve making apparent which part of the code
    // needs to change if fields are changed in the future.
    int toHitBonuses() const  { return proficiencyToHit  + styleToHit + weaponToHit; }
    int damageBonuses() const { return proficiencyDamage; }

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
        // Class+misc damage. To split(?) if the breakdown chart gets implemented.
        int otherDamage = 0;

        // This silly helpers just serve making apparent which part of the code
        // needs to change if fields are changed in the future.
        int toHitBonuses() const  { return statToHit  + otherToHit; }
        int damageBonuses() const { return statDamage + otherDamage; }
    };

    explicit Damage(const WeaponArrangement& one, const WeaponArrangement& two,
                    const Damage::Common& common)
        : m_1(one)
        , m_2(two)
        , m_common(common)
    {
    }

    enum Hand {One, Two};
    enum Stat {Regular, Critical};

    int hit(Hand hand, int ac, int roll) const;
    // Returns how many rolls should be a normal or a critical hit (in the 1-20
    // range). No need to return the failures, because those are the remaining.
    QPair<int, int> hitDistribution(Hand hand, int ac) const;
    // TODO: The name is not too good as the THAC0 should be used only for base THAC0.
    // But effectively, this is the number to hit AC 0, once modifiers are applied.
    int thac0(Hand hand) const;
    QHash<DamageType, double> onHitDamages(Hand hand, Stat stat) const;
    double onHitDamage(Hand hand, Stat stat) const;

private:
    WeaponArrangement m_1, m_2;
    Damage::Common m_common;
};

};

Q_DECLARE_METATYPE(Calculators::DamageType)
Q_DECLARE_METATYPE(Calculators::WeaponArrangement)
Q_DECLARE_METATYPE(Calculators::Damage::Common)
