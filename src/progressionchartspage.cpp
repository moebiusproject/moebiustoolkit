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

#include "progressionchartspage.h"

#include "ui_progressionchartswidget.h"

#include "xplevels.h"
#include "debugcharts.h"

#include <QAbstractAxis>
#include <QBoxLayout>
#include <QChart>
#include <QChartView>
#include <QDataWidgetMapper>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QLegendMarker>
#include <QLineSeries>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QStyledItemDelegate>
#include <QValueAxis>

using namespace QtCharts;

enum Progression {
    SingleClass = 0, DoubleClass = 1, TripleClass = 2
};

enum ChartType {
    LevelType = 0, Thac0Warrior = 1, Thac0Priest = 2, Thac0Rogue = 3, Thac0Wizard = 4
};


class CustomDelegate : public QStyledItemDelegate
{
public:
    explicit CustomDelegate(QObject* parentObject = nullptr)
        : QStyledItemDelegate(parentObject)
    {}

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        if (auto comboBox = qobject_cast<QComboBox*>(editor))
        {
            const QVariant metaData = index.data(Qt::UserRole);
            comboBox->setCurrentIndex(metaData.toInt());
        }
        else
            QStyledItemDelegate::setEditorData(editor, index);
    }


    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override
    {
        if (auto comboBox = qobject_cast<QComboBox*>(editor))
        {
            const int currentIndex = comboBox->currentIndex();
            const QString currentText = comboBox->currentText();
            model->setData(index, currentText,  Qt::EditRole);
            model->setData(index, currentIndex, Qt::UserRole);
        }
        else
            QStyledItemDelegate::setModelData(editor, model, index);
    }

};

// TODO: Logarithmic steps, or something adjusted to the visible chart.
class SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget* parentObject = nullptr)
        : QSpinBox(parentObject)
    {
        setGroupSeparatorShown(true);
    }

    void stepBy(int step) override
    {
        QSpinBox::stepBy(step);
    }

};

struct ProgressionChartsPage::Private
{
    Private(ProgressionChartsPage& page, const QString& path)
        : parent(page)
        , xplevels(path)
    {}

    ProgressionChartsPage& parent;

    QChart* chart = nullptr;
    QChartView* chartView = nullptr;
    QValueAxis* xpAxis = nullptr;
    QValueAxis* levelAxis = nullptr;
    QValueAxis* thac0Axis = nullptr;
    QVector<QLineSeries*> lineSeries;

    SpinBox* minimumX = nullptr;
    SpinBox* maximumX = nullptr;
    Ui::ProgressionChartsWidget ui;
    QDataWidgetMapper* mapper;

    XpLevels xplevels;

    void addNew();
    void loaded();
    void setupAxes();
    void updateSeries(int index);
    void updateSeriesAtCurrentIndex() {
        updateSeries(mapper->currentIndex());
    }
    void updateAllSeries() {
        for (int index = 0; index < ui.table->model()->rowCount(); ++index) {
            updateSeries(index);
        }
    }
};

// Private /////////////////////////////////////////////////////////////////////

void ProgressionChartsPage::Private::addNew()
{
    auto model = ui.table->model();
    model->insertRow(model->rowCount(), QModelIndex());
    auto series = new QLineSeries;
    lineSeries.append(series);
    chart->addSeries(series);
    mapper->toNext();

    //
    // TODO: this is duplicated from the damage calculator. Move somewhere shared.
    //
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
        parent.statusBar()->showMessage(tr("%1. Value: %L2. XP: %L3") .arg(series->name())
                                        .arg(static_cast<long>(point.y()))
                                        .arg(static_cast<long>(point.x())), 5000);
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
}

