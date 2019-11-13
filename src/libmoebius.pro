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
    tdafile.h \
    tlkfile.h \

SOURCES = \
    diceroll.cpp \
    keyfile.cpp \
    tdafile.cpp \
    tlkfile.cpp \
