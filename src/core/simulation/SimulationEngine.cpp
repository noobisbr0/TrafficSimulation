#include "SimulationEngine.h"
#include "MovementLogic.h"
#include <cmath>
#include <algorithm>

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
    constexpr double laneWidth = 3.5;

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
        const bool green = trafficLight.getColor() == LightColor::Green;
        trafficLight.setRightArrow(m_config.hasRightTurnArrow, m_config.hasRightTurnArrow && green);
    }
}

void SimulationEngine::updateVehicles(double dt) {
    if (dt <= 0.0) return;

    for (Vehicle& vehicle : m_vehicles) {
        const DirectionId direction = vehicle.getApproachDirection();
        const double halfRoadWidth = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
        const double stopLine = halfRoadWidth + 4.0 + 2.25;

        const Vector2D currentPosition = vehicle.getPosition();
        bool reachedIntersection = false;

        if (direction == DirectionId::North) reachedIntersection = currentPosition.y <= halfRoadWidth;
        else if (direction == DirectionId::South) reachedIntersection = currentPosition.y >= -halfRoadWidth;
        else if (direction == DirectionId::East) reachedIntersection = currentPosition.x <= halfRoadWidth;
        else if (direction == DirectionId::West) reachedIntersection = currentPosition.x >= -halfRoadWidth;

        // Связываем реальный физический подход со светофором перед капотом
        DirectionId controllingLightDir = direction;
        if (direction == DirectionId::North) controllingLightDir = DirectionId::South;
        else if (direction == DirectionId::South) controllingLightDir = DirectionId::North;

        bool redOrYellow = false;
        bool leftArrowGreen = false;
        for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
            if (trafficLight.getDirection() == controllingLightDir) {
                const LightColor color = trafficLight.getColor();
                redOrYellow = (color == LightColor::Red || color == LightColor::Yellow || color == LightColor::RedYellow);
                leftArrowGreen = trafficLight.isLeftArrowGreen();
                break;
            }
        }

        bool stopAtLight = redOrYellow;
        if (vehicle.getTurnDirection() == TurnDirection::Left && !m_config.permitLeftTurnFilter) {
            stopAtLight = !leftArrowGreen;
        }

        bool hasLeader = false;
        double nearestGap = 1000000.0;
        double leaderSpeedKmh = vehicle.getDesiredSpeed();

        for (const Vehicle& other : m_vehicles) {
            if (other.getId() == vehicle.getId() || other.getApproachDirection() != direction || other.getLaneId() != vehicle.getLaneId()) {
                continue;
            }

            const Vector2D otherPosition = other.getPosition();
            double distanceAhead = -1.0;

            if (direction == DirectionId::North) distanceAhead = currentPosition.y - otherPosition.y;
            else if (direction == DirectionId::South) distanceAhead = otherPosition.y - currentPosition.y;
            else if (direction == DirectionId::East) distanceAhead = currentPosition.x - otherPosition.x;
            else if (direction == DirectionId::West) distanceAhead = otherPosition.x - currentPosition.x;

            if (distanceAhead <= 0.0) continue;
            const double gap = distanceAhead - 4.5;
            if (gap < nearestGap) {
                nearestGap = gap;
                leaderSpeedKmh = other.getSpeed();
                hasLeader = true;
            }
        }

        for (const Pedestrian& ped : m_pedestrians) {
            if (ped.isWaiting()) continue;
            const Vector2D pPos = ped.getPosition();
            const Vector2D vPos = currentPosition;
            const double dist = (pPos - vPos).length();

            if (dist < 20.0) {
                const double rad = vehicle.getAngleDeg() * 3.14159265358979323846 / 180.0;
                const Vector2D dir(std::cos(rad), std::sin(rad));
                const Vector2D toPed = pPos - vPos;
                const double forwardDist = dir.x * toPed.x + dir.y * toPed.y;

                if (forwardDist > 0 && forwardDist < 15.0) {
                    const double latDist = std::abs(dir.x * toPed.y - dir.y * toPed.x);
                    if (latDist < 3.0) {
                        double gap = forwardDist - 2.5;
                        if (gap < 0.5) gap = 0.5;
                        if (gap < nearestGap) {
                            nearestGap = gap;
                            leaderSpeedKmh = 0.0;
                            hasLeader = true;
                        }
                    }
                }
            }
        }

        if (stopAtLight) {
            double distanceToStopLine = -1.0;
            if (direction == DirectionId::North) distanceToStopLine = currentPosition.y - stopLine;
            else if (direction == DirectionId::South) distanceToStopLine = -currentPosition.y - stopLine;
            else if (direction == DirectionId::East) distanceToStopLine = currentPosition.x - stopLine;
            else if (direction == DirectionId::West) distanceToStopLine = -currentPosition.x - stopLine;

            if (distanceToStopLine > 0.0 && distanceToStopLine <= m_config.visibilityDistance && distanceToStopLine < nearestGap) {
                nearestGap = distanceToStopLine;
                leaderSpeedKmh = 0.0;
                hasLeader = true;
            }
        }

        // Физический барьер безопасности: машины ни при каких условиях не наезжают друг на друга
        if (hasLeader && nearestGap <= 2.0) {
            vehicle.setSpeed(0.0);
            vehicle.setAcceleration(-4.5);
            vehicle.setBraking(true);
            vehicle.setWaitingInQueue(true);
            continue;
        }

        const double acceleration = MovementLogic::calculateIDMAcceleration(vehicle, nearestGap, leaderSpeedKmh);
        vehicle.setAcceleration(acceleration);
        vehicle.setBraking(acceleration < -0.5);
        vehicle.setWaitingInQueue(vehicle.getSpeed() < 0.5 && hasLeader);

        if ((vehicle.getTurnDirection() == TurnDirection::Right || vehicle.getTurnDirection() == TurnDirection::Left) &&
            !vehicle.isTurning() && !vehicle.isTurnCompleted() && reachedIntersection) {
            MovementLogic::startTurn(vehicle, m_intersection);
        }

        if (vehicle.isTurning()) {
            bool blocked = false;
            const double progress = vehicle.getTurnProgress();

            for (const Pedestrian& ped : m_pedestrians) {
                if (ped.isWaiting()) continue;
                const Vector2D pPos = ped.getPosition();
                const Vector2D vPos = vehicle.getPosition();
                if ((pPos - vPos).length() < 10.0) {
                    const double rad = vehicle.getAngleDeg() * 3.14159265358979323846 / 180.0;
                    const Vector2D dir(std::cos(rad), std::sin(rad));
                    const Vector2D toPed = pPos - vPos;
                    const double forwardDist = dir.x * toPed.x + dir.y * toPed.y;
                    if (forwardDist > 0 && forwardDist < 8.0) {
                        const double latDist = std::abs(dir.x * toPed.y - dir.y * toPed.x);
                        if (latDist < 3.0) { blocked = true; break; }
                    }
                }
            }

            if (!blocked && vehicle.getTurnDirection() == TurnDirection::Left && m_config.permitLeftTurnFilter) {
                if (progress >= 0.10 && progress < 0.8) {
                    const DirectionId myDir = vehicle.getApproachDirection();
                    DirectionId oncomingDir;

                    if (myDir == DirectionId::North) oncomingDir = DirectionId::South;
                    else if (myDir == DirectionId::South) oncomingDir = DirectionId::North;
                    else if (myDir == DirectionId::East) oncomingDir = DirectionId::West;
                    else oncomingDir = DirectionId::East;

                    const double hw = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;

                    for (const Vehicle& other : m_vehicles) {
                        if (other.getId() == vehicle.getId()) continue;

                        if (other.getApproachDirection() == oncomingDir) {
                            const TurnDirection otherTurn = other.getTurnDirection();

                            if (otherTurn == TurnDirection::Straight || otherTurn == TurnDirection::Right) {
                                const Vector2D otherPos = other.getPosition();
                                double distToCenter = 0.0;

                                if (oncomingDir == DirectionId::North) distToCenter = -otherPos.y;
                                else if (oncomingDir == DirectionId::South) distToCenter = otherPos.y;
                                else if (oncomingDir == DirectionId::East) distToCenter = otherPos.x;
                                else if (oncomingDir == DirectionId::West) distToCenter = -otherPos.x;

                                const double speedMs = other.getSpeed() / 3.6;

                                if (distToCenter > -2.0 && distToCenter <= (hw + 2.0)) {
                                    blocked = true;
                                    break;
                                } else if (distToCenter > (hw + 2.0) && distToCenter < 60.0 && speedMs > 1.5) {
                                    const double timeToIntersection = (distToCenter - hw) / speedMs;
                                    if (timeToIntersection < 3.5) {
                                        blocked = true;
                                        break;
                                    }
                                }
                            } else if (otherTurn == TurnDirection::Left && other.isTurning()) {
                                const Vector2D myPos = vehicle.getPosition();
                                const Vector2D theirPos = other.getPosition();

                                if ((myPos - theirPos).length() < 6.0) {
                                    const double otherProgress = other.getTurnProgress();
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
    constexpr double limit = 110.0;
    for (auto it = m_vehicles.begin(); it != m_vehicles.end(); ) {
        const Vector2D pos = it->getPosition();
        const DirectionId dir = it->getApproachDirection();
        bool isLeaving = false;

        // Удаляем только машины, пересекшие перекресток и покидающие сцену
        if (dir == DirectionId::North && pos.y < -limit) isLeaving = true;
        else if (dir == DirectionId::South && pos.y > limit) isLeaving = true;
        else if (dir == DirectionId::East && pos.x < -limit) isLeaving = true;
        else if (dir == DirectionId::West && pos.x > limit) isLeaving = true;

        if (isLeaving) {
            m_statisticsCollector.registerPassedVehicles(it->getId());
            it = m_vehicles.erase(it);
        } else {
            ++it;
        }
    }
}

void SimulationEngine::updatePedestrians(double dt) {
    if (dt <= 0.0) return;

    bool northRed = false, southRed = false, eastRed = false, westRed = false;
    for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        const bool red = trafficLight.getColor() == LightColor::Red;
        if (trafficLight.getDirection() == DirectionId::North) northRed = red;
        else if (trafficLight.getDirection() == DirectionId::South) southRed = red;
        else if (trafficLight.getDirection() == DirectionId::East) eastRed = red;
        else if (trafficLight.getDirection() == DirectionId::West) westRed = red;
    }

    const bool allRed = northRed && southRed && eastRed && westRed;
    const bool parallelAllowed = m_config.hasRightTurnArrow;

    for (Pedestrian& pedestrian : m_pedestrians) {
        bool canCross = false;
        const DirectionId crossing = pedestrian.getTargetCrossing();

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
            const double dist = pedestrian.getSpeed() * dt;
            const DirectionId moveDir = pedestrian.getMoveDirection();
            bool hitWaitLine = false;

            const double halfNS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
            const double halfEW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;

            if (moveDir == DirectionId::East) {
                const double waitX = -halfNS - 0.5;
                if (pos.x < waitX && pos.x + dist >= waitX && !canCross) {
                    pos.x = waitX; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            } else if (moveDir == DirectionId::West) {
                const double waitX = halfNS + 0.5;
                if (pos.x > waitX && pos.x - dist <= waitX && !canCross) {
                    pos.x = waitX; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            } else if (moveDir == DirectionId::North) {
                const double waitY = halfEW + 0.5;
                if (pos.y > waitY && pos.y - dist <= waitY && !canCross) {
                    pos.y = waitY; pedestrian.setWaiting(true); hitWaitLine = true;
                }
            } else if (moveDir == DirectionId::South) {
                const double waitY = -halfEW - 0.5;
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
    constexpr double limit = 110.0;
    for (auto it = m_pedestrians.begin(); it != m_pedestrians.end();) {
        const Vector2D position = it->getPosition();
        if (position.x < -limit || position.x > limit || position.y < -limit || position.y > limit) {
            it = m_pedestrians.erase(it);
        } else {
            ++it;
        }
    }
}

void SimulationEngine::updatePedestrianLights(std::vector<PedestrianTrafficLightRenderData>& lights) const {
    lights.clear();
    bool northRed = false, southRed = false, eastRed = false, westRed = false;

    for (const TrafficLight& trafficLight : m_intersection.getTrafficLights()) {
        const bool red = trafficLight.getColor() == LightColor::Red;
        if (trafficLight.getDirection() == DirectionId::North) northRed = red;
        else if (trafficLight.getDirection() == DirectionId::South) southRed = red;
        else if (trafficLight.getDirection() == DirectionId::East) eastRed = red;
        else if (trafficLight.getDirection() == DirectionId::West) westRed = red;
    }

    const bool allRed = northRed && southRed && eastRed && westRed;
    const bool parallelAllowed = m_config.hasRightTurnArrow;

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
    m_statisticsCollector.reset();
}

void SimulationEngine::step(double dt) {
    if (!m_isRunning || dt <= 0.0) return;

    m_currentTime += dt;
    m_trafficGenerator.update(dt);
    updateTrafficLights(dt);

    std::vector<Vehicle> generatedVehicles = m_trafficGenerator.takeGeneratedVehicles();
    for (Vehicle& vehicle : generatedVehicles) {
        const DirectionId dir = vehicle.getApproachDirection();
        const int laneId = vehicle.getLaneId();

        const Vehicle* rearmostVehicle = nullptr;
        double maxDistFromCenter = -1.0;

        for (const Vehicle& existing : m_vehicles) {
            if (existing.getApproachDirection() != dir || existing.getLaneId() != laneId) continue;

            double dist = 0.0;
            if (dir == DirectionId::North) dist = existing.getPosition().y;
            else if (dir == DirectionId::South) dist = -existing.getPosition().y;
            else if (dir == DirectionId::East) dist = existing.getPosition().x;
            else if (dir == DirectionId::West) dist = -existing.getPosition().x;

            if (dist > maxDistFromCenter) {
                maxDistFromCenter = dist;
                rearmostVehicle = &existing;
            }
        }

        constexpr double kSpawnSpacing = 7.0; // Габарит авто 4.5м + дистанция 2.5м

        if (rearmostVehicle != nullptr && maxDistFromCenter >= 93.0) {
            // Очередь растянулась за экран: спавним новую машину строго позади хвоста за пределами видимости
            Vector2D newPos = rearmostVehicle->getPosition();
            if (dir == DirectionId::North) newPos.y += kSpawnSpacing;
            else if (dir == DirectionId::South) newPos.y -= kSpawnSpacing;
            else if (dir == DirectionId::East) newPos.x += kSpawnSpacing;
            else if (dir == DirectionId::West) newPos.x -= kSpawnSpacing;

            vehicle.setPosition(newPos);

            if (rearmostVehicle->getSpeed() < 5.0) {
                vehicle.setSpeed(0.0);
                vehicle.setDesiredSpeed(rearmostVehicle->getDesiredSpeed());
                vehicle.setAcceleration(0.0);
                vehicle.setBraking(true);
                vehicle.setWaitingInQueue(true);
            }
            m_vehicles.push_back(vehicle);
        } else {
            bool canSpawn = true;
            if (rearmostVehicle != nullptr) {
                const Vector2D diff = vehicle.getPosition() - rearmostVehicle->getPosition();
                if (diff.length() < kSpawnSpacing) {
                    canSpawn = false;
                }
            }
            if (canSpawn) {
                m_vehicles.push_back(vehicle);
            }
        }
    }

    std::vector<Pedestrian> generatedPedestrians = m_trafficGenerator.takeGeneratedPedestrians();
    for (Pedestrian& pedestrian : generatedPedestrians) {
        m_pedestrians.push_back(pedestrian);
    }

    updateVehicles(dt);
    updatePedestrians(dt);

    std::vector<int> waitingVehiclesId;
    int currentCarsInQueue = 0;
    for (const Vehicle& vehicle : m_vehicles) {
        if (vehicle.isWaitingInQueue()) {
            waitingVehiclesId.push_back(vehicle.getId());
            currentCarsInQueue++;
        }
    }

    m_statisticsCollector.update(dt, m_currentTime, waitingVehiclesId, currentCarsInQueue);
    removeVehiclesOutsideScene();
    removePedestriansOutsideScene();
}

void SimulationEngine::updateConfig(const SimulationConfig& config) {
    const bool topologyChanged = (m_config.topology != config.topology);
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
    snapshot.stats.averageWaitTimeSec = m_statisticsCollector.getAverageWaitTimeSec();
    snapshot.stats.totalCarsPassed = m_statisticsCollector.getTotalCarsPassed();
    snapshot.stats.currentCarsInQueue = m_statisticsCollector.getCurrentCarsInQueue();

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