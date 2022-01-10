TEMPLATE = app
TARGET = tst_tlkfile

QT = core testlib
CONFIG += testcase no_testcase_installs
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_tlkfile.cpp

