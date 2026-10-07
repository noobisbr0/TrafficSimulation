#include "SimulationEngine.h"
#include "MovementLogic.h"
#include <cmath>

SimulationEngine::SimulationEngine()
  : m_isRunning(false),
  m_currentTime(0.0),
  m_trafficLightPhaseTime(0.0),
  m_intersection(Vector2D(0.0, 0.0)),
  m_trafficGenerator(&m_intersection),
  m_staticController(),
  m_dynamicController(),
  m_pedestrianTimeUntilNext{0.0, 0.0, 0.0, 0.0},
  m_nextPedestrianId(1) {
  m_trafficGenerator.setConfig(m_config);
  m_staticController.setConfig(m_config);
  m_dynamicController.setConfig(m_config);
  initializeIntersection();
}

void SimulationEngine::initializeIntersection() {
  int northLanes = 2;
  int southLanes = 2;
  int eastLanes = 2;
  int westLanes = 2;

  if (m_config.topology == IntersectionTopology::Lanes_2x3) {
    eastLanes = 3;
    westLanes = 3;
  } else if (m_config.topology == IntersectionTopology::Lanes_3x3) {
    northLanes = 3;
    southLanes = 3;
    eastLanes = 3;
    westLanes = 3;
  }

  int laneId = 1;
  const double laneWidth = 3.5;

  for (int i = 0; i < northLanes; ++i) {
    double x = (i + 0.5) * laneWidth;

    Lane lane(
        laneId++,
        Vector2D(x, 100.0),
        Vector2D(x, 0.0),
        20.0
    );

    m_intersection.addLane(DirectionId::North, lane);
  }

  for (int i = 0; i < southLanes; ++i) {
    double x = -(i + 0.5) * laneWidth;

    Lane lane(
        laneId++,
        Vector2D(x, -100.0),
        Vector2D(x, 0.0),
        20.0
    );

    m_intersection.addLane(DirectionId::South, lane);
  }

  for (int i = 0; i < eastLanes; ++i) {
    double y = -(i + 0.5) * laneWidth;

    Lane lane(
        laneId++,
        Vector2D(100.0, y),
        Vector2D(0.0, y),
        20.0
    );

    m_intersection.addLane(DirectionId::East, lane);
  }

  for (int i = 0; i < westLanes; ++i) {
    double y = (i + 0.5) * laneWidth;

    Lane lane(
        laneId++,
        Vector2D(-100.0, y),
        Vector2D(0.0, y),
        20.0
    );

    m_intersection.addLane(DirectionId::West, lane);
  }

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
  bool pedestrianExclusivePhase = false;

  if (!m_config.hasRightTurnArrow &&
      m_config.totalCycleSec > 0.0 &&
      m_config.pedestrianGreenSec > 0.0) {

    double pedestrianGreenSec = m_config.pedestrianGreenSec;

    if (pedestrianGreenSec > m_config.totalCycleSec) {
      pedestrianGreenSec = m_config.totalCycleSec;
    }

    double cycleTime =
        std::fmod(m_currentTime, m_config.totalCycleSec);

    pedestrianExclusivePhase =
        cycleTime >= m_config.totalCycleSec - pedestrianGreenSec;
  }

  if (pedestrianExclusivePhase) {
    for (TrafficLight& trafficLight :
         m_intersection.getTrafficLights()) {

      trafficLight.setColor(LightColor::Red);

      trafficLight.setLeftArrow(false, false);
      trafficLight.setRightArrow(false, false);
    }

    return;
  }

  if (m_config.mode == ControllerMode::Static) {
    m_staticController.update(
        dt,
        m_intersection.getTrafficLights());

  } else if (m_config.mode == ControllerMode::Dynamic) {
    m_dynamicController.update(
        dt,
        m_intersection.getTrafficLights(),
        m_vehicles);
  }

  for (TrafficLight& trafficLight :
       m_intersection.getTrafficLights()) {

    bool green =
        trafficLight.getColor() == LightColor::Green;

    trafficLight.setRightArrow(
        m_config.hasRightTurnArrow,
        m_config.hasRightTurnArrow && green);
  }
}

