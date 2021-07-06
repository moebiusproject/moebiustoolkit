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

#include "basepage.h"

// TODO: Remove from this header, but requires adjusting promoted widgets.
#include "specialdamagewidget.h"
#include "weaponarrangementwidget.h"

class DamageCalculatorPage : public BasePage
{
    Q_OBJECT

public:
    explicit DamageCalculatorPage(QWidget* parent = nullptr);
    ~DamageCalculatorPage();

    QList<QtCharts::QChartView*> charts() const override;

protected:
    bool event(QEvent* event) override;

private:
    struct Private;
    Private* d;
};
