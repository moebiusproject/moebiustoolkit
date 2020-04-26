#include "damagecalculatorpage.h"

// TODO: rename this forms, classes, etc, to something specific for this page.
#include "ui_configuration.h"
#include "ui_enemy.h"

// TODO: make their own pages.
// #include "attackbonuses.h"
// #include "rollprobabilities.h"

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

class ManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ManageDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setLayout(new QVBoxLayout);
        layout()->addWidget(&m_table);
        m_table.setColumnCount(2);
        // Ideas for changing style.
//        m_table.setShowGrid(false);
//        m_table.setFrameStyle(QFrame::NoFrame);
        m_table.setSelectionMode(QAbstractItemView::NoSelection);
        m_table.setFocusPolicy(Qt::NoFocus);
        m_table.verticalHeader()->hide();
        m_table.horizontalHeader()->hide();
    }

    void setNames(const QStringList& names)
    {
        m_table.clear();
        m_table.setRowCount(names.size());
        // TODO: see QTableWidget::setItem on sorting
        m_table.setSortingEnabled(false);
        for (int i = 0, size = names.size(); i < size; ++i) {
            m_table.setItem(i, 0, new QTableWidgetItem(names[i]));
            auto button = new QPushButton(tr("Show"));
            connect(button, &QPushButton::clicked, this, [this, i]() {
                emit showClicked(i);
            });
            m_table.setCellWidget(i, 1, button);
        }
        m_table.setSortingEnabled(true);
        m_table.resizeColumnsToContents();
        m_table.setVerticalHeaderLabels(QStringList());
    }

signals:
    void showClicked(int i);

private:
    QTableWidget m_table;
};

struct DamageCalculatorPage::Private
{
    Private(DamageCalculatorPage& window)
        : q(window)
    {}
    DamageCalculatorPage& q;
    QVector<int> armorClasses;

    QMenu* fileMenu = nullptr;
    QMenu* mainMenu = nullptr;
    QMenu* loadSavedMenu = nullptr;
    QMenu* deleteSavedMenu = nullptr;

    ManageDialog* manageDialog = nullptr;

    QChart* chart = nullptr;
    QChartView* chartView = nullptr;
    QSpinBox* minimumX = nullptr;
    QSpinBox* maximumX = nullptr;
    QCheckBox* pointLabels = nullptr;
    QCheckBox* pointLabelsClipping = nullptr;
    QCheckBox* reverse = nullptr;

    Ui::Enemy enemy;

    QTabWidget* tabs = nullptr;

    QVector<Ui::configuration> configurations;
    QVector<QLineSeries*> lineSeries;
    QVector<QVariantHash> savedConfigurations;

    // TODO: move to their own location, to make them testable.
    static void deserialize(QWidget* tab, QVariantHash data);
    static QVariantHash serialize(QWidget* tab);

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
        const QVariantHash toSave = serialize(tabs->currentWidget());

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

