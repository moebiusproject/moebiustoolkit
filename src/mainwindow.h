#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parentWidget = nullptr);
    ~MainWindow();

private:
    struct Private;
    Private* d;
};
