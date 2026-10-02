#pragma once

#include <vector>

#include "ISimulationEngine.h"
#include "Intersection.h"
#include "TrafficGenerator.h"
#include "../entities/Vehicle.h"
#include "../entities/Lane.h"
#include "../entities/TrafficLight.h"

class SimulationEngine : public ISimulationEngine {
public:
  SimulationEngine();

  void start() override;
  void pause() override;
  void reset() override;
  void step(double dt) override;
  void updateConfig(const SimulationConfig& config) override;

  [[nodiscard]] SimulationSnapshot getSnapshot() const override;

private:
  void initializeIntersection();
  void updateTrafficLights(double dt);
  void updateVehicles(double dt);

  bool m_isRunning;
  double m_currentTime;
  double m_trafficLightPhaseTime;
  SimulationConfig m_config;

  Intersection m_intersection;
  TrafficGenerator m_trafficGenerator;
  std::vector<Vehicle> m_vehicles;
  std::vector<Lane> m_lanes;
  std::vector<TrafficLight> m_trafficLights;
};
