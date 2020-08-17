TEMPLATE = lib
win32:CONFIG += staticlib
TARGET = moebius
QT = core

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin


win32:!win32-g++:QMAKE_CXXFLAGS += /Zi

HEADERS = \
    bifffile.h \
    calculators.h \
    diceroll.h \
    keyfile.h \
    packed.h \
    resourcetype.h \
    tdafile.h \
    tlkfile.h \

SOURCES = \
    bifffile.cpp \
    calculators.cpp \
    diceroll.cpp \
    keyfile.cpp \
    tdafile.cpp \
    tlkfile.cpp \
