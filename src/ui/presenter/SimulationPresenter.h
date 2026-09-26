#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include "common/ISimulationEngine.h"
#include "common/SimulationConfig.h"

class SimulationPresenter : public QObject {
    Q_OBJECT

public:
    explicit SimulationPresenter(std::shared_ptr<ISimulationEngine> engine, QObject* parent = nullptr);
    ~SimulationPresenter() override = default;

public slots:
    void onStartClicked();
    void onPauseClicked();
    void onResetClicked();
    void onStepClicked();
    void onConfigChanged(const SimulationConfig& newConfig);
    void onSimulationSpeedMultiplierChanged(double multiplier);

signals:
    void snapshotUpdated(const SimulationSnapshot& snapshot);

private slots:
    void onTimerTick();

private:
    std::shared_ptr<ISimulationEngine> m_engine;
    QTimer m_tickTimer;
    double m_speedMultiplier{1.0};
    static constexpr double kBaseDt{0.033};
};