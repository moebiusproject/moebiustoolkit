#include "mainwindow.h"

#include "ui_configuration.h"

#include <QtCore>
#include <QtWidgets>
#include <QtCharts>

#include <functional>

using namespace QtCharts;

struct ArmorModifiers
{
    ArmorModifiers() = default;
    explicit ArmorModifiers(int c, int m, int p, int s)
        : crushing(c), missile(m), piercing(p), slashing(s)
    {}
    int crushing = 0;
    int missile = 0;
    int piercing = 0;
    int slashing = 0;
};
Q_DECLARE_METATYPE(ArmorModifiers)

struct MainWindow::Private
{
    QTabWidget* tabs = nullptr;
    QChart* chart = nullptr;
    QChartView* chartView = nullptr;

    QVector<Ui::configuration> configurations;

    void newPage();
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
    connect(newButton, &QPushButton::clicked,
            std::bind(&MainWindow::Private::newPage, d));
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}

void MainWindow::Private::newPage()
{
    auto widget = new QWidget;
    configurations.append(Ui::configuration());
    auto configuration = configurations.last();
    configuration.setupUi(widget);
    tabs->addTab(widget, tr("Configuration %1").arg(tabs->count()));
    tabs->setCurrentWidget(widget);

    auto setupStatsLabel = [](QSpinBox* proficiency, QSpinBox* dice, QSpinBox* sides,
                              QSpinBox* bonus, QLabel* result)
    {
        auto calculateStats = [=](){
            const double min = proficiency->value() + bonus->value()
                             + dice->value();
            const double max = proficiency->value() + bonus->value()
                             + dice->value() * sides->value();
            const double avg = proficiency->value() + bonus->value()
                             + dice->value() * (0.5 + sides->value()/2.0);

            result->setText(tr("Minimum/Maximum/Average: %1/%2/%3")
                            .arg(int(min)).arg(int(max))
                            .arg(avg, 0, 'f', 2));
        };

        connect(proficiency, qOverload<int>(&QSpinBox::valueChanged), calculateStats);
        connect(dice,        qOverload<int>(&QSpinBox::valueChanged), calculateStats);
        connect(sides,       qOverload<int>(&QSpinBox::valueChanged), calculateStats);
        connect(bonus,       qOverload<int>(&QSpinBox::valueChanged), calculateStats);
        calculateStats();
    };

    setupStatsLabel(configuration.proficiencyDamageModifier1,
                    configuration.weaponDamageDiceNumber1,
                    configuration.weaponDamageDiceSide1,
                    configuration.weaponDamageDiceBonus1,
                    configuration.damageDetail1);

    setupStatsLabel(configuration.proficiencyDamageModifier2,
                    configuration.weaponDamageDiceNumber2,
                    configuration.weaponDamageDiceSide2,
                    configuration.weaponDamageDiceBonus2,
                    configuration.damageDetail2);


    configuration.targetArmor->addItem(tr("Neutral"),
                                       QVariant::fromValue(ArmorModifiers(+0, +0, +0, +0)));
    configuration.targetArmor->addItem(tr("Leather"),
                                       QVariant::fromValue(ArmorModifiers(+0, +0, +2, +2)));
    configuration.targetArmor->addItem(tr("Studded Leather"),
                                       QVariant::fromValue(ArmorModifiers(+0, -1, -1, -2)));
    configuration.targetArmor->addItem(tr("Chain Mail"),
                                       QVariant::fromValue(ArmorModifiers(+2, +0, +0, -2)));
    configuration.targetArmor->addItem(tr("Splint Mail"),
                                       QVariant::fromValue(ArmorModifiers(-2, -1, -1, +0)));
    configuration.targetArmor->addItem(tr("Plate Mail"),
                                       QVariant::fromValue(ArmorModifiers(+0, +0, +0, -3)));
    configuration.targetArmor->addItem(tr("Full Plate Mail"),
                                       QVariant::fromValue(ArmorModifiers(+0, -3, -3, -4)));
    // TODO: add more armors, or modifiers from creatures.


    // FIXME: Review the captures. This can be fishy if there are removals
    connect(configuration.targetArmor, &QComboBox::currentTextChanged,
            [choice = configuration.targetArmor,
            crushing = configuration.crushingModifier, missile = configuration.missileModifier,
            piercing = configuration.piercingModifier, slashing = configuration.slashingModifier]
    {
        crushing->setValue(choice->currentData().value<ArmorModifiers>().crushing);
        missile->setValue(choice->currentData().value<ArmorModifiers>().missile);
        piercing->setValue(choice->currentData().value<ArmorModifiers>().piercing);
        slashing->setValue(choice->currentData().value<ArmorModifiers>().slashing);
    });
}
