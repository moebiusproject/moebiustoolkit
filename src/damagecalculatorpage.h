#pragma once

#include <QWidget>

// TODO: REMOVE this porting workaround.
class QMenuBar;
class QStatusBar;

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
