TEMPLATE = app
TARGET = tst_tdafile

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

useLibMoebius()

SOURCES += tst_tdafile.cpp

