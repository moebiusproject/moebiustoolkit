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

#include "calculators.h"

#include <QDebug>

using namespace Calculators;

DiceRoll WeaponArrangement::physicalDamage() const
{
    Q_ASSERT(damage.contains(physicalDamageType()));
    DiceRoll result = damage.value(physicalDamageType());
    result.bonus(result.bonus() + damageBonuses());
    return result;
}

DamageType WeaponArrangement::physicalDamageType() const
{
    for (auto entry = damage.constBegin(), last = damage.constEnd(); entry != last; ++entry) {
        const DamageType type = entry.key();
        if (type < DamageType::ElementalBit)
            return type;
    }
    Q_UNREACHABLE();
    return DamageType::Crushing;
}

int Damage::hit(Hand hand, int ac, int roll) const
{
    const WeaponArrangement& arrangement = hand == One ? m_1 : m_2;
    // According to IESDP, on the documentation of effect #362 ("critical miss
    // bonus"), a critical hit bonus has precedence over critical miss one.
    const int criticalHitRoll = 21 - (arrangement.criticalHit/5);
    if (roll >= criticalHitRoll)
        return 2;
    else if (roll <= arrangement.criticalMiss/5)
        return 0;
    const int toHit = thac0(hand) - ac;
    if (roll >= toHit)
        return 1;
    return 0;
}

QPair<int, int> Damage::hitDistribution(Hand hand, int ac) const
{
    QPair<int, int> result;
    for (int roll = 1; roll <= 20; ++roll) {
        const int outcome = hit(hand, ac, roll);
        if (outcome == 1)
            ++result.first;
        else if (outcome == 2)
            ++result.second;
    }
    return result;
}

int Damage::thac0(Damage::Hand hand) const
{
    const WeaponArrangement& arrangement = hand == One ? m_1 : m_2;
    return m_common.thac0 - m_common.toHitBonuses() - arrangement.toHitBonuses();
}

QHash<DamageType, double> Damage::onHitDamages(Damage::Hand hand, Damage::Stat stat) const
{
    const WeaponArrangement& arrangement = hand == One ? m_1 : m_2;
    DiceRoll physicalDamageRoll = arrangement.physicalDamage();
    physicalDamageRoll.bonus(physicalDamageRoll.bonus() + m_common.damageBonuses());

    const double physicalDamage = physicalDamageRoll.average() * (stat == Regular ? 1 : 2);

    QHash<DamageType, double> result;
    for (auto entry = arrangement.damage.constBegin(), last = arrangement.damage.constEnd();
         entry != last; ++entry)
    {
        const DamageType type = entry.key();
        if (type & DamageType::ElementalBit) {
            // Elemental damage doesn't get bonuses, nor maximum damage (because
            // that's for Critical Strike).
            const DiceRoll roll = entry.value();
            result.insert(type, roll.average());
        }
        else // Physical damage.
            result.insert(type, physicalDamage);
    }
    return result;
}

double Damage::onHitDamage(Hand hand, Stat stat) const
{
    const auto damages = onHitDamages(hand, stat);
    double result = 0.0;
    for (auto damage : damages)
        result += damage;
    return result;
}

