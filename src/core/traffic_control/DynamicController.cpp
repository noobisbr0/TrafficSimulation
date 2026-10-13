#include "DynamicController.h"
#include <algorithm>
#include <cmath>

DynamicController::DynamicController() {
    reset();
}

void DynamicController::setConfig(const SimulationConfig& config) {
    m_config = config;
    reset();
}

void DynamicController::reset() {
    m_state = ControllerState::Green;
    m_currentPhase = m_config.permitLeftTurnFilter ? LogicalPhase::NS_Concurrent : LogicalPhase::North_Split;
    m_nextPhase = m_currentPhase;
    m_stateTime = 0.0;
    m_currentGreenTime = 0.0;
    m_timeSinceLastPedPhase = 0.0;

    for (auto& app : m_approaches) {
        app = ApproachData{};
    }
    m_carsInIntersection = 0;
    m_totalWaitingQueue = 0;
}

double DynamicController::getDistanceToStopLine(DirectionId dir, const Vector2D& pos) const {
    const double hwNS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
    const double hwEW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;

    const double stopLineNS = hwEW + 4.0 + 2.25;
    const double stopLineEW = hwNS + 4.0 + 2.25;

    if (dir == DirectionId::North) return pos.y - stopLineNS;
    if (dir == DirectionId::South) return -pos.y - stopLineNS;
    if (dir == DirectionId::East)  return pos.x - stopLineEW;
    if (dir == DirectionId::West)  return -pos.x - stopLineEW;
    return -1.0;
}

void DynamicController::updateMetrics(const std::vector<Vehicle>& vehicles) {
    for (auto& app : m_approaches) {
        app.queueCars = 0;
        app.movingCars = 0;
    }
    m_carsInIntersection = 0;
    m_totalWaitingQueue = 0;

    const double hwNS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
    const double hwEW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;

    for (const auto& v : vehicles) {
        const double x = v.getPosition().x;
        const double y = v.getPosition().y;
        const double speedMs = v.getSpeed() / 3.6;

        // Фиксация машин на самом перекрестке (включая поворачивающих налево)
        if (x > -(hwNS + 2.0) && x < (hwNS + 2.0) && y > -(hwEW + 2.0) && y < (hwEW + 2.0)) {
            m_carsInIntersection++;
        }

        const DirectionId dir = v.getApproachDirection();
        const double dist = getDistanceToStopLine(dir, v.getPosition());

        if (dist < 0.0 || dist > m_config.visibilityDistance) {
            continue;
        }

        // Индексы подходов:
        // dir == South (находится на севере, y < 0) -> 0 (Северный подход)
        // dir == North (находится на юге, y > 0)   -> 1 (Южный подход)
        // dir == East  (находится на востоке, x > 0) -> 2 (Восток)
        // dir == West  (находится на западе, x < 0)  -> 3 (Запад)
        int idx = 0;
        if (dir == DirectionId::South) idx = 0;
        else if (dir == DirectionId::North) idx = 1;
        else if (dir == DirectionId::East)  idx = 2;
        else if (dir == DirectionId::West)  idx = 3;

        if (speedMs < 2.0) {
            m_approaches[idx].queueCars++;
            m_totalWaitingQueue++;
        } else {
            m_approaches[idx].movingCars++;
        }
    }
}

