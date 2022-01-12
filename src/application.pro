TEMPLATE = app
TARGET = moebiustoolkit
mac:TARGET = MoebiusToolkit
QT = core gui widgets charts

projectGlobals()
useLibMoebius()

DESTDIR = $$BUILD_TREE/bin
linux {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = $$PREFIX/bin
    desktop_file.files = moebiustoolkit.desktop
    desktop_file.path  = $$PREFIX/share/applications
    icon_file.files = moebiustoolkit.png
    icon_file.path  = $$PREFIX/share/pixmaps

    INSTALLS += target desktop_file icon_file
    DISTFILES += desktop_file icon_file
}

mac {
    # This silences a warning when using a too new SDK (which Qt has not been
    # tested with), but really doesn't seem to do much else.
    # CONFIG += sdk_no_version_check

    # TODO: The plist can have some "templated" values, so that qmake can
    # replace them based on the build. Review.
    QMAKE_INFO_PLIST = moebiustoolkit.plist
}

RESOURCES += resources.qrc

HEADERS = \
    backstabcalculatorpage.h \
    basepage.h \
    buffcalculatorpage.h \
    damagecalculatorpage.h \
    debugcharts.h \
    dualcalculatorpage.h \
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
    debugcharts.cpp \
    dualcalculatorpage.cpp \
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
    dualcalculatorwidget.ui \
    enemy.ui \
    gamebrowserpage.ui \
    progressionchartswidget.ui \
    specialdamagewidget.ui \
    weaponarrangementwidget.ui \
    welcomepage.ui \
