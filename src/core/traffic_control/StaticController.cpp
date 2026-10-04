#include "StaticController.h"

StaticController::StaticController()
  : m_config(),
    m_phase(Phase::NorthSouthGreen),
    m_phaseTime(0.0) {
}

void StaticController::setConfig(const SimulationConfig& config) {
  m_config = config;
  reset();
}

void StaticController::reset() {
  m_phase = Phase::NorthSouthGreen;
  m_phaseTime = 0.0;
}

void StaticController::setMainColors(
    std::vector<TrafficLight>& trafficLights,
    LightColor northSouthColor,
    LightColor eastWestColor) {

  for (TrafficLight& trafficLight : trafficLights) {

    if (trafficLight.getDirection() == DirectionId::North ||
        trafficLight.getDirection() == DirectionId::South) {

      trafficLight.setColor(northSouthColor);

    } else {

      trafficLight.setColor(eastWestColor);
    }

    trafficLight.setLeftArrowGreen(false);
    trafficLight.setRightArrowGreen(false);
  }
}

void StaticController::updatePhase(
    std::vector<TrafficLight>& trafficLights) {

  switch (m_phase) {

    case Phase::NorthSouthGreen:
      setMainColors(
          trafficLights,
          LightColor::Green,
          LightColor::Red);

      break;

    case Phase::NorthSouthYellow:
      setMainColors(
          trafficLights,
          LightColor::Yellow,
          LightColor::Red);

      break;

    case Phase::EastWestRedYellow:
      setMainColors(
          trafficLights,
          LightColor::Red,
          LightColor::RedYellow);

      break;

    case Phase::EastWestGreen:
      setMainColors(
          trafficLights,
          LightColor::Red,
          LightColor::Green);

      break;

    case Phase::EastWestYellow:
      setMainColors(
          trafficLights,
          LightColor::Red,
          LightColor::Yellow);

      break;

    case Phase::NorthSouthRedYellow:
      setMainColors(
          trafficLights,
          LightColor::RedYellow,
          LightColor::Red);

      break;
  }

  if (!m_config.permitLeftTurnFilter) {

    for (TrafficLight& trafficLight : trafficLights) {
      trafficLight.setLeftArrowGreen(true);
    }
  }
}

void StaticController::update(
    double dt,
    std::vector<TrafficLight>& trafficLights) {

  if (dt <= 0.0) {
    return;
  }

  if (trafficLights.empty()) {
    return;
  }

  m_phaseTime += dt;

  double greenTime =
      static_cast<double>(m_config.north.greenZ);

  if (greenTime <= 0.0) {
    greenTime = 30.0;
  }

  constexpr double yellowTime = 3.0;
  constexpr double redYellowTime = 1.5;

  double phaseDuration = 0.0;

  switch (m_phase) {

    case Phase::NorthSouthGreen:
      phaseDuration = greenTime;
      break;

    case Phase::NorthSouthYellow:
      phaseDuration = yellowTime;
      break;

    case Phase::EastWestRedYellow:
      phaseDuration = redYellowTime;
      break;

    case Phase::EastWestGreen:
      phaseDuration =
          static_cast<double>(m_config.east.greenZ);

      if (phaseDuration <= 0.0) {
        phaseDuration = greenTime;
      }

      break;

    case Phase::EastWestYellow:
      phaseDuration = yellowTime;
      break;

    case Phase::NorthSouthRedYellow:
      phaseDuration = redYellowTime;
      break;
  }

  if (m_phaseTime < phaseDuration) {
    updatePhase(trafficLights);
    return;
  }

  m_phaseTime -= phaseDuration;

  switch (m_phase) {

    case Phase::NorthSouthGreen:
      m_phase = Phase::NorthSouthYellow;
      break;

    case Phase::NorthSouthYellow:
      m_phase = Phase::EastWestRedYellow;
      break;

    case Phase::EastWestRedYellow:
      m_phase = Phase::EastWestGreen;
      break;

    case Phase::EastWestGreen:
      m_phase = Phase::EastWestYellow;
      break;

    case Phase::EastWestYellow:
      m_phase = Phase::NorthSouthRedYellow;
      break;

    case Phase::NorthSouthRedYellow:
      m_phase = Phase::NorthSouthGreen;
      break;
  }

  updatePhase(trafficLights);
}