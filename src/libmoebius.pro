TEMPLATE = lib
TARGET = moebius
QT = core

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin

HEADERS = \
    packed.h \
    tlkfile.h \

SOURCES = \
    tlkfile.cpp \
