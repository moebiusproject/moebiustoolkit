TEMPLATE = app
TARGET = tst_resourcemanager

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

projectGlobals()
useLibMoebius()

SOURCES += tst_resourcemanager.cpp

