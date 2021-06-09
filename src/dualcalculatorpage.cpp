/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2021 Alejandro Exojo Piqueras
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


#include "dualcalculatorpage.h"
#include "ui_dualcalculatorwidget.h"

#include "debugcharts.h"
#include "xplevels.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QChartView>
#include <QDebug>
#include <QSplitter>
#include <QStackedBarSeries>
#include <QValueAxis>

using namespace QtCharts;

struct DualCalculatorPage::Private
{
    Private(DualCalculatorPage& page, const QString& location)
        : parent(page)
        , xpLevels(location)
    {}

    void loaded();
    void addCalculation();
    void recalculate(int index);
    void setNamesAxis(int index, const QString& text)
    {
        if (namesAxis->count() <= index)
            namesAxis->append(text);
        else
            namesAxis->replace(namesAxis->at(index), text);
    }

    DualCalculatorPage& parent;

    XpLevels xpLevels;

    QChart* chart = nullptr;
    QBarSet* class1 = nullptr;
    QBarSet* class2 = nullptr;
    QValueAxis* xpAxis = nullptr;
    QBarCategoryAxis* namesAxis = nullptr;

    QVector<Ui::DualCalculatorWidget> widgets;
    QVBoxLayout* inputsLayout = nullptr;
};

// Private /////////////////////////////////////////////////////////////////////

void DualCalculatorPage::Private::loaded()
{
    const QVector<XpLevels::Level> levels = xpLevels.levels();
    for (const XpLevels::Level& level : levels) {
        QVector<QPointF> points;
        for (int index = 1; index < level.thresholds.count() && index <= 40; ++index) {
            const uint x = qMax(uint(1), level.thresholds.at(index));
            points.append(QPointF(x, index));
        }
    }

    for (auto& widget : widgets) {
        QSignalBlocker blocker1(widget.firstClass);
        QSignalBlocker blocker2(widget.secondClass);
        widget.firstClass->addItems(xpLevels.classes());
        widget.secondClass->addItems(xpLevels.classes());
    }
    // TODO: now we should enable the "add new" button here, which should be disabled initially

    // TODO. Hardcoded values for now.
    auto firstWidget = widgets.first();

    const QString initialName = QLatin1String("Fighter 9 > Mage 10");
    firstWidget.name->setText(initialName);

    QSignalBlocker blocker1(firstWidget.firstClassLevel);
    QSignalBlocker blocker2(firstWidget.secondClassLevel);
    QSignalBlocker blocker3(firstWidget.firstClass);
    QSignalBlocker blocker4(firstWidget.secondClass);
    firstWidget.firstClassLevel->setValue(9);
    firstWidget.secondClassLevel->setValue(10);
    firstWidget.firstClass->setCurrentText(QLatin1String("FIGHTER"));
    firstWidget.secondClass->setCurrentText(QLatin1String("MAGE"));
    recalculate(0);
}

void DualCalculatorPage::Private::addCalculation()
{
    auto input = new QWidget;
    widgets.append(Ui::DualCalculatorWidget());
    const int index = widgets.size() - 1;
    Ui::DualCalculatorWidget& widget = widgets.last();
    widget.setupUi(input);
    widget.close->hide(); // TODO: implement the "delete" feature

    inputsLayout->insertWidget(index + 1, input);

    widget.firstClass->addItems(xpLevels.classes());
    widget.secondClass->addItems(xpLevels.classes());

    auto recalculateThis = [this, index] { recalculate(index); };

    connect(widget.firstClass, &QComboBox::currentTextChanged,
            recalculateThis);
    connect(widget.secondClass, &QComboBox::currentTextChanged,
            recalculateThis);
    connect(widget.firstClassLevel, &QSpinBox::valueChanged,
            recalculateThis);
    connect(widget.secondClassLevel, &QSpinBox::valueChanged,
            recalculateThis);

    connect(widget.name, &QLineEdit::textChanged, [this, index](const QString& text)
    {
        setNamesAxis(index, text);
    });
}

