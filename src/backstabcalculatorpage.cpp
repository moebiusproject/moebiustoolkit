/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020-2021 Alejandro Exojo Piqueras
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

#include "backstabcalculatorpage.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QBoxLayout>
#include <QChart>
#include <QChartView>
#include <QDebug>
#include <QHorizontalStackedBarSeries>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QValueAxis>

#include "backstabstats.h"
#include "calculators.h"
#include "diceroll.h"
#include "ui_backstabsetup.h"
#include "ui_weaponarrangementwidget.h"
#include "weaponarrangementwidget.h"

using namespace Calculators;
using namespace QtCharts;

struct BackstabCalculatorPage::Private
{
    Private(BackstabCalculatorPage& page_)
        : page(page_)
    {}
    BackstabCalculatorPage& page;

    QChart* chart = nullptr;
    QChartView* chartView = nullptr;

    QTabWidget* tabs = nullptr;

    QVector<Ui::BackstabSetup> setups;

    QHorizontalStackedBarSeries* series = nullptr;
    QBarSet* setStrength = nullptr;
    QBarSet* setBase = nullptr;
    QBarSet* setMultiplied = nullptr;
    QStringList names;

    void setupChart();
    void newPage();
    void setupAxes();
    void updateCurrentSeries();
};

BackstabCalculatorPage::BackstabCalculatorPage(QWidget *parent)
    : BasePage(parent)
    , d(new Private(*this))
{
    auto chartControlsLayout = new QHBoxLayout;
    // This is just unused for now.
//    chartControlsLayout->addWidget(new QLabel(tr("TODO")));

    d->chart = new QChart;
    d->chart->setAnimationOptions(QChart::SeriesAnimations);
    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    auto chartViewLayout = new QVBoxLayout;
    chartViewLayout->addLayout(chartControlsLayout);
    chartViewLayout->addWidget(d->chartView);

    d->tabs = new QTabWidget;
    auto newButton = new QPushButton(tr("New"));
    d->tabs->setCornerWidget(newButton);
    connect(newButton, &QPushButton::clicked,
            std::bind(&Private::newPage, d));

    d->tabs->setTabsClosable(true);
    connect(d->tabs, &QTabWidget::tabCloseRequested, [this](int index) {
        if (d->tabs->count() == 1)
            return;
        d->setups.removeAt(index);
        // TODO: implement instead of crash
//        d->chart->removeSeries(d->series[index]);
//        delete d->series.takeAt(index);
        d->setupAxes();
        delete d->tabs->widget(index);
        for (int tab = index ; tab < d->tabs->count(); ++tab)
            d->tabs->setTabText(tab, tr("Configuration %1").arg(tab + 1));
    });

    auto inputWidget = new QWidget;
    // This layout is irrelevant for now, but it was necessary for the damage
    // calculator to have an extra "row" with the resistances of the enemy.
    auto inputLayout = new QVBoxLayout(inputWidget);
    inputWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    auto inputArea = new QScrollArea;
    inputArea->setWidget(inputWidget);
    inputArea->setFrameShape(QFrame::NoFrame);
    inputArea->setWidgetResizable(true);
    inputLayout->addWidget(d->tabs);

    auto layout = new QHBoxLayout;
    setLayout(layout);
    layout->addLayout(chartViewLayout, 1);
    layout->addWidget(inputArea, 0);

    d->setupChart();
    d->newPage();
}

BackstabCalculatorPage::~BackstabCalculatorPage()
{
    delete d;
    d = nullptr;
}

QList<QChartView*> BackstabCalculatorPage::charts() const
{
    return { d->chartView };
}

void BackstabCalculatorPage::Private::setupChart()
{
    // TODO: review if the ownership is fully passed to the chart or we need
    // some parent for all this heap allocated objects below.

    series = new QHorizontalStackedBarSeries;

    setStrength = new QBarSet(page.tr("Strength"));
    setBase = new QBarSet(page.tr("Base"));
    setMultiplied = new QBarSet(page.tr("Multiplied"));

    series->append(setStrength);
    series->append(setBase);
    series->append(setMultiplied);
    chart->addSeries(series);
}