void ProgressionChartsPage::Private::loaded()
{
    ui.className->addItems(xplevels.classes());
    ui.className->setCurrentIndex(0);

    // Now that we have loaded data we can connect and set/change values.
    connect(minimumX, qOverload<int>(&QSpinBox::valueChanged), minimumX, [this](int value) {
        setupAxes();
        maximumX->setMinimum(value);
    });
    connect(maximumX, qOverload<int>(&QSpinBox::valueChanged), maximumX, [this](int value) {
        setupAxes();
        minimumX->setMaximum(value);
    });

    auto updateCurrent = [this]() {
        // We submit manually because the automatic mode only works on focus
        // change. We want updates even sooner. So we call submit ourselves.
        mapper->submit();
        updateSeriesAtCurrentIndex();
        ui.table->resizeColumnsToContents();
        // TODO: This works well only on the startup. Here it seems to do nothing.
        ui.table->horizontalHeader()->setStretchLastSection(true);
    };
    connect(ui.className, &QComboBox::currentTextChanged, updateCurrent);
    connect(ui.progression, &QComboBox::currentTextChanged, updateCurrent);
    connect(ui.type, &QComboBox::currentTextChanged, updateCurrent);
    connect(ui.thac0Bonus, &QSpinBox::valueChanged, updateCurrent);
    connect(ui.name, &QLineEdit::textChanged, updateCurrent);
    connect(ui.add, &QPushButton::clicked, [this] { addNew(); });

    mapper = new QDataWidgetMapper(&parent);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    // The previous delegate has the mapper as a parent, so it will be deleted.
    mapper->setItemDelegate(new CustomDelegate(mapper));
    mapper->setModel(ui.table->model());
    mapper->addMapping(ui.className, 0);
    mapper->addMapping(ui.progression, 1);
    mapper->addMapping(ui.type, 2);
    mapper->addMapping(ui.thac0Bonus, 3);
    mapper->addMapping(ui.name, 4);
    ui.table->resizeColumnsToContents();
    ui.table->horizontalHeader()->setStretchLastSection(true);

    connect(ui.table, &QTableWidget::itemSelectionChanged, [this] {
        const QList<QTableWidgetItem*> selection =  ui.table->selectedItems();
        if (selection.size() > 0) {
            mapper->setCurrentIndex(selection.first()->row());
        }
    });

    addNew();
    mapper->submit();
    updateSeries(0);
}

// Updates min/max/range values according to direct user input (spinboxes) or
// the values in the chart.
void ProgressionChartsPage::Private::setupAxes()
{
    xpAxis->setMin(minimumX->value());
    xpAxis->setMax(maximumX->value());

    auto setYAxisRange = [this](QValueAxis* yAxis) {
        double min = +qInf();
        double max = -qInf();
        for (QLineSeries* series : qAsConst(lineSeries)) {
            if (!series->attachedAxes().contains(yAxis))
                continue;
            const QVector<QPointF> points = series->pointsVector();
            for (auto point : points) {
                if (point.x() > maximumX->value() || point.x() < minimumX->value())
                    continue;
                min = qMin(point.y(), min);
                max = qMax(point.y(), max);
            }
        }
        yAxis->setRange(min, max);
        yAxis->applyNiceNumbers();
    };
    setYAxisRange(levelAxis);
    setYAxisRange(thac0Axis);
}

void ProgressionChartsPage::Private::updateSeries(int index)
{
    QLineSeries* series = lineSeries.at(index);
    QAbstractItemModel* model = ui.table->model();

    // Extract data from the model that corresponds to this series.
    const QString className = model->data(model->index(index, 0)).toString();
    const int progressionInt = model->data(model->index(index, 1), Qt::UserRole).toInt();
    const Progression progression = Progression(progressionInt);
    const int typeInt = model->data(model->index(index, 2), Qt::UserRole).toInt();
    const ChartType type = ChartType(typeInt);
    const int thac0BonusDenominator = model->data(model->index(index, 3)).toInt();
    const QString name = model->data(model->index(index, 4)).toString();

    auto pen = series->pen();
    pen.setStyle(progression == SingleClass ? Qt::SolidLine
               : progression == DoubleClass ? Qt::DashDotLine : Qt::DashDotDotLine);
    series->setPen(pen);

    series->setName(name.isEmpty() ? className : name);

    const QVector<quint32> values = xplevels.thresholds(className);
    QVector<QPointF> points;
    const int xpScale = progression + 1;
    for (int level = 0; level < values.count() && level <= 40; ++level) {
        const quint32 xp = values.at(level);
        const quint32 x = xp * xpScale;
        if (type == LevelType)
            points.append(QPointF(x, level+1));
        else { // THAC0
            // Stops improving at level 22, except for Rogues, who for some
            // reason stop at 21 (THAC0=10), or Warriors, who are capped at
            // THAC0=0 (bounded below).
            const int levelCap = type == Thac0Rogue ? 21 : 22;
            const int validLevel = qMin(level, levelCap);
            int thac0 = 20;
            if (type == Thac0Warrior) // 1/1 ratio
                thac0 -= validLevel;
            else if (type == Thac0Priest) // 2/3 ratio
                thac0 -= 2*(validLevel/3);
            else if (type == Thac0Rogue) // 1/2 ratio
                thac0 -= validLevel/2;
            else if (type == Thac0Wizard) // 1/3 ratio
                thac0 -= validLevel/3;
            thac0 = qBound(0, thac0, 20);
            // The THAC0 gets capped at 0, but the bonuses from kits do not.
            if (thac0BonusDenominator) { // Check division by 0!
                const int bonus = (validLevel+1) / thac0BonusDenominator;
                thac0 -= bonus;
            }
            points.append(QPointF(x, thac0));
        }
    }
    series->replace(points);
    series->setPointsVisible(true);

    // FIXME: There is something wrong in the axis setup. The axis don't seem to
    // really be attached when I intend to. Additionally, this thing gets called
    // more than needed, and the reason is likely a messed up connection on the
    // QDataWidgetMapper.

    // TODO: This series of calls to contains and attach and detach would be
    // simpler if QAbstractSeries would not print a warning on the calls. We
    // can only fix it on the QtCharts side.
    const QList<QAbstractAxis*> attached = series->attachedAxes();
    if (!attached.contains(xpAxis))
        series->attachAxis(xpAxis);
    if (type == LevelType) { // level
        if (attached.contains(thac0Axis))
            series->detachAxis(thac0Axis);
        if (!attached.contains(levelAxis))
            series->attachAxis(levelAxis);
    } else {
        if (attached.contains(levelAxis))
            series->detachAxis(levelAxis);
        if (!attached.contains(thac0Axis))
            series->attachAxis(thac0Axis);
    }

    setupAxes();
}


