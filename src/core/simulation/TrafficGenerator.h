#pragma once

#include <random>
#include <array>
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
  int directionIndex(DirectionId direction) const;
  double getFlow(DirectionId direction) const;
  double generateNextSpawnInterval(DirectionId direction);
  bool generateVehicle(DirectionId direction);

  Intersection* m_intersection;

  SimulationConfig m_config;

  std::mt19937 m_random;

  int m_nextVehicleId;
  std::array<double, 4> m_timeUntilNextVehicle;

  std::vector<Vehicle> m_generatedVehicles;
};