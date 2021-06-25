/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2018-2021 Alejandro Exojo Piqueras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "damagecalculatorpage.h"

// TODO: rename this forms, classes, etc, to something specific for this page.
#include "ui_damagecalculationwidget.h"
#include "ui_enemy.h"
#include "ui_weaponarrangementwidget.h"

#include "calculators.h"
#include "diceroll.h"

// TODO: make their own pages.
// #include "attackbonuses.h"
// #include "rollprobabilities.h"

#include <QBuffer>
#include <QChartView>
#include <QClipboard>
#include <QColorDialog>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLegendMarker>
#include <QLineSeries>
#include <QMenu>
#include <QMenuBar>
#include <QScrollBar>
#include <QSettings>
#include <QStatusBar>
#include <QTableWidget>
#include <QValueAxis>

#define TOML_EXCEPTIONS 0
#define TOML_IMPLEMENTATION
#include "tomlplusplus/include/toml++/toml.h"

#include <functional>
#include <iostream>
#include <numeric>

using namespace Calculators;
using namespace QtCharts;

static const auto keyDamageCalculations = QStringLiteral("DamageCalculations");

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


// Widgets /////////////////////////////////////////////////////////////////////

struct Enemy : public Ui::Enemy
{
    int acModifier(DamageType type) const
    {
        return ( type == DamageType::Crushing ? crushingModifier->value()
               : type == DamageType::Missile  ? missileModifier->value()
               : type == DamageType::Piercing ? piercingModifier->value()
               : type == DamageType::Slashing ? slashingModifier->value()
               : 0 );
    }

    double resistanceFactor(DamageType type) const
    {
        return resistanceValue(type) / 100.0;
    }

    double resistanceValue(DamageType type) const
    {
        return type == DamageType::Crushing ? crushingResistance->value()
             : type == DamageType::Missile  ? missileResistance->value()
             : type == DamageType::Piercing ? piercingResistance->value()
             : type == DamageType::Slashing ? slashingResistance->value()

             : type == DamageType::Acid        ? acidResistance->value()
             : type == DamageType::Cold        ? coldResistance->value()
             : type == DamageType::Electricity ? electricityResistance->value()
             : type == DamageType::Fire        ? fireResistance->value()
             : 0;
    }
};

struct Calculation : public Ui::calculation
{
    void setupUi(QWidget* widget)
    {
        Ui::calculation::setupUi(widget);
        // Change the two weapons for the different limits of the two weapons.
        weapon1->ui->attacksPerRound2->hide();
        weapon2->ui->attacksPerRound1->hide();
        weapon2->ui->styleModifier->setMinimum(-8);
    }
};

// Dialogs /////////////////////////////////////////////////////////////////////

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