void SimulationEngine::updateVehicles(double dt) {
  if (dt <= 0.0) {
    return;
  }

  for (Vehicle& vehicle : m_vehicles) {
    DirectionId direction = vehicle.getApproachDirection();

  double halfRoadWidth = 7.0;

  if (m_config.topology == IntersectionTopology::Lanes_3x3) {
    halfRoadWidth = 10.5;
  }

  double stopLine = halfRoadWidth + 4.0 + 2.25;

  Vector2D position = vehicle.getPosition();

  bool reachedIntersection = false;

  if (direction == DirectionId::North) {
    reachedIntersection =
        position.y <= halfRoadWidth;
  } else if (direction == DirectionId::South) {
    reachedIntersection =
        position.y >= -halfRoadWidth;
  } else if (direction == DirectionId::East) {
    reachedIntersection =
        position.x <= halfRoadWidth;
  } else if (direction == DirectionId::West) {
    reachedIntersection =
        position.x >= -halfRoadWidth;
  }

    bool redOrYellow = false;

    for (const TrafficLight& trafficLight :
         m_intersection.getTrafficLights()) {

      if (trafficLight.getDirection() == direction) {
        LightColor color = trafficLight.getColor();

        redOrYellow = color == LightColor::Red || color == LightColor::Yellow || color == LightColor::RedYellow;
        break;
      }
    }

    bool mustWaitForLeftArrow = false;

    if (vehicle.getTurnDirection() == TurnDirection::Left &&
        !m_config.permitLeftTurnFilter) {

        for (const TrafficLight& trafficLight :
            m_intersection.getTrafficLights()) {

            if (trafficLight.getDirection() == direction) {
                mustWaitForLeftArrow =
                    !trafficLight.isLeftArrowGreen();
                break;
            }
        }
    }

    Vector2D currentPosition = vehicle.getPosition();

    bool hasLeader = false;
    double nearestGap = 1000000.0;
    double leaderSpeedKmh = vehicle.getDesiredSpeed();

    for (const Vehicle& other : m_vehicles) {

      if (other.getId() == vehicle.getId()) {
        continue;
      }

      if (other.getApproachDirection() != direction) {
        continue;
      }

      if (other.getLaneId() != vehicle.getLaneId()) {
        continue;
      }

      Vector2D otherPosition = other.getPosition();

      double distanceAhead = -1.0;

      if (direction == DirectionId::North) {
        distanceAhead =
            currentPosition.y - otherPosition.y;
      } else if (direction == DirectionId::South) {
        distanceAhead =
            otherPosition.y - currentPosition.y;
      } else if (direction == DirectionId::East) {
        distanceAhead =
            currentPosition.x - otherPosition.x;
      } else if (direction == DirectionId::West) {
        distanceAhead =
            otherPosition.x - currentPosition.x;
      }

      if (distanceAhead <= 0.0) {
        continue;
      }

      double gap = distanceAhead - 4.5;

      if (gap < nearestGap) {
        nearestGap = gap;
        leaderSpeedKmh = other.getSpeed();
        hasLeader = true;
      }
    }

    if (redOrYellow || mustWaitForLeftArrow) {

      double distanceToStopLine = -1.0;

      if (direction == DirectionId::North) {
        distanceToStopLine =
            currentPosition.y - stopLine;
      } else if (direction == DirectionId::South) {
        distanceToStopLine =
            -currentPosition.y - stopLine;
      } else if (direction == DirectionId::East) {
        distanceToStopLine =
            currentPosition.x - stopLine;
      } else if (direction == DirectionId::West) {
        distanceToStopLine =
            -currentPosition.x - stopLine;
      }

      if (distanceToStopLine > 0.0 &&
          distanceToStopLine <= m_config.visibilityDistance &&
          distanceToStopLine < nearestGap) {

        nearestGap = distanceToStopLine;
        leaderSpeedKmh = 0.0;
        hasLeader = true;
      }
    }

    double acceleration =
        MovementLogic::calculateIDMAcceleration(
            vehicle,
            nearestGap,
            leaderSpeedKmh);

    vehicle.setAcceleration(acceleration);

    vehicle.setBraking(acceleration < -0.5);

    vehicle.setWaitingInQueue(vehicle.getSpeed() < 0.5 && hasLeader);

    if ((vehicle.getTurnDirection() == TurnDirection::Right ||
      vehicle.getTurnDirection() == TurnDirection::Left) &&
      !vehicle.isTurning() &&
      !vehicle.isTurnCompleted() &&
      reachedIntersection) {

        MovementLogic::startTurn(vehicle);
    }

    if (vehicle.isTurning()) {
      MovementLogic::processTurn(vehicle, dt);
      continue;
    }

    MovementLogic::moveVehicle(vehicle, dt);
  }
}

void SimulationEngine::removeVehiclesOutsideScene() {
  const double limit = 110.0;

  for (auto it = m_vehicles.begin(); it != m_vehicles.end(); ) {
    const Vector2D position = it->getPosition();

    if (position.x < -limit || position.x > limit ||
        position.y < -limit || position.y > limit) {
      it = m_vehicles.erase(it);
    } else {
      ++it;
    }
  }
}

void SimulationEngine::updatePedestrians(double dt) {
  if (dt <= 0.0) {
    return;
  }

  bool northRed = false;
  bool southRed = false;
  bool eastRed = false;
  bool westRed = false;

  for (const TrafficLight& trafficLight :
       m_intersection.getTrafficLights()) {

    bool red = trafficLight.getColor() == LightColor::Red;

    if (trafficLight.getDirection() == DirectionId::North) {
      northRed = red;
    } else if (trafficLight.getDirection() == DirectionId::South) {
      southRed = red;
    } else if (trafficLight.getDirection() == DirectionId::East) {
      eastRed = red;
    } else if (trafficLight.getDirection() == DirectionId::West) {
      westRed = red;
    }
  }

  for (Pedestrian& pedestrian : m_pedestrians) {
    bool canCross = false;

    DirectionId crossing = pedestrian.getTargetCrossing();

    if (crossing == DirectionId::North) {
      canCross = northRed;
    } else if (crossing == DirectionId::South) {
      canCross = southRed;
    } else if (crossing == DirectionId::East) {
      canCross = eastRed;
    } else if (crossing == DirectionId::West) {
      canCross = westRed;
    }

    if (pedestrian.isWaiting()) {
      if (!canCross) {
        continue;
      }

      pedestrian.setWaiting(false);
    }

    Vector2D position = pedestrian.getPosition();
    double distance = pedestrian.getSpeed() * dt;

    DirectionId moveDirection = pedestrian.getMoveDirection();

    if (moveDirection == DirectionId::East) {
      position.x += distance;
    } else if (moveDirection == DirectionId::West) {
      position.x -= distance;
    } else if (moveDirection == DirectionId::North) {
      position.y -= distance;
    } else if (moveDirection == DirectionId::South) {
      position.y += distance;
    }

    pedestrian.setPosition(position);
  }
}

