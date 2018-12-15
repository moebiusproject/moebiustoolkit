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
        auto configuration = d->configurations.last();
        configuration.setupUi(widget);
        d->tabs->addTab(widget, tr("Configuration %1").arg(d->tabs->count()));
        d->tabs->setCurrentWidget(widget);


        auto setup = [](QSpinBox* proficiency, QSpinBox* dice, QSpinBox* sides,
                QSpinBox* bonus, QLabel* result)
        {
            auto calculateStats = [=](){
                const double min = proficiency->value() + bonus->value()
                                 + dice->value();
                const double max = proficiency->value() + bonus->value()
                                 + dice->value() * sides->value();
                const double avg = proficiency->value() + bonus->value()
                                 + dice->value() * (0.5 + sides->value()/2.0);

                result->setText(tr("Min/Max/Average: %1/%2/%3")
                                .arg(int(min)).arg(int(max))
                                .arg(avg, 0, 'f', 2));
            };

            connect(proficiency, qOverload<int>(&QSpinBox::valueChanged), calculateStats);
            connect(dice,        qOverload<int>(&QSpinBox::valueChanged), calculateStats);
            connect(sides,       qOverload<int>(&QSpinBox::valueChanged), calculateStats);
            connect(bonus,       qOverload<int>(&QSpinBox::valueChanged), calculateStats);
            calculateStats();
        };

        setup(configuration.proficiencyDamageModifier1,
              configuration.weaponDamageDiceNumber1,
              configuration.weaponDamageDiceSide1,
              configuration.weaponDamageDiceBonus1,
              configuration.damageDetail1);

        setup(configuration.proficiencyDamageModifier2,
              configuration.weaponDamageDiceNumber2,
              configuration.weaponDamageDiceSide2,
              configuration.weaponDamageDiceBonus2,
              configuration.damageDetail2);
    });
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}
