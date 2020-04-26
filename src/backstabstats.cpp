#include "backstabstats.h"

BackstabResult calculateBackstab(const BackstabInput& input)
{
    const auto m = input.multiplier;
    return BackstabResult {
        .weapon = m * (input.weapon.maximum()),
        .other  = m * (input.proficiency + input.style + input.kit + input.other)
    };
}
