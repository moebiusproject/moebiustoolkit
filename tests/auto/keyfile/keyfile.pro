TEMPLATE = app
TARGET = tst_keyfile

QT = core testlib
CONFIG += testcase no_testcase_installs
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_keyfile.cpp

