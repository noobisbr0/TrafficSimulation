#pragma once

#include <array>
#include <vector>

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
    enum class Phase {
        GreenNS,
        YellowNS,
        RedYellowEW,
        GreenEW,
        YellowEW,
        AllRedPedestrian,
        RedYellowNS
    };

    static int directionIndex(DirectionId direction);

    void updateQueueCounts(
        const std::vector<Vehicle>& vehicles);

    void applyPhase(
        std::vector<TrafficLight>& trafficLights) const;

    void switchToYellowNS(
        std::vector<TrafficLight>& trafficLights);

    void switchToRedYellowEW(
        std::vector<TrafficLight>& trafficLights);

    void switchToYellowEW(
        std::vector<TrafficLight>& trafficLights);

    void switchToAllRedPedestrian(
        std::vector<TrafficLight>& trafficLights);

    void switchToRedYellowNS(
        std::vector<TrafficLight>& trafficLights);

    int getCurrentQueue() const;
    int getOppositeQueue() const;

private:
    SimulationConfig m_config;

    Phase m_phase;
    double m_phaseTime;
    double m_greenDuration;

    std::array<int, 4> m_queueCounts;
};