// Public //////////////////////////////////////////////////////////////////////

ProgressionChartsPage::ProgressionChartsPage(QWidget* parent)
    : BasePage(parent)
    , d(new Private(*this, m_currentLocation))
{
    connect(&d->xplevels, &XpLevels::loaded, this, [this]{ d->loaded(); });

    auto chartControlsLayout = new QHBoxLayout;
    chartControlsLayout->addWidget(new QLabel(tr("Game: %1 (%2)")
                                              .arg(m_currentName, m_currentLocation)));
    d->minimumX = new SpinBox;
    d->minimumX->setMinimum(0);
    d->minimumX->setMaximum(8000000);
    d->minimumX->setValue(d->minimumX->minimum());
    d->minimumX->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
    chartControlsLayout->addWidget(d->minimumX);
    d->maximumX = new SpinBox;
    d->maximumX->setMinimum(0);
    d->maximumX->setMaximum(8000000);
    d->maximumX->setValue(d->maximumX->maximum());
    d->maximumX->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
    chartControlsLayout->addWidget(d->maximumX);
    // TODO: A line input to change the chart title.

    d->chart = new QChart;
    d->chart->setAnimationOptions(QChart::SeriesAnimations);
    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    d->xpAxis = new QValueAxis;
    d->levelAxis = new QValueAxis;
    d->thac0Axis = new QValueAxis;
    d->chart->addAxis(d->xpAxis, Qt::AlignBottom);
    d->chart->addAxis(d->levelAxis, Qt::AlignLeft);
    d->chart->addAxis(d->thac0Axis, Qt::AlignLeft);
    d->xpAxis->setTitleText(tr("Experience"));
    d->levelAxis->setTitleText(tr("Level"));
    d->thac0Axis->setTitleText(tr("THAC0"));
    d->xpAxis->setLabelFormat(QLatin1String("%i"));
    d->levelAxis->setLabelFormat(QLatin1String("%i"));
    d->thac0Axis->setLabelFormat(QLatin1String("%i"));

    auto chartViewLayout = new QVBoxLayout;
    chartViewLayout->addLayout(chartControlsLayout);
    chartViewLayout->addWidget(d->chartView);
    auto chartViewWrapper = new QWidget;
    chartViewWrapper->setLayout(chartViewLayout);

    auto inputs = new QWidget;
    d->ui.setupUi(inputs);
    // TODO: Implement the removal feature.
    d->ui.remove->hide();

    d->ui.thac0Bonus->setEnabled(false);
    connect(d->ui.type, &QComboBox::currentIndexChanged, this, [this](int index) {
        d->ui.thac0Bonus->setEnabled(index != 0);
    });

    setLayout(new QHBoxLayout);
    auto splitter = new QSplitter;
    splitter->addWidget(chartViewWrapper);
    splitter->addWidget(inputs);
    layout()->addWidget(splitter);
}

ProgressionChartsPage::~ProgressionChartsPage()
{
    delete d;
    d = nullptr;
}

QList<QChartView *> ProgressionChartsPage::charts() const
{
    return { d->chartView };
}


#include "progressionchartspage.moc"
