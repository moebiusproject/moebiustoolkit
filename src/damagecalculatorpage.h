#pragma once

#include <QWidget>

#include "ui_specialdamagewidget.h"

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
};

class WeaponArrangementWidget : public QWidget {
    Q_OBJECT

public:
    explicit WeaponArrangementWidget(QWidget* parent = nullptr);
    ~WeaponArrangementWidget();
    double attacksPerRound() const;
    double criticalHitChance() const;

    Ui::WeaponArrangementWidget* ui;
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
