#include "TrafficGenerator.h"

TrafficGenerator::TrafficGenerator(Intersection* intersection)
    : m_intersection(intersection),
    m_random(std::random_device{}()),
    m_nextVehicleId(1),
    m_timeUntilNextVehicle{0.0, 0.0, 0.0, 0.0},
    m_timeUntilNextPedestrian{0.0, 0.0, 0.0, 0.0},
    m_nextPedestrianId(1) {
}

void TrafficGenerator::setConfig(const SimulationConfig& config) {
    m_config = config;
}

int TrafficGenerator::directionIndex(DirectionId direction) const {
    if (direction == DirectionId::North) return 0;
    if (direction == DirectionId::South) return 1;
    if (direction == DirectionId::East) return 2;
    return 3;
}

double TrafficGenerator::getFlow(DirectionId direction) const {
    if (direction == DirectionId::North) return static_cast<double>(m_config.north.flowP);
    if (direction == DirectionId::South) return static_cast<double>(m_config.south.flowP);
    if (direction == DirectionId::East) return static_cast<double>(m_config.east.flowP);
    return static_cast<double>(m_config.west.flowP);
}

double TrafficGenerator::generateNextSpawnInterval(DirectionId direction) {
    double flow = getFlow(direction);
    if (flow <= 0.0) return 1.0;
    std::uniform_real_distribution<double> distribution(1e-9, 1.0);
    return -3600.0 / flow * std::log(distribution(m_random));
}

bool TrafficGenerator::generateVehicle(DirectionId direction) {
    if (m_intersection == nullptr) return false;
    std::vector<Lane*> lanes = m_intersection->getLanes(direction);
    if (lanes.empty()) return false;

    std::uniform_real_distribution<double> speedDistribution(m_config.minSpeedKmh, m_config.maxSpeedKmh);
    double speed = speedDistribution(m_random);
    double turnValue = std::uniform_real_distribution<double>(0.0, 1.0)(m_random);
    TurnDirection turnDirection;

    if (turnValue < 0.60) turnDirection = TurnDirection::Straight;
    else if (turnValue < 0.80) turnDirection = TurnDirection::Left;
    else turnDirection = TurnDirection::Right;

    int laneIndex = 0;
    int laneCount = static_cast<int>(lanes.size());

    if (laneCount == 1) {
        laneIndex = 0;
    } else if (laneCount == 2) {
        if (turnDirection == TurnDirection::Right) laneIndex = 1;
        else if (turnDirection == TurnDirection::Left) laneIndex = 0;
        else {
            std::uniform_int_distribution<int> straightLaneDistribution(0, 1);
            laneIndex = straightLaneDistribution(m_random);
        }
    } else {
        if (turnDirection == TurnDirection::Right) laneIndex = 2;
        else if (turnDirection == TurnDirection::Straight) laneIndex = 1;
        else laneIndex = 0;
    }

    Lane* lane = lanes[laneIndex];
    Vector2D position = lane->getStart();
    Vehicle vehicle(m_nextVehicleId, position, speed, turnDirection);
    DirectionId targetDirection = direction;

    if (turnDirection == TurnDirection::Right) {
        if (direction == DirectionId::North) targetDirection = DirectionId::West;
        else if (direction == DirectionId::South) targetDirection = DirectionId::East;
        else if (direction == DirectionId::East) targetDirection = DirectionId::North;
        else targetDirection = DirectionId::South;
    } else if (turnDirection == TurnDirection::Left) {
        if (direction == DirectionId::North) targetDirection = DirectionId::East;
        else if (direction == DirectionId::South) targetDirection = DirectionId::West;
        else if (direction == DirectionId::East) targetDirection = DirectionId::South;
        else targetDirection = DirectionId::North;
    }

    vehicle.setTargetDirection(targetDirection);
    vehicle.setDesiredSpeed(speed);

    double angleDeg = 0.0;
    if (direction == DirectionId::North) angleDeg = -90.0;
    else if (direction == DirectionId::South) angleDeg = 90.0;
    else if (direction == DirectionId::East) angleDeg = 180.0;
    else angleDeg = 0.0;

    vehicle.setAngleDeg(angleDeg);
    vehicle.setApproachDirection(direction);
    vehicle.setLaneId(lane->getId());

    m_nextVehicleId++;
    m_generatedVehicles.push_back(vehicle);
    return true;
}

