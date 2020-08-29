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
    backstabstats.h \
    diceroll.h \
    keyfile.h \
    packed.h \
    resourcemanager.h \
    resourcetype.h \
    tdafile.h \
    tlkfile.h \

SOURCES = \
    bifffile.cpp \
    calculators.cpp \
    backstabstats.cpp \
    diceroll.cpp \
    keyfile.cpp \
    resourcemanager.cpp \
    tdafile.cpp \
    tlkfile.cpp \
