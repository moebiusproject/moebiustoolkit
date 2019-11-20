TEMPLATE = app
TARGET = tst_tlkfile

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

useLibMoebius()

SOURCES += tst_tlkfile.cpp

