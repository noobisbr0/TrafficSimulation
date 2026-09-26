#pragma once
#include "Types.h"

enum class IntersectionTopology {
    Lanes_2x2,
    Lanes_2x3,
    Lanes_3x3
};

struct ApproachParams {
    int flowP{600};
    int greenZ{30};
    int redK{60};
};

struct SimulationConfig {
    ControllerMode mode{ControllerMode::Static};
    IntersectionTopology topology{IntersectionTopology::Lanes_2x2};

    // Уникальные опции по доске
    bool permitLeftTurnFilter{true}; // ВСТР
    bool hasRightTurnArrow{false};   // Стрелка направо

    double totalCycleSec{90.0};
    double visibilityDistance{60.0};
    double pedestrianGreenSec{15.0};
    double pedestrianFlow{300.0};

    // --- Диапазон начальной скорости автомобилей (км/ч) ---
    double minSpeedKmh{30.0};
    double maxSpeedKmh{80.0};

    // Индивидуальные параметры потоков и фаз для каждого светофора
    ApproachParams north, south, east, west;

    double timeStepDt{0.05};
};