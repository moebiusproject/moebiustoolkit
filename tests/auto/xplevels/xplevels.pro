TEMPLATE = app
TARGET = tst_xplevels

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_xplevels.cpp

