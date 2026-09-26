#pragma once

#include <QMainWindow>

class SimulationPresenter;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    SimulationPresenter* m_presenter{nullptr};
};