#include "TrafficGenerator.h"

TrafficGenerator::TrafficGenerator(Intersection* intersection)
  : m_intersection(intersection),
    m_random(std::random_device{}()),
    m_nextVehicleId(1),
    m_timeUntilNextVehicle{0.0, 0.0, 0.0, 0.0} {
}

void TrafficGenerator::setConfig(const SimulationConfig& config) {
  m_config = config;
}
int TrafficGenerator::directionIndex(DirectionId direction) const {
  if (direction == DirectionId::North) {
    return 0;
  }

  if (direction == DirectionId::South) {
    return 1;
  }

  if (direction == DirectionId::East) {
    return 2;
  }

  return 3;
}

double TrafficGenerator::getFlow(DirectionId direction) const {
  if (direction == DirectionId::North) {
    return static_cast<double>(m_config.north.flowP);
  }

  if (direction == DirectionId::South) {
    return static_cast<double>(m_config.south.flowP);
  }

  if (direction == DirectionId::East) {
    return static_cast<double>(m_config.east.flowP);
  }

  return static_cast<double>(m_config.west.flowP);
}

double TrafficGenerator::generateNextSpawnInterval(DirectionId direction) {
  double flow = getFlow(direction);

  if (flow <= 0.0) {
    return 1.0;
  }

  std::uniform_real_distribution<double> distribution(
      1e-9,
      1.0);

  double u = distribution(m_random);

  return -3600.0 / flow * std::log(u);
}
bool TrafficGenerator::generateVehicle(DirectionId direction) {
  if (m_intersection == nullptr) {
    return false;
  }

  std::vector<Lane*> lanes =
      m_intersection->getLanes(direction);

  if (lanes.empty()) {
    return false;
  }

  std::uniform_real_distribution<double> speedDistribution(
      m_config.minSpeedKmh,
      m_config.maxSpeedKmh);

  double speed = speedDistribution(m_random);

  double turnValue =
      std::uniform_real_distribution<double>(0.0, 1.0)(m_random);

  TurnDirection turnDirection;

  if (turnValue < 0.60) {
    turnDirection = TurnDirection::Straight;
  } else if (turnValue < 0.80) {
    turnDirection = TurnDirection::Left;
  } else {
    turnDirection = TurnDirection::Right;
  }

  int laneIndex = 0;

  int laneCount = static_cast<int>(lanes.size());

  if (laneCount == 1) {

    laneIndex = 0;

  } else if (laneCount == 2) {

    if (turnDirection == TurnDirection::Left) {
      laneIndex = 0;
    } else if (turnDirection == TurnDirection::Right) {
      laneIndex = 1;
    } else {
      std::uniform_int_distribution<int> straightLaneDistribution(0, 1);
      laneIndex = straightLaneDistribution(m_random);
    }

  } else {

    if (turnDirection == TurnDirection::Left) {
      laneIndex = 0;
    } else if (turnDirection == TurnDirection::Straight) {
      laneIndex = 1;
    } else {
      laneIndex = 2;
    }
  }

  Lane* lane = lanes[laneIndex];

  Vector2D position = lane->getStart();

  Vehicle vehicle(
      m_nextVehicleId,
      position,
      speed,
      turnDirection);

  vehicle.setDesiredSpeed(speed);

  double angleDeg = 0.0;

  if (direction == DirectionId::North) {
    angleDeg = -90.0;
  } else if (direction == DirectionId::South) {
    angleDeg = 90.0;
  } else if (direction == DirectionId::East) {
    angleDeg = 180.0;
  } else {
    angleDeg = 0.0;
  }

  vehicle.setAngleDeg(angleDeg);
  vehicle.setApproachDirection(direction);
  vehicle.setLaneId(lane->getId());

  m_nextVehicleId++;

  m_generatedVehicles.push_back(vehicle);

  return true;
}

void TrafficGenerator::update(double dt) {
  if (dt <= 0.0) {
    return;
  }

  if (m_intersection == nullptr) {
    return;
  }

  const DirectionId directions[4] = {
      DirectionId::North,
      DirectionId::South,
      DirectionId::East,
      DirectionId::West
  };

  for (int i = 0; i < 4; ++i) {
    DirectionId direction = directions[i];

    double flow = getFlow(direction);

    if (flow <= 0.0) {
      continue;
    }

    m_timeUntilNextVehicle[i] -= dt;

    int generatedThisStep = 0;

    while (m_timeUntilNextVehicle[i] <= 0.0 &&
           generatedThisStep < 10) {

      bool generated = generateVehicle(direction);

      if (!generated) {
        m_timeUntilNextVehicle[i] = 1.0;
        break;
      }

      m_timeUntilNextVehicle[i] +=
          generateNextSpawnInterval(direction);

      generatedThisStep++;
    }
  }
}
std::vector<Vehicle> TrafficGenerator::takeGeneratedVehicles() {
  std::vector<Vehicle> vehicles = std::move(m_generatedVehicles);
  m_generatedVehicles.clear();
  return vehicles;
}