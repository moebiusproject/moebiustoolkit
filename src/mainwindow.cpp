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

#include "mainwindow.h"

#include "backstabcalculatorpage.h"
#include "buffcalculatorpage.h"
#include "damagecalculatorpage.h"
#include "dualcalculatorpage.h"
#include "gamebrowserpage.h"
#include "pageselector.h"
#include "pagetype.h"
#include "progressionchartspage.h"
#include "repeatedprobabilitypage.h"
#include "welcomepage.h"

#include <QBuffer>
#include <QChart>
#include <QChartView>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStackedWidget>

#include <functional>

struct MainWindow::Private
{
    Private(MainWindow& window)
        : parent(window)
    {}

    void addNewPage(PageType type);
    void updateMainMenu();

    MainWindow& parent;
    PageSelector* selector;
    WelcomePage* welcomePage;
    QStackedWidget* view;

    QHash<QWidget*, QList<QMenu*>> pageMenus;
    QAction* rotateChartTheme = nullptr;
    QAction* copyChartToClipboard = nullptr;
    QAction* saveChartAs = nullptr;

    QMenu* mainMenu = nullptr;
};

MainWindow::MainWindow(QWidget* parentWidget)
    : QMainWindow(parentWidget)
    , d(new Private(*this))
{
    BasePage::m_menuBar = menuBar();
    BasePage::m_statusBar = statusBar();

    //
    // Setup application-wide menu ("Moebius Toolkit").
    //
    d->mainMenu = menuBar()->addMenu(tr("Moebius Toolkit"));

    auto action = new QAction(tr("Change chart theme"), this);
    d->rotateChartTheme = action;
    action->setShortcut(tr("Ctrl+T"));
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, this, [this]() {
        auto page = qobject_cast<BasePage*>(d->view->currentWidget());
        for (QtCharts::QChartView* view : page->charts()) {
            int themeInt = view->chart()->theme() + 1;
            themeInt = themeInt % (QtCharts::QChart::ChartThemeQt + 1);
            view->chart()->setTheme(QtCharts::QChart::ChartTheme(themeInt));
        }
    });
    action->setEnabled(false); // Starts disabled. No charts on the welcome.

    action = new QAction(tr("Copy chart to clipboard"), this);
    d->copyChartToClipboard = action;
    action->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, this, [this]() {
        auto page = qobject_cast<BasePage*>(d->view->currentWidget());
        if (page->charts().isEmpty())
            return;
        const QPixmap pixmap = page->charts().first()->grab();
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setPixmap(pixmap);
    });
    action->setEnabled(false); // Starts disabled. No charts on the welcome.

    action = new QAction(tr("Save chart as..."), this);
    d->saveChartAs = action;
    action->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered, [this] {
        auto page = qobject_cast<BasePage*>(d->view->currentWidget());
        if (page->charts().isEmpty())
            return;
        const QPixmap pixmap = page->charts().first()->grab();
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
    action->setEnabled(false); // Starts disabled. No charts on the welcome.

    d->mainMenu->addSeparator();

    action = new QAction(tr("Show full screen"), this);
    action->setShortcut(QKeySequence::FullScreen);
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered,
            this, [this]{ setWindowState(windowState() ^ Qt::WindowFullScreen); });

    action = new QAction(tr("Quit"), this);
    action->setShortcut(QKeySequence::Quit);
    d->mainMenu->addAction(action);
    connect(action, &QAction::triggered,
            qApp, &QCoreApplication::quit, Qt::QueuedConnection);

    d->view = new QStackedWidget(this);

    d->selector = new PageSelector(this);
    connect(d->selector, &PageSelector::buttonActivated, this, [this](int index) {
        QWidget* currentWidget = d->view->currentWidget();
        for (QMenu* menu : d->pageMenus.value(currentWidget)) {
            menu->menuAction()->setVisible(false);
        }

        d->view->setCurrentIndex(index);

        currentWidget = d->view->currentWidget();
        for (QMenu* menu : d->pageMenus.value(currentWidget)) {
            menu->menuAction()->setVisible(true);
        }

        d->updateMainMenu();
    });

    d->welcomePage = new WelcomePage(this);
    d->view->addWidget(d->welcomePage);

    using namespace std::placeholders;
    connect(d->welcomePage, &WelcomePage::newPageRequested,
            std::bind(&Private::addNewPage, d, _1));

    auto central = new QWidget(this);
    central->setLayout(new QHBoxLayout(central));
    central->layout()->addWidget(d->selector);
    central->layout()->addWidget(d->view);
    setCentralWidget(central);
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}

void MainWindow::Private::addNewPage(PageType type)
{
    BasePage::m_currentName = welcomePage->gameName();
    BasePage::m_currentLocation = welcomePage->gameLocation();
#ifdef Q_OS_WASM
    Q_ASSERT(BasePage::m_currentName.isEmpty());
    Q_ASSERT(BasePage::m_currentLocation.isEmpty());
#endif

    switch (type) {
    case PageType::BackstabCalculator:
        view->addWidget(new BackstabCalculatorPage(&parent));
        selector->addButton(tr("Backstab\nCalculator"));
        break;
    case PageType::BuffCalculator:
        view->addWidget(new BuffCalculatorPage(&parent));
        selector->addButton(tr("Buff\nCalculator"));
        break;
    case PageType::DamageCalculator:
        view->addWidget(new DamageCalculatorPage(&parent));
        selector->addButton(tr("Damage\nCalculator"));
        break;
    case PageType::DualCalculator:
        view->addWidget(new DualCalculatorPage(&parent));
        selector->addButton(tr("Dual XP\nCalculator"));
        break;
    case PageType::GameBrowser:
        view->addWidget(new GameBrowserPage(&parent));
        selector->addButton(tr("Game\nBrowser"));
        break;
    case PageType::ProgressionCharts:
        view->addWidget(new ProgressionChartsPage(&parent));
        selector->addButton(tr("Progression\nCharts"));
        break;
    case PageType::RepeatedProbability:
        view->addWidget(new RepeatedProbabilityPage(&parent));
        selector->addButton(tr("Repated\nProbability"));
        break;
    }

    view->setCurrentIndex(view->count() - 1);

    updateMainMenu();
    auto page = qobject_cast<BasePage*>(view->currentWidget());
    const QList<QMenu*> newMenus = page->makeMenus();
    if (newMenus.isEmpty())
        return;
    pageMenus.insert(view->currentWidget(), newMenus);
    for (auto pageMenu : newMenus)
        parent.menuBar()->addMenu(pageMenu);
}

void MainWindow::Private::updateMainMenu()
{
    auto page = qobject_cast<BasePage*>(view->currentWidget());
    const bool hasCharts = !page->charts().isEmpty();
    rotateChartTheme->setEnabled(hasCharts);
    copyChartToClipboard->setEnabled(hasCharts);
    saveChartAs->setEnabled(hasCharts);
#ifdef Q_OS_WASM
    // Seems that it only supports copying text for now. :-/
    // https://developer.mozilla.org/en-US/docs/Web/API/Clipboard_API
    copyChartToClipboard->setEnabled(false);
#endif

}
