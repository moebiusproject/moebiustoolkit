#include "rollprobabilities.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QChartView>
#include <QDebug>
#include <QStackedBarSeries>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QtMath>

using namespace QtCharts;

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

RollProbabilities::RollProbabilities(QWidget* parent)
    : QDialog(parent)
{
    QStackedBarSeries* series = new QStackedBarSeries;

    enum { Sets = 9, From = 0, To = 100, Step = 5, Steps = (To-From)/Step +1 };

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

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis* axis = new QBarCategoryAxis();
    axis->append(axisTexts);
    // Ironically enough, the default axis is fine: rolls from 1 to 21.
    chart->createDefaultAxes();
    // But _add_ (not set) the custom one with percentages.
    chart->addAxis(axis, Qt::AlignBottom);
    if (QValueAxis* verticalAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first())) {
        verticalAxis->setTickCount(11);
        verticalAxis->setMinorTickCount(1);
        verticalAxis->setLabelFormat(QString::fromLatin1("%.1f"));
    }

    chart->setAcceptHoverEvents(true);
    QGraphicsSimpleTextItem* tooltip = new QGraphicsSimpleTextItem(chart);
    tooltip->setPos(30, 30);
    QObject::connect(series, &QAbstractBarSeries::hovered,
                     chart, [tooltip, series, results](bool over, int index, QBarSet* set)
    {
        if (!over) {
            tooltip->hide();
            return;
        }

        const int setIndex = series->barSets().indexOf(set);
        double value = 0.0;
        for (int i = 0; i <= setIndex; ++i)
            value += results[i].at(index);
        tooltip->show();
        tooltip->setText(tr("Value: %1").arg(value));
    });

    auto chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    setLayout(new QVBoxLayout);
    layout()->addWidget(chartView);
}
