TEMPLATE = app
TARGET = tst_itmfile

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_itmfile.cpp

