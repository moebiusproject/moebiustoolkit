TEMPLATE = lib
TARGET = moebius
QT = core

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin

HEADERS = \
    calculators.h \
    diceroll.h \
    keyfile.h \
    packed.h \
    tdafile.h \
    tlkfile.h \

SOURCES = \
    calculators.cpp \
    diceroll.cpp \
    keyfile.cpp \
    tdafile.cpp \
    tlkfile.cpp \
