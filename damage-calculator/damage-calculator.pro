TEMPLATE = app
TARGET = damage-calculator
QT = core gui widgets charts
CONFIG += c++14

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
           QT_NO_CAST_FROM_ASCII \

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -Wshadow

SOURCES += damage-calculator.cpp \
           attackbonuses.cpp \
           mainwindow.cpp \
           rollprobabilities.cpp \

HEADERS += attackbonuses.h \
           mainwindow.h \
           rollprobabilities.h \

FORMS += configuration.ui enemy.ui
