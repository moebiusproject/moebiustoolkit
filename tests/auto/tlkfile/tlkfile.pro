TEMPLATE = app
TARGET = tst_tlkfile

QT += testlib
CONFIG += testcase
CONFIG -= app_bundle

useLibMoebius()

SOURCES += tst_tlkfile.cpp

