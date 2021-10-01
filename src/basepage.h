/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020-2021 Alejandro Exojo Piqueras
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

#pragma once

#include <QList>
#include <QString>
#include <QWidget>

class QMenu;
class QMenuBar;
class QStatusBar;

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
namespace QtCharts {
    class QChartView;
}
using QChartView = QtCharts::QChartView;
#else
class QChartView;
#endif

class BasePage : public QWidget
{
    Q_OBJECT
public:
    explicit BasePage(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual QList<QMenu*> makeMenus() { return QList<QMenu*>(); };
    virtual QList<QChartView*> charts() const { return {}; }

protected:
    QMenuBar* menuBar();
    QStatusBar* statusBar();
    static QString m_currentName;
    static QString m_currentLocation;
private:
    static QMenuBar* m_menuBar;
    static QStatusBar* m_statusBar;
    friend class MainWindow;
};
