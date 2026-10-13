#pragma once

#include <vector>
#include <array>

#include "../../common/SimulationConfig.h"
#include "../entities/TrafficLight.h"
#include "../entities/Vehicle.h"

class DynamicController {
public:
    DynamicController();

    void setConfig(const SimulationConfig& config);

    void update(
        double dt,
        std::vector<TrafficLight>& trafficLights);

    void update(
        double dt,
        std::vector<TrafficLight>& trafficLights,
        const std::vector<Vehicle>& vehicles);

    void reset();

private:
    enum class ControllerState {
        Green,
        Yellow,
        AllRed,
        RedYellow
    };

    enum class LogicalPhase {
        NS_Concurrent,
        EW_Concurrent,
        North_Split,
        South_Split,
        East_Split,
        West_Split,
        Pedestrian
    };

    struct ApproachData {
        int queueCars{0};       // Автомобили в заторе (v < 2.0 м/с)
        int movingCars{0};      // Приближающиеся автомобили
    };

    void updateMetrics(const std::vector<Vehicle>& vehicles);
    double getDistanceToStopLine(DirectionId dir, const Vector2D& pos) const;

    bool canSwitchFromGreen() const;
    LogicalPhase determineNextPhase() const;

    void applyPhaseLights(std::vector<TrafficLight>& trafficLights) const;

private:
    SimulationConfig m_config;

    ControllerState m_state;
    LogicalPhase m_currentPhase;
    LogicalPhase m_nextPhase;

    double m_stateTime;
    double m_currentGreenTime;
    double m_timeSinceLastPedPhase;

    // 0: Север (верх), 1: Юг (низ), 2: Восток (право), 3: Запад (лево)
    std::array<ApproachData, 4> m_approaches;
    int m_carsInIntersection;
    int m_totalWaitingQueue;
};