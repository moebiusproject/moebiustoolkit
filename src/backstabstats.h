#pragma once

#include <QtGlobal>

#include "diceroll.h"

struct BackstabInput
{
    quint8 multiplier;
    DiceRoll weapon;
    quint8 proficiency;
    quint8 style;
    quint8 kit;
    quint8 other;
};

struct BackstabResult
{
    int weapon;
    int other;
};

BackstabResult calculateBackstab(const BackstabInput& input);