bool DynamicController::canSwitchFromGreen() const {
    if (m_currentPhase == LogicalPhase::Pedestrian) {
        return m_currentGreenTime >= m_config.pedestrianGreenSec;
    }

    // 1. Не переключать фазу, пока машина не завершила левый поворот в центре
    if (m_carsInIntersection > 0 && m_currentGreenTime < 45.0) {
        return false;
    }

    int activeQueue = 0;
    int competingQueue = 0;

    if (m_config.permitLeftTurnFilter) {
        if (m_currentPhase == LogicalPhase::NS_Concurrent) {
            activeQueue = m_approaches[0].queueCars + m_approaches[1].queueCars;
            competingQueue = m_approaches[2].queueCars + m_approaches[3].queueCars;
        } else {
            activeQueue = m_approaches[2].queueCars + m_approaches[3].queueCars;
            competingQueue = m_approaches[0].queueCars + m_approaches[1].queueCars;
        }
    } else {
        if (m_currentPhase == LogicalPhase::North_Split) {
            activeQueue = m_approaches[0].queueCars;
            competingQueue = m_approaches[1].queueCars + m_approaches[2].queueCars + m_approaches[3].queueCars;
        } else if (m_currentPhase == LogicalPhase::South_Split) {
            activeQueue = m_approaches[1].queueCars;
            competingQueue = m_approaches[0].queueCars + m_approaches[2].queueCars + m_approaches[3].queueCars;
        } else if (m_currentPhase == LogicalPhase::East_Split) {
            activeQueue = m_approaches[2].queueCars;
            competingQueue = m_approaches[0].queueCars + m_approaches[1].queueCars + m_approaches[3].queueCars;
        } else if (m_currentPhase == LogicalPhase::West_Split) {
            activeQueue = m_approaches[3].queueCars;
            competingQueue = m_approaches[0].queueCars + m_approaches[1].queueCars + m_approaches[2].queueCars;
        }
    }

    // 2. Адаптивное минимальное время зеленого (расчет на прохождение волны разряжения очереди)
    const double minGreen = std::max(12.0, 4.0 + 1.8 * activeQueue);
    if (m_currentGreenTime < minGreen) {
        return false;
    }

    // 3. Rest-in-Green: если на конкурирующих направлениях пусто — держим зеленый дальше
    if (competingQueue == 0) {
        return false;
    }

    // 4. Очередь на текущем направлении полностью рассосалась
    if (activeQueue == 0) {
        return true;
    }

    // 5. Предел удержания зеленого при наличии пробки на поперечном направлении
    constexpr double kMaxGreenSec = 45.0;
    if (m_currentGreenTime >= kMaxGreenSec) {
        return true;
    }

    return false;
}

DynamicController::LogicalPhase DynamicController::determineNextPhase() const {
    // Пешеходная фаза включается ТОЛЬКО если:
    // 1) Нет параллельных пешеходов (hasRightTurnArrow == false)
    // 2) Автомобили НЕ стоят в огромной пробке (суммарная очередь <= 2 авто)
    // 3) Прошло не менее 120 секунд с прошлой пешеходной фазы
    // 4) Центр перекрестка чист
    if (!m_config.hasRightTurnArrow && m_timeSinceLastPedPhase >= 120.0 &&
        m_totalWaitingQueue <= 2 && m_carsInIntersection == 0) {
        return LogicalPhase::Pedestrian;
    }

    if (m_config.permitLeftTurnFilter) {
        // ВСТР ВКЛ: чередование коридоров NS и EW
        if (m_currentPhase == LogicalPhase::NS_Concurrent) {
            return LogicalPhase::EW_Concurrent;
        }
        return LogicalPhase::NS_Concurrent;
    }

    // ВСТР ВЫКЛ: бессикловый выбор подхода с наибольшей реальной очередью стоящих авто
    const LogicalPhase allSplits[4] = {
        LogicalPhase::North_Split,
        LogicalPhase::South_Split,
        LogicalPhase::East_Split,
        LogicalPhase::West_Split
    };

    int maxQueue = -1;
    LogicalPhase best = m_currentPhase;

    for (int i = 0; i < 4; ++i) {
        if (allSplits[i] == m_currentPhase) continue;
        const int score = m_approaches[i].queueCars * 2 + m_approaches[i].movingCars;
        if (score > maxQueue) {
            maxQueue = score;
            best = allSplits[i];
        }
    }

    // Если все остальные направления пусты, не тратим время на переключение
    if (maxQueue == 0) {
        return m_currentPhase;
    }

    return best;
}

