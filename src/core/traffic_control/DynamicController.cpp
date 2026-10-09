#include "DynamicController.h"
#include <algorithm>

DynamicController::DynamicController()
    : m_config(),
    m_phase(Phase::GreenNS),
    m_phaseTime(0.0),
    m_greenDuration(10.0),
    m_queueCounts{0, 0, 0, 0} {
}

void DynamicController::setConfig(const SimulationConfig& config) {
    m_config = config;
    reset();
}

int DynamicController::directionIndex(DirectionId direction) {
    if (direction == DirectionId::North) return 0;
    if (direction == DirectionId::South) return 1;
    if (direction == DirectionId::East) return 2;
    return 3;
}

void DynamicController::updateQueueCounts(const std::vector<Vehicle>& vehicles) {
    m_queueCounts = {0, 0, 0, 0};
    double halfRoadWidthNS = 7.0;
    double halfRoadWidthEW = 7.0;

    if (m_config.topology == IntersectionTopology::Lanes_2x3) {
        halfRoadWidthEW = 10.5;
    }
    if (m_config.topology == IntersectionTopology::Lanes_3x3) {
        halfRoadWidthNS = 10.5;
        halfRoadWidthEW = 10.5;
    }

    for (const Vehicle& vehicle : vehicles) {
        if (vehicle.getSpeed() >= 0.5) {
            continue;
        }

        DirectionId direction = vehicle.getApproachDirection();
        Vector2D position = vehicle.getPosition();
        double distanceToStopLine = -1.0;
        double stopLine = 0.0;

        if (direction == DirectionId::North) {
            stopLine = halfRoadWidthEW + 4.0 + 2.25;
            distanceToStopLine = position.y - stopLine;
        } else if (direction == DirectionId::South) {
            stopLine = halfRoadWidthEW + 4.0 + 2.25;
            distanceToStopLine = -position.y - stopLine;
        } else if (direction == DirectionId::East) {
            stopLine = halfRoadWidthNS + 4.0 + 2.25;
            distanceToStopLine = position.x - stopLine;
        } else if (direction == DirectionId::West) {
            stopLine = halfRoadWidthNS + 4.0 + 2.25;
            distanceToStopLine = -position.x - stopLine;
        }

        if (distanceToStopLine < 0.0 || distanceToStopLine > m_config.visibilityDistance) {
            continue;
        }
        int index = directionIndex(direction);
        ++m_queueCounts[index];
    }
}

void DynamicController::applyPhase(std::vector<TrafficLight>& trafficLights) const {
    bool nsGreen = false;
    bool ewGreen = false;
    bool nsYellow = false;
    bool ewYellow = false;
    bool nsRedYellow = false;
    bool ewRedYellow = false;
    bool allRed = (m_phase == Phase::AllRedPedestrian);

    if (m_phase == Phase::GreenNS) nsGreen = true;
    else if (m_phase == Phase::YellowNS) nsYellow = true;
    else if (m_phase == Phase::RedYellowEW) ewRedYellow = true;
    else if (m_phase == Phase::GreenEW) ewGreen = true;
    else if (m_phase == Phase::YellowEW) ewYellow = true;
    else if (m_phase == Phase::RedYellowNS) nsRedYellow = true;

    for (TrafficLight& trafficLight : trafficLights) {
        DirectionId direction = trafficLight.getDirection();
        if (direction == DirectionId::North || direction == DirectionId::South) {
            if (allRed) trafficLight.setColor(LightColor::Red);
            else if (nsGreen) trafficLight.setColor(LightColor::Green);
            else if (nsYellow) trafficLight.setColor(LightColor::Yellow);
            else if (nsRedYellow) trafficLight.setColor(LightColor::Yellow);
            else trafficLight.setColor(LightColor::Red);
        } else {
            if (allRed) trafficLight.setColor(LightColor::Red);
            else if (ewGreen) trafficLight.setColor(LightColor::Green);
            else if (ewYellow) trafficLight.setColor(LightColor::Yellow);
            else if (ewRedYellow) trafficLight.setColor(LightColor::Yellow);
            else trafficLight.setColor(LightColor::Red);
        }
    }
}

int DynamicController::getCurrentQueue() const {
    if (m_phase == Phase::GreenNS || m_phase == Phase::YellowNS || m_phase == Phase::RedYellowEW) {
        return m_queueCounts[0] + m_queueCounts[1];
    }
    return m_queueCounts[2] + m_queueCounts[3];
}

