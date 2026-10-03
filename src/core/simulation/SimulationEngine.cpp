#include "SimulationEngine.h"
#include <cmath>

SimulationEngine::SimulationEngine()
  : m_isRunning(false),
  m_currentTime(0.0),
  m_trafficLightPhaseTime(0.0),
  m_intersection(Vector2D(0.0, 0.0)),
  m_trafficGenerator(&m_intersection) {
  m_trafficGenerator.setConfig(m_config);
  initializeIntersection();
}

void SimulationEngine::initializeIntersection() {
  Lane northLane(
      1,
      Vector2D(0.0, 100.0),
      Vector2D(0.0, 0.0),
      20.0
  );

  Lane southLane(
      2,
      Vector2D(0.0, -100.0),
      Vector2D(0.0, 0.0),
      20.0
  );

  Lane eastLane(
      3,
      Vector2D(100.0, 0.0),
      Vector2D(0.0, 0.0),
      20.0
  );

  Lane westLane(
      4,
      Vector2D(-100.0, 0.0),
      Vector2D(0.0, 0.0),
      20.0
  );

  m_intersection.addLane(DirectionId::North, northLane);
  m_intersection.addLane(DirectionId::South, southLane);
  m_intersection.addLane(DirectionId::East, eastLane);
  m_intersection.addLane(DirectionId::West, westLane);

  TrafficLight northLight(1, DirectionId::North);
  TrafficLight southLight(2, DirectionId::South);
  TrafficLight eastLight(3, DirectionId::East);
  TrafficLight westLight(4, DirectionId::West);

  northLight.setColor(LightColor::Green);
  southLight.setColor(LightColor::Green);

  eastLight.setColor(LightColor::Red);
  westLight.setColor(LightColor::Red);

  m_intersection.addTrafficLight(northLight);
  m_intersection.addTrafficLight(southLight);
  m_intersection.addTrafficLight(eastLight);
  m_intersection.addTrafficLight(westLight);
}

void SimulationEngine::updateTrafficLights(double dt) {
  if (dt <= 0.0) {
      return;
  }

  m_trafficLightPhaseTime += dt;

  double verticalGreenTime =
      static_cast<double>(m_config.north.greenZ);

  double horizontalGreenTime =
      static_cast<double>(m_config.east.greenZ);

  if (m_trafficLightPhaseTime < verticalGreenTime) {
      for (TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
          if (trafficLight.getDirection() == DirectionId::North ||
              trafficLight.getDirection() == DirectionId::South) {
              trafficLight.setColor(LightColor::Green);
          } else {
              trafficLight.setColor(LightColor::Red);
          }
      }
  } else if (m_trafficLightPhaseTime < verticalGreenTime + horizontalGreenTime) {
      for (TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
          if (trafficLight.getDirection() == DirectionId::North ||
              trafficLight.getDirection() == DirectionId::South) {
              trafficLight.setColor(LightColor::Red);
          } else {
              trafficLight.setColor(LightColor::Green);
          }
      }
  } else {
      m_trafficLightPhaseTime = 0.0;
      for (TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        if (trafficLight.getDirection() == DirectionId::North ||
            trafficLight.getDirection() == DirectionId::South) {
            trafficLight.setColor(LightColor::Green);
        } else {
            trafficLight.setColor(LightColor::Red);
          }
    }
  }
}

void SimulationEngine::updateVehicles(double dt) {
  if (dt <= 0.0) {
      return;
  }

  for (Vehicle& vehicle : m_vehicles) {
    double speedMetersPerSecond = vehicle.getSpeed() / 3.6;

    double angleRadians = vehicle.getAngleDeg() * std::acos(-1.0) / 180.0;

    Vector2D position = vehicle.getPosition();

    DirectionId direction;

    if (vehicle.getAngleDeg() == -90.0) {
        direction = DirectionId::North;
    } else if (vehicle.getAngleDeg() == 90.0) {
        direction = DirectionId::South;
    } else if (vehicle.getAngleDeg() == 180.0) {
        direction = DirectionId::East;
    } else {
        direction = DirectionId::West;
    }

    double stopLine = 13.25;

    bool isRed = false;

    for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
      if (trafficLight.getDirection() == direction) {
          isRed = (trafficLight.getColor() == LightColor::Red);
          break;
      }
    }

    bool shouldStop = false;

    if (isRed) {
      if (direction == DirectionId::North && position.y >= stopLine) {
          shouldStop = true;
      } else if (direction == DirectionId::South && position.y <= -stopLine) {
          shouldStop = true;
      } else if (direction == DirectionId::East && position.x >= stopLine) {
          shouldStop = true;
      } else if (direction == DirectionId::West && position.x <= -stopLine) {
          shouldStop = true;
      }
    }

    if (shouldStop) {
      double nextX = position.x + std::cos(angleRadians) * speedMetersPerSecond * dt;

      double nextY = position.y + std::sin(angleRadians) * speedMetersPerSecond * dt;

      if (direction == DirectionId::North && nextY < stopLine) {
        nextY = stopLine;
      }

      if (direction == DirectionId::South && nextY > -stopLine) {
        nextY = -stopLine;
      }

      if (direction == DirectionId::East && nextX < stopLine) {
        nextX = stopLine;
      }

      if (direction == DirectionId::West && nextX > -stopLine) {
        nextX = -stopLine;
      }

      position.x = nextX;
      position.y = nextY;

      vehicle.setPosition(position);
      vehicle.setBraking(true);
      vehicle.setWaitingInQueue(true);

      continue;
    }

    vehicle.setBraking(false);
    vehicle.setWaitingInQueue(false);

    position.x += std::cos(angleRadians) * speedMetersPerSecond * dt;

    position.y += std::sin(angleRadians) * speedMetersPerSecond * dt;

    vehicle.setPosition(position);
  }
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
  m_trafficLightPhaseTime = 0.0;
}

void SimulationEngine::step(double dt) {
  if (!m_isRunning || dt <= 0.0) {
      return;
  }

  m_currentTime += dt;

  m_trafficGenerator.update(dt);

  updateTrafficLights(dt);

  std::vector<Vehicle> generatedVehicles = m_trafficGenerator.takeGeneratedVehicles();

  for (Vehicle& vehicle : generatedVehicles) {
    m_vehicles.push_back(vehicle);
  }

  updateVehicles(dt);
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

  for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
    TrafficLightRenderData renderData;

    renderData.direction = trafficLight.getDirection();
    renderData.mainColor = trafficLight.getColor();
    renderData.hasLeftArrow = trafficLight.hasLeftArrow();
    renderData.leftArrowGreen = trafficLight.isLeftArrowGreen();
    renderData.hasRightArrow = trafficLight.hasRightArrow();
    renderData.rightArrowGreen = trafficLight.isRightArrowGreen();

    snapshot.trafficLights.push_back(renderData);
  }

  return snapshot;
}
