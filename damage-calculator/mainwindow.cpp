#include "mainwindow.h"

#include "ui_configuration.h"

#include <QtCore>
#include <QtWidgets>
#include <QtCharts>

using namespace QtCharts;

struct MainWindow::Private
{
    QTabWidget* tabs = nullptr;
    QChart* chart = nullptr;
    QChartView* chartView = nullptr;

    QVector<Ui::configuration> configurations;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(new MainWindow::Private)
{
    d->chart = new QChart;
    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    d->tabs = new QTabWidget(this);
    setCentralWidget(d->tabs);

    // TODO: investigate if QTabBar::setTabButton could be used to manually add
    // a close button to the tabs that we want, and a "New..." to the chart.
    d->tabs->tabBar()->setTabsClosable(true);
    d->tabs->addTab(d->chartView, tr("Chart"));

    auto newButton = new QPushButton(tr("New"));
    d->tabs->setCornerWidget(newButton);
    connect(newButton, &QPushButton::clicked, [this](){
        auto widget = new QWidget;
        d->configurations.append(Ui::configuration());
        d->configurations.last().setupUi(widget);
        d->tabs->addTab(widget, tr("Configuration %1").arg(d->tabs->count()));
        d->tabs->setCurrentWidget(widget);
    });
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}
