#pragma once

#include <QWidget>

// TODO: REMOVE this porting workaround.
class QMenuBar;
class QStatusBar;

namespace Ui {
    class SpecialDamageWidget;
}

class SpecialDamageWidget : public QWidget {
    Q_OBJECT

public:
    explicit SpecialDamageWidget(QWidget* parent = nullptr);
    ~SpecialDamageWidget();

    Ui::SpecialDamageWidget* ui;
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