// Private class ///////////////////////////////////////////////////////////////

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

    QLineEdit* titleLine = nullptr;
    QSpinBox* minimumX = nullptr;
    QSpinBox* maximumX = nullptr;
    QCheckBox* pointLabels = nullptr;
    QCheckBox* pointLabelsClipping = nullptr;
    QCheckBox* axisTitle = nullptr;
    QCheckBox* reverse = nullptr;

    Enemy enemy;

    QTabWidget* tabs = nullptr;

    QVector<Calculation> calculations;
    QVector<QLineSeries*> lineSeries;
    QVector<QVariantHash> savedCalculations;

    // TODO: move to their own location, to make them testable.
    static void deserialize(QWidget* root, QVariantHash data);
    static QVariantHash serialize(QWidget* root);

    void loadSavedCalculations()
    {
        QSettings settings;
        const int size = settings.beginReadArray(keyDamageCalculations);
        for (int index = 0; index < size; ++index) {
            settings.setArrayIndex(index);
            QVariantHash loadedData;
            for (const QString& entry : settings.childKeys())
                loadedData.insert(entry, settings.value(entry));
            savedCalculations.append(loadedData);
        }
        settings.endArray();
    }

    // Due to how QSettings works, this actually saves them all, but we need to
    // ensure the current one is added to the list (or that it updates the list)
    // that are going to be saved.
    void saveCurrentCalculation()
    {
        const QVariantHash toSave = serialize(tabs->currentWidget());

        // Check for duplicates, to allow changing a calculation already saved
        const QString name = toSave.value(QLatin1String("name")).toString();
        bool alreadySaved = false;
        for (int index = 0, size = savedCalculations.size(); index < size; ++index) {
            QVariantHash& entry = savedCalculations[index];
            const QString savedName = entry.value(QLatin1String("name")).toString();
            if (savedName == name) { // Then overwrite, and done.
                entry = toSave;
                alreadySaved = true;
                break;
            }
        }
        if (!alreadySaved)
            savedCalculations.append(toSave);

        saveCalculationsToDisk();
        populateEntriesMenu();
    }

    void saveCalculationsToDisk()
    {
        QSettings settings;
        settings.beginWriteArray(keyDamageCalculations);
        for (int index = 0, size = savedCalculations.size(); index < size; ++index) {
            settings.setArrayIndex(index);
            const QVariantHash& entry = savedCalculations.at(index);
            for (auto record = entry.begin(), last = entry.end(); record != last; ++record) {
                settings.setValue(record.key(), record.value());
            }
        }
        settings.endArray();
    }

    void saveCalculationsToFile(const QString& chosenFileName)
    {
        const bool json = chosenFileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive);
        const bool toml = chosenFileName.endsWith(QLatin1String(".toml"), Qt::CaseInsensitive);
        // We use TOML by default, so append it.
        const QString fileName = (toml || json) ? chosenFileName : chosenFileName + QLatin1String(".toml");
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        saveCalculationsToDevice(&file, json);
    }

    void saveCalculationsToDevice(QIODevice* device, bool json)
    {
        toml::table tomlRoot;
        QJsonObject jsonRoot;
        const QString title = chart->title();
        if (!title.isEmpty()) {
            jsonRoot.insert(QLatin1String("title"), title);
            tomlRoot.insert("title", qUtf8Printable(title));
        }

        QJsonArray jsonArray;
        toml::array tomlArray;
        for (int index = 0, last = tabs->count(); index < last; ++index) {
            const QVariantHash toSave = serialize(tabs->widget(index));
            if (json)
            {
                jsonArray.append(QJsonValue::fromVariant(toSave));
                continue;
            }

            toml::table calculation;
            for (auto entry = toSave.keyValueBegin(),
                 lastEntry = toSave.keyValueEnd(); entry != lastEntry; ++entry)
            {
                const QString& key = entry.base().key();
                const QVariant& variant = entry.base().value();
                switch (variant.type()) {
                case QVariant::Bool:
                    calculation.insert(key.toStdString(), variant.toBool());
                    break;
                case QVariant::Int:
                    calculation.insert(key.toStdString(), variant.toInt());
                    break;
                case QVariant::Double:
                    calculation.insert(key.toStdString(), variant.toDouble());
                    break;
                case QVariant::String:
                case QVariant::Color:
                    calculation.insert(key.toStdString(), qUtf8Printable(variant.toString()));
                    break;
                default:
                    qWarning() << "Value of an unexpected type for serialization:" << key << variant;
                }
            }
            tomlArray.push_back(calculation);
        }

        if (json) {
            jsonRoot.insert(keyDamageCalculations, jsonArray);
            QJsonDocument document(jsonRoot);
            device->write(document.toJson(QJsonDocument::Indented));
        }
        else {
            tomlRoot.insert(keyDamageCalculations.toStdString(), tomlArray);
            std::stringstream stream;
            stream << tomlRoot;
            device->write(stream.str().c_str());
        }
    }

    void loadCalculationsFromFile(const QString& fileName)
    {
        auto errorOut = [this](const char* text) {
            q.statusBar()->showMessage(tr("The file can not be loaded"));
            qWarning() << text;
        };

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            errorOut("File could not be opened");
            return;
        }
        const QByteArray fileData = file.readAll();

        const bool json = fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive);
        const bool toml = fileName.endsWith(QLatin1String(".toml"), Qt::CaseInsensitive);

        if (json) {
            const QJsonDocument document = QJsonDocument::fromJson(fileData);
            if (!document.isObject()) {
                errorOut("Cannot load JSON: root not an object");
                return;
            }
            const QJsonObject root = document.object();
            const QJsonValue title = root.value(QLatin1String("title"));
            if (title.isString())
                titleLine->setText(title.toString());

            if (!root.contains(keyDamageCalculations)) {
                errorOut("Cannot load JSON: root not an object");
                return;
            }
            const QJsonArray array = root.value(keyDamageCalculations).toArray();
            for (int index = 0, last = array.count(); index < last; ++index) {
                const QJsonValue value = array.at(index);
                if (!value.isObject()) {
                    errorOut("Cannot load JSON: element is not an object");
                    continue;
                }
                newPage();
                deserialize(tabs->currentWidget(), value.toObject().toVariantHash());
                const int current = tabs->currentIndex();
                const QVariant color = calculations[current].color->property("color");
                if (color.isValid())
                    lineSeries[current]->setColor(color.value<QColor>());
            }
        } else {
            if (!toml)
                qDebug() << "Attempting to parse" << fileName << "as TOML";

            const auto fileDataView = std::string_view(fileData.data(), fileData.size());
            toml::parse_result result = toml::parse(fileDataView);
            if (!result) {
                errorOut("Cannot load TOML: parse error");
                qWarning() << result.error().description().data() << "\n";
                return;
            }
            const auto table = result.table();
            const toml::node* title = table.get("title");
            if (title && title->is_string())
                titleLine->setText(QString::fromStdString(title->as_string()->get()));

            const std::string keyCalculations = keyDamageCalculations.toStdString();
            if (!table.contains(keyCalculations)) {
                errorOut("Cannot load TOML: doesn't contain calculations!");
                return;
            }
            const toml::node* arrayNode = table.get(keyCalculations);
            if (!arrayNode || !arrayNode->is_array()) {
                errorOut("Cannot load TOML: the calculations entry is not an array!");
                return;
            }
            const toml::array* array = arrayNode->as_array();
            for (int index = 0, last = array->size(); index < last; ++index) {
                const toml::node* value = array->get(index);
                if (!value->is_table()) {
                    errorOut("Cannot load TOML element in calculations: is not a table");
                    continue;
                }

                QVariantHash calculation;
                for (const auto& entry : *value->as_table()) {
                    const QString key = QString::fromStdString(entry.first);
                    QVariant variant;

#if 0 // TODO: Test that this works in all of our compilers.
                    entry.second.visit([&variant](auto&& element) noexcept
                    {
                        if constexpr (toml::is_number<decltype(element)>)
                            variant = QVariant::fromValue(element.as_integer()->get());
                        else if constexpr (toml::is_string<decltype(element)>)
                            variant = QVariant(QString::fromStdString(element.as_string()->get()));
                    });
#else
                    switch (entry.second.type()) {
                    case toml::node_type::none:
                    case toml::node_type::table:
                    case toml::node_type::array:
                        break;
                    case toml::node_type::string:
                        variant = QString::fromStdString(entry.second.as_string()->get());
                        break;
                    case toml::node_type::integer:
                        variant = QVariant::fromValue(entry.second.as_integer()->get());
                        break;
                    case toml::node_type::floating_point:
                        variant = QVariant::fromValue(entry.second.as_floating_point()->get());
                        break;
                    case toml::node_type::boolean:
                        variant = QVariant::fromValue(entry.second.as_boolean()->get());
                        break;
                    case toml::node_type::date:
                    case toml::node_type::time:
                    case toml::node_type::date_time:
                        break;
                    }
#endif
                    if (variant.isValid())
                        calculation.insert(key, variant);
                    else
                        qWarning() << "Entry at" << key << "is of a non-implemented type";
                }
                // TODO: the next lines are fully duplicated with the JSON block.

                newPage();
                deserialize(tabs->currentWidget(), calculation);
                const int current = tabs->currentIndex();
                const QVariant color = calculations[current].color->property("color");
                if (color.isValid())
                    lineSeries[current]->setColor(color.value<QColor>());
            }
        }
    }

    void populateEntriesMenu()
    {
        loadSavedMenu->clear();
        deleteSavedMenu->clear();
        auto loadEntry = [this](int index) {
            newPage();
            deserialize(tabs->currentWidget(), savedCalculations.at(index));
        };
        auto deleteEntry = [this](int index) {
            savedCalculations.remove(index);
            saveCalculationsToDisk();
            populateEntriesMenu();
        };

        for (int index = 0, size = savedCalculations.size(); index < size; ++index) {
            const auto& entry = savedCalculations.at(index);
            const QString name = entry.value(QLatin1String("name")).toString();
            loadSavedMenu->addAction(name, std::bind(loadEntry, index));
            deleteSavedMenu->addAction(name, std::bind(deleteEntry, index));
        }
        loadSavedMenu->setEnabled(savedCalculations.size() > 0);
        deleteSavedMenu->setEnabled(savedCalculations.size() > 0);
    }

    void newPage();
    void setupAxes();

    void updateAllSeries() {
        for (int index = 0; index < tabs->count(); ++index) {
            updateSeries(calculations[index], lineSeries[index]);
        }
    }
    // TODO: previously this accepted the index as parameter, which is better,
    // but requires knowing the proper index of a series/calculation. This was
    // causing a crash when deleting tabs. But I will have to support something
    // if I want to allow moving tabs around.
    void updateSeriesAtCurrentIndex() {
        const int index = tabs->currentIndex();
        updateSeries(calculations[index], lineSeries[index]);
    }

    WeaponArrangement makeArrangement(WeaponArrangementWidget* widget, int luck) const;
    QVector<QPointF> pointsFromInput(const Calculation& c) const;
    void updateSeries(const Calculation& c, QLineSeries* series);

    static void setColorInButton(const QColor& color, QPushButton* button)
    {
        const int side = button->height();
        QPixmap pixmap(side, side);
        pixmap.fill(color);
        button->setIcon(QIcon(pixmap));
        button->setProperty("color", color);
    }
};

