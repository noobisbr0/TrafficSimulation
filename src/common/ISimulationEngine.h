#pragma once

#include <vector>
#include "Types.h"
#include "Vector2D.h"
#include "SimulationConfig.h"

// Данные об автомобиле для отрисовки
struct VehicleRenderData {
    int id{0};
    Vector2D position;
    double angleDeg{0.0};
    double speed{0.0};
    TurnDirection turnDirection{TurnDirection::Straight};
    bool isBraking{false};
};

// Данные о пешеходе для отрисовки
struct PedestrianRenderData {
    int id{0};
    Vector2D position;
    bool isWaiting{false};
};

// Данные о светофоре для отрисовки
struct TrafficLightRenderData {
    DirectionId direction{DirectionId::North};
    LightColor mainColor{LightColor::Red};
    bool rightArrowGreen{false};
};

// Выходная статистика моделирования
struct SimulationStats {
    double averageWaitTimeSec{0.0}; // Среднее время остановки на перекрестке
    int totalCarsPassed{0};
    int currentCarsInQueue{0};
    double currentSimTimeSec{0.0};
};

// Полный снимок кадра симуляции
struct SimulationSnapshot {
    std::vector<VehicleRenderData> vehicles;
    std::vector<PedestrianRenderData> pedestrians;
    std::vector<TrafficLightRenderData> trafficLights;
    SimulationStats stats;
};

// Контракт ядра симуляции
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
