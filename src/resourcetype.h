#pragma once

#include <QMetaType>

// https://gibberlings3.github.io/iesdp/file_formats/general.htm
// TODO: Add a lot more

enum ResourceType : quint16
{
    NoType    = 0x0000,
    BmpType   = 0x0001,
    WavType   = 0x0004,
    BamType   = 0x03E8,
    MosType   = 0x03EC,
    ItmType   = 0x03ED,
    SplType   = 0x03EE,
    BcsType   = 0x03EF,
    IdsType   = 0x03F0,
    CreType   = 0x03F1,
    TdaType   = 0x03F4,
    BsType    = 0x03F9,
    PvrzType  = 0x0404,
    MenuType  = 0x0408,
};
Q_DECLARE_METATYPE(ResourceType)

inline constexpr const char* resourceTypeName(ResourceType type)
{
    switch (type) {
    case ItmType:
        return "ITM";
    case SplType:
        return "SPL";
    case CreType:
        return "CRE";
    case TdaType:
        return "2DA";
    default:
        return nullptr;
    }
}
