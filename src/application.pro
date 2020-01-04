TEMPLATE = app
TARGET = moebiustoolkit
QT = core gui widgets charts

projectGlobals()

HEADERS = \
    damagecalculatorpage.h \
    mainwindow.h \
    pageselector.h \
    pagetype.h \
    welcomepage.h \

SOURCES = main.cpp \
    damagecalculatorpage.cpp \
    mainwindow.cpp \
    pageselector.cpp \
    welcomepage.cpp \

FORMS += \
    configuration.ui \
    enemy.ui \
    welcomepage.ui \
