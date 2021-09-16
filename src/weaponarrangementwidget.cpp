#include "weaponarrangementwidget.h"

#include "ui_weaponarrangementwidget.h"

using namespace Calculators;

WeaponArrangementWidget::WeaponArrangementWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::WeaponArrangementWidget)
{
    // TODO: Fix the ugly hack in the .ui file (minimum size in the QGroupBox)
    ui->setupUi(this);
    ui->damageType->addItem(tr("Crushing"), DamageType::Crushing);
    ui->damageType->addItem(tr("Missile"),  DamageType::Missile);
    ui->damageType->addItem(tr("Piercing"), DamageType::Piercing);
    ui->damageType->addItem(tr("Slashing"), DamageType::Slashing);

    // Hide it for now. It uses space that I care a lot about right now.
    ui->damageDetail->hide();

    // TODO: this is maybe not worth it, as it's only the physical damage, and
    // I don't even look at this anymore. Could be replaced with an extra chart.
    // Additionally, this widget is unaware of the luck value, so the numbers
    // will never be _that_ correct anymore if luck is non-zero.
    auto proficiency = ui->proficiencyDamageModifier;
    auto dice = ui->weaponDamageDiceNumber;
    auto sides = ui->weaponDamageDiceSide;
    auto bonus = ui->weaponDamageDiceBonus;
    auto result = ui->damageDetail;

    auto calculateStats = [=]() {
        DiceRoll roll = DiceRoll()
                .number(dice->value())
                .sides(sides->value())
                .bonus(bonus->value() + proficiency->value());
        result->setText(tr("Min/Avg/Max: %1/%2/%3")
                        .arg(roll.minimum())
                        .arg(roll.average())
                        .arg(roll.maximum()));
    };

    connect(proficiency, qOverload<int>(&QSpinBox::valueChanged), calculateStats);
    connect(dice,        qOverload<int>(&QSpinBox::valueChanged), calculateStats);
    connect(sides,       qOverload<int>(&QSpinBox::valueChanged), calculateStats);
    connect(bonus,       qOverload<int>(&QSpinBox::valueChanged), calculateStats);
    calculateStats();
}

WeaponArrangementWidget::~WeaponArrangementWidget()
{
    delete ui;
}

WeaponArrangement WeaponArrangementWidget::toData() const
{
    WeaponArrangement result;

    result.proficiencyToHit = ui->proficiencyThac0Modifier->value();
    result.styleToHit = ui->styleModifier->value();
    result.weaponToHit = ui->weaponThac0Modifier->value();

    // TODO: consider having "style damage" on the UI to distinguish bonus
    // damage from proficiency (0-5) or from fighting style (0-1).
    result.proficiencyDamage = ui->proficiencyDamageModifier->value();
    result.damage.insert(ui->damageType->currentData().value<DamageType>(),
                         DiceRoll().number(ui->weaponDamageDiceNumber->value())
                                   .sides(ui->weaponDamageDiceSide->value())
                                   .bonus(ui->weaponDamageDiceBonus->value())
                         );

    for (auto [type, damage] : elementalDamages()) {
        // Avoid adding the damage if the amount is 0. But number() is not
        // enough, as a weapon like Varscona would do 0d2+1 cold damage.
        if ((damage.average() == 0 && damage.sigma() == 0) ||
            qFuzzyIsNull(damage.probability()))
            continue;
        result.damage.insert(type, damage);
    }

    result.attacks = attacksPerRound();
    result.criticalHit = ui->criticalHitChance->value();
    result.criticalMiss = ui->criticalMissChance->value();

    return result;
}

void WeaponArrangementWidget::setAsWeaponOne()
{
    ui->attacksPerRound2->hide();
}

void WeaponArrangementWidget::setAsWeaponTwo()
{
    ui->attacksPerRound1->hide();
    ui->styleModifier->setMinimum(-8);
}

void WeaponArrangementWidget::setAsBackstabWeapon()
{
    ui->damageGroup->setTitle(tr("Weapon damage"));
    ui->thac0Group->hide();
    ui->damageType->hide();
    ui->damageTypeLabel->hide();
    ui->attacksPerRound1->hide();
    ui->attacksPerRound2->hide();
    ui->attacksPerRoundLabel->hide();
    ui->criticalHitChance->hide();
    ui->criticalHitChanceLabel->hide();
}

QList<QPair<DamageType, DiceRoll>>
WeaponArrangementWidget::elementalDamages() const
{
    return {
        qMakePair(DamageType::Acid, ui->acidDamage->toData()),
        qMakePair(DamageType::Cold, ui->coldDamage->toData()),
        qMakePair(DamageType::Electricity, ui->electricityDamage->toData()),
        qMakePair(DamageType::Fire, ui->fireDamage->toData()),
        // TODO: MagicDamage, PoisonDamage.
    };
}

double WeaponArrangementWidget::attacksPerRound() const
{
    return !ui->attacksPerRound1->isHidden() ? ui->attacksPerRound1->value()
                                             : double(ui->attacksPerRound2->value());
}

