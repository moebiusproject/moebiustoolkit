/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019-2020 Alejandro Exojo Piqueras
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

#include <QVector>

#pragma once

class QDebug;

class DiceRoll
{
    struct Arguments {
        int number = 1, sides = 1, bonus = 0, luck = 0;
        double resistance = 0.0, probability = 1.0;
    };
public:
    using Permutations = QVector<QVector<int>>;

    explicit DiceRoll(const Arguments& arguments);
    explicit DiceRoll() = default;

    Permutations permutations() const;

    [[nodiscard]] inline constexpr int number() const {return m_number;}
    [[nodiscard]] inline constexpr int sides() const {return m_sides;}
    [[nodiscard]] inline constexpr int bonus() const {return m_bonus;}
    [[nodiscard]] inline constexpr int luck() const {return m_luck;}
    [[nodiscard]] inline constexpr double resistance() const {return m_resistance;}
    [[nodiscard]] inline constexpr double probability() const {return m_probability;}

    DiceRoll& number(int number);
    DiceRoll& sides(int sides);
    DiceRoll& bonus(int bonus);
    DiceRoll& luck(int luck);
    DiceRoll& resistance(double resistance);
    DiceRoll& probability(double probability);

    DiceRoll number(int number) const;
    DiceRoll sides(int sides) const;
    DiceRoll bonus(int bonus) const;
    DiceRoll luck(int luck) const;
    DiceRoll resistance(double resistance) const;
    DiceRoll probability(double probability) const;

    int maximum() const;
    int minimum() const;
    double average() const;
    // TODO: Needs unit testing with probability being different from 1. Need to
    // find a site that allows to calculate the variance with a PMF.
    double sigma() const;

    int luckified(int value) const;
    int resistified(int value) const;

private:
    int m_number = 1;
    int m_sides = 1;
    int m_bonus = 0;
    int m_luck = 0;
    double m_resistance = 0.0;
    double m_probability = 1.0;
};

bool operator==(const DiceRoll& a, const DiceRoll& b);

QDebug operator<<(QDebug debug, const DiceRoll& roll);
