#pragma once

#include <cstdint>
#include <vector>
#include "Types.h"
#include "Vector2D.h"
#include "SimulationConfig.h"

struct ColorRGB {
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};

    constexpr ColorRGB() = default;
    constexpr ColorRGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

struct VehicleRenderData {
    int id{0};
    Vector2D position;
    double angleDeg{0.0};
    double speed{0.0};
    TurnDirection turnDirection{TurnDirection::Straight};
    bool isBraking{false};
    bool isWaitingInQueue{false};

    bool hasCustomColor{false};
    ColorRGB customColor{33, 150, 243};
};

struct PedestrianRenderData {
    int id{0};
    Vector2D position;
    bool isWaiting{false};

    bool hasCustomColor{false};
    ColorRGB customColor{0, 230, 118};
};

struct SimulationStats {
    double averageWaitTimeSec{0.0};
    int totalCarsPassed{0};
    int currentCarsInQueue{0};
    double currentSimTimeSec{0.0};
};

struct SimulationSnapshot {
    std::vector<VehicleRenderData> vehicles;
    std::vector<PedestrianRenderData> pedestrians;
    std::vector<TrafficLightRenderData> trafficLights;
    std::vector<PedestrianTrafficLightRenderData> pedestrianLights;
    SimulationStats stats;
};

class ISimulationEngine {
public:
    virtual ~ISimulationEngine() = default;

    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void reset() = 0;
    virtual void step(double dt) = 0;

    virtual void updateConfig(const SimulationConfig& config) = 0;
    [[nodiscard]] virtual SimulationSnapshot getSnapshot() const = 0;
};