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
