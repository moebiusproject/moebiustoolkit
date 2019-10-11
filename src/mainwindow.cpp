#include "mainwindow.h"

struct MainWindow::Private
{
    Private(MainWindow& window)
        : parent(window)
    {}

    MainWindow& parent;
};

MainWindow::MainWindow(QWidget* parentWidget)
    : QMainWindow(parentWidget)
    , d(new Private(*this))
{
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}

