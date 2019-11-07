TEMPLATE = lib
TARGET = moebius
QT = core

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin

HEADERS = \
    diceroll.h \
    keyfile.h \
    packed.h \
    tlkfile.h \

SOURCES = \
    diceroll.cpp \
    keyfile.cpp \
    tlkfile.cpp \