bool TrafficGenerator::generatePedestrian(int crossingIndex) {
    if (crossingIndex < 0 || crossingIndex >= 4) return false;

    double halfNS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
    double halfEW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;
    const double offset = 1.5;
    const double spawnDist = 30.0;

    Vector2D position;
    DirectionId moveDirection;
    DirectionId crossingDirection;
    bool dirChoice = std::uniform_int_distribution<int>(0, 1)(m_random) == 0;

    if (crossingIndex == 0) {
        crossingDirection = DirectionId::North;
        if (dirChoice) { position = Vector2D(-spawnDist, -halfEW - offset); moveDirection = DirectionId::East; }
        else { position = Vector2D(spawnDist, -halfEW - offset); moveDirection = DirectionId::West; }
    } else if (crossingIndex == 1) {
        crossingDirection = DirectionId::South;
        if (dirChoice) { position = Vector2D(-spawnDist, halfEW + offset); moveDirection = DirectionId::East; }
        else { position = Vector2D(spawnDist, halfEW + offset); moveDirection = DirectionId::West; }
    } else if (crossingIndex == 2) {
        crossingDirection = DirectionId::East;
        if (dirChoice) { position = Vector2D(halfNS + offset, spawnDist); moveDirection = DirectionId::North; }
        else { position = Vector2D(halfNS + offset, -spawnDist); moveDirection = DirectionId::South; }
    } else {
        crossingDirection = DirectionId::West;
        if (dirChoice) { position = Vector2D(-halfNS - offset, spawnDist); moveDirection = DirectionId::North; }
        else { position = Vector2D(-halfNS - offset, -spawnDist); moveDirection = DirectionId::South; }
    }

    Pedestrian pedestrian(m_nextPedestrianId++, position);
    std::uniform_real_distribution<double> speedDistribution(1.0, 1.5);
    pedestrian.setSpeed(speedDistribution(m_random));
    pedestrian.setMoveDirection(moveDirection);
    pedestrian.setTargetCrossing(crossingDirection);
    pedestrian.setWaiting(false);

    m_generatedPedestrians.push_back(pedestrian);
    return true;
}

void TrafficGenerator::updatePedestrians(double dt) {
    if (dt <= 0.0) return;

    const double totalFlow = m_config.pedestrianFlow;
    if (totalFlow <= 0.0) return;

    const double flowPerCrossing = totalFlow / 4.0;

    for (int i = 0; i < 4; ++i) {
        m_timeUntilNextPedestrian[i] -= dt;
        int generatedThisStep = 0;

        while (m_timeUntilNextPedestrian[i] <= 0.0 && generatedThisStep < 10) {
            if (!generatePedestrian(i)) {
                m_timeUntilNextPedestrian[i] = 1.0;
                break;
            }

            std::uniform_real_distribution<double> distribution(1e-9, 1.0);
            double interval = -3600.0 / flowPerCrossing * std::log(distribution(m_random));

            m_timeUntilNextPedestrian[i] += interval;
            generatedThisStep++;
        }
    }
}

void TrafficGenerator::update(double dt) {
    if (dt <= 0.0 || m_intersection == nullptr) return;

    const DirectionId directions[4] = {
        DirectionId::North, DirectionId::South, DirectionId::East, DirectionId::West
    };

    for (int i = 0; i < 4; ++i) {
        DirectionId direction = directions[i];
        double flow = getFlow(direction);
        if (flow <= 0.0) continue;

        m_timeUntilNextVehicle[i] -= dt;
        int generatedThisStep = 0;

        while (m_timeUntilNextVehicle[i] <= 0.0 && generatedThisStep < 10) {
            if (!generateVehicle(direction)) {
                m_timeUntilNextVehicle[i] = 1.0;
                break;
            }
            m_timeUntilNextVehicle[i] += generateNextSpawnInterval(direction);
            generatedThisStep++;
        }
    }
    updatePedestrians(dt);
}

std::vector<Vehicle> TrafficGenerator::takeGeneratedVehicles() {
    std::vector<Vehicle> vehicles = std::move(m_generatedVehicles);
    m_generatedVehicles.clear();
    return vehicles;
}

std::vector<Pedestrian> TrafficGenerator::takeGeneratedPedestrians() {
    std::vector<Pedestrian> pedestrians = std::move(m_generatedPedestrians);
    m_generatedPedestrians.clear();
    return pedestrians;
}