TEMPLATE = app
TARGET = tst_keyfile

QT += testlib
CONFIG += testcase
CONFIG -= app_bundle

useLibMoebius()

SOURCES += tst_keyfile.cpp

