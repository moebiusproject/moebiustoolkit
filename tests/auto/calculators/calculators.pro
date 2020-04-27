TEMPLATE = app
TARGET = tst_calculators

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_calculators.cpp

