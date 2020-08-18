TEMPLATE = app
TARGET = moebiustoolkit
QT = core gui widgets charts

projectGlobals()
useLibMoebius()

HEADERS = \
    backstabcalculatorpage.h \
    basepage.h \
    damagecalculatorpage.h \
    mainwindow.h \
    pageselector.h \
    pagetype.h \
    repeatedprobabilitypage.h \
    welcomepage.h \

SOURCES = main.cpp \
    backstabcalculatorpage.cpp \
    basepage.cpp \
    damagecalculatorpage.cpp \
    mainwindow.cpp \
    pageselector.cpp \
    repeatedprobabilitypage.cpp \
    welcomepage.cpp \

FORMS += \
    backstabsetup.ui \
    damagecalculationwidget.ui \
    enemy.ui \
    specialdamagewidget.ui \
    weaponarrangementwidget.ui \
    welcomepage.ui \
