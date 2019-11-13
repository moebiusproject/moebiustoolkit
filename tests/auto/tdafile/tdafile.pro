TEMPLATE = app
TARGET = tst_tdafile

QT += testlib
CONFIG += testcase
CONFIG -= app_bundle

useLibMoebius()

SOURCES += tst_tdafile.cpp

