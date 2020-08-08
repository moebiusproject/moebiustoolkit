#include "backstabcalculatorpage.h"

#include <QBoxLayout>
#include <QChart>
#include <QChartView>
#include <QHorizontalStackedBarSeries>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>

#include "ui_backstabsetup.h"

using namespace QtCharts;

using Series = QHorizontalStackedBarSeries;

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
    QVector<Series*> series;

    void newPage();
    void updateSeriesAtCurrentIndex() {
#if 0
        const int index = tabs->currentIndex();
        Series* series = this->series[index];
        Ui::BackstabSetup setup = setups[index];
        // TODO: Calculate here, and assign to the series the values.
#endif
    }
};

BackstabCalculatorPage::BackstabCalculatorPage(QWidget *parent)
    : QWidget(parent)
    , d(new Private(*this))
{
    auto chartControlsLayout = new QHBoxLayout;
    chartControlsLayout->addWidget(new QLabel(tr("TODO")));

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
            return; // don't close the last one for now, to keep the "New" button
        d->setups.removeAt(index);
        d->chart->removeSeries(d->series[index]);
        delete d->series.takeAt(index);
//        // TODO: with the new approach, this might be a tad heavy. Review.
//        d->setupAxes();
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
    // TODO: Better looking when no scrollbar only... re-consider it.
    // inputArea->setFrameShape(QFrame::NoFrame);
    inputArea->setWidgetResizable(true);
    inputLayout->addWidget(d->tabs);

    auto layout = new QHBoxLayout;
    setLayout(layout);
    layout->addLayout(chartViewLayout, 1);
    layout->addWidget(inputArea, 0);

    d->newPage();
}

BackstabCalculatorPage::~BackstabCalculatorPage()
{
    delete d;
    d = nullptr;
}

void BackstabCalculatorPage::Private::newPage()
{
    auto widget = new QWidget;

    setups.append(Ui::BackstabSetup());
    Ui::BackstabSetup& setup = setups.last();
    setup.setupUi(widget);
    tabs->addTab(widget, tr("Setup %1").arg(tabs->count() + 1));
    tabs->setCurrentWidget(widget);

    connect(setup.name, &QLineEdit::textChanged, [this](const QString& text) {
        series.at(tabs->currentIndex())->setName(text);
    });

    auto update =  std::bind(&Private::updateSeriesAtCurrentIndex, this);
    for (auto child : widget->findChildren<QSpinBox*>())
        connect(child, qOverload<int>(&QSpinBox::valueChanged), update);
    for (auto child : widget->findChildren<QDoubleSpinBox*>())
        connect(child, qOverload<double>(&QDoubleSpinBox::valueChanged), update);
//    for (auto child : widget->findChildren<QComboBox*>())
//        connect(child, &QComboBox::currentTextChanged, update);
//    for (auto child : widget->findChildren<QCheckBox*>())
//        connect(child, &QCheckBox::toggled, update);
//    connect(configuration.offHandGroup, &QGroupBox::toggled, update);
}

