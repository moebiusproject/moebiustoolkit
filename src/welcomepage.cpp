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

#include "pagetype.h"
#include "welcomepage.h"

#include "ui_welcomepage.h"

struct WelcomePage::Private
{
    Ui::WelcomePage ui;
};

WelcomePage::WelcomePage(QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    d->ui.setupUi(this);

    connect(d->ui.damageCalculator, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::DamageCalculator); });
    connect(d->ui.backstabCalculator, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::BackstabCalculator); });
    connect(d->ui.repeatedProbability, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::RepeatedProbability); });
}
