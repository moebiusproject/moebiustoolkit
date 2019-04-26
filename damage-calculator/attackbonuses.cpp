#include "attackbonuses.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QHorizontalBarSeries>
#include <QVBoxLayout>
#include <QValueAxis>

using namespace QtCharts;

AttackBonuses::AttackBonuses(QWidget* parent)
    : QDialog(parent)
{
    auto attack = new QBarSet(tr("Attack"));
    auto damage = new QBarSet(tr("Damage"));

    *attack << 0 << 1 << 1 << 1 << 2 << 2 << 2 << 3 << 4 << 4 << 5;
    *damage << 0 << 0 << 1 << 2 << 2 << 2 << 2 << 3 << 4 << 4 << 5;

    auto series = new QHorizontalBarSeries;
    series->append(attack);
    series->append(damage);

    auto chart = new QChart;
    chart->addSeries(series);
    chart->setTitle(tr("Bonuses to attack and damage"));
    chart->setAnimationOptions(QChart::AllAnimations);

    const QStringList variants = QStringList()
            << tr("Fighter")
            << tr("Helm of Balduran")
            << tr("Kensai 3")
            << tr("Legacy of the Masters")
            << tr("Kensai 6")
            << tr("Enraged Berserker")
            << tr("Helm of Balduran & Legacy of the Masters")
            << tr("Kensai 9")
            << tr("Kensai 12")
            << tr("Enraged Berserker + Helm of Balduran & Legacy of the Masters")
            << tr("Kensai 15")
            ;
    auto axisY = new QBarCategoryAxis;
    axisY->append(variants);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    auto axisX = new QValueAxis;
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    axisX->applyNiceNumbers();

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    auto chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    setLayout(new QVBoxLayout);
    layout()->addWidget(chartView);
}
