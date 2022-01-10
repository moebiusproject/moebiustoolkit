TEMPLATE = lib
win32:CONFIG += staticlib
CONFIG(release, debug|release):CONFIG += staticlib
TARGET = moebius
QT = core

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin


win32:!win32-g++:QMAKE_CXXFLAGS += /Zi

HEADERS = \
    backstabstats.h \
    bifffile.h \
    calculators.h \
    diceroll.h \
    keyfile.h \
    packed.h \
    resourcemanager.h \
    resourcetype.h \
    tdafile.h \
    tlkfile.h \
    xplevels.h \

SOURCES = \
    backstabstats.cpp \
    bifffile.cpp \
    calculators.cpp \
    diceroll.cpp \
    keyfile.cpp \
    resourcemanager.cpp \
    tdafile.cpp \
    tlkfile.cpp \
    xplevels.cpp \
