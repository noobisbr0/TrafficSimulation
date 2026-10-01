#pragma once

#include <random>
#include <vector>

#include "SimulationConfig.h"
#include "../entities/Vehicle.h"
#include "Intersection.h"

class TrafficGenerator {
public:
  TrafficGenerator(Intersection* intersection);

  void setConfig(const SimulationConfig& config);
  void update(double dt);

  std::vector<Vehicle> takeGeneratedVehicles();

private:
  DirectionId chooseDirection();
  double generateNextSpawnInterval();
  double getTotalFlow() const;

  Intersection* m_intersection;

  SimulationConfig m_config;

  std::mt19937 m_random;

  int m_nextVehicleId;
  double m_timeUntilNextVehicle;

  std::vector<Vehicle> m_generatedVehicles;
};