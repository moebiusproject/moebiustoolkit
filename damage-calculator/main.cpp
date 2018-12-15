#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    MainWindow window;
    window.showMaximized();
    return application.exec();
}
