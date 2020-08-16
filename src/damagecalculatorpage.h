#pragma once

#include <QWidget>

#include "ui_specialdamagewidget.h"
#include "calculators.h"

// TODO: REMOVE this porting workaround.
class QMenuBar;
class QStatusBar;

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

class DamageCalculatorPage : public QWidget
{
    Q_OBJECT

public:
    explicit DamageCalculatorPage(QWidget* parent = nullptr);
    ~DamageCalculatorPage();

protected:
    bool event(QEvent* event) override;

private:
    // TODO: REMOVE this porting workaround.
    QMenuBar* menuBar();
    QStatusBar* statusBar();
    struct Private;
    Private* d;
};
