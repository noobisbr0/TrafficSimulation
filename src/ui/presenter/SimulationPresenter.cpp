#include "SimulationPresenter.h"
#include <algorithm>

SimulationPresenter::SimulationPresenter(std::shared_ptr<ISimulationEngine> engine, QObject* parent)
    : QObject(parent), m_engine(std::move(engine)) {
    connect(&m_tickTimer, &QTimer::timeout, this, &SimulationPresenter::onTimerTick);
}

void SimulationPresenter::onStartClicked() {
    if (m_engine) {
        m_engine->start();
    }
    if (!m_tickTimer.isActive()) {
        m_tickTimer.start(33);
    }
}

void SimulationPresenter::onPauseClicked() {
    if (m_engine) {
        m_engine->pause();
    }
    m_tickTimer.stop();
}

void SimulationPresenter::onResetClicked() {
    if (m_engine) {
        m_engine->reset();
        emit snapshotUpdated(m_engine->getSnapshot());
    }
}

void SimulationPresenter::onStepClicked() {
    if (m_engine) {
        m_engine->step(kBaseDt * m_speedMultiplier);
        emit snapshotUpdated(m_engine->getSnapshot());
    }
}

void SimulationPresenter::onConfigChanged(const SimulationConfig& newConfig) {
    if (m_engine) {
        m_engine->updateConfig(newConfig);
    }
}

void SimulationPresenter::onSimulationSpeedMultiplierChanged(double multiplier) {
    m_speedMultiplier = std::max(0.1, multiplier);
}

void SimulationPresenter::onTimerTick() {
    if (!m_engine) {
        return;
    }
    m_engine->step(kBaseDt * m_speedMultiplier);
    emit snapshotUpdated(m_engine->getSnapshot());
}