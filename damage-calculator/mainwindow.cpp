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
    QMenu* entriesMenu = nullptr;

    QVector<Ui::configuration> configurations;
    QVector<QVariantHash> savedConfigurations;

    void load(QWidget* tab, QVariantHash data);
    QVariantHash save(QWidget* tab);

    void loadSavedConfigurations()
    {
        QSettings settings;
        const int size = settings.beginReadArray(QLatin1String("configurations"));
        for (int index = 0; index < size; ++index) {
            settings.setArrayIndex(index);
            QVariantHash loadedData;
            for (auto entry : settings.childKeys())
                loadedData.insert(entry, settings.value(entry));
            savedConfigurations.append(loadedData);
        }
        settings.endArray();
    }

    // Due to how QSettings works, this actually saves them all, but we need to
    // ensure the current one is added to the list (or that it updates the list)
    // that are going to be saved.
    void saveCurrentConfiguration()
    {
        const QVariantHash toSave = save(tabs->currentWidget());

        // Check for duplicates, to allow changing a configuration already saved
        const QString name = toSave.value(QLatin1String("name")).toString();
        bool alreadySaved = false;
        for (int index = 0, size = savedConfigurations.size(); index < size; ++index) {
            QVariantHash& entry = savedConfigurations[index];
            const QString savedName = entry.value(QLatin1String("name")).toString();
            if (savedName == name) { // Then overwrite, and done.
                entry = toSave;
                alreadySaved = true;
                break;
            }
        }
        if (!alreadySaved)
            savedConfigurations.append(toSave);

        QSettings settings;
        settings.beginWriteArray(QLatin1String("configurations"));
        for (int index = 0, size = savedConfigurations.size(); index < size; ++index) {
            settings.setArrayIndex(index);
            const QVariantHash& entry = savedConfigurations.at(index);
            for (auto record = entry.begin(), last = entry.end(); record != last; ++record) {
                settings.setValue(record.key(), record.value());
            }
        }
        settings.endArray();

        populateEntriesMenu();
    }

    void populateEntriesMenu()
    {
        entriesMenu->clear();
        auto loadEntry = [this](int index) {
            newPage();
            load(tabs->currentWidget(), savedConfigurations.at(index));
        };

        for (int index = 0, size = savedConfigurations.size(); index < size; ++index) {
            const auto& entry = savedConfigurations.at(index);
            const QString name = entry.value(QLatin1String("name")).toString();
            entriesMenu->addAction(name, std::bind(loadEntry, index));
        }
        entriesMenu->setEnabled(savedConfigurations.size() > 0);
    }

    void newPage();
    void setupAxes();

    // TODO: previously this accepted the index as parameter, which is better,
    // but requires knowing the proper index of a series/configuration. This was
    // causing a crash when deleting tabs. But I will have to support something
    // if I want to allow moving tabs around.
    void updateSeriesAtCurrentIndex() {
        const int index = tabs->currentIndex();
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

    d->loadSavedConfigurations();

    QMenu* configurationsMenu = menuBar()->addMenu(tr("Configurations"));

    auto action = new QAction(tr("Save current"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    configurationsMenu->addAction(action);

    connect(action, &QAction::triggered, std::bind(&Private::saveCurrentConfiguration, d));

    action = new QAction(tr("Duplicate current"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    configurationsMenu->addAction(action);

    connect(action, &QAction::triggered, [this] {
        const QVariantHash saved = d->save(d->tabs->currentWidget());
        d->newPage();
        // TODO: block signals recursively, load UI values, then update chart.
        d->load(d->tabs->currentWidget(), saved);
    });

    d->entriesMenu = new QMenu(tr("Saved entries"));
    d->populateEntriesMenu();
    configurationsMenu->addMenu(d->entriesMenu);

    d->chart = new QChart;
    d->chart->setAnimationOptions(QChart::SeriesAnimations);
    auto chartView = new QChartView(d->chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    d->tabs = new QTabWidget;

    auto widget = new QWidget;
    auto layout = new QHBoxLayout;
    layout->addWidget(chartView, 1);
    layout->addWidget(d->tabs, 0);
    widget->setLayout(layout);
    setCentralWidget(widget);

    auto newButton = new QPushButton(tr("New"));
    d->tabs->setCornerWidget(newButton);
    connect(newButton, &QPushButton::clicked,
            std::bind(&MainWindow::Private::newPage, d));

    d->tabs->setTabsClosable(true);
    connect(d->tabs, &QTabWidget::tabCloseRequested, [this](int index) {
        if (d->tabs->count() == 1)
            return; // don't close the last one for now, to keep the "New" button
        d->configurations.removeAt(index);
        d->chart->removeSeries(d->chart->series().at(index));
        delete d->tabs->widget(index);
    });

    d->newPage();
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}

void MainWindow::Private::load(QWidget *tab, QVariantHash data)
{
    for (auto child : tab->findChildren<QWidget*>()) {
        if (qobject_cast<QLabel*>(child))
            continue;
        else if (auto spinbox1 = qobject_cast<QSpinBox*>(child)) {
            spinbox1->setValue(data.take(child->objectName()).toInt());
        } else if (auto spinbox2 = qobject_cast<QDoubleSpinBox*>(child)) {
            spinbox2->setValue(data.take(child->objectName()).toDouble());
        }
        else if (auto combobox = qobject_cast<QComboBox*>(child)) {
            combobox->setCurrentIndex(data.take(child->objectName()).toInt());
        }
        else if (auto checkbox = qobject_cast<QCheckBox*>(child)) {
            checkbox->setChecked(data.take(child->objectName()).toBool());
        }
        else if (auto line = qobject_cast<QLineEdit*>(child)) {
            if (child->objectName() == QLatin1String("name"))
                line->setText(data.take(child->objectName()).toString());
        }
        else if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            if (groupbox->isCheckable())
                groupbox->setChecked(data.take(child->objectName()).toBool());
        }
    }

    if (!data.isEmpty())
        qWarning() << "This data was not loaded:\n" << data;
}

QVariantHash MainWindow::Private::save(QWidget *tab)
{
    QVariantHash result;
    for (auto child : tab->findChildren<QWidget*>()) {
        if (qobject_cast<QLabel*>(child) || child->objectName().isEmpty()
                || child->objectName().startsWith(QLatin1String("qt_")))
            continue;
        else if (auto spinbox1 = qobject_cast<QSpinBox*>(child))
            result.insert(child->objectName(), spinbox1->value());
        else if (auto spinbox2 = qobject_cast<QDoubleSpinBox*>(child))
            result.insert(child->objectName(), spinbox2->value());
        else if (auto combobox = qobject_cast<QComboBox*>(child))
            result.insert(child->objectName(), combobox->currentIndex());
        else if (auto checkbox = qobject_cast<QCheckBox*>(child))
            result.insert(child->objectName(), checkbox->isChecked());
        else if (auto line = qobject_cast<QLineEdit*>(child)) {
            if (line->objectName() == QLatin1String("name"))
                result.insert(line->objectName(), line->text());
        }
        else if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            if (groupbox->isCheckable())
                result.insert(child->objectName(), groupbox->isChecked());
        }
        else
            qWarning() << "Not serialized:" << child;
    }

    return result;
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
             [this](const QString& text) {
         chart->series().at(tabs->currentIndex())->setName(text);
     });

    // Make this connections BEFORE the ones that update the chart! Since the
    // calculations of the chart rely on the value set here.
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

    auto update =  std::bind(&Private::updateSeriesAtCurrentIndex, this);
    for (auto child : widget->findChildren<QSpinBox*>())
        connect(child, qOverload<int>(&QSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QDoubleSpinBox*>())
        connect(child, qOverload<double>(&QDoubleSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QComboBox*>())
        connect(child, &QComboBox::currentTextChanged, update);
    for (auto child : widget->findChildren<QCheckBox*>())
        connect(child, &QCheckBox::toggled, update);
    connect(configuration.offHandGroup, &QGroupBox::toggled, update);

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
    updateSeriesAtCurrentIndex();
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

#if 0
    // TODO: Uuuuuugly workaround for the axis not updating themselves well.
    // Works smoothly with animations included, but patching QtCharts.
    chart->removeSeries(series);
    chart->addSeries(series);
#endif

    setupAxes();
}