void SimulationEngine::removePedestriansOutsideScene() {
  const double limit = 110.0;

  for (auto it = m_pedestrians.begin();
       it != m_pedestrians.end();) {

    const Vector2D position = it->getPosition();

    if (position.x < -limit || position.x > limit ||
        position.y < -limit || position.y > limit) {

      it = m_pedestrians.erase(it);
    } else {
      ++it;
    }
  }
}

void SimulationEngine::updatePedestrianLights(std::vector<PedestrianTrafficLightRenderData>& lights) const {

  lights.clear();

  bool northRed = false;
  bool southRed = false;
  bool eastRed = false;
  bool westRed = false;

  for (const TrafficLight& trafficLight :
       m_intersection.getTrafficLights()) {

    bool red = trafficLight.getColor() == LightColor::Red;

    if (trafficLight.getDirection() == DirectionId::North) {
      northRed = red;
    } else if (trafficLight.getDirection() == DirectionId::South) {
      southRed = red;
    } else if (trafficLight.getDirection() == DirectionId::East) {
      eastRed = red;
    } else if (trafficLight.getDirection() == DirectionId::West) {
      westRed = red;
    }
  }

  // На каждом углу есть пешеходный светофор
  // с двумя сигналами: для двух направлений перехода.
  const std::array<Vector2D, 4> positions = {
      Vector2D(-8.5, -8.5),
      Vector2D(8.5, -8.5),
      Vector2D(8.5, 8.5),
      Vector2D(-8.5, 8.5)
  };

  for (int i = 0; i < 4; ++i) {
    PedestrianTrafficLightRenderData light;

    light.id = i + 1;
    light.position = positions[i];
    light.corner = i;

    // Переход север-юг разрешён,
    // когда восточный и западный потоки остановлены.
    light.signalNS =
        (eastRed && westRed)
        ? PedestrianLightSignal::Green
        : PedestrianLightSignal::Red;

    // Переход восток-запад разрешён,
    // когда северный и южный потоки остановлены.
    light.signalEW =
        (northRed && southRed)
        ? PedestrianLightSignal::Green
        : PedestrianLightSignal::Red;

    lights.push_back(light);
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

  m_vehicles.clear();
  m_pedestrians.clear();

  m_pedestrianTimeUntilNext = {0.0, 0.0, 0.0, 0.0};
  m_nextPedestrianId = 1;

  m_staticController.reset();
  m_dynamicController.reset();
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
    bool canSpawn = true;

    for (const Vehicle& existingVehicle : m_vehicles) {
      if (vehicle.getApproachDirection() != existingVehicle.getApproachDirection()) {
        continue;
      }
      if (vehicle.getLaneId() != existingVehicle.getLaneId()) {
        continue;
      }
        Vector2D difference = vehicle.getPosition() - existingVehicle.getPosition();
        if (difference.length() < 8.0) {
          canSpawn = false;
          break;
        }
    }
    if (canSpawn) {
      m_vehicles.push_back(vehicle);
    }
  }

  std::vector<Pedestrian> generatedPedestrians = m_trafficGenerator.takeGeneratedPedestrians();

  for (Pedestrian& pedestrian : generatedPedestrians) {
    m_pedestrians.push_back(pedestrian);
  }

  updateVehicles(dt);
  updatePedestrians(dt);

  removeVehiclesOutsideScene();
  removePedestriansOutsideScene();
}


void SimulationEngine::updateConfig(const SimulationConfig& config) {
  bool topologyChanged = (m_config.topology != config.topology);

  m_config = config;
  m_trafficGenerator.setConfig(m_config);
  m_staticController.setConfig(m_config);
  m_dynamicController.setConfig(m_config);

    if (topologyChanged) {
    m_vehicles.clear();
    m_pedestrians.clear();

    m_intersection = Intersection(Vector2D(0.0, 0.0));

    initializeIntersection();
  }
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

  for (const Pedestrian& pedestrian : m_pedestrians) {
  PedestrianRenderData renderData;

  renderData.id = pedestrian.getId();
  renderData.position = pedestrian.getPosition();
  renderData.isWaiting = pedestrian.isWaiting();

  snapshot.pedestrians.push_back(renderData);
  }

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

  updatePedestrianLights(snapshot.pedestrianLights);

  return snapshot;
}