// Main class //////////////////////////////////////////////////////////////////

DamageCalculatorPage::DamageCalculatorPage(QWidget* parent)
    : QWidget(parent)
    , d(new Private(*this))
{
    for (int i = 10; i >= -20; --i)
        d->armorClasses << i;

    d->loadSavedCalculations();

    // Menus ///////////////////////////////////////////////////////////////////
    d->fileMenu = menuBar()->addMenu(tr("File"));

    auto action = new QAction(tr("Save visible calculations as..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    d->fileMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        auto dialog = new QFileDialog(this);
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setNameFilter(tr("TOML or JSON files (*.toml *.json)"));
        connect(dialog, &QFileDialog::fileSelected, this,
            std::bind(&Private::saveCalculationsToFile, d, std::placeholders::_1));
        dialog->setModal(true);
        dialog->show();
    });

    action = new QAction(tr("Load calculations from..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    d->fileMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        auto dialog = new QFileDialog(this);
        dialog->setFileMode(QFileDialog::ExistingFile);
        connect(dialog, &QFileDialog::fileSelected, this,
            std::bind(&Private::loadCalculationsFromFile, d, std::placeholders::_1));
        dialog->setModal(true);
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
#ifdef Q_OS_WASM
    // Seems that it only supports copying text for now. :-/
    // https://developer.mozilla.org/en-US/docs/Web/API/Clipboard_API
    action->setEnabled(false);
#endif

    action = new QAction(tr("Save chart as..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    d->fileMenu->addAction(action);

    connect(action, &QAction::triggered, [this] {
        const QPixmap pixmap = d->chartView->grab();
#ifndef Q_OS_WASM
        auto dialog = new QFileDialog(this);
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        connect(dialog, &QFileDialog::fileSelected, [pixmap, dialog](const QString& name) {
            pixmap.save(name, "png");
            dialog->deleteLater();
        });
        dialog->open();
#else
        QByteArray fileData;
        QBuffer buffer(&fileData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "png");
        QFileDialog::saveFileContent(fileData, QLatin1String("chart.png"));
#endif
    });

    d->mainMenu = menuBar()->addMenu(tr("Damage calculator calculations"));

    action = new QAction(tr("Duplicate current calculation"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        const QVariantHash saved = d->serialize(d->tabs->currentWidget());
        d->newPage();
        // Save the new color set on the new page.
        const int current = d->tabs->currentIndex();
        const QColor color = d->calculations[current].color->property("color").value<QColor>();
        // TODO: block signals recursively, load UI values, then update chart.
        d->deserialize(d->tabs->currentWidget(), saved);
        d->setColorInButton(color, d->calculations.last().color);
    });

    action = new QAction(tr("Save current calculation to preferences"), this);
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, std::bind(&Private::saveCurrentCalculation, d));

    d->loadSavedMenu = new QMenu(tr("Load calculation from preferences"), d->mainMenu);
    d->mainMenu->addMenu(d->loadSavedMenu);
    d->deleteSavedMenu = new QMenu(tr("Delete calculation from preferences"), d->mainMenu);
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
        for (const auto& entry : qAsConst(d->savedCalculations)) {
            names << entry.value(QLatin1String("name")).toString();
        }
        d->manageDialog->setNames(names);
        d->manageDialog->show();
    });
    connect(d->manageDialog, &ManageDialog::showClicked, this, [this](int index) {
        d->newPage();
        d->deserialize(d->tabs->currentWidget(), d->savedCalculations.at(index));
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
    chartControlsLayout->addWidget(new QLabel(tr("Title:")));
    d->titleLine = new QLineEdit;
    d->titleLine->setPlaceholderText(tr("Optional. Set a title for the chart. Accepts some HTML."));
    connect(d->titleLine, &QLineEdit::textChanged, [this](const QString& text) {
        d->chart->setTitle(text);
    });
    chartControlsLayout->addWidget(d->titleLine);
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
            std::bind(&Private::setupAxes, d));
    connect(d->maximumX, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::setupAxes, d));
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
    d->axisTitle = new QCheckBox(tr("Axis description"));
    chartControlsLayout->addWidget(d->axisTitle);
    d->axisTitle->setChecked(true);
    connect(d->axisTitle, &QCheckBox::toggled, [this](bool value) {
        if (auto axis = qobject_cast<QValueAxis*>(d->chart->axes(Qt::Horizontal).constFirst())) {
            axis->setTitleVisible(value);
        }
        if (auto axis = qobject_cast<QValueAxis*>(d->chart->axes(Qt::Vertical).constFirst())) {
            axis->setTitleVisible(value);
        }
    });
    d->reverse = new QCheckBox(tr("AC: worst to best"));
    chartControlsLayout->addWidget(d->reverse);
    d->reverse->setChecked(true);
    connect(d->reverse, &QCheckBox::toggled, [this](bool value) {
        // FIXME: assumption on QLineSeries
        if (auto axis = qobject_cast<QValueAxis*>(d->chart->axes(Qt::Horizontal).constFirst())) {
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
    connect(d->enemy.acidResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.coldResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.electricityResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.fireResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.magicDamageResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.poisonDamageResistance, qOverload<int>(&QSpinBox::valueChanged),
            std::bind(&Private::updateAllSeries, d));
    connect(d->enemy.helmet, &QCheckBox::toggled, std::bind(&Private::updateAllSeries, d));

    // Tab widget with calculations / //////////////////////////////////////////
    d->tabs = new QTabWidget;
    auto newButton = new QPushButton(tr("New"));
    d->tabs->setCornerWidget(newButton);
    connect(newButton, &QPushButton::clicked,
            std::bind(&Private::newPage, d));

    d->tabs->setTabsClosable(true);
    connect(d->tabs, &QTabWidget::tabCloseRequested, [this](int index) {
        if (d->tabs->count() == 1)
            return; // don't close the last one for now, to keep the "New" button
        d->calculations.removeAt(index);
        d->chart->removeSeries(d->lineSeries[index]);
        delete d->lineSeries.takeAt(index);
        // TODO: with the new approach, this might be a tad heavy. Review.
        d->setupAxes();
        delete d->tabs->widget(index);
        for (int tab = index ; tab < d->tabs->count(); ++tab)
            d->tabs->setTabText(tab, tr("Calculation %1").arg(tab + 1));
    });

    // Layout grouping the calculations and the enemy controls /////////////////
    auto inputArea = new QToolBox;
    inputArea->addItem(enemyControls, tr("Opponent"));
    inputArea->addItem(d->tabs, tr("Attacker"));
    inputArea->setCurrentWidget(d->tabs);

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
    // FIXME: Doesn't work on Plasma with the global menu, debug why.
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

// TODO: Ideas for serialization. Take advantage that in the findChildren loop
// we get the parent first, then the children. We could reach a certain kind of
// widget, and instead of skipping it to let the loop reach the children, stop
// there, get the "prefix" of it, and serialize the children with the prefix. We
// can't really stop the loop, but we could either loop only through the direct
// children (unlike now) or keep a list of visited children, to avoid visiting
// them twice. This would allow deserializing the tree in a way which is not
// flat. We can also keep the "last ParentSpecialWhateverWidget visited", then
// use that to get a prefix on its children (use isAncestorOf, etc.).

void DamageCalculatorPage::Private::deserialize(QWidget* root, QVariantHash data)
{
    // TODO: remove on newer releases.
    // Inject some values for the new features not in the old saves.
    if (!data.contains(QLatin1String("criticalHitChance1")))
        data.insert(QLatin1String("criticalHitChance1"), 5);
    if (!data.contains(QLatin1String("criticalHitChance2")))
        data.insert(QLatin1String("criticalHitChance2"), 5);
    if (!data.contains(QLatin1String("criticalStrike")))
        data.insert(QLatin1String("criticalStrike"), false);
    if (!data.contains(QLatin1String("maximumDamage")))
        data.insert(QLatin1String("maximumDamage"), false);

    auto migrateKey = [&data](const QString& from, const QString& to, QVariant value) {
        if (!data.contains(to)) {
            if (data.contains(from))
                value = data.take(from);
            data.insert(to, value);
        }
    };
    // TODO: Likewise. This is a trival change to apply to my saved files. I
    // doubt anyone else is caring for the format as in 0.1. This change was
    // done right before 0.2 was published, and saved files can easily be modified.
    migrateKey(QLatin1String("strengthThac0Bonus"), QLatin1String("statThac0Bonus"), 0);
    migrateKey(QLatin1String("strengthDamageBonus"), QLatin1String("statDamageBonus"), 0);

    for (auto child : root->findChildren<QWidget*>()) {
        if (child->isHidden()) // Skip the duplicated APR spinbox (int/double)
            continue;
        if (qobject_cast<QLabel*>(child) ||
            qobject_cast<QScrollArea*>(child) ||
            qobject_cast<QScrollBar*>(child) ||
            qobject_cast<QToolBox*>(child) ||
            qobject_cast<SpecialDamageWidget*>(child) ||
            qobject_cast<WeaponArrangementWidget*>(child))
            continue;
        // Skip wrapper QWidget without subclass.
        if (child->metaObject() == &QWidget::staticMetaObject)
            continue;
        if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            if (!groupbox->isCheckable())
                continue;
        }

        if (child->objectName().startsWith(QLatin1String("qt_")))
            continue;

        // FIXME: duplicated in serialize/deserialize.
        const QString name = [child]() {
            QString n; // Weapon number. Empty, "1" or "2"
            QWidget* ancestor = child->parentWidget();
            do {
                if (qobject_cast<WeaponArrangementWidget*>(ancestor))
                    n = ancestor->objectName().back();
            } while ((ancestor = ancestor->parentWidget()));
            Q_ASSERT(n.isEmpty() || n == QLatin1Char('1') || n == QLatin1Char('2'));

            if (qobject_cast<SpecialDamageWidget*>(child->parent()))
                return child->parent()->objectName() + n + child->objectName();

            if (child->objectName().contains(QLatin1String("attacks")))
                return child->objectName();
            return child->objectName() + n;
        }();

        const QVariant value = data.take(name);
        if (!value.isValid()) {
            // TODO: Many old saves don't have this yet. Remove the check eventually.
            if (qobject_cast<SpecialDamageWidget*>(child->parent()))
                continue;
            qInfo() << "Data for" << child << "not found; using key:" << name;
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
        else if (auto button = qobject_cast<QPushButton*>(child)) {
            if (qobject_cast<SpecialDamageWidget*>(button->parent()))
                continue;
            setColorInButton(value.value<QColor>(), button);
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
        // FIXME: duplicated in serialize/deserialize.
        const QString name = [child]() {
            QString n; // Weapon number. Empty, "1" or "2"
            QWidget* ancestor = child->parentWidget();
            do {
                if (qobject_cast<WeaponArrangementWidget*>(ancestor))
                    n = ancestor->objectName().back();
            } while ((ancestor = ancestor->parentWidget()));
            Q_ASSERT(n.isEmpty() || n == QLatin1Char('1') || n == QLatin1Char('2'));

            if (qobject_cast<SpecialDamageWidget*>(child->parent()))
                return child->parent()->objectName() + n + child->objectName();

            if (child->objectName().contains(QLatin1String("attacks")))
                return child->objectName();
            return child->objectName() + n;
        }();

        if (child->isHidden()) // Skip the float APR spinbox or the int one.
            continue;
        if (qobject_cast<QLabel*>(child) ||
            qobject_cast<QScrollArea*>(child) ||
            qobject_cast<QScrollBar*>(child) ||
            qobject_cast<QToolBox*>(child) ||
            qobject_cast<SpecialDamageWidget*>(child) ||
            qobject_cast<WeaponArrangementWidget*>(child))
            continue;
        // Skip wrapper QWidget without subclass.
        if (child->metaObject() == &QWidget::staticMetaObject)
            continue;
        if (name.isEmpty() || name.startsWith(QLatin1String("qt_")))
            continue;
        else if (auto spinbox1 = qobject_cast<QSpinBox*>(child))
            result.insert(name, spinbox1->value());
        else if (auto spinbox2 = qobject_cast<QDoubleSpinBox*>(child))
            result.insert(name, spinbox2->value());
        else if (auto combobox = qobject_cast<QComboBox*>(child))
            result.insert(name, combobox->currentIndex());
        else if (auto checkbox = qobject_cast<QCheckBox*>(child))
            result.insert(name, checkbox->isChecked());
        else if (auto button = qobject_cast<QPushButton*>(child)) {
            const QVariant color = button->property("color");
            if (color.isValid())
                result.insert(name, color);
        }
        else if (auto line = qobject_cast<QLineEdit*>(child)) {
            if (name == QLatin1String("name"))
                result.insert(name, line->text());
        }
        else if (auto groupbox = qobject_cast<QGroupBox*>(child)) {
            if (groupbox->isCheckable())
                result.insert(name, groupbox->isChecked());
        }
        // Skip the list views inside the combo boxes.
        else if (auto grandParent = child->parent()->parent();
                 grandParent && grandParent->metaObject() == &QComboBox::staticMetaObject)
            continue;
        else
            qWarning() << "Not serialized:" << child << child->parent() << name;
    }

    return result;
}

void DamageCalculatorPage::Private::newPage()
{
    auto widget = new QWidget;

    calculations.append(Calculation());
    Calculation& calculation = calculations.last();
    calculation.setupUi(widget);
    tabs->addTab(widget, tr("Calculation %1").arg(tabs->count() + 1));
    tabs->setCurrentWidget(widget);

    connect(calculation.name, &QLineEdit::textChanged,
            [this](const QString& text) {
        lineSeries.at(tabs->currentIndex())->setName(text);
    });
    connect(calculation.color, &QPushButton::clicked, [this, calculation]() {
        const QColor initial = lineSeries[tabs->currentIndex()]->color();
        auto dialog = new QColorDialog(initial, &q);
        q.connect(dialog, &QColorDialog::colorSelected,
                  &q, [this, calculation](const QColor& color)
        {
            lineSeries[tabs->currentIndex()]->setColor(color);
            setColorInButton(color, calculation.color);
            calculation.color->setProperty("color", color);
        });
        dialog->setModal(true);
        dialog->show();
    });

    auto update = std::bind(&Private::updateSeriesAtCurrentIndex, this);
    for (auto child : widget->findChildren<QSpinBox*>())
        connect(child, qOverload<int>(&QSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QDoubleSpinBox*>())
        connect(child, qOverload<double>(&QDoubleSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QComboBox*>())
        connect(child, &QComboBox::currentTextChanged, update);
    for (auto child : widget->findChildren<QCheckBox*>())
        connect(child, &QCheckBox::toggled, update);
    connect(calculation.offHandGroup, &QGroupBox::toggled, update);

    // TODO: Decouple this, from setting the UI to setting the whole calculation.
    auto series = new QLineSeries;
    lineSeries.append(series);
    chart->addSeries(series);
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
        if (!over)
            return;
        point = closestSeriesPoint(series->pointsVector(), point);
        q.statusBar()->showMessage(tr("%1. Damage: %2").arg(series->name()).arg(point.y()), 2000);
    });
    QLegendMarker* marker = chart->legend()->markers(series).constFirst();
    connect(marker, &QLegendMarker::clicked, chart, [series, marker] {
        series->setVisible(!series->isVisible());
        marker->setVisible(true);
        QFont font = marker->font();
        font.setStrikeOut(!series->isVisible());
        marker->setFont(font);
    });
    connect(marker, &QLegendMarker::hovered, chart, [series](bool over) {
        static int defaultSize = series->pen().width();
        QPen pen = series->pen();
        pen.setWidth(defaultSize + (over ? 1 : 0));
        series->setPen(pen);
    });

    setColorInButton(series->color(), calculation.color);

    updateSeriesAtCurrentIndex();
}

void DamageCalculatorPage::Private::setupAxes()
{
    if (chart->axes().size() == 0)
        chart->createDefaultAxes();

    if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).constFirst())) {
        axis->setReverse(reverse->isChecked());
        axis->setMin(minimumX->value());
        axis->setMax(maximumX->value());
        axis->setTickCount(maximumX->value() - minimumX->value() + 1);

        axis->setLabelFormat(QLatin1String("%i"));
        axis->setTitleText(tr("Opponent's Armor Class"));
        axis->setTitleVisible(axisTitle->isChecked());
#if 0
        QFont font = axis->labelsFont();
        font.setPointSize(font.pointSize() - 2);
        axis->setLabelsFont(font);
#endif
    }
    // FIXME: assumption on QLineSeries
    if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).constFirst())) {
        double min = +qInf();
        double max = -qInf();
        for (auto series : qAsConst(lineSeries)) {
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
        axis->setTitleText(tr("Damage per round"));
        axis->setTitleVisible(axisTitle->isChecked());
    }
}

WeaponArrangement DamageCalculatorPage::Private::makeArrangement(WeaponArrangementWidget* widget,
                                                                 int luck) const
{
    // This does most of the work, missing only the parts not on WeaponArrangementWidget
    WeaponArrangement result = widget->toData();

    result.damage.find(result.physicalDamageType()).value().luck(luck);

    for (auto entry = result.damage.keyValueBegin(),
         last = result.damage.keyValueEnd(); entry != last; ++entry)
    {
        const Calculators::DamageType type = entry.base().key();
        DiceRoll& roll = entry.base().value();
        roll.resistance(enemy.resistanceFactor(type));
    }

    return result;
}

QVector<QPointF> DamageCalculatorPage::Private::pointsFromInput(const Calculation& c) const
{
    const bool doubleCriticalDamage = !enemy.helmet->isChecked();
    const bool offHand = c.offHandGroup->isChecked();
    const bool maximumDamage = c.maximumDamage->isChecked();
    const bool criticalStrike = c.criticalStrike->isChecked();

    // TODO: support damage resistance over 100%, which should obviously heal
    // a creature, and hence subtract from the total damage.
    const WeaponArrangement weapon1 = makeArrangement(c.weapon1, c.luck->value());
    const WeaponArrangement weapon2 = makeArrangement(c.weapon2, c.luck->value());

    Damage::Common common;
    common.thac0 = c.baseThac0->value();
    common.statToHit = c.statThac0Bonus->value();
    common.otherToHit = c.classThac0Bonus->value() + c.miscThac0Bonus->value();

    common.statDamage = c.statDamageBonus->value();
    common.otherDamage = c.classDamageBonus->value() + c.miscDamageBonus->value();

    const int mainAcModifier = enemy.acModifier(c.weapon1->damageType());
    const int offAcModifier  = enemy.acModifier(c.weapon2->damageType());

    const Damage calculator(weapon1, weapon2, common);
    const Damage::Stat stat = maximumDamage ? Damage::Maximum : Damage::Average;

    const auto damages1 = calculator.onHitDamages(Damage::Main, stat);
    const auto damages2 = calculator.onHitDamages(Damage::OffHand, stat);

    double totalDamage1 = 0.0;
    for (auto damage : damages1)
        totalDamage1 += damage;
    double totalDamage2 = 0.0;
    for (auto damage : damages2)
        totalDamage2 += damage;

    const auto criticalHitChance1 = criticalStrike ? 1.0 : c.weapon1->criticalHitChance();
    const auto criticalHitChance2 = criticalStrike ? 1.0 : c.weapon2->criticalHitChance();

    QVector<QPointF> points;
    for (const int ac : armorClasses) {
        const int mainToHit = calculator.thac0(Damage::Main)    - ac - mainAcModifier;
        const int offToHit  = calculator.thac0(Damage::OffHand) - ac - offAcModifier;

        //
        // TODO: Move to-hit calculations to the damage calculation code.
        //
        const double toHit1 = Calculators::chanceToHit(mainToHit, criticalHitChance1);
        const double toHit2 = Calculators::chanceToHit(offToHit, criticalHitChance2);
        double damage = toHit1 * totalDamage1 * weapon1.attacks;
        if (offHand)
            damage += toHit2 * totalDamage2 * weapon2.attacks;

        // FIXME adjust to the new damage calculation!
        if (doubleCriticalDamage) { // Unprotected against critical hits.
            if (criticalStrike) // All damage doubled!
                damage *= 2;
            else {
                // Otherwise, consider the damage again, this time without
                // chance to failure, but scaled by the chance of critical hit.
                damage += criticalHitChance1 * weapon1.physicalDamage().average() * weapon1.attacks;
                if (offHand)
                    damage += criticalHitChance2 * weapon2.physicalDamage().average() * weapon2.attacks;
            }
        }
        points.append(QPointF(ac, damage));
    }
    return points;
}

void DamageCalculatorPage::Private::updateSeries(const Calculation& c, QLineSeries* series)
{
    series->replace(pointsFromInput(c));
    setupAxes();

    if (series->attachedAxes().size() == 0) {
        // FIXME: assumption on QLineSeries
        if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).constFirst()))
            series->attachAxis(axis);
        if (auto axis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).constFirst()))
            series->attachAxis(axis);
    }
}

#include "damagecalculatorpage.moc"
