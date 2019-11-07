TEMPLATE = lib
TARGET = moebius
QT = core

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin

HEADERS = \
    keyfile.h \
    packed.h \
    tlkfile.h \

SOURCES = \
    keyfile.cpp \
    tlkfile.cpp \
