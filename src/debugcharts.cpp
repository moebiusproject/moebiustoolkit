/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2021 Alejandro Exojo Piqueras
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

#include "debugcharts.h"

#include <QAbstractAxis>
#include <QLineSeries>
#include <QValueAxis>

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
using namespace QtCharts;
#endif

QDebug operator<<(QDebug dbg, QAbstractAxis* axis)
{
    if (!axis)
        return dbg << "QAbstractAxis(nullptr)", dbg;

    QDebugStateSaver saver(dbg);
    dbg.nospace();

    switch (axis->type())
    {
    case QAbstractAxis::AxisTypeNoAxis:
    case QAbstractAxis::AxisTypeValue:
    {
        auto value = static_cast<QValueAxis*>(axis);
        dbg << "QValueAxis("
            << "title " << value->titleText()
            << " min " << value->min()
            << " max " << value->max()
            << ")";
        return dbg;
    }
    case QAbstractAxis::AxisTypeBarCategory:
    case QAbstractAxis::AxisTypeCategory:
    case QAbstractAxis::AxisTypeDateTime:
    case QAbstractAxis::AxisTypeLogValue:
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    case QAbstractAxis::AxisTypeColor:
#endif
        break;
    }
    dbg.maybeSpace() << static_cast<QObject*>(axis);

    return dbg;
}

QDebug operator<<(QDebug dbg, QLineSeries* series)
{
    if (!series)
        return dbg << "QAbstractAxis(nullptr)", dbg;

    QDebugStateSaver saver(dbg);
    dbg.nospace();

    dbg << "QLineSeries("
        << "name " << series->name()
        << " first " << series->points().first()
        << " last " << series->points().last()
        << ")";
    return dbg;
}
