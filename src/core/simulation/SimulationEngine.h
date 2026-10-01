#pragma once

#include "ISimulationEngine.h"

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
  bool m_isRunning;
  double m_currentTime;
  SimulationConfig m_config;
};