void DynamicController::applyPhaseLights(std::vector<TrafficLight>& trafficLights) const {
    LogicalPhase targetPhase = m_currentPhase;
    LightColor targetColor = LightColor::Green;

    if (m_state == ControllerState::Green) {
        targetPhase = m_currentPhase;
        targetColor = LightColor::Green;
    } else if (m_state == ControllerState::Yellow) {
        targetPhase = m_currentPhase;
        targetColor = LightColor::Yellow;
    } else if (m_state == ControllerState::AllRed) {
        targetColor = LightColor::Red;
    } else if (m_state == ControllerState::RedYellow) {
        // Оставляем только желтый цвет перед включением зеленого
        targetPhase = m_nextPhase;
        targetColor = LightColor::Yellow;
    }

    const bool showLeftArrow = !m_config.permitLeftTurnFilter;

    for (auto& light : trafficLights) {
        const DirectionId dir = light.getDirection();
        bool isActivePhase = false;
        bool isLeftActivePhase = false;

        if (m_state != ControllerState::AllRed) {
            switch (targetPhase) {
            case LogicalPhase::NS_Concurrent:
                if (dir == DirectionId::North || dir == DirectionId::South) isActivePhase = true;
                break;
            case LogicalPhase::EW_Concurrent:
                if (dir == DirectionId::East || dir == DirectionId::West) isActivePhase = true;
                break;
            // Северный светофор (сверху) открывается для Северного подхода
            case LogicalPhase::North_Split:
                if (dir == DirectionId::North) { isActivePhase = true; isLeftActivePhase = true; }
                break;
            // Южный светофор (снизу) открывается для Южного подхода
            case LogicalPhase::South_Split:
                if (dir == DirectionId::South) { isActivePhase = true; isLeftActivePhase = true; }
                break;
            case LogicalPhase::East_Split:
                if (dir == DirectionId::East) { isActivePhase = true; isLeftActivePhase = true; }
                break;
            case LogicalPhase::West_Split:
                if (dir == DirectionId::West) { isActivePhase = true; isLeftActivePhase = true; }
                break;
            case LogicalPhase::Pedestrian:
                break;
            }
        }

        if (isActivePhase) {
            light.setColor(targetColor);
            const bool arrowOn = (targetColor != LightColor::Red && targetColor != LightColor::Yellow);
            light.setLeftArrow(showLeftArrow, showLeftArrow && isLeftActivePhase && arrowOn);
        } else {
            light.setColor(LightColor::Red);
            light.setLeftArrow(showLeftArrow, false);
        }
    }
}

void DynamicController::update(double dt, std::vector<TrafficLight>& trafficLights) {
    if (dt <= 0.0) return;
    m_stateTime += dt;
    m_timeSinceLastPedPhase += dt;

    if (m_state == ControllerState::Green) {
        m_currentGreenTime += dt;

        if (canSwitchFromGreen()) {
            const LogicalPhase next = determineNextPhase();
            if (next != m_currentPhase) {
                m_nextPhase = next;
                m_state = ControllerState::Yellow;
                m_stateTime = 0.0;
            }
        }
    } else if (m_state == ControllerState::Yellow) {
        if (m_stateTime >= 3.0) {
            m_state = ControllerState::AllRed;
            m_stateTime = 0.0;
        }
    } else if (m_state == ControllerState::AllRed) {
        // Если кто-то еще завершает маневр на перекрестке — ждем его съезда
        if (m_stateTime >= 1.5) {
            if (m_carsInIntersection > 0 && m_stateTime < 5.0) {
                // ждем очистки центра перекрестка
            } else {
                m_state = ControllerState::RedYellow;
                m_stateTime = 0.0;
            }
        }
    } else if (m_state == ControllerState::RedYellow) {
        if (m_stateTime >= 1.5) {
            if (m_nextPhase == LogicalPhase::Pedestrian) {
                m_timeSinceLastPedPhase = 0.0;
            }
            m_currentPhase = m_nextPhase;
            m_state = ControllerState::Green;
            m_stateTime = 0.0;
            m_currentGreenTime = 0.0;
        }
    }

    applyPhaseLights(trafficLights);
}

void DynamicController::update(
    double dt,
    std::vector<TrafficLight>& trafficLights,
    const std::vector<Vehicle>& vehicles) {
    updateMetrics(vehicles);
    update(dt, trafficLights);
}