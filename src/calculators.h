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

namespace Calculators
{

/*!
 * \brief Return the chance of hitting for a certain to-hit number in a d20.
 * \param toHit The number to achieve in a d20 to hit a certain AC.
 * \param criticalChance The chance of critical hit. Typically 0.05 or 0.10 (or 1.00 during Critical Strike).
 * \return Chance to hit, from 0.05 to 1.00.
 */
double chanceToHit(int toHit, double criticalChance);

};
