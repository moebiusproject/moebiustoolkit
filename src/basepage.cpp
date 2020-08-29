/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020 Alejandro Exojo Piqueras
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

#include "basepage.h"

#include <QMenuBar>
#include <QStatusBar>

QString BasePage::m_currentName;
QString BasePage::m_currentLocation;

QMenuBar* BasePage::m_menuBar = nullptr;
QStatusBar* BasePage::m_statusBar = nullptr;

QMenuBar* BasePage::menuBar()
{
    return m_menuBar;
}

QStatusBar* BasePage::statusBar()
{
    return m_statusBar;
}
