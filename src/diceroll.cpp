#include "diceroll.h"

#include <QDebug>
#include <QtMath>

DiceRoll &DiceRoll::sides(int sides)
{
    Q_ASSERT(sides >= 1);
    m_sides = sides;
    return *this;
}

DiceRoll& DiceRoll::number(int number)
{
    Q_ASSERT(number >= 1);
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
        result += luckified(i);
    result /= m_sides;
    result *= m_number;
    result += m_bonus;
    return result;
}

double DiceRoll::sigma() const
{
    // Get the average of one single roll, without bonuses.
    const double average = (this->average() - m_bonus)/m_number;
    double result = 0.0;
    for (int i = 1; i <= m_sides; ++i)
        result += qPow(luckified(i) - average, 2);
    result /= m_sides;
    result *= m_number;
    return qSqrt(result);
}

int DiceRoll::luckified(int value) const
{
    Q_ASSERT(value >= 1 && value <= m_sides);
    return qBound(1, value+m_luck, m_sides);
}

QDebug operator<<(QDebug debug, const DiceRoll& roll)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << roll.number() << "d" << roll.sides()
                    << "+" << roll.bonus() << "@" << roll.luck();
    return debug;
}
