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

struct MainWindow::Private
{
    Private(MainWindow& window)
        : parent(window)
    {}

    MainWindow& parent;
};

MainWindow::MainWindow(QWidget* parentWidget)
    : QMainWindow(parentWidget)
    , d(new Private(*this))
{
}

MainWindow::~MainWindow()
{
    delete d;
    d = nullptr;
}

