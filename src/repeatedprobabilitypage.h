#pragma once

#include <QWidget>

class RepeatedProbabilityPage : public QWidget
{
    Q_OBJECT
public:
    explicit RepeatedProbabilityPage(QWidget* parent = nullptr);
    ~RepeatedProbabilityPage();

private:
    struct Private;
    Private* d;
};
