#pragma once

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

// Данные об автомобиле для отрисовки
struct VehicleRenderData {
    int id{0};
    Vector2D position;
    double angleDeg{0.0};
    double speed{0.0};
    TurnDirection turnDirection{TurnDirection::Straight};
    bool isBraking{false};
    bool isWaitingInQueue{false};

    // Кастомный цвет кузова (по умолчанию нулевой/не задан)
    bool hasCustomColor{false};
    ColorRGB customColor{33, 150, 243}; // Дефолтный синий
};

// Данные о пешеходе для отрисовки
struct PedestrianRenderData {
    int id{0};
    Vector2D position;
    bool isWaiting{false};

    // Кастомный цвет пешехода
    bool hasCustomColor{false};
    ColorRGB customColor{0, 230, 118};
};

// Данные о светофоре для отрисовки
/*struct TrafficLightRenderData {
    DirectionId direction{DirectionId::North};
    LightColor mainColor{LightColor::Red};
    bool rightArrowGreen{false};
};*/

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
    std::vector<PedestrianTrafficLightRenderData> pedestrianLights; // <-- Пешеходные светофоры
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
