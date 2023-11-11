TEMPLATE = app
TARGET = tst_gamesearcher

QT = core testlib
CONFIG += testcase no_testcase_installs
CONFIG -= app_bundle

projectGlobals()
useLibToolkit()

SOURCES += tst_gamesearcher.cpp

