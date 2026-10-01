#include "SimulationEngine.h"

SimulationEngine::SimulationEngine()
    : m_isRunning(false),
      m_currentTime(0.0) {
}

void SimulationEngine::start() {
    m_isRunning = true;
}

void SimulationEngine::pause() {
    m_isRunning = false;
}

void SimulationEngine::reset() {
    m_isRunning = false;
    m_currentTime = 0.0;
}

void SimulationEngine::step(double dt) {
    if (!m_isRunning || dt <= 0.0) {
        return;
    }

    m_currentTime += dt;
}

void SimulationEngine::updateConfig(const SimulationConfig& config) {
    m_config = config;
}

SimulationSnapshot SimulationEngine::getSnapshot() const {
    SimulationSnapshot snapshot;

    snapshot.stats.currentSimTimeSec = m_currentTime;

    return snapshot;
}