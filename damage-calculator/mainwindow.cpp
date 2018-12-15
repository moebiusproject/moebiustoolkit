#include "mainwindow.h"

#include <QtCore>
#include <QtWidgets>

struct MainWindow::Private
{
    QTabWidget* tabs;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(new MainWindow::Private)
{
    d->tabs = new QTabWidget(this);
    setCentralWidget(d->tabs);
    d->tabs->tabBar()->setTabsClosable(true);
    d->tabs->addTab(new QWidget, QLatin1String("foo"));
    d->tabs->addTab(new QWidget, QLatin1String("bar"));
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}
