TEMPLATE = app
TARGET = moebiustoolkit
QT = core gui widgets charts

projectGlobals()
useLibMoebius()

HEADERS = \
    backstabcalculatorpage.h \
    basepage.h \
    buffcalculatorpage.h \
    damagecalculatorpage.h \
    gamebrowserpage.h \
    mainwindow.h \
    pageselector.h \
    pagetype.h \
    progressionchartspage.h \
    repeatedprobabilitypage.h \
    specialdamagewidget.h \
    weaponarrangementwidget.h \
    welcomepage.h \

SOURCES = main.cpp \
    backstabcalculatorpage.cpp \
    basepage.cpp \
    buffcalculatorpage.cpp \
    damagecalculatorpage.cpp \
    gamebrowserpage.cpp \
    mainwindow.cpp \
    pageselector.cpp \
    progressionchartspage.cpp \
    repeatedprobabilitypage.cpp \
    specialdamagewidget.cpp \
    weaponarrangementwidget.cpp \
    welcomepage.cpp \

FORMS += \
    backstabsetup.ui \
    damagecalculationwidget.ui \
    enemy.ui \
    gamebrowserpage.ui \
    specialdamagewidget.ui \
    weaponarrangementwidget.ui \
    welcomepage.ui \