        saveConfigurationsToDisk();
        populateEntriesMenu();
    }

    void saveConfigurationsToDisk()
    {
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
    }

    void saveConfigurationsToFile(const QString& fileName)
    {
        QJsonArray array;
        for (int index = 0, last = tabs->count(); index < last; ++index) {
            const QVariantHash toSave = serialize(tabs->widget(index));
            array.append(QJsonValue::fromVariant(toSave));
        }
        QJsonDocument document(array);
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(document.toJson(QJsonDocument::Indented));
    }

    void loadConfigurationsFromFile(const QString& fileName)
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (!document.isArray()) {
            q.statusBar()->showMessage(tr("The file can not be loaded"));
            qWarning() << "Cannot load JSON: root not an array";
            return;
        }
        const QJsonArray array = document.array();
        for (int index = 0, last = array.count(); index < last; ++index) {
            const QJsonValue value = array.at(index);
            if (!value.isObject()) {
                q.statusBar()->showMessage(tr("The file can not be loaded"));
                qWarning() << "Cannot load JSON: element is not an object";
                continue;
            }
            newPage();
            deserialize(tabs->currentWidget(), value.toObject().toVariantHash());
        }
    }

    void populateEntriesMenu()
    {
        loadSavedMenu->clear();
        deleteSavedMenu->clear();
        auto loadEntry = [this](int index) {
            newPage();
            deserialize(tabs->currentWidget(), savedConfigurations.at(index));
        };
        auto deleteEntry = [this](int index) {
            savedConfigurations.remove(index);
            saveConfigurationsToDisk();
            populateEntriesMenu();
        };

        for (int index = 0, size = savedConfigurations.size(); index < size; ++index) {
            const auto& entry = savedConfigurations.at(index);
            const QString name = entry.value(QLatin1String("name")).toString();
            loadSavedMenu->addAction(name, std::bind(loadEntry, index));
            deleteSavedMenu->addAction(name, std::bind(deleteEntry, index));
        }
        loadSavedMenu->setEnabled(savedConfigurations.size() > 0);
        deleteSavedMenu->setEnabled(savedConfigurations.size() > 0);
    }

    void newPage();
    void setupAxes();

    static auto weaponDamage(QSpinBox* proficiency, QSpinBox* dices,
                             QSpinBox* sides,       QSpinBox* bonus)
    {
        const int    min = proficiency->value() + bonus->value()
                         + dices->value();
        const int    max = proficiency->value() + bonus->value()
                         + dices->value() * sides->value();
        const double avg = proficiency->value() + bonus->value()
                         + dices->value() * (0.5 + sides->value()/2.0);
        return std::make_tuple(min, avg, max);
    }

    void updateAllSeries() {
        for (int index = 0; index < tabs->count(); ++index) {
            updateSeries(configurations[index], lineSeries[index]);
        }
    }
    // TODO: previously this accepted the index as parameter, which is better,
    // but requires knowing the proper index of a series/configuration. This was
    // causing a crash when deleting tabs. But I will have to support something
    // if I want to allow moving tabs around.
    void updateSeriesAtCurrentIndex() {
        const int index = tabs->currentIndex();
        updateSeries(configurations[index], lineSeries[index]);
    }
    void updateSeries(const Ui::configuration& c, QLineSeries* series);
};