void BackstabCalculatorPage::Private::newPage()
{
    auto widget = new QWidget;

    setups.append(Ui::BackstabSetup());
    Ui::BackstabSetup& setup = setups.last();
    setup.setupUi(widget);

    // Hide/change parts that are irrelevant to us.
    setup.weapon->ui->damageGroup->setTitle(tr("Weapon damage"));
    setup.weapon->ui->thac0Group->hide();
    setup.weapon->ui->damageType->hide();
    setup.weapon->ui->damageTypeLabel->hide();
    setup.weapon->ui->attacksPerRound1->hide();
    setup.weapon->ui->attacksPerRound2->hide();
    setup.weapon->ui->attacksPerRoundLabel->hide();
    setup.weapon->ui->criticalHitChance->hide();
    setup.weapon->ui->criticalHitChanceLabel->hide();

    tabs->addTab(widget, tr("Setup %1").arg(tabs->count() + 1));
    tabs->setCurrentWidget(widget);

    connect(setup.name, &QLineEdit::textChanged, [this](const QString& text) {
        (void)text;
        // TODO: something lighter than this to change the name without flicker.
        setupAxes();
    });

    auto update =  std::bind(&Private::updateCurrentSeries, this);
    for (auto child : widget->findChildren<QSpinBox*>())
        connect(child, qOverload<int>(&QSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QDoubleSpinBox*>())
        connect(child, qOverload<double>(&QDoubleSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QCheckBox*>())
        connect(child, &QCheckBox::toggled, update);

    // We just ensure the count is correct, then the value will be set properly.
    setStrength->append(0);
    setBase->append(0);
    setMultiplied->append(0);

    updateCurrentSeries();
}

void BackstabCalculatorPage::Private::setupAxes()
{
    QBarCategoryAxis* vertical = nullptr;
    if (auto axes = chart->axes(Qt::Vertical); axes.size() != 0) {
        vertical = static_cast<QBarCategoryAxis*>(axes.first());
        vertical->clear();
    }
    else {
        vertical = new QBarCategoryAxis;
        chart->addAxis(vertical, Qt::AlignLeft);
        series->attachAxis(vertical);
    }
    names.clear();
    for (int index = 0; index < tabs->count(); ++index) {
        names << setups.at(index).name->text();
    }
    vertical->append(names);


    QValueAxis* horizontal = nullptr;
    if (auto axes = chart->axes(Qt::Horizontal); axes.size() != 0) {
        horizontal = static_cast<QValueAxis*>(axes.first());
    }
    else {
        horizontal = new QValueAxis;
        chart->addAxis(horizontal, Qt::AlignBottom);
        series->attachAxis(horizontal);
    }

    // This is a bit weird: we need to sum the i-th value of each bar set, then
    // find the maximum of those to set the range of the axis.
    const auto barSets = series->barSets();
    // Assumption: how we chart this, for now, implies that all the sets are
    // having the same number of points.
    const int entries = barSets.first()->count();

    QVector<double> maximums(entries, 0.0);
    for (int entry = 0; entry < entries; ++entry) {
        for (auto barSet : barSets) {
            maximums[entry] += barSet->at(entry);
        }
    }

    double max = 0.0;
    for (double maximum : qAsConst(maximums)) {
        max = qMax(maximum, max);
    }
    horizontal->setRange(0, max);
    horizontal->applyNiceNumbers();
}

void BackstabCalculatorPage::Private::updateCurrentSeries()
{
    const int index = tabs->currentIndex();
    const Ui::BackstabSetup& setup = setups[index];
    const WeaponArrangement weapon = setup.weapon->toData();
    // TODO: luck support
    // FIXME: this line is copy/pasted from damage calculator page. Also, this
    // line is like the only outside use of physicalDamageType(), and shows
    // that physicalDamage() is not that useful if it returns a copy that
    // we can't use to modify the luck value. Should be easy to fix.
    // weapon.damage.find(weapon.physicalDamageType()).value().luck(42);

    const bool max = setup.maximumDamage->isChecked();
    double totalWeaponDamage = 0.0;
    for (const auto& damage : weapon.damage)
        totalWeaponDamage += (max ? damage.maximum() : damage.average());
    const DiceRoll physicalDamage = weapon.physicalDamage();
    const double physicalPart = (max ? physicalDamage.maximum() : physicalDamage.average())
                              + setup.kit->value() + setup.other->value();

    // TODO: use this in a testable way instead of the quick solution. :-)
    Backstab::Other other;
    other.strength = setup.strength->value();
    other.multiplier = setup.multiplier->value();
    other.kit = setup.kit->value();
    other.bonus = setup.other->value();
    const Backstab calculator(weapon, other);

    const int remainingMultiplier = other.multiplier - 1;

    setStrength->replace(index, other.strength);
    setBase->replace(index, totalWeaponDamage);
    setMultiplied->replace(index, remainingMultiplier * physicalPart);
    qDebug() << setup.name->text() << totalWeaponDamage << (remainingMultiplier*physicalPart);

    setupAxes();
}

