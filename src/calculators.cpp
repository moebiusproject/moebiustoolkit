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
    DiceRoll result = damage.find(physicalDamageType()).value();
    // TODO: This could be a nice opportunity for operator overloading in DiceRoll
    result.bonus(result.bonus() + proficiencyDamage);
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

int Damage::thac0(Damage::Hand hand) const
{
    // Common to both hands.
    int result = m_common.thac0 - m_common.strengthToHit - m_common.otherToHit;

    const WeaponArrangement& arrangement = hand == Main ? m_1 : m_2;
    result -= (arrangement.proficiencyToHit + arrangement.styleToHit + arrangement.weaponToHit);

    return result;
}

QHash<DamageType, double> Damage::onHitDamages(Damage::Hand hand, Damage::Stat stat) const
{
    const WeaponArrangement& arrangement = hand == Main ? m_1 : m_2;
    const double bonusPhysicalDamage = m_common.strengthDamage + m_common.otherDamage;
    DiceRoll physicalDamageRoll = arrangement.physicalDamage();
    physicalDamageRoll.bonus(physicalDamageRoll.bonus() + bonusPhysicalDamage);

    const double physicalDamage = stat == Average ? physicalDamageRoll.average()
                                                  : physicalDamageRoll.maximum();

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

double Calculators::chanceToHit(int toHit, double criticalChance)
{
    Q_ASSERT(criticalChance >= 0.05 && criticalChance <= 1.00);

    if (qFuzzyCompare(1.0, criticalChance))
        return 1.0;

    const int firstCriticalRoll = (21 - criticalChance*20);
    // Equivalent to:
    // const int firstCriticalRoll = (105 - criticalChance*100) / 5;
    if (toHit <= 2) // Only critical failures fail: 95% chance of hitting
        return 0.95;
    if (toHit < firstCriticalRoll)
        return 1 - (0.05 * (toHit-1));
    else // Only critical hits work.
        return criticalChance;
}
