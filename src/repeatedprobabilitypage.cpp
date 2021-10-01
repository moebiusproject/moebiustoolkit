/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019-2021 Alejandro Exojo Piqueras
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

#include "repeatedprobabilitypage.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QChartView>
#include <QDebug>
#include <QStackedBarSeries>
#include <QTimer>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QtMath>

#include <array>

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
using namespace QtCharts;
#endif

static unsigned long factorial(unsigned n)
{
    return (n == 0 || n == 1) ? 1 : n*factorial(n-1);
}

static unsigned long binomial(unsigned n, unsigned k)
{
    return factorial(n) / ( factorial(k) * factorial(n-k));
}

static double binomialFunction(unsigned k, unsigned n, double p)
{
    return binomial(n, k) * qPow(p, k) * qPow(1-p, n-k);
}

struct RepeatedProbabilityPage::Private
{
    Private(RepeatedProbabilityPage& window)
        : q(window)
    {}

    RepeatedProbabilityPage& q;
    QChart* chart = nullptr;
    QChartView* chartView = nullptr;
};

// TODO: Unroll the mess of the constructor to be able to regenerate teh chart
// with different values, starting with different count of sets (not fixed to 9).

RepeatedProbabilityPage::RepeatedProbabilityPage(QWidget* parent)
    : BasePage(parent)
    , d(new Private(*this))
{
    QStackedBarSeries* series = new QStackedBarSeries;

    enum { Sets = 9, From = 0, To = 100, Step = 5, Steps = (To-From)/Step +1 };

    // TODO: QBarSet::append only takes a QList, not a QVector like the line series.
    // Consider improving upstream.
    std::array<QList<double>, Sets> results;
    QStringList axisTexts;

    for (int rolls = 1; rolls <= Sets; ++rolls) {
        for (int rollProbability = From; rollProbability <= To; rollProbability += Step) {
            axisTexts.append(QString::fromLatin1("%1%").arg(rollProbability));

            // Probability for at least one success: [1, rolls]
            double probability = 0.0;
            for (int successes = 1; successes <= rolls; ++successes) {
                probability += binomialFunction(successes, rolls, rollProbability/100.0);
            }
            // But... to plot it stacked on top of the previous, we need to substract
            // the probability of all the previous ones.
            for (int x = rolls; x > 1; --x) {
                auto previousResults = results[x-2];
                probability -= previousResults.at(results[rolls-1].size());
            }
            results[rolls-1].append(probability);
        }

        QBarSet* set = new QBarSet(QString());
        set->setLabel(tr("Rolls: %1").arg(rolls));
        set->append(results[rolls-1]);
        series->append(set);
    }

    d->chart = new QChart();
    d->chart->addSeries(series);
    d->chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis* axis = new QBarCategoryAxis();
    axis->append(axisTexts);
    // Ironically enough, the default axis is fine: rolls from 1 to 21.
    d->chart->createDefaultAxes();
    // But _add_ (not set) the custom one with percentages.
    d->chart->addAxis(axis, Qt::AlignBottom);
    if (QValueAxis* verticalAxis = qobject_cast<QValueAxis*>(d->chart->axes(Qt::Vertical).constFirst())) {
        verticalAxis->setTickCount(11);
        verticalAxis->setMinorTickCount(1);
        verticalAxis->setLabelFormat(QString::fromLatin1("%.1f"));
    }

    d->chart->setAcceptHoverEvents(true);
    QGraphicsSimpleTextItem* tooltip = new QGraphicsSimpleTextItem(d->chart);
    tooltip->setPos(30, 30);
    QFont tooltipFont = tooltip->font();
    tooltipFont.setPointSize(tooltipFont.pointSize() + 4);
    tooltip->setFont(tooltipFont);
    QTimer* hideTimer = new QTimer(this);
    hideTimer->setInterval(1000);
    connect(hideTimer, &QTimer::timeout, series, [tooltip] { tooltip->hide(); });
    QObject::connect(series, &QAbstractBarSeries::hovered,
                     d->chart, [hideTimer, tooltip, series, results](bool over, int index, QBarSet* set)
    {
        if (!over) {
            hideTimer->start();
            return;
        }
        hideTimer->stop();

        const int setIndex = series->barSets().indexOf(set);
        double value = 0.0;
        for (int i = 0; i <= setIndex; ++i)
            value += results[i].at(index);
        tooltip->show();
        tooltip->setText(tr("Value: %1").arg(value));
    },
    // To avoid a crash at shutdown: chart destroys the tooltip before the bar,
    // so the bar emits the hovered signal to a destroyed tooltip, so crashes.
    Qt::QueuedConnection);

    d->chartView = new QChartView(d->chart);
    d->chartView->setRenderHint(QPainter::Antialiasing);

    setLayout(new QVBoxLayout);
    layout()->addWidget(d->chartView);
}

RepeatedProbabilityPage::~RepeatedProbabilityPage()
{
    delete d;
    d = nullptr;
}

QList<QChartView*> RepeatedProbabilityPage::charts() const
{
    return { d->chartView };
}