int DynamicController::getOppositeQueue() const {
    if (m_phase == Phase::GreenNS || m_phase == Phase::YellowNS || m_phase == Phase::RedYellowEW) {
        return m_queueCounts[2] + m_queueCounts[3];
    }
    return m_queueCounts[0] + m_queueCounts[1];
}

void DynamicController::switchToYellowNS(std::vector<TrafficLight>& trafficLights) {
    m_phase = Phase::YellowNS;
    m_phaseTime = 0.0;
    applyPhase(trafficLights);
}

void DynamicController::switchToRedYellowEW(std::vector<TrafficLight>& trafficLights) {
    m_phase = Phase::RedYellowEW;
    m_phaseTime = 0.0;
    applyPhase(trafficLights);
}

void DynamicController::switchToYellowEW(std::vector<TrafficLight>& trafficLights) {
    m_phase = Phase::YellowEW;
    m_phaseTime = 0.0;
    applyPhase(trafficLights);
}

void DynamicController::switchToAllRedPedestrian(std::vector<TrafficLight>& trafficLights) {
    m_phase = Phase::AllRedPedestrian;
    m_phaseTime = 0.0;
    applyPhase(trafficLights);
}

void DynamicController::switchToRedYellowNS(std::vector<TrafficLight>& trafficLights) {
    m_phase = Phase::RedYellowNS;
    m_phaseTime = 0.0;
    applyPhase(trafficLights);
}

void DynamicController::update(double dt, std::vector<TrafficLight>& trafficLights) {
    if (dt <= 0.0) {
        return;
    }
    m_phaseTime += dt;

    if (m_phase == Phase::GreenNS) {
        if (m_phaseTime < m_greenDuration) {
            applyPhase(trafficLights);
            return;
        }
        int currentQueue = getCurrentQueue();
        int oppositeQueue = getOppositeQueue();

        if (m_greenDuration < 60.0 && currentQueue > 0 && currentQueue >= oppositeQueue) {
            m_greenDuration = std::min(60.0, m_greenDuration + 2.0);
            m_phaseTime = 0.0;
            applyPhase(trafficLights);
            return;
        }
        switchToYellowNS(trafficLights);
        return;
    }

    if (m_phase == Phase::YellowNS) {
        if (m_phaseTime >= 3.0) {
            switchToRedYellowEW(trafficLights);
        }
        return;
    }

    if (m_phase == Phase::RedYellowEW) {
        if (m_phaseTime >= 1.5) {
            m_phase = Phase::GreenEW;
            m_phaseTime = 0.0;
            m_greenDuration = 10.0;
            applyPhase(trafficLights);
        }
        return;
    }

    if (m_phase == Phase::GreenEW) {
        if (m_phaseTime < m_greenDuration) {
            applyPhase(trafficLights);
            return;
        }
        int currentQueue = getCurrentQueue();
        int oppositeQueue = getOppositeQueue();

        if (m_greenDuration < 60.0 && currentQueue > 0 && currentQueue >= oppositeQueue) {
            m_greenDuration = std::min(60.0, m_greenDuration + 2.0);
            m_phaseTime = 0.0;
            applyPhase(trafficLights);
            return;
        }
        switchToYellowEW(trafficLights);
        return;
    }

    if (m_phase == Phase::YellowEW) {
        if (m_phaseTime >= 3.0) {
            if (!m_config.hasRightTurnArrow) {
                switchToAllRedPedestrian(trafficLights);
            } else {
                switchToRedYellowNS(trafficLights);
            }
        }
        return;
    }

    if (m_phase == Phase::AllRedPedestrian) {
        if (m_phaseTime >= m_config.pedestrianGreenSec) {
            switchToRedYellowNS(trafficLights);
        }
        return;
    }

    if (m_phase == Phase::RedYellowNS) {
        if (m_phaseTime >= 1.5) {
            m_phase = Phase::GreenNS;
            m_phaseTime = 0.0;
            m_greenDuration = 10.0;
            applyPhase(trafficLights);
        }
        return;
    }
}

void DynamicController::update(
    double dt,
    std::vector<TrafficLight>& trafficLights,
    const std::vector<Vehicle>& vehicles) {
    updateQueueCounts(vehicles);
    update(dt, trafficLights);
}

void DynamicController::reset() {
    m_phase = Phase::GreenNS;
    m_phaseTime = 0.0;
    m_greenDuration = 10.0;
    m_queueCounts = {0, 0, 0, 0};
}