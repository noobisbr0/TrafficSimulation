#pragma once
#include <QMainWindow>
#include "presenter/SimulationPresenter.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    SimulationPresenter* m_presenter;
};