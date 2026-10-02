#include "SimulationEngine.h"

SimulationEngine::SimulationEngine()
  : m_isRunning(false),
  m_currentTime(0.0),
  m_intersection(Vector2D(0.0, 0.0)),
  m_trafficGenerator(&m_intersection) {
    m_trafficGenerator.setConfig(m_config);
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

  m_trafficGenerator.update(dt);

std::vector<Vehicle> generatedVehicles = m_trafficGenerator.takeGeneratedVehicles();

for (Vehicle& vehicle : generatedVehicles) {
  m_vehicles.push_back(vehicle);
}
}

void SimulationEngine::updateConfig(const SimulationConfig& config) {
  m_config = config;
  m_trafficGenerator.setConfig(m_config);
}

SimulationSnapshot SimulationEngine::getSnapshot() const {
  SimulationSnapshot snapshot;

  for (const Vehicle& vehicle : m_vehicles) {
    VehicleRenderData renderData;

    renderData.id = vehicle.getId();
    renderData.position = vehicle.getPosition();
    renderData.angleDeg = vehicle.getAngleDeg();
    renderData.speed = vehicle.getSpeed();
    renderData.turnDirection = vehicle.getTurnDirection();
    renderData.isBraking = vehicle.isBraking();
    renderData.isWaitingInQueue = vehicle.isWaitingInQueue();

    snapshot.vehicles.push_back(renderData);
  }

  snapshot.stats.currentSimTimeSec = m_currentTime;

  return snapshot;
}
