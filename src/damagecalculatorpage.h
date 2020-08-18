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

#pragma once

#include <QWidget>

#include "ui_specialdamagewidget.h"
#include "basepage.h"
#include "calculators.h"

namespace Ui {
    class WeaponArrangementWidget;
}

class SpecialDamageWidget : public QWidget, public Ui::SpecialDamageWidget {
    Q_OBJECT

public:
    explicit SpecialDamageWidget(QWidget* parent = nullptr);
    DiceRoll toData() const;
};

class WeaponArrangementWidget : public QWidget {
    Q_OBJECT

public:
    explicit WeaponArrangementWidget(QWidget* parent = nullptr);
    ~WeaponArrangementWidget();

    Calculators::WeaponArrangement toData() const;

    Ui::WeaponArrangementWidget* ui;

    double attacksPerRound() const;
    double criticalHitChance() const;
    Calculators::DamageType damageType() const;

private:
    QList<QPair<Calculators::DamageType, DiceRoll>> elementalDamages() const;
};

class DamageCalculatorPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit DamageCalculatorPage(QWidget* parent = nullptr);
    ~DamageCalculatorPage();

protected:
    bool event(QEvent* event) override;

private:
    struct Private;
    Private* d;
};
