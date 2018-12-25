TEMPLATE = app
TARGET = damage-calculator
QT = core gui widgets charts
CONFIG += c++14

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
           QT_NO_CAST_FROM_ASCII \

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -Wshadow

SOURCES += damage-calculator.cpp \
           mainwindow.cpp \

HEADERS += mainwindow.h \

FORMS += configuration.ui enemy.ui
