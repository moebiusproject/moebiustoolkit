#pragma once

#include <QWidget>

// TODO: REMOVE this porting workaround.
class QMenuBar;
class QStatusBar;

class DamageCalculatorPage : public QWidget
{
    Q_OBJECT

public:
    DamageCalculatorPage(QWidget* parent = 0);
    ~DamageCalculatorPage();

private:
    // TODO: REMOVE this porting workaround.
    QMenuBar* menuBar();
    QStatusBar* statusBar();
    struct Private;
    Private* d;
};
