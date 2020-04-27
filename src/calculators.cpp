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
