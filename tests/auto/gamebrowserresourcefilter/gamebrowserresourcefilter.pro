TEMPLATE = app
TARGET = tst_gamebrowserresourcefilter

QT = core testlib
CONFIG += testcase no_testcase_installs
CONFIG -= app_bundle

projectGlobals()
useLibToolkit()

SOURCES += tst_gamebrowserresourcefilter.cpp

