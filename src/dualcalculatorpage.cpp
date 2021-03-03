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
#include "resourcemanager.h"
#include "tdafile.h"

#include "ui_dualcalculatorwidget.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QBuffer>
#include <QChartView>
#include <QDebug>
#include <QEvent>
#include <QStackedBarSeries>
#include <QThreadPool>
#include <QValueAxis>

using namespace QtCharts;

QDebug operator<<(QDebug dbg, QAbstractAxis* axis)
{
    if (!axis)
        return dbg << "QAbstractAxis(nullptr)", dbg;

    QDebugStateSaver saver(dbg);
    dbg.nospace();

    switch (axis->type())
    {
    case QAbstractAxis::AxisTypeNoAxis:
    case QAbstractAxis::AxisTypeValue:
    {
        auto value = static_cast<QValueAxis*>(axis);
        dbg << "QValueAxis("
            << " min " << value->min()
            << " max " << value->max()
            << ")"
               ;
        return dbg;
    }
    case QAbstractAxis::AxisTypeBarCategory:
    case QAbstractAxis::AxisTypeCategory:
    case QAbstractAxis::AxisTypeDateTime:
    case QAbstractAxis::AxisTypeLogValue:
        break;
    }
    dbg.maybeSpace() << static_cast<QObject*>(axis);

    return dbg;
}

struct DualCalculatorPage::Private
{
    Private(DualCalculatorPage& page) : parent(page) {}
    void load(const QString& path);
    void loaded();
    void addCalculation();
    void recalculate(int index);

    DualCalculatorPage& parent;

    ResourceManager manager;
    QString location;

    QChart* chart = nullptr;
    QBarSet* class1 = nullptr;
    QBarSet* class2 = nullptr;
    QValueAxis* xpAxis = nullptr;
    QBarCategoryAxis* namesAxis = nullptr;

    QVector<Ui::DualCalculatorWidget> widgets;
    QVBoxLayout* inputsLayout = nullptr;

    QHash<QString, QVector<quint32>> levels;
};

// Private /////////////////////////////////////////////////////////////////////

void DualCalculatorPage::Private::load(const QString& path)
{
    QThreadPool::globalInstance()->start( [this, path] { manager.load(path); } );
}

void DualCalculatorPage::Private::loaded()
{
    QByteArray data = manager.resource(QLatin1String("XPLEVEL.2DA"));
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    const TdaFile file = TdaFile::from(buffer);
    buffer.close();

    for (const QStringList& entry : file.entries) {
        const QString name = entry.first();

        // For now, just skip some of the ones we know are uninteresting for this
        if (name == QLatin1String("MONK") ||
            name == QLatin1String("PALADIN") ||
            name == QLatin1String("SORCERER") ||
            name == QLatin1String("SHAMAN") ||
            name == QLatin1String("BARD"))
            continue;

        QVector<QPointF> points;
        // TODO: check the validity of the string conversion to number.
        QVector<quint32> values;
        for (int level = 1; level < entry.count() && level <= 40; ++level) {
            const uint x = qMax(uint(1), entry.at(level).toUInt());
            points.append(QPointF(x, level));
            values.append(entry.at(level).toULong());
        }
        levels.insert(name, values);
    }

    for (auto& widget : widgets) {
        QSignalBlocker blocker1(widget.firstClass);
        QSignalBlocker blocker2(widget.secondClass);
        widget.firstClass->addItems(levels.keys());
        widget.secondClass->addItems(levels.keys());
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

    inputsLayout->addWidget(input);

    widget.firstClass->addItems(levels.keys());
    widget.secondClass->addItems(levels.keys());

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
        if (namesAxis->count() <= index)
            namesAxis->append(text);
        else
            namesAxis->replace(namesAxis->at(index), text);
    });
}

void DualCalculatorPage::Private::recalculate(int index)
{
    const Ui::DualCalculatorWidget& widget = widgets.at(index);
    const QString firstClass = widget.firstClass->currentText();
    const QString secondClass = widget.secondClass->currentText();
    const int firstLevel = widget.firstClassLevel->value();
    const int secondLevel = widget.secondClassLevel->value();

    // Level 1 is index=0 on the container, so subtract 1.
    const int xp1 = levels.value(firstClass).at(firstLevel - 1);
    const int xp2 = levels.value(secondClass).at(secondLevel - 1);

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
}


// Public //////////////////////////////////////////////////////////////////////

DualCalculatorPage::DualCalculatorPage(QWidget* parent)
    : QWidget(parent)
    , d(new Private(*this))
{
    connect(&d->manager, &ResourceManager::loaded, this, [this]{ d->loaded(); });

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

    // Main layout of this page: an horizontal row chart<->inputs
    setLayout(new QHBoxLayout);
    layout()->addWidget(chartView);
    layout()->addWidget(inputs);
}

DualCalculatorPage::~DualCalculatorPage()
{
    delete d;
    d = nullptr;
}

bool DualCalculatorPage::event(QEvent* event)
{
    // On first show, start browsing.
    if (event->type() == QEvent::Show && d->location.isEmpty())
        d->load(m_currentLocation);

    return QWidget::event(event);
}

