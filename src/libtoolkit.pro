TEMPLATE = lib
win32:CONFIG += staticlib
CONFIG(release, debug|release):CONFIG += staticlib
TARGET = toolkit
QT = core gui

projectGlobals()

INCLUDEPATH += .
DESTDIR = $$BUILD_TREE/lib
DLLDESTDIR = $$BUILD_TREE/bin


win32:!win32-g++:QMAKE_CXXFLAGS += /Zi

HEADERS = \
    gamebrowserresourcefilter.h \
    gamesearcher.h \

SOURCES = \
    gamebrowserresourcefilter.cpp \
    gamesearcher.cpp \
