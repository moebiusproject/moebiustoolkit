TEMPLATE = app
TARGET = tst_diceroll

QT += testlib
CONFIG += testcase
CONFIG -= app_bundle

useLibMoebius()

SOURCES += tst_diceroll.cpp

