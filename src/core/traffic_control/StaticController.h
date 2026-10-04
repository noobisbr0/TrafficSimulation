#pragma once

#include "ITrafficLightController.h"

class StaticController : public ITrafficLightController {
public:
  StaticController();

  void setConfig(const SimulationConfig& config) override;

  void update(
      double dt,
      std::vector<TrafficLight>& trafficLights) override;

  void reset() override;

private:
  enum class Phase {
    NorthSouthGreen,
    NorthSouthYellow,
    EastWestRedYellow,
    EastWestGreen,
    EastWestYellow,
    NorthSouthRedYellow
  };

  void setMainColors(
      std::vector<TrafficLight>& trafficLights,
      LightColor northSouthColor,
      LightColor eastWestColor);

  void updatePhase(
      std::vector<TrafficLight>& trafficLights);

  SimulationConfig m_config;
  Phase m_phase;
  double m_phaseTime;
};