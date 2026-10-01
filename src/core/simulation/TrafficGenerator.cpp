#include "TrafficGenerator.h"

TrafficGenerator::TrafficGenerator(Intersection* intersection)
  : m_intersection(intersection),
    m_random(std::random_device{}()),
    m_nextVehicleId(1),
    m_timeUntilNextVehicle(0.0) {
}

void TrafficGenerator::setConfig(const SimulationConfig& config) {
  m_config = config;
}

double TrafficGenerator::getTotalFlow() const {
    return (m_config.north.flowP +
           m_config.south.flowP +
           m_config.east.flowP +
           m_config.west.flowP);
}

DirectionId TrafficGenerator::chooseDirection() {
  double totalFlow = getTotalFlow();

  std::uniform_real_distribution<double> distribution(0.0, totalFlow);

  double value = distribution(m_random);

  if (value < m_config.north.flowP) {
    return DirectionId::North;
  }

  value -= m_config.north.flowP;

  if (value < m_config.south.flowP) {
    return DirectionId::South;
  }

  value -= m_config.south.flowP;

  if (value < m_config.east.flowP) {
    return DirectionId::East;
  }

  return DirectionId::West;
}

double TrafficGenerator::generateNextSpawnInterval() {
  double totalFlow = getTotalFlow();

  if (totalFlow <= 0.0) {
    return 1.0;
  }

  double averageInterval = 3600.0 / totalFlow;

  std::uniform_real_distribution<double> distribution(
    averageInterval * 0.5,
    averageInterval * 1.5
  );

  return distribution(m_random);
}

void TrafficGenerator::update(double dt) {
  if (dt <= 0.0) {
    return;
  }

  double totalFlow = getTotalFlow();

  if (totalFlow <= 0.0) {
      return;
  }

  if (m_intersection == nullptr) {
    return;
  }

  m_timeUntilNextVehicle -= dt;

  if (m_timeUntilNextVehicle > 0.0) {
    return;
  }

  std::uniform_real_distribution<double> speedDistribution(
  m_config.minSpeedKmh,
  m_config.maxSpeedKmh);

  double speed = speedDistribution(m_random);

  std::uniform_int_distribution<int> turnDistribution(0, 2);

  int turnValue = turnDistribution(m_random);

  TurnDirection turnDirection;

  if (turnValue == 0) {
    turnDirection = TurnDirection::Straight;
  } else if (turnValue == 1) {
    turnDirection = TurnDirection::Left;
  } else {
    turnDirection = TurnDirection::Right;
  }

  DirectionId direction = chooseDirection();

  const std::vector<Lane*>& lanes = m_intersection->getLanes(direction);
  if (lanes.empty()) {
    return;
  }
  std::uniform_int_distribution<int> laneDistribution(0, static_cast<int>(lanes.size()) - 1);

  int laneIndex = laneDistribution(m_random);

  Lane* lane = lanes[laneIndex];

  Vector2D position = lane->getStart();

  Vehicle vehicle(
    m_nextVehicleId,
    position,
    speed,
    turnDirection
  );

  m_nextVehicleId++;

  m_generatedVehicles.push_back(vehicle);

  m_timeUntilNextVehicle = generateNextSpawnInterval();
}

std::vector<Vehicle> TrafficGenerator::takeGeneratedVehicles() {
  std::vector<Vehicle> vehicles = std::move(m_generatedVehicles);
  m_generatedVehicles.clear();
  return vehicles;
}