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

#include "calculators.h"

namespace Ui {
    class WeaponArrangementWidget;
}

class WeaponArrangementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WeaponArrangementWidget(QWidget* parent = nullptr);
    ~WeaponArrangementWidget();

    // Serialize to a struct.
    Calculators::WeaponArrangement toData() const;
    void setAsWeaponOne();
    void setAsWeaponTwo();
    void setAsBackstabWeapon();

private:
    Ui::WeaponArrangementWidget* ui;
    double attacksPerRound() const;
    QList<QPair<Calculators::DamageType, DiceRoll>> elementalDamages() const;
};

