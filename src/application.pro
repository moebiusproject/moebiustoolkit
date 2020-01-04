TEMPLATE = app
TARGET = moebiustoolkit
QT = core gui widgets charts

projectGlobals()

HEADERS = \
    backstabcalculatorpage.h \
    damagecalculatorpage.h \
    mainwindow.h \
    pageselector.h \
    pagetype.h \
    welcomepage.h \

SOURCES = main.cpp \
    backstabcalculatorpage.cpp \
    damagecalculatorpage.cpp \
    mainwindow.cpp \
    pageselector.cpp \
    welcomepage.cpp \

FORMS += \
    backstabsetup.ui \
    configuration.ui \
    enemy.ui \
    welcomepage.ui \
