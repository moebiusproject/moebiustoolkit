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

#include "diceroll.h"

#include <QDebug>
#include <QtMath>

namespace
{
// This function is CC-BY-SA 3.0: https://stackoverflow.com/a/17050528/9164547
    QVector<QVector<int>> cartesianProduct(const QVector<QVector<int>>& inputs)
    {
        QVector<QVector<int>> result = {{}};
        for (const auto& u : inputs) {
            QVector<QVector<int>> r;
            for (const auto& x : result) {
                for (const auto y : u) {
                    r.append(x);
                    r.last().append(y);
                }
            }
            result = std::move(r);
        }
        return result;
    }
}

DiceRoll::DiceRoll(const Arguments& arguments)
    : m_number(arguments.number)
    , m_sides(arguments.sides)
    , m_bonus(arguments.bonus)
    , m_luck(arguments.luck)
    , m_resistance(arguments.resistance)
    , m_probability(arguments.probability)
{
}

// TODO: simplify the cartesian product function with something that can assume
// our case: each dice is equal, so {1,2,3}{1,2,3} is redundant. We could pass
// something like {1,2,3}{2} as input instead, and ditch the "more advanced"
// cartesian product if needed.
DiceRoll::Permutations DiceRoll::permutations() const
{
    QVector<QVector<int>> inputs;

    QVector<int> oneDiceRolls;
    for (int x = 1; x <= m_sides; ++x)
        oneDiceRolls.append(luckified(x));
    for (int x = 1; x <= m_number; ++x)
        inputs.append(oneDiceRolls);

    return cartesianProduct(inputs);
}

DiceRoll& DiceRoll::sides(int sides)
{
    Q_ASSERT(sides >= 1);
    m_sides = sides;
    return *this;
}

DiceRoll& DiceRoll::number(int number)
{
    Q_ASSERT(number >= 0);
    m_number = number;
    return *this;
}

DiceRoll& DiceRoll::bonus(int bonus)
{
    m_bonus = bonus;
    return *this;
}

DiceRoll& DiceRoll::luck(int luck)
{
    m_luck = luck;
    return *this;
}

DiceRoll& DiceRoll::resistance(double resistance)
{
    Q_ASSERT(resistance >= 0.0 && resistance <= 1.0);
    m_resistance = resistance;
    return *this;
}

DiceRoll& DiceRoll::probability(double probability)
{
    Q_ASSERT(probability >= 0.0 && probability <= 1.0);
    m_probability = probability;
    return *this;
}

int DiceRoll::maximum() const
{
    return m_number * qBound(1, m_sides+m_luck, m_sides) + m_bonus;
}

int DiceRoll::minimum() const
{
    return m_number * qBound(1, 1+m_luck, m_sides) + m_bonus;
}

double DiceRoll::average() const
{
    double result = 0.0;
    for (int i = 1; i <= m_sides; ++i)
        result += resistified(m_number * luckified(i) + m_bonus);
    result /= m_sides;
    result *= m_probability;
    return result;
}

// Using this as reference:
// https://en.wikipedia.org/w/index.php?title=Variance&oldid=971548883#Discrete_random_variable
// This function started using the simplest formula, where each possible outcome
// is equally likely:
//     \frac{1}{n} \sum_{i=1}^n (x_i - \mu)^2
// However, since we supported the probability (as in Infinity Engine's
// probability of an effect happening), we have to use the one that has the
// Probability Mass Function (PMF):
//     \sum_{i=1}^n p_i\cdot(x_i - \mu)^2
// We can simplify it a bit to our use case: the probability of each entry in
// the permutations is the same, and the probability of 0 (equivalent to the
// effect not happening anything) is (1-m_probability). The contribution of each
// value is (p_i * (x_i - mu)^2), so for 0 is ((1-probability)*mu^2).
double DiceRoll::sigma() const
{
    const auto mu = average();
    const auto permutations = this->permutations();
    // Probability of each different XdY roll is the same.
    const auto p = (m_probability / permutations.size());

    double result = 0.0;
    for (const QVector<int>& rolls : permutations) {
        double value = 0.0;
        for (const auto& roll : rolls) {
            value += roll;
        }
        // The bonus doesn't get added to the values from the permutations, as
        // it should be added only once, here. Then lowered by resistance.
        value = resistified(value + m_bonus);
        value = qPow(value - mu, 2);
        result += p * value;
    }
    // Contribution of 0 happenning.
    result += (1.0 - m_probability) * qPow(mu, 2);
    return qSqrt(result);
}

int DiceRoll::luckified(int value) const
{
    Q_ASSERT(value >= 1 && value <= m_sides);
    return qBound(1, value+m_luck, m_sides);
}

int DiceRoll::resistified(int value) const
{
    Q_ASSERT(value >= minimum() && value <= maximum());
    return qCeil(value * (1 - m_resistance));
}

QDebug operator<<(QDebug debug, const DiceRoll& roll)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << roll.number() << "d" << roll.sides()
                    << "+" << roll.bonus() << "@" << roll.luck();
    return debug;
}

// TODO: unit test it.
bool operator==(const DiceRoll& a, const DiceRoll& b)
{
    return a.sides()  == b.sides() &&
           a.number() == b.number() &&
           a.bonus()  == b.bonus() &&
           a.luck()   == b.luck() &&
           // qFuzzyCompare can't deal well with 0.0. Shifting both is fine.
           qFuzzyCompare(a.resistance() + 1.0, b.resistance() + 1.0);
}
