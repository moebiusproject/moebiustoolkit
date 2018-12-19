#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QLatin1String("AlejandroExojo"));
    QCoreApplication::setOrganizationDomain(QLatin1String("exojo.org"));
    QCoreApplication::setApplicationName(QLatin1String("DamageCalculator"));
    MainWindow window;
    window.showMaximized();
    return application.exec();
}
