TEMPLATE = app
TARGET = tst_tdafile

QT = core testlib
CONFIG += testcase
CONFIG -= app_bundle

# TODO: tons of strings to use QT_NO_CAST_FROM_ASCII so widely yet.
#projectGlobals()
useLibMoebius()

SOURCES += tst_tdafile.cpp

