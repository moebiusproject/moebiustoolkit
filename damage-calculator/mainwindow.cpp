#include "mainwindow.h"

#include "ui_configuration.h"

#include <QtCore>
#include <QtWidgets>
#include <QtCharts>

#include <functional>
#include <numeric>

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
    QVector<int> armorClasses;

    QTabWidget* tabs = nullptr;
    QChart* chart = nullptr;
    QChartView* chartView = nullptr;

    QVector<Ui::configuration> configurations;

    void newPage();
    void generateCharts()
    {
        chart->removeAllSeries();
        for (int tab = 1; tab < tabs->count(); ++tab) {
            generateChart(configurations.at(tab-1));
        }
        chart->createDefaultAxes();

        if (QValueAxis* axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first())) {
            axis->setTickCount(armorClasses.count());
            axis->setLabelFormat(QLatin1String("%i"));
        }
        if (QValueAxis* axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first())) {
            axis->setMin(0);
            axis->applyNiceNumbers();
#if 0 // Keep just in case. But the above seems to work well
            const int rounded = int(std::ceil(axis->max()));
            axis->setMax(rounded);
            axis->setTickCount(rounded+1); // Because we have to count the one at 0
#endif
            axis->setLabelFormat(QLatin1String("%d"));
        }
    }
    void generateChart(const Ui::configuration &c);
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(new MainWindow::Private)
{
    for (int i = 10; i >= -20; --i)
        d->armorClasses << i;

    d->chart = new QChart;
    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    d->tabs = new QTabWidget(this);
    setCentralWidget(d->tabs);

    // TODO: investigate if QTabBar::setTabButton could be used to manually add
    // a close button to the tabs that we want, and a "New..." to the chart.
    d->tabs->tabBar()->setTabsClosable(true);
    d->tabs->addTab(d->chartView, tr("Chart"));
    connect(d->tabs, &QTabWidget::currentChanged, [this](int index){
        if (index == 0)
            d->generateCharts();
    });

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
    Ui::configuration& configuration = configurations.last();
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
                            .arg(avg, 0, 'f', 1));
            result->setProperty("avg", avg);
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



void MainWindow::Private::generateChart(const Ui::configuration& c)
{
    const bool offHand = c.offHandGroup->isChecked();

    const int thac0 = c.baseThac0->value() - c.strengthThac0Bonus->value() - c.classThac0Bonus->value();
    const int mainThac0 = thac0 - c.proficiencyThac0Modifier1->value() - c.styleModifier1->value() - c.weaponThac0Modifier1->value();
    const int offThac0  = thac0 - c.proficiencyThac0Modifier2->value() - c.styleModifier2->value() - c.weaponThac0Modifier2->value();

    // TODO: brittle approach. Relies on the order set on the UI. Set user data instead.
    auto modifierFromUi = [&c](QComboBox* box) {
        return ( box->currentIndex() == 0 ? c.crushingModifier->value()
               : box->currentIndex() == 1 ? c.missileModifier->value()
               : box->currentIndex() == 2 ? c.piercingModifier->value()
                                          : c.slashingModifier->value() );
    };
    const int mainAcModifier = modifierFromUi(c.damageType1);
    const int offAcModifier  = modifierFromUi(c.damageType2);

    // TODO: don't use an ugly property for this.
    const double mainDamage = c.damageDetail1->property("avg").toDouble()
                            + c.strengthDamageBonus->value()
                            + c.classDamageBonus->value();
    const double offDamage  = c.damageDetail2->property("avg").toDouble()
                            + c.strengthDamageBonus->value()
                            + c.classDamageBonus->value();

    const double mainApr = c.attacksPerRound1->value();
    const int    offApr  = c.attacksPerRound2->value();

    QLineSeries* series = new QLineSeries;

    for (const int ac : armorClasses) {
        const int mainToHit = mainThac0 - ac - mainAcModifier;
        const int offToHit  = offThac0  - ac - offAcModifier;

        const bool doubleCriticalDamage = false; // TODO: add to UI
        const bool doubleCriticalChance = false; // TODO: add to UI

        auto chance = [](int toHit)
        {
            if (toHit <= 1) // Only critical failures fail: 95% chance of hitting
                return 0.95;
            // TODO: critical hits on 19 require changing this.
            else if (toHit > 1 && toHit < 20) {
                return 1 - (0.05 * toHit);
            } else
                return 0.05;
        };

        double damage = chance(mainToHit) * mainDamage * mainApr;
        if (offHand)
            damage += chance(offToHit) * offDamage * offApr;

        if (doubleCriticalDamage) {
            damage += mainApr * mainDamage * (0.05 + 0.05 * doubleCriticalChance);
            if (offHand)
                damage += offApr * offDamage * 0.05;
        }

        series->append(ac, damage);
    }

    series->setName(c.name->text());
    chart->addSeries(series);
}
