TEMPLATE = app
TARGET = tst_bifffile

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_bifffile.cpp

