TEMPLATE = app
TARGET = tst_bifffile

QT = core testlib
CONFIG += testcase no_testcase_installs
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_bifffile.cpp