DamageCalculatorPage::DamageCalculatorPage(QWidget* parent)
    : QWidget(parent)
    , d(new Private(*this))
{
    for (int i = 10; i >= -20; --i)
        d->armorClasses << i;

    d->loadSavedConfigurations();

    // Menus ///////////////////////////////////////////////////////////////////
    d->fileMenu = menuBar()->addMenu(tr("File"));

    auto action = new QAction(tr("Save visible configurations as..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    d->fileMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        auto dialog = new QFileDialog(this);
        dialog->setFileMode(QFileDialog::AnyFile);
        connect(dialog, &QFileDialog::fileSelected, this,
            std::bind(&Private::saveConfigurationsToFile, d, std::placeholders::_1));
        dialog->show();
    });

    action = new QAction(tr("Load configurations from..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    d->fileMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        auto dialog = new QFileDialog(this);
        dialog->setFileMode(QFileDialog::AnyFile);
        connect(dialog, &QFileDialog::fileSelected, this,
            std::bind(&Private::loadConfigurationsFromFile, d, std::placeholders::_1));
        dialog->show();
    });

    action = new QAction(tr("Copy chart to clipboard"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
    d->fileMenu->addAction(action);

    connect(action, &QAction::triggered, [this] {
        const QPixmap pixmap = d->chartView->grab();
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setPixmap(pixmap);
    });

    action = new QAction(tr("Save chart as..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    d->fileMenu->addAction(action);

    connect(action, &QAction::triggered, [this] {
        const QPixmap pixmap = d->chartView->grab();
        auto dialog = new QFileDialog(this);
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        connect(dialog, &QFileDialog::fileSelected, [pixmap](const QString& name) {
            pixmap.save(name, "png");
        });
        dialog->open();
    });

    d->mainMenu = menuBar()->addMenu(tr("Damage calculator configurations"));

    action = new QAction(tr("Duplicate current configuration"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        const QVariantHash saved = d->serialize(d->tabs->currentWidget());
        d->newPage();
        // TODO: block signals recursively, load UI values, then update chart.
        d->deserialize(d->tabs->currentWidget(), saved);
    });

    action = new QAction(tr("Save current configuration to preferences"), this);
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, std::bind(&Private::saveCurrentConfiguration, d));

    d->loadSavedMenu = new QMenu(tr("Load configuration from preferences"));
    d->mainMenu->addMenu(d->loadSavedMenu);
    d->deleteSavedMenu = new QMenu(tr("Delete configuration from preferences"));
    d->mainMenu->addMenu(d->deleteSavedMenu);
    d->populateEntriesMenu();

    action = new QAction(tr("Manage entries"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    d->mainMenu->addAction(action);
    d->mainMenu->setEnabled(false);

    d->manageDialog = new ManageDialog(this);
    // TODO: pick something reasonable, yet not hardcoded?
    d->manageDialog->setMinimumWidth(600);
    connect(action, &QAction::triggered, [this] {
        QStringList names;
        for (const auto& entry : qAsConst(d->savedConfigurations)) {
            names << entry.value(QLatin1String("name")).toString();
        }
        d->manageDialog->setNames(names);
        d->manageDialog->show();
    });
    connect(d->manageDialog, &ManageDialog::showClicked, this, [this](int index) {
        d->newPage();
        d->deserialize(d->tabs->currentWidget(), d->savedConfigurations.at(index));
    });

#if 0 // TODO: migrate to its own page
    QMenu* extrasMenu = menuBar()->addMenu(tr("Extras"));
    action = new QAction(tr("Attack bonuses"), this);
    extrasMenu->addAction(action);

    connect(action, &QAction::triggered, this, [this] {
        auto dialog = new AttackBonuses(this);
        dialog->showMaximized();
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    });

    action = new QAction(tr("Repeated roll probability"), this);
    extrasMenu->addAction(action);

    connect(action, &QAction::triggered, this, [this] {
        auto dialog = new RollProbabilities(this);
        dialog->showMaximized();
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    });
#endif

    // Chart controls //////////////////////////////////////////////////////////
    auto chartControlsLayout = new QHBoxLayout;
    chartControlsLayout->addWidget(new QLabel(tr("Best AC:")));
    d->minimumX = new QSpinBox;
    d->minimumX->setMinimum(d->armorClasses.last());
    d->minimumX->setMaximum(d->armorClasses.first());
    d->minimumX->setValue(d->minimumX->minimum());
    chartControlsLayout->addWidget(d->minimumX);
    chartControlsLayout->addWidget(new QLabel(tr("Worst AC:")));
    d->maximumX = new QSpinBox;
    d->maximumX->setMinimum(d->armorClasses.last());
    d->maximumX->setMaximum(d->armorClasses.first());
    d->maximumX->setValue(d->maximumX->maximum());
    chartControlsLayout->addWidget(d->maximumX);
    // FIXME: improve min/max spinbox values (to disallow crossed values).
    connect(d->minimumX, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->maximumX, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    d->pointLabels = new QCheckBox(tr("Show point labels"));
    chartControlsLayout->addWidget(d->pointLabels);
    d->pointLabels->setChecked(true);
    connect(d->pointLabels, &QCheckBox::toggled, [this](bool value) {
        for (auto series : d->chart->series()) {
            if (auto line = qobject_cast<QLineSeries*>(series)) {
                line->setPointLabelsVisible(value);
            }
        }
    });
    d->pointLabelsClipping = new QCheckBox(tr("Clip point labels"));
    chartControlsLayout->addWidget(d->pointLabelsClipping);
    d->pointLabelsClipping->setChecked(false);
    connect(d->pointLabelsClipping, &QCheckBox::toggled, [this](bool value) {
        for (auto series : d->chart->series()) {
            if (auto line = qobject_cast<QLineSeries*>(series)) {
                line->setPointLabelsClipping(value);
            }
        }
    });
    d->reverse = new QCheckBox(tr("AC: worst to best"));
    chartControlsLayout->addWidget(d->reverse);
    d->reverse->setChecked(true);
    connect(d->reverse, &QCheckBox::toggled, [this](bool value) {
        // FIXME: assumption on QLineSeries
        if (auto axis = qobject_cast<QValueAxis*>(d->chart->axes(Qt::Horizontal).first())) {
            axis->setReverse(value);
        }
    });

    // The chart itself ////////////////////////////////////////////////////////
    d->chart = new QChart;
    d->chart->setAnimationOptions(QChart::SeriesAnimations);
    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    // The chart layout grouping the two ///////////////////////////////////////
    auto chartViewLayout = new QVBoxLayout;
    chartViewLayout->addLayout(chartControlsLayout);
    chartViewLayout->addWidget(d->chartView);

    // The common enemy controls ///////////////////////////////////////////////
    auto enemyControls = new QWidget;
    d->enemy.setupUi(enemyControls);
    d->enemy.armor->addItem(tr("Neutral"),
                            QVariant::fromValue(ArmorModifiers(+0, +0, +0, +0)));
    d->enemy.armor->addItem(tr("Leather"),
                            QVariant::fromValue(ArmorModifiers(+0, +0, +2, +2)));
    d->enemy.armor->addItem(tr("Studded Leather"),
                            QVariant::fromValue(ArmorModifiers(+0, -1, -1, -2)));
    d->enemy.armor->addItem(tr("Chain Mail"),
                            QVariant::fromValue(ArmorModifiers(+2, +0, +0, -2)));
    d->enemy.armor->addItem(tr("Splint Mail"),
                            QVariant::fromValue(ArmorModifiers(-2, -1, -1, +0)));
    d->enemy.armor->addItem(tr("Plate Mail"),
                            QVariant::fromValue(ArmorModifiers(+0, +0, +0, -3)));
    d->enemy.armor->addItem(tr("Full Plate Mail"),
                            QVariant::fromValue(ArmorModifiers(+0, -3, -3, -4)));
    // TODO: add more armors, or modifiers from creatures.

    connect(d->enemy.armor, &QComboBox::currentTextChanged,
            [choice = d->enemy.armor, this] {
        const auto modifiers = choice->currentData().value<ArmorModifiers>();
        d->enemy.crushingModifier->setValue(modifiers.crushing);
        d->enemy.missileModifier ->setValue(modifiers.missile);
        d->enemy.piercingModifier->setValue(modifiers.piercing);
        d->enemy.slashingModifier->setValue(modifiers.slashing);
    });
    connect(d->enemy.crushingModifier, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.missileModifier, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.piercingModifier, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.slashingModifier, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.crushingResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.missileResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.piercingResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.slashingResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.helmet, &QCheckBox::toggled, std::bind(&Private::updateAllSeries, d));

    // Tab widget with configurations //////////////////////////////////////////
    d->tabs = new QTabWidget;
    auto newButton = new QPushButton(tr("New"));
    d->tabs->setCornerWidget(newButton);
    connect(newButton, &QPushButton::clicked,
            std::bind(&Private::newPage, d));

    d->tabs->setTabsClosable(true);
    connect(d->tabs, &QTabWidget::tabCloseRequested, [this](int index) {
        if (d->tabs->count() == 1)
            return; // don't close the last one for now, to keep the "New" button
        d->configurations.removeAt(index);
        d->chart->removeSeries(d->lineSeries[index]);
        delete d->lineSeries.takeAt(index);
        // TODO: with the new approach, this might be a tad heavy. Review.
        d->setupAxes();
        delete d->tabs->widget(index);
        for (int tab = index ; tab < d->tabs->count(); ++tab)
            d->tabs->setTabText(tab, tr("Configuration %1").arg(tab + 1));
    });

    // Layout grouping the configurations and the enemy controls ///////////////
    auto inputWidget = new QWidget;
    auto inputLayout = new QVBoxLayout(inputWidget);
    inputWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    auto inputArea = new QScrollArea;
    inputArea->setWidget(inputWidget);
    // TODO: Better looking when no scrollbar only... re-consider it.
    // inputArea->setFrameShape(QFrame::NoFrame);
    inputArea->setWidgetResizable(true);
    inputLayout->addWidget(enemyControls);
    inputLayout->addWidget(d->tabs);

    // Now group everything together ///////////////////////////////////////////
    auto layout = new QHBoxLayout;
    setLayout(layout);
    layout->addLayout(chartViewLayout, 1);
    layout->addWidget(inputArea, 0);
    statusBar()->showMessage(tr("Hover the lines to see the value"));

    d->newPage();
}

DamageCalculatorPage::~DamageCalculatorPage()
{
    delete d;
    d = nullptr;
}

bool DamageCalculatorPage::event(QEvent* event)
{
    // FIXME: Doesn't work... debug why.
    if (event->type() == QEvent::Hide) {
        d->fileMenu->menuAction()->setVisible(false);
        d->mainMenu->menuAction()->setVisible(false);
    }
    if (event->type() == QEvent::Show) {
        d->fileMenu->menuAction()->setVisible(true);
        d->mainMenu->menuAction()->setVisible(true);
    }

    return QWidget::event(event);
}

// TODO: REMOVE this porting workaround.
QMenuBar* DamageCalculatorPage::menuBar()
{
    QWidget* widget = this;
    while ((widget = widget->parentWidget())) {
        if (QMainWindow* window = qobject_cast<QMainWindow*>(widget))
            return window->menuBar();
    }
    qFatal("Should not happen...");
    return nullptr;
}

QStatusBar* DamageCalculatorPage::statusBar()
{
    QWidget* widget = this;
    while ((widget = widget->parentWidget())) {
        if (QMainWindow* window = qobject_cast<QMainWindow*>(widget))
            return window->statusBar();
    }
    qFatal("Should not happen...");
    return nullptr;
}

void DamageCalculatorPage::Private::deserialize(QWidget* root, QVariantHash data)
{
    // TODO: remove for any kind of release. Is just a backwards compatibility
    // addition to ease my testing with the old configuration.
    if (data.contains(QLatin1String("doubleCriticalChance"))) {
        if (data.take(QLatin1String("doubleCriticalChance")).toBool()) {
            if (QSpinBox* box = root->findChild<QSpinBox*>(QLatin1String("criticalHitChance1"))) {
                box->setValue(10);
            }
        }
    }
    // TODO: ditto. Inject some values for the new features not in the old saves.
    if (!data.contains(QLatin1String("criticalHitChance1")))
        data.insert(QLatin1String("criticalHitChance1"), 5);
    if (!data.contains(QLatin1String("criticalHitChance2")))
        data.insert(QLatin1String("criticalHitChance2"), 5);
    if (!data.contains(QLatin1String("criticalStrike")))
        data.insert(QLatin1String("criticalStrike"), false);
    if (!data.contains(QLatin1String("maximumDamage")))
        data.insert(QLatin1String("maximumDamage"), false);

    for (auto child : root->findChildren<QWidget*>()) {
        if (qobject_cast<QLabel*>(child))
            continue;
        if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            if (!groupbox->isCheckable())
                continue;
        }

        const QString name = child->objectName();
        if (name.startsWith(QLatin1String("qt_")))
            continue;

        const QVariant value = data.take(name);
        if (!value.isValid()) {
            qInfo() << child << "not loaded with serialized data";
            continue;
        }
        if (auto spinbox1 = qobject_cast<QSpinBox*>(child)) {
            spinbox1->setValue(value.toInt());
        } else if (auto spinbox2 = qobject_cast<QDoubleSpinBox*>(child)) {
            spinbox2->setValue(value.toDouble());
        }
        else if (auto combobox = qobject_cast<QComboBox*>(child)) {
            combobox->setCurrentIndex(value.toInt());
        }
        else if (auto checkbox = qobject_cast<QCheckBox*>(child)) {
            checkbox->setChecked(value.toBool());
        }
        else if (auto line = qobject_cast<QLineEdit*>(child)) {
            // Skip several "fake" line edits used in spinboxes, etc.
            if (name == QLatin1String("name"))
                line->setText(value.toString());
        }
        else if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            groupbox->setChecked(value.toBool());
        }
    }

    if (!data.isEmpty())
        qWarning() << "This data was not loaded:\n" << data;
}

QVariantHash DamageCalculatorPage::Private::serialize(QWidget* root)
{
    QVariantHash result;
    for (auto child : root->findChildren<QWidget*>()) {
        const QString name = child->objectName();
        if (qobject_cast<QLabel*>(child) || name.isEmpty()
                || name.startsWith(QLatin1String("qt_")))
            continue;
        else if (auto spinbox1 = qobject_cast<QSpinBox*>(child))
            result.insert(name, spinbox1->value());
        else if (auto spinbox2 = qobject_cast<QDoubleSpinBox*>(child))
            result.insert(name, spinbox2->value());
        else if (auto combobox = qobject_cast<QComboBox*>(child))
            result.insert(name, combobox->currentIndex());
        else if (auto checkbox = qobject_cast<QCheckBox*>(child))
            result.insert(name, checkbox->isChecked());
        else if (auto line = qobject_cast<QLineEdit*>(child)) {
            if (name == QLatin1String("name"))
                result.insert(name, line->text());
        }
        else if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            if (groupbox->isCheckable())
                result.insert(name, groupbox->isChecked());
        }
        else
            qWarning() << "Not serialized:" << child;
    }

    return result;
}

void DamageCalculatorPage::Private::newPage()
{
    auto widget = new QWidget;

    configurations.append(Ui::configuration());
    Ui::configuration& configuration = configurations.last();
    configuration.setupUi(widget);
    tabs->addTab(widget, tr("Configuration %1").arg(tabs->count() + 1));
    tabs->setCurrentWidget(widget);

    connect(configuration.name, &QLineEdit::textChanged,
            [this](const QString& text) {
        lineSeries.at(tabs->currentIndex())->setName(text);
    });

    auto setupStatsLabel = [](QSpinBox* proficiency, QSpinBox* dice, QSpinBox* sides,
                              QSpinBox* bonus, QLabel* result)
    {
        auto calculateStats = [=]() {
            auto damage = Private::weaponDamage(proficiency, dice, sides, bonus);
            result->setText(tr("Min/Avg/Max: %1/%2/%3")
                            .arg(std::get<0>(damage))
                            .arg(std::get<1>(damage), 0, 'f', 1)
                            .arg(std::get<2>(damage)));
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

    auto update = std::bind(&Private::updateSeriesAtCurrentIndex, this);
    for (auto child : widget->findChildren<QSpinBox*>())
        connect(child, qOverload<int>(&QSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QDoubleSpinBox*>())
        connect(child, qOverload<double>(&QDoubleSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QComboBox*>())
        connect(child, &QComboBox::currentTextChanged, update);
    for (auto child : widget->findChildren<QCheckBox*>())
        connect(child, &QCheckBox::toggled, update);
    connect(configuration.offHandGroup, &QGroupBox::toggled, update);

    // TODO: Decouple this, from setting the UI to setting the whole configuration.
    auto series = new QLineSeries;
    lineSeries.append(series);
    series->setPointsVisible(true);
    series->setPointLabelsVisible(pointLabels->isChecked());
    series->setPointLabelsClipping(pointLabelsClipping->isChecked());
    series->setPointLabelsFormat(QLatin1String("@yPoint"));
    auto closestSeriesPoint = [](const QVector<QPointF>& realPoints,
                                 const QPointF& domainPoint)
    {
        QPointF closest;
        qreal minimum = qInf();
        for (const auto& point : realPoints) {
            const QPointF delta = domainPoint - point;
            const qreal distance = qAbs(delta.manhattanLength());
            if (distance < minimum) {
                minimum = distance;
                closest = point;
            }
        }
        return closest;
    };
    connect(series, &QLineSeries::hovered,
            [this, series, closestSeriesPoint](QPointF point, bool over) {
        point = closestSeriesPoint(series->pointsVector(), point);
        if (over)
            q.statusBar()->showMessage(tr("%1. Damage: %2").arg(series->name()).arg(point.y()), 2000);
    });
    chart->addSeries(series);
    QLegendMarker* marker = chart->legend()->markers(series).first();
    connect(marker, &QLegendMarker::clicked, chart, [series, marker] {
        series->setVisible(!series->isVisible());
        marker->setVisible(true);
        QFont font = marker->font();
        font.setStrikeOut(!series->isVisible());
        marker->setFont(font);
    });

    updateSeriesAtCurrentIndex();
}

void DamageCalculatorPage::Private::setupAxes()
{
    if (chart->axes().size() == 0)
        chart->createDefaultAxes();

    if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first())) {
        axis->setReverse(reverse->isChecked());
        axis->setMin(minimumX->value());
        axis->setMax(maximumX->value());
        axis->setTickCount(maximumX->value() - minimumX->value() + 1);

        axis->setLabelFormat(QLatin1String("%i"));
#if 0
        QFont font = axis->labelsFont();
        font.setPointSize(font.pointSize() - 2);
        axis->setLabelsFont(font);
#endif
    }
    // FIXME: assumption on QLineSeries
    if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first())) {
        double min = +qInf();
        double max = -qInf();
        for (auto series : lineSeries) {
            const QVector<QPointF> points = series->pointsVector();
            for (auto point : points) {
                if (point.x() > maximumX->value() || point.x() < minimumX->value())
                    continue;
                min = qMin(point.y(), min);
                max = qMax(point.y(), max);
            }
        }
        axis->setRange(min, max);
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



void DamageCalculatorPage::Private::updateSeries(const Ui::configuration& c, QLineSeries* series)
{
    const bool offHand = c.offHandGroup->isChecked();

    const int thac0 = c.baseThac0->value() - c.strengthThac0Bonus->value() - c.classThac0Bonus->value();
    const int mainThac0 = thac0 - c.proficiencyThac0Modifier1->value() - c.styleModifier1->value() - c.weaponThac0Modifier1->value();
    const int offThac0  = thac0 - c.proficiencyThac0Modifier2->value() - c.styleModifier2->value() - c.weaponThac0Modifier2->value();

    // TODO: brittle approach. Relies on the order set on the UI. Set user data instead.
    auto modifierFromUi = [this](QComboBox* box) {
        return ( box->currentIndex() == 0 ? enemy.crushingModifier->value()
               : box->currentIndex() == 1 ? enemy.missileModifier->value()
               : box->currentIndex() == 2 ? enemy.piercingModifier->value()
                                          : enemy.slashingModifier->value() );
    };
    auto resistanceFromUi = [this](QComboBox* box) {
        return ( box->currentIndex() == 0 ? enemy.crushingResistance->value()
               : box->currentIndex() == 1 ? enemy.missileResistance->value()
               : box->currentIndex() == 2 ? enemy.piercingResistance->value()
                                          : enemy.slashingResistance->value() );
    };

    const int mainAcModifier = modifierFromUi(c.damageType1);
    const int offAcModifier  = modifierFromUi(c.damageType2);

    const double mainResistance = (100.0 - resistanceFromUi(c.damageType1)) / 100.0;
    const double offResistance  = (100.0 - resistanceFromUi(c.damageType2)) / 100.0;

    const bool maximumDamage = c.maximumDamage->isChecked();
    const bool criticalStrike = c.criticalStrike->isChecked();
    const auto weapon1 = Private::weaponDamage(c.proficiencyDamageModifier1,
                                               c.weaponDamageDiceNumber1,
                                               c.weaponDamageDiceSide1,
                                               c.weaponDamageDiceBonus1);
    const auto weapon2 = Private::weaponDamage(c.proficiencyDamageModifier2,
                                               c.weaponDamageDiceNumber2,
                                               c.weaponDamageDiceSide2,
                                               c.weaponDamageDiceBonus2);
    const double mainDamage = (maximumDamage ? std::get<2>(weapon1) : std::get<1>(weapon1))
                            + c.strengthDamageBonus->value()
                            + c.classDamageBonus->value();
    const double offDamage  = (maximumDamage ? std::get<2>(weapon2) : std::get<1>(weapon2))
                            + c.strengthDamageBonus->value()
                            + c.classDamageBonus->value();

    const double mainApr = c.attacksPerRound1->value();
    const int    offApr  = c.attacksPerRound2->value();

    QVector<QPointF> points;
    for (const int ac : armorClasses) {
        const int mainToHit = mainThac0 - ac - mainAcModifier;
        const int offToHit  = offThac0  - ac - offAcModifier;

        const bool doubleCriticalDamage = !enemy.helmet->isChecked();
        const auto criticalHitChance1 = c.criticalHitChance1->value() / 100.0;
        const auto criticalHitChance2 = c.criticalHitChance2->value() / 100.0;

        // TODO: move this to a unit testable place, remove criticalChance
        // capture and rework to support 100% criical hit chance.
        auto chanceToHit = [criticalStrike](int toHit, double criticalChance) {
            if (criticalStrike)
                return 1.0;
            // FIXME: this doesn't really support a critical chance of 100%
            // because it evolved from when only 5 or 10% were allowed. Use
            // critical strike for 100%.
            const int firstCriticalRoll = (21 - criticalChance*20);
            // Equivalent to:
            // const int firstCriticalRoll = (105 - criticalChance*100) / 5;
            if (toHit <= 2) // Only critical failures fail: 95% chance of hitting
                return 0.95;
            else if (toHit > 2 && toHit < firstCriticalRoll) {
                return 1 - (0.05 * (toHit-1));
            } else // Only critical hits work.
                return criticalChance;
        };

        double damage = chanceToHit(mainToHit, criticalHitChance1) * mainDamage * mainApr * mainResistance;
        if (offHand)
            damage += chanceToHit(offToHit, criticalHitChance2) * offDamage * offApr * offResistance;

        if (doubleCriticalDamage) { // Unprotected against critical hits.
            if (criticalStrike) // All damage doubled!
                damage *= 2;
            else { // Then 1 of each 20, or 2 of each 20 do extra damage.
                damage += mainApr * mainDamage * criticalHitChance1 * mainResistance;
                if (offHand)
                    damage += offApr * offDamage * criticalHitChance2 * offResistance;
            }
        }

        points.append(QPointF(ac, damage));
    }
    series->replace(points);

    setupAxes();

    if (series->attachedAxes().size() == 0) {
        // FIXME: assumption on QLineSeries
        if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first()))
            series->attachAxis(axis);
        if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first()))
            series->attachAxis(axis);
    }
}

#include "damagecalculatorpage.moc"
