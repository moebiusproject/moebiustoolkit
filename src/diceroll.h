/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019 Alejandro Exojo Piqueras
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

class QDebug;

class DiceRoll
{
public:
    explicit DiceRoll() = default;

    int sides() const {return m_sides;}
    int number() const {return m_number;}
    int bonus() const {return m_bonus;}
    int luck() const {return m_luck;}

    DiceRoll& sides(int sides);
    DiceRoll& number(int number);
    DiceRoll& bonus(int bonus);
    DiceRoll& luck(int luck);

    int maximum() const;
    int minimum() const;
    double average() const;
    double sigma() const;

    int luckified(int value) const;

private:
    int m_sides = 1;
    int m_number = 1;
    int m_bonus = 0;
    int m_luck = 0;
};

QDebug operator<<(QDebug debug, const DiceRoll& roll);
