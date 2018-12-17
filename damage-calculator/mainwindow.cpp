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
    void setupAxes();

    void updateSeriesAt(int index) {
        if (auto series = qobject_cast<QLineSeries*>(chart->series().at(index)))
            updateSeries(configurations[index], series);
    }
    void updateSeries(const Ui::configuration &c, QLineSeries* series);
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(new MainWindow::Private)
{
    for (int i = 10; i >= -20; --i)
        d->armorClasses << i;

    d->chart = new QChart;
    d->chart->setAnimationOptions(QChart::SeriesAnimations);
    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    d->tabs = new QTabWidget;

    auto widget = new QWidget;
    auto layout = new QHBoxLayout;
    layout->addWidget(d->chartView, 1);
    layout->addWidget(d->tabs, 0);
    widget->setLayout(layout);
    setCentralWidget(widget);

    auto newButton = new QPushButton(tr("New"));
    d->tabs->setCornerWidget(newButton);
    connect(newButton, &QPushButton::clicked,
            std::bind(&MainWindow::Private::newPage, d));

    d->newPage();
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
    tabs->addTab(widget, tr("Configuration %1").arg(tabs->count() + 1));
    tabs->setCurrentWidget(widget);

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


     connect(configuration.name, &QLineEdit::textChanged,
             [this, index = tabs->currentIndex()](const QString& text) {
         chart->series().at(index)->setName(text);
     });

    auto update =  std::bind(&Private::updateSeriesAt, this, tabs->currentIndex());
    for (auto child : widget->findChildren<QSpinBox*>())
        connect(child, qOverload<int>(&QSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QDoubleSpinBox*>())
        connect(child, qOverload<double>(&QDoubleSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QComboBox*>())
        connect(child, &QComboBox::currentTextChanged, update);
    connect(configuration.offHandGroup, &QGroupBox::toggled, update);

    auto setupStatsLabel = [](QSpinBox* proficiency, QSpinBox* dice, QSpinBox* sides,
                              QSpinBox* bonus, QLabel* result)
    {
        auto calculateStats = [=]() {
            const double min = proficiency->value() + bonus->value()
                             + dice->value();
            const double max = proficiency->value() + bonus->value()
                             + dice->value() * sides->value();
            const double avg = proficiency->value() + bonus->value()
                             + dice->value() * (0.5 + sides->value()/2.0);

            result->setText(tr("Min/Max/Avg: %1/%2/%3")
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

    // TODO: Decouple this, from setting the UI to setting the whole configuration.
    chart->addSeries(new QLineSeries);
    updateSeriesAt(tabs->currentIndex());
}

void MainWindow::Private::setupAxes()
{
    chart->createDefaultAxes();

    if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first())) {
        axis->setTickCount(armorClasses.count());
        axis->setLabelFormat(QLatin1String("%i"));
#if 0
        QFont font = axis->labelsFont();
        font.setPointSize(font.pointSize() - 2);
        axis->setLabelsFont(font);
#endif
        axis->setReverse(true);
    }
    if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first())) {
        axis->applyNiceNumbers();
#if 0 // Keep just in case. But the above seems to work well
        const int rounded = int(std::ceil(axis->max()));
        axis->setMax(rounded);
        axis->setTickCount(rounded+1); // Because we have to count the one at 0
#endif
        axis->setMinorTickCount(1);
        axis->setLabelFormat(QLatin1String("%d"));
    }
}



void MainWindow::Private::updateSeries(const Ui::configuration& c, QLineSeries* series)
{
    series->clear();

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

    for (const int ac : armorClasses) {
        const int mainToHit = mainThac0 - ac - mainAcModifier;
        const int offToHit  = offThac0  - ac - offAcModifier;

        const bool doubleCriticalDamage = !c.helmet->isChecked();
        const bool doubleCriticalChance = false; // TODO: add to UI

        auto chance = [](int toHit) {
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

    // TODO: Uuuuuugly workaround for the axis not updating themselves well.
    // Works smoothly with animations included, but patching QtCharts.
    chart->removeSeries(series);
    chart->addSeries(series);

    setupAxes();
}
