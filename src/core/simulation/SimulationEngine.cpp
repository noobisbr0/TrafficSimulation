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
    int northLanes = 2, southLanes = 2, eastLanes = 2, westLanes = 2;

    if (m_config.topology == IntersectionTopology::Lanes_2x3) {
        eastLanes = 3; westLanes = 3;
    } else if (m_config.topology == IntersectionTopology::Lanes_3x3) {
        northLanes = 3; southLanes = 3; eastLanes = 3; westLanes = 3;
    }

    int laneId = 1;
    const double laneWidth = 3.5;

    for (int i = 0; i < northLanes; ++i) {
        m_intersection.addLane(DirectionId::North, Lane(laneId++, Vector2D((i + 0.5) * laneWidth, 100.0), Vector2D((i + 0.5) * laneWidth, 0.0), 20.0));
    }
    for (int i = 0; i < southLanes; ++i) {
        m_intersection.addLane(DirectionId::South, Lane(laneId++, Vector2D(-(i + 0.5) * laneWidth, -100.0), Vector2D(-(i + 0.5) * laneWidth, 0.0), 20.0));
    }
    for (int i = 0; i < eastLanes; ++i) {
        m_intersection.addLane(DirectionId::East, Lane(laneId++, Vector2D(100.0, -(i + 0.5) * laneWidth), Vector2D(0.0, -(i + 0.5) * laneWidth), 20.0));
    }
    for (int i = 0; i < westLanes; ++i) {
        m_intersection.addLane(DirectionId::West, Lane(laneId++, Vector2D(-100.0, (i + 0.5) * laneWidth), Vector2D(0.0, (i + 0.5) * laneWidth), 20.0));
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
    if (m_config.mode == ControllerMode::Static) {
        m_staticController.update(dt, m_intersection.getTrafficLights());
    } else if (m_config.mode == ControllerMode::Dynamic) {
        m_dynamicController.update(dt, m_intersection.getTrafficLights(), m_vehicles);
    }

    for (TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        bool green = trafficLight.getColor() == LightColor::Green;
        trafficLight.setRightArrow(m_config.hasRightTurnArrow, m_config.hasRightTurnArrow && green);
    }
}

