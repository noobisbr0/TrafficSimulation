#pragma once

#include <QWidget>
#include <QPushButton>
#include <QDoubleSpinBox>

class SimulationPresenter;

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit ControlPanel(SimulationPresenter* presenter, QWidget* parent = nullptr);

private:
    QPushButton* m_btnStart{nullptr};
    QPushButton* m_btnPause{nullptr};
    QPushButton* m_btnStep{nullptr};
    QPushButton* m_btnReset{nullptr};
    QDoubleSpinBox* m_sbSpeed{nullptr};
};