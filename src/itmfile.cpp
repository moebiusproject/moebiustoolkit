#include "itmfile.h"

#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <QScopeGuard>

namespace
{

class DataStream : public QDataStream {};

DataStream& operator>>(DataStream& stream, QString& string)
{
    char rawName[8];
    stream.readRawData(rawName, sizeof(rawName));
    const uint nameLength = qMin(uint(8), qstrlen(rawName));
    string = QString::fromLatin1(rawName, nameLength);
    return stream;
}

template<typename T>
DataStream& operator>>(DataStream& stream, T& type)
{
    QDataStream& base = static_cast<QDataStream&>(stream);
    base >> type;
    return stream;
}


}

// https://gibberlings3.github.io/iesdp/file_formats/ie_formats/itm_v1.htm
// Structure:
// - HEADER
// - ABILITIES: As many as defined in the header (abilities count).
// - EFFECTS: As many as equipped effects ("Global effects" in NI),
//          + the effects declared in each ability (loop through them to count).
// Also: https://www.gibberlings3.net/forums/topic/2581-tutorial-item-patching/

bool ItmFile::isValid() const
{
    return fileType != Invalid;
}

QDataStream& operator>>(QDataStream& originalStream, ItmFile& file)
{
    DataStream& stream = static_cast<DataStream&>(originalStream);
    stream.setByteOrder(QDataStream::LittleEndian);

    auto onFailure = qScopeGuard([&file] {
        file.fileType = ItmFile::Invalid;
    });

    char signature[8];
    stream.readRawData(signature, sizeof(signature));
    constexpr auto start = "ITM V1  ";
    // TODO: Version 1.1 for PST, 2.0 for IWD 2.
    if (qstrncmp(signature, start, sizeof(start)) == 0)
        file.fileType = ItmFile::ItmV1;
    else
        return stream;

    // Discarded approach, for now. The idea is to make a raw read to the struct
    // that would not require any manual handling, just the declaration of the
    // struct makes the readRawData put everything into place. But then, somehow
    // copy as automatically as possible all the integers, but converting all
    // the strings to the right type (from raw to QString).
    // IOW, if we could have a struct for reading from disk, based on raw data,
    // then a struct for normal use, with the strings in a proper QString.
#if 0
    using RawStringType = std::array<char, 8>;
    ItmFile::BaseHeader<RawStringType> header;
    stream.readRawData(reinterpret_cast<char*>(&header), sizeof(header));

    file.header.unidentifiedName = header.unidentifiedName;
    file.header.identifiedName = header.identifiedName;
    file.header.replacementItem = QString::fromLatin1(header.replacementItem,
                                                      qstrlen(header.replacementItem.data()));
#endif

    stream >> file.unidentifiedName
           >> file.identifiedName
           >> file.replacementItem
           >> file.flags
           >> file.type
           >> file.usability
           >> file.animation0
           >> file.animation1
           >> file.requirementLevel
           >> file.requiredStrength
           >> file.requiredStrengthBonus
           >> file.kitUsability1
           >> file.requiredIntelligence
           >> file.kitUsability2
           >> file.requiredDexterity
           >> file.kitUsability3
           >> file.requiredWisdom
           >> file.kitUsability4
           >> file.requiredConstitution
           >> file.proficiency
           >> file.requiredCharisma
           >> file.price
           >> file.stackLimit
           >> file.inventoryIcon
           >> file.lore
           >> file.groundIcon
           >> file.weight
           >> file.unidentifiedDescription
           >> file.identifiedDescription
           >> file.descriptionIcon
           >> file.enchantment
           >> file.abilitiesStart
           >> file.abilitiesCount
           >> file.equippedEffectsStart
           >> file.equippedEffectsFirst
           >> file.equippedEffectsCount;

    file.abilities.reserve(file.abilitiesCount);
    stream.device()->seek(file.abilitiesStart);

    for (int count = 0; count < file.abilitiesCount; ++count) {
        ItmFile::ItmAbility ability;
        stream >> ability.type
               >> ability.identification
               >> ability.location
               >> ability.diceSidesAlternative
               >> ability.icon
               >> ability.targetType
               >> ability.targetCount
               >> ability.range
               >> ability.projectileType
               >> ability.diceNumberAlternative
               >> ability.speed
               >> ability.diceBonusAlternative
               >> ability.thac0Bonus
               >> ability.diceSides
               >> ability.primaryType
               >> ability.diceNumber
               >> ability.secondaryType
               >> ability.diceBonus
               >> ability.damageType
               >> ability.effectCount
               >> ability.effectIndex
               >> ability.charges
               >> ability.onChargeDepletion
               >> ability.flags
               >> ability.projectileAnimation1
               >> ability.projectileAnimation2
               >> ability.meleeAnimation1
               >> ability.meleeAnimation2
               >> ability.meleeAnimation3
               >> ability.bowArrow
               >> ability.crossbowBolt
               >> ability.otherProjectile;
    }

    if (stream.status() == QDataStream::Ok)
        onFailure.dismiss();
    return stream;

}