void SimulationEngine::updateVehicles(double dt) {
    if (dt <= 0.0) return;

    for (Vehicle& vehicle : m_vehicles) {
        DirectionId direction = vehicle.getApproachDirection();
        double halfRoadWidth = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
        double stopLine = halfRoadWidth + 4.0 + 2.25;

        Vector2D currentPosition = vehicle.getPosition();
        bool reachedIntersection = false;

        if (direction == DirectionId::North) reachedIntersection = currentPosition.y <= halfRoadWidth;
        else if (direction == DirectionId::South) reachedIntersection = currentPosition.y >= -halfRoadWidth;
        else if (direction == DirectionId::East) reachedIntersection = currentPosition.x <= halfRoadWidth;
        else if (direction == DirectionId::West) reachedIntersection = currentPosition.x >= -halfRoadWidth;

        bool redOrYellow = false;
        for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
            if (trafficLight.getDirection() == direction) {
                LightColor color = trafficLight.getColor();
                redOrYellow = color == LightColor::Red || color == LightColor::Yellow || color == LightColor::RedYellow;
                break;
            }
        }

        bool mustWaitForLeftArrow = false;
        if (vehicle.getTurnDirection() == TurnDirection::Left && !m_config.permitLeftTurnFilter) {
            for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
                if (trafficLight.getDirection() == direction) {
                    mustWaitForLeftArrow = !trafficLight.isLeftArrowGreen();
                    break;
                }
            }
        }

        bool hasLeader = false;
        double nearestGap = 1000000.0;
        double leaderSpeedKmh = vehicle.getDesiredSpeed();

        for (const Vehicle& other : m_vehicles) {
            if (other.getId() == vehicle.getId() || other.getApproachDirection() != direction || other.getLaneId() != vehicle.getLaneId()) continue;

            Vector2D otherPosition = other.getPosition();
            double distanceAhead = -1.0;

            if (direction == DirectionId::North) distanceAhead = currentPosition.y - otherPosition.y;
            else if (direction == DirectionId::South) distanceAhead = otherPosition.y - currentPosition.y;
            else if (direction == DirectionId::East) distanceAhead = currentPosition.x - otherPosition.x;
            else if (direction == DirectionId::West) distanceAhead = otherPosition.x - currentPosition.x;

            if (distanceAhead <= 0.0) continue;
            double gap = distanceAhead - 4.5;
            if (gap < nearestGap) {
                nearestGap = gap; leaderSpeedKmh = other.getSpeed(); hasLeader = true;
            }
        }

        for (const Pedestrian& ped : m_pedestrians) {
            if (ped.isWaiting()) continue;
            Vector2D pPos = ped.getPosition();
            Vector2D vPos = currentPosition;
            double dist = (pPos - vPos).length();

            if (dist < 20.0) {
                double rad = vehicle.getAngleDeg() * 3.14159265358979323846 / 180.0;
                Vector2D dir(std::cos(rad), std::sin(rad));
                Vector2D toPed = pPos - vPos;
                double forwardDist = dir.x * toPed.x + dir.y * toPed.y;

                if (forwardDist > 0 && forwardDist < 15.0) {
                    double latDist = std::abs(dir.x * toPed.y - dir.y * toPed.x);
                    if (latDist < 3.0) {
                        double gap = forwardDist - 2.5;
                        if (gap < 0.5) gap = 0.5;
                        if (gap < nearestGap) {
                            nearestGap = gap; leaderSpeedKmh = 0.0; hasLeader = true;
                        }
                    }
                }
            }
        }

        if (redOrYellow || mustWaitForLeftArrow) {
            double distanceToStopLine = -1.0;
            if (direction == DirectionId::North) distanceToStopLine = currentPosition.y - stopLine;
            else if (direction == DirectionId::South) distanceToStopLine = -currentPosition.y - stopLine;
            else if (direction == DirectionId::East) distanceToStopLine = currentPosition.x - stopLine;
            else if (direction == DirectionId::West) distanceToStopLine = -currentPosition.x - stopLine;

            if (distanceToStopLine > 0.0 && distanceToStopLine <= m_config.visibilityDistance && distanceToStopLine < nearestGap) {
                nearestGap = distanceToStopLine; leaderSpeedKmh = 0.0; hasLeader = true;
            }
        }

        double acceleration = MovementLogic::calculateIDMAcceleration(vehicle, nearestGap, leaderSpeedKmh);
        vehicle.setAcceleration(acceleration);
        vehicle.setBraking(acceleration < -0.5);
        vehicle.setWaitingInQueue(vehicle.getSpeed() < 0.5 && hasLeader);

        if ((vehicle.getTurnDirection() == TurnDirection::Right || vehicle.getTurnDirection() == TurnDirection::Left) &&
            !vehicle.isTurning() && !vehicle.isTurnCompleted() && reachedIntersection) {
            MovementLogic::startTurn(vehicle, m_intersection);
        }

        if (vehicle.isTurning()) {
            bool blocked = false;
            double progress = vehicle.getTurnProgress();

            // 1. Проверка пешеходов
            for (const Pedestrian& ped : m_pedestrians) {
                if (ped.isWaiting()) continue;
                Vector2D pPos = ped.getPosition();
                Vector2D vPos = vehicle.getPosition();
                if ((pPos - vPos).length() < 10.0) {
                    double rad = vehicle.getAngleDeg() * 3.14159265358979323846 / 180.0;
                    Vector2D dir(std::cos(rad), std::sin(rad));
                    Vector2D toPed = pPos - vPos;
                    double forwardDist = dir.x * toPed.x + dir.y * toPed.y;
                    if (forwardDist > 0 && forwardDist < 8.0) {
                        double latDist = std::abs(dir.x * toPed.y - dir.y * toPed.x);
                        if (latDist < 3.0) { blocked = true; break; }
                    }
                }
            }

            // 2. Логика просачивания налево (ВСТР)
            if (!blocked && vehicle.getTurnDirection() == TurnDirection::Left && m_config.permitLeftTurnFilter) {
                // Машина начинает проверку, выкатившись на 10% (за стоп-линию)
                if (progress >= 0.10 && progress < 0.8) {
                    DirectionId myDir = vehicle.getApproachDirection();
                    DirectionId oncomingDir;

                    if (myDir == DirectionId::North) oncomingDir = DirectionId::South;
                    else if (myDir == DirectionId::South) oncomingDir = DirectionId::North;
                    else if (myDir == DirectionId::East) oncomingDir = DirectionId::West;
                    else oncomingDir = DirectionId::East;

                    // Половинная ширина перекрестка
                    double hw = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;

                    for (const Vehicle& other : m_vehicles) {
                        if (other.getId() == vehicle.getId()) continue;

                        if (other.getApproachDirection() == oncomingDir) {
                            TurnDirection otherTurn = other.getTurnDirection();

                            // А. Встречные машины, едущие прямо или направо
                            if (otherTurn == TurnDirection::Straight || otherTurn == TurnDirection::Right) {
                                Vector2D otherPos = other.getPosition();
                                double distToCenter = 0.0;

                                if (oncomingDir == DirectionId::North) distToCenter = -otherPos.y;
                                else if (oncomingDir == DirectionId::South) distToCenter = otherPos.y;
                                else if (oncomingDir == DirectionId::East) distToCenter = otherPos.x;
                                else if (oncomingDir == DirectionId::West) distToCenter = -otherPos.x;

                                double speedMs = other.getSpeed() / 3.6;

                                // Условие 1: Встречная машина находится прямо на перекрестке.
                                // distToCenter > -2.0 означает, что её задний бампер еще не покинул опасную зону
                                if (distToCenter > -2.0 && distToCenter <= (hw + 2.0)) {
                                    blocked = true;
                                    break;
                                }
                                // Условие 2: Машина быстро приближается к перекрестку
                                else if (distToCenter > (hw + 2.0) && distToCenter < 60.0 && speedMs > 1.5) {
                                    // Считаем время не до центра, а до въезда на перекресток
                                    double timeToIntersection = (distToCenter - hw) / speedMs;
                                    if (timeToIntersection < 3.5) {
                                        blocked = true;
                                        break;
                                    }
                                }
                            }
                            // Б. Предотвращение дедлока при встречном левом повороте
                            else if (otherTurn == TurnDirection::Left && other.isTurning()) {
                                Vector2D myPos = vehicle.getPosition();
                                Vector2D theirPos = other.getPosition();

                                // Если машины оказались слишком близко друг к другу в центре
                                if ((myPos - theirPos).length() < 6.0) {
                                    double otherProgress = other.getTurnProgress();
                                    // Разрешаем конфликт: едет тот, у кого прогресс поворота больше (кто начал раньше)
                                    // Если прогресс одинаковый, уступает тот, у кого ID больше
                                    if (otherProgress > progress || (otherProgress == progress && other.getId() < vehicle.getId())) {
                                        blocked = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!blocked) {
                MovementLogic::processTurn(vehicle, dt);
            } else {
                vehicle.setSpeed(0.0);
                vehicle.setBraking(true);
            }
            continue;
        }

        MovementLogic::moveVehicle(vehicle, dt);
    }
}

void SimulationEngine::removeVehiclesOutsideScene() {
    const double limit = 110.0;
    for (auto it = m_vehicles.begin(); it != m_vehicles.end(); ) {
        const Vector2D position = it->getPosition();
        if (position.x < -limit || position.x > limit || position.y < -limit || position.y > limit) it = m_vehicles.erase(it);
        else ++it;
    }
}

void SimulationEngine::updatePedestrians(double dt) {
    if (dt <= 0.0) return;

    bool northRed = false, southRed = false, eastRed = false, westRed = false;
    for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        bool red = trafficLight.getColor() == LightColor::Red;
        if (trafficLight.getDirection() == DirectionId::North) northRed = red;
        else if (trafficLight.getDirection() == DirectionId::South) southRed = red;
        else if (trafficLight.getDirection() == DirectionId::East) eastRed = red;
        else if (trafficLight.getDirection() == DirectionId::West) westRed = red;
    }

    bool allRed = northRed && southRed && eastRed && westRed;
    bool parallelAllowed = m_config.hasRightTurnArrow;

    for (Pedestrian& pedestrian : m_pedestrians) {
        bool canCross = false;
        DirectionId crossing = pedestrian.getTargetCrossing();

        if (parallelAllowed) {
            if (crossing == DirectionId::North || crossing == DirectionId::South) canCross = northRed && southRed;
            else canCross = eastRed && westRed;
        } else {
            canCross = allRed;
        }

        if (pedestrian.isWaiting()) {
            if (canCross) pedestrian.setWaiting(false);
        }

        if (!pedestrian.isWaiting()) {
            Vector2D pos = pedestrian.getPosition();
            double dist = pedestrian.getSpeed() * dt;
            DirectionId moveDir = pedestrian.getMoveDirection();
            bool hitWaitLine = false;

            double halfNS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
            double halfEW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;

            if (moveDir == DirectionId::East) {
                double waitX = -halfNS - 0.5;
                if (pos.x < waitX && pos.x + dist >= waitX && !canCross) {
                    pos.x = waitX; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            } else if (moveDir == DirectionId::West) {
                double waitX = halfNS + 0.5;
                if (pos.x > waitX && pos.x - dist <= waitX && !canCross) {
                    pos.x = waitX; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            } else if (moveDir == DirectionId::North) {
                double waitY = halfEW + 0.5;
                if (pos.y > waitY && pos.y - dist <= waitY && !canCross) {
                    pos.y = waitY; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            } else if (moveDir == DirectionId::South) {
                double waitY = -halfEW - 0.5;
                if (pos.y < waitY && pos.y + dist >= waitY && !canCross) {
                    pos.y = waitY; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            }

            if (!hitWaitLine) {
                if (moveDir == DirectionId::East) pos.x += dist;
                else if (moveDir == DirectionId::West) pos.x -= dist;
                else if (moveDir == DirectionId::North) pos.y -= dist;
                else if (moveDir == DirectionId::South) pos.y += dist;
            }
            pedestrian.setPosition(pos);
        }
    }
}

void SimulationEngine::removePedestriansOutsideScene() {
    const double limit = 110.0;
    for (auto it = m_pedestrians.begin(); it != m_pedestrians.end();) {
        const Vector2D position = it->getPosition();
        if (position.x < -limit || position.x > limit || position.y < -limit || position.y > limit) it = m_pedestrians.erase(it);
        else ++it;
    }
}

void SimulationEngine::updatePedestrianLights(std::vector<PedestrianTrafficLightRenderData>& lights) const {
    lights.clear();
    bool northRed = false, southRed = false, eastRed = false, westRed = false;

    for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        bool red = trafficLight.getColor() == LightColor::Red;
        if (trafficLight.getDirection() == DirectionId::North) northRed = red;
        else if (trafficLight.getDirection() == DirectionId::South) southRed = red;
        else if (trafficLight.getDirection() == DirectionId::East) eastRed = red;
        else if (trafficLight.getDirection() == DirectionId::West) westRed = red;
    }

    bool allRed = northRed && southRed && eastRed && westRed;
    bool parallelAllowed = m_config.hasRightTurnArrow;

    const std::array<Vector2D, 4> positions = {
        Vector2D(-8.5, -8.5), Vector2D(8.5, -8.5), Vector2D(8.5, 8.5), Vector2D(-8.5, 8.5)
};

for (int i = 0; i < 4; ++i) {
    PedestrianTrafficLightRenderData light;
    light.id = i + 1;
    light.position = positions[i];
    light.corner = i;

    if (parallelAllowed) {
        light.signalNS = (eastRed && westRed) ? PedestrianLightSignal::Green : PedestrianLightSignal::Red;
        light.signalEW = (northRed && southRed) ? PedestrianLightSignal::Green : PedestrianLightSignal::Red;
    } else {
        light.signalNS = allRed ? PedestrianLightSignal::Green : PedestrianLightSignal::Red;
        light.signalEW = allRed ? PedestrianLightSignal::Green : PedestrianLightSignal::Red;
    }
    lights.push_back(light);
}
}

void SimulationEngine::start() { m_isRunning = true; }
void SimulationEngine::pause() { m_isRunning = false; }
void SimulationEngine::reset() {
    m_isRunning = false; m_currentTime = 0.0; m_trafficLightPhaseTime = 0.0;
    m_vehicles.clear(); m_pedestrians.clear();
    m_pedestrianTimeUntilNext = {0.0, 0.0, 0.0, 0.0}; m_nextPedestrianId = 1;
    m_staticController.reset(); m_dynamicController.reset();
}

void SimulationEngine::step(double dt) {
    if (!m_isRunning || dt <= 0.0) return;

    m_currentTime += dt;
    m_trafficGenerator.update(dt);
    updateTrafficLights(dt);

    std::vector<Vehicle> generatedVehicles = m_trafficGenerator.takeGeneratedVehicles();
    for (Vehicle& vehicle : generatedVehicles) {
        bool canSpawn = true;
        for (const Vehicle& existingVehicle : m_vehicles) {
            if (vehicle.getApproachDirection() != existingVehicle.getApproachDirection() || vehicle.getLaneId() != existingVehicle.getLaneId()) continue;
            Vector2D difference = vehicle.getPosition() - existingVehicle.getPosition();
            if (difference.length() < 8.0) { canSpawn = false; break; }
        }
        if (canSpawn) m_vehicles.push_back(vehicle);
    }

    std::vector<Pedestrian> generatedPedestrians = m_trafficGenerator.takeGeneratedPedestrians();
    for (Pedestrian& pedestrian : generatedPedestrians) m_pedestrians.push_back(pedestrian);

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
        m_vehicles.clear(); m_pedestrians.clear();
        m_intersection = Intersection(Vector2D(0.0, 0.0));
        initializeIntersection();
    }
}

SimulationSnapshot SimulationEngine::getSnapshot() const {
    SimulationSnapshot snapshot;
    for (const Vehicle& vehicle : m_vehicles) {
        VehicleRenderData renderData;
        renderData.id = vehicle.getId(); renderData.position = vehicle.getPosition(); renderData.angleDeg = vehicle.getAngleDeg();
        renderData.speed = vehicle.getSpeed(); renderData.turnDirection = vehicle.getTurnDirection();
        renderData.isBraking = vehicle.isBraking(); renderData.isWaitingInQueue = vehicle.isWaitingInQueue();
        snapshot.vehicles.push_back(renderData);
    }
    snapshot.stats.currentSimTimeSec = m_currentTime;
    for (const Pedestrian& pedestrian : m_pedestrians) {
        PedestrianRenderData renderData;
        renderData.id = pedestrian.getId(); renderData.position = pedestrian.getPosition(); renderData.isWaiting = pedestrian.isWaiting();
        snapshot.pedestrians.push_back(renderData);
    }
    for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        TrafficLightRenderData renderData;
        renderData.direction = trafficLight.getDirection(); renderData.mainColor = trafficLight.getColor();
        renderData.hasLeftArrow = trafficLight.hasLeftArrow(); renderData.leftArrowGreen = trafficLight.isLeftArrowGreen();
        renderData.hasRightArrow = trafficLight.hasRightArrow(); renderData.rightArrowGreen = trafficLight.isRightArrowGreen();
        snapshot.trafficLights.push_back(renderData);
    }
    updatePedestrianLights(snapshot.pedestrianLights);
    return snapshot;
}