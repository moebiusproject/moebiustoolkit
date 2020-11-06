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

#include "globals.h"
#include "packed.h"
#include "resourcetype.h"

#include <QByteArray>
#include <QDataStream>
#include <QMetaType>
#include <QVector>

// TODO: One use case that seems worth optimizing: doing only a minimal parse of
// an ITM file, where only the very first fields of the headers are read. The
// signature and the string references (i.e. simple integers) that give the name
// of the item, but nothing else. Anything below not only is not needed for
// showing the initial list of items/spells/etc, it also starts with a ResRef
// (that is, an actual string which requires memory allocation to hold).

struct ItmFile
{
    enum Type {
        Invalid = 0,
        ItmV1,
        ItmV1x1,
        ItmV2,
    } fileType = Invalid;

    StrRef unidentifiedName = 0;
    StrRef identifiedName = 0;
    ResRef replacementItem; ///< For when the item gets the charges drained.
    quint32 flags = 0;
    quint16 type = 0;
    quint32 usability = 0;
    quint8 animation0 = 0;
    quint8 animation1 = 0;
    quint16 requirementLevel = 0;
    quint16 requiredStrength = 0;
    quint8 requiredStrengthBonus = 0;
    quint8 kitUsability1 = 0;
    quint8 requiredIntelligence = 0;
    quint8 kitUsability2 = 0;
    quint8 requiredDexterity = 0;
    quint8 kitUsability3 = 0;
    quint8 requiredWisdom = 0;
    quint8 kitUsability4 = 0;
    quint8 requiredConstitution = 0;
    quint8 proficiency = 0;
    quint16 requiredCharisma = 0;
    quint32 price = 0;
    quint16 stackLimit = 0;
    ResRef inventoryIcon;
    quint16 lore = 0;
    ResRef groundIcon;
    quint32 weight = 0;
    StrRef unidentifiedDescription = 0;
    StrRef identifiedDescription = 0;
    ResRef descriptionIcon;
    quint32 enchantment = 0;
    quint32 abilitiesStart = 0;
    quint16 abilitiesCount = 0;
    quint32 equippedEffectsStart = 0;
    quint16 equippedEffectsFirst = 0;
    quint16 equippedEffectsCount = 0;

    // TODO: Many of this numbers could be an enum of N bytes ìnstead, so it
    // would be clearer. But I would need to decide where to put them, after I
    // know if are specific for this structure or more general in the game.
    struct ItmAbility {
        quint8 type = 0;
        // TODO: This probably needs a better name, after I figure out how
        // identification works. How does an item gets stored in the ID/non-ID
        // states? Does this field change value?
        bool identification = false;
        quint8 location = 0; //< Location in the quick slot? Weapon, Spell, Equipment, Innate.
        quint8 diceSidesAlternative = 0;
        ResRef icon; ///< Icon for when the ability is activated/used.
        quint8 targetType; ///< Who/how to target an action of the ability
        quint8 targetCount;
        quint16 range;
        quint8 projectileType;
        quint8 diceNumberAlternative;
        quint8 speed;
        quint8 diceBonusAlternative;
        quint16 thac0Bonus; // Make it 0x7fff for "always hit".
        quint8 diceSides;
        quint8 primaryType;
        quint8 diceNumber;
        quint8 secondaryType;
        quint16 diceBonus;
        quint16 damageType;
        quint16 effectCount;
        quint16 effectIndex;
        quint16 charges;
        quint16 onChargeDepletion;
        quint32 flags;
        quint8 projectileAnimation1;
        quint8 projectileAnimation2;
        quint16 meleeAnimation1;
        quint16 meleeAnimation2;
        quint16 meleeAnimation3;
        quint16 bowArrow; ///< 0/1 for no/yes
        quint16 crossbowBolt; ///< 0/1 for no/yes
        quint16 otherProjectile; ///< 0/1 for no/yes
    };

    QVector<ItmAbility> abilities;

    bool isValid() const;

private:
    friend class tst_KeyFile;
    friend QDataStream& operator>>(QDataStream& stream, ItmFile& file);
};

QDataStream& operator>>(QDataStream& stream, ItmFile& file);
