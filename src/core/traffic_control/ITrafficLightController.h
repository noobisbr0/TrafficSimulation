#pragma once

#include <vector>

#include "../../common/SimulationConfig.h"
#include "../entities/TrafficLight.h"

class ITrafficLightController {
public:
    virtual ~ITrafficLightController() = default;

    virtual void setConfig(const SimulationConfig& config) = 0;

    virtual void update(
        double dt,
        std::vector<TrafficLight>& trafficLights) = 0;

    virtual void reset() = 0;
};