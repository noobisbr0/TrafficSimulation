#pragma once
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include "presenter/SimulationPresenter.h"

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit ControlPanel(SimulationPresenter* presenter, QWidget* parent = nullptr);
private:
    QPushButton *btnStart, *btnPause, *btnReset, *btnStep;
    QComboBox *cbSpeed;
};