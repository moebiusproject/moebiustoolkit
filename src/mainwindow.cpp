/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019 Alejandro Exojo Piqueras
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
#include "damagecalculatorpage.h"
#include "pageselector.h"
#include "pagetype.h"
#include "welcomepage.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QStackedWidget>

#include <functional>

struct MainWindow::Private
{
    Private(MainWindow& window)
        : parent(window)
    {}

    void addNewPage(PageType type);

    MainWindow& parent;
    PageSelector* selector;
    WelcomePage* welcomePage;
    QStackedWidget* view;
};

MainWindow::MainWindow(QWidget* parentWidget)
    : QMainWindow(parentWidget)
    , d(new Private(*this))
{
    d->view = new QStackedWidget(this);

    d->selector = new PageSelector(this);
    connect(d->selector, &PageSelector::buttonActivated, this, [this](int index) {
        d->view->setCurrentIndex(index);
    });

    d->welcomePage = new WelcomePage(this);
    d->view->addWidget(d->welcomePage);
    connect(d->welcomePage, &WelcomePage::newPageRequested,
            std::bind(&Private::addNewPage, d, std::placeholders::_1));

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
    switch (type) {
    case PageType::DamageCalculator: {
        view->addWidget(new DamageCalculatorPage(&parent));
        selector->addButton(tr("Damage\nCalculator"));
        break;
    }
    case PageType::BackstabCalculator:
        view->addWidget(new BackstabCalculatorPage(&parent));
        selector->addButton(tr("Backstab\nCalculator"));
        break;
    case PageType::RepeatedProbability:
        break;
    }

    view->setCurrentIndex(view->count() - 1);

}
