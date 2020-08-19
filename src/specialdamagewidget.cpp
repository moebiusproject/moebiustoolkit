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

#include "specialdamagewidget.h"

SpecialDamageWidget::SpecialDamageWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    // Disable "sides" if the number of dice is 0, so it's more natural to input
    // fixed elemental damage, like Varscona's/Ashideena's +1 (which is 0d2+1).
    // TODO: Disable other controls if probability is 0%.
    auto enabler = [this](int diceNumberValue) {
        sides->setEnabled(diceNumberValue != 0);
    };
    enabler(number->value());
    connect(number, qOverload<int>(&QSpinBox::valueChanged), enabler);

    connect(reset, &QPushButton::clicked, [this] {
        number->setValue(number->minimum());
        sides->setValue(sides->minimum());
        bonus->setValue(bonus->minimum());
        probability->setValue(probability->maximum());
    });
}

DiceRoll SpecialDamageWidget::toData() const
{
    return DiceRoll().number(number->value())
                     .sides(sides->value())
                     .bonus(bonus->value())
                     .probability(probability->value() / 100.0);
}
