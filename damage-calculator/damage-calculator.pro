TEMPLATE = app
TARGET = damage-calculator
QT = core gui widgets charts
CONFIG += c++11

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
           QT_NO_CAST_FROM_ASCII \

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -Wshadow

SOURCES += main.cpp \
           mainwindow.cpp \

HEADERS += mainwindow.h \

FORMS += window.ui
