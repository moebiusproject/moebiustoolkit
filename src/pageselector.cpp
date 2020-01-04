/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019-2020 Alejandro Exojo Piqueras
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

#include "pageselector.h"

#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>

struct PageSelector::Private
{
    PageSelector& parent;
    QVBoxLayout* layout;

    Private(PageSelector& parent)
        : parent(parent)
    {}

    void setupButton(QPushButton* button);
};

PageSelector::PageSelector(QWidget* parent)
    : QFrame(parent)
    , d(new Private(*this))
{
    setFrameStyle(QFrame::Box);

    d->layout = new QVBoxLayout(this);

    auto button = new QPushButton(this);
    d->layout->addWidget(button);
    d->layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum,
                                             QSizePolicy::MinimumExpanding));

    d->setupButton(button);
    button->setText(tr("Welcome"));

    setLayout(d->layout);
}

void PageSelector::addButton(const QString& text)
{
    auto button = new QPushButton(this);
    d->layout->insertWidget(d->layout->count() - 1, button);
    d->setupButton(button);
    button->setText(text);
}

void PageSelector::Private::setupButton(QPushButton *button)
{
    button->setAutoExclusive(true);
    button->setCheckable(true);
    button->setChecked(true);
    button->setMinimumHeight(100);

    const int index = layout->count() - 2; // -2 because of the QSpacerItem.
    QObject::connect(button, &QAbstractButton::toggled,
                     &parent, [this, index](bool checked) {
        if (checked)
            emit parent.buttonActivated(index);
    });
}