void DualCalculatorPage::Private::recalculate(int index)
{
    const Ui::DualCalculatorWidget& widget = widgets.at(index);
    const QString firstClass = widget.firstClass->currentText();
    const QString secondClass = widget.secondClass->currentText();

    const QVector<quint32> firstClassData = xpLevels.thresholds(firstClass);
    const QVector<quint32> secondClassData = xpLevels.thresholds(secondClass);
    const int firstLevel = qBound(0, widget.firstClassLevel->value(), firstClassData.size() - 1);
    const int secondLevel = qBound(0, widget.secondClassLevel->value(), secondClassData.size() - 1);

    // Level 1 is index=0 on the container, so subtract 1.
    const int xp1 = firstClassData.at(firstLevel - 1);
    const int xp2 = secondClassData.at(secondLevel - 1);

    // qDebug() << firstClass << firstLevel << xp1;
    // qDebug() << secondClass << secondLevel << xp2;

    if (index < class1->count())
        class1->replace(index, xp1);
    else
        class1->append(xp1);

    if (index < class2->count())
        class2->replace(index, xp2);
    else
        class2->append(xp2);

    int max = 0;
    for (int i = 0, last = widgets.size(); i < last; ++i) {
        const int total = class1->at(i) + class2->at(i);
        max = qMax(total, max);
    }
    xpAxis->setRange(0, max);
    xpAxis->applyNiceNumbers();

    if (widget.name->text().isEmpty()) {
        const QString text = secondLevel > 1 ?
                tr("%1 %2 > %3 %4").arg(firstClass).arg(firstLevel)
                                   .arg(secondClass).arg(secondLevel) :
                tr("%1 %2").arg(firstClass).arg(firstLevel);
        setNamesAxis(index, text);
    }
}


// Public //////////////////////////////////////////////////////////////////////

DualCalculatorPage::DualCalculatorPage(QWidget* parent)
    : QWidget(parent)
    , d(new Private(*this, m_currentLocation))
{
    connect(&d->xpLevels, &XpLevels::loaded, this, [this]{ d->loaded(); });

    d->class1 = new QBarSet(tr("First class"));
    d->class2 = new QBarSet(tr("Second class"));

    auto series = new QStackedBarSeries;
    series->append(d->class1);
    series->append(d->class2);

    d->chart = new QChart;
    d->chart->addSeries(series);
    d->chart->setAnimationOptions(QChart::SeriesAnimations);
    // TODO: hide the legend, because it's just obvious what the stuff is?
//    d->chart->legend()->setVisible(false);
    // Hide it later? see the temperature examples, it's done a bit below

    d->xpAxis = new QValueAxis;
    d->xpAxis->setLabelFormat(QLatin1String("%i"));
    d->chart->addAxis(d->xpAxis, Qt::AlignLeft);
    series->attachAxis(d->xpAxis);

    d->namesAxis = new QBarCategoryAxis;
    d->chart->addAxis(d->namesAxis, Qt::AlignBottom);
    series->attachAxis(d->namesAxis);

    auto chartView = new QChartView(d->chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // A vertical column with inputs
    auto inputs = new QWidget;
    d->inputsLayout = new QVBoxLayout;
    inputs->setLayout(d->inputsLayout);
    auto addButton = new QPushButton(tr("Add more"));
    connect(addButton, &QPushButton::clicked, [this]
    {
        d->addCalculation();
        d->recalculate(d->widgets.count() - 1);
    });
    d->inputsLayout->addWidget(addButton);

    d->addCalculation();

    auto bottomSpacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    d->inputsLayout->addItem(bottomSpacer);

    // Main layout of this page: an horizontal row chart<->inputs
    setLayout(new QHBoxLayout);
    auto splitter = new QSplitter;
    splitter->addWidget(chartView);
    splitter->addWidget(inputs);
    splitter->setSizes(QList<int>{splitter->width()*2/3, splitter->width()/3});
    layout()->addWidget(splitter);
}

DualCalculatorPage::~DualCalculatorPage()
{
    delete d;
    d = nullptr;
}

