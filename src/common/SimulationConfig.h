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

    bool permitLeftTurnFilter{true};
    bool hasRightTurnArrow{false};

    double totalCycleSec{90.0};
    double visibilityDistance{60.0};
    double pedestrianGreenSec{15.0};
    double pedestrianFlow{300.0};

    double minSpeedKmh{30.0};
    double maxSpeedKmh{80.0};

    ApproachParams north;
    ApproachParams south;
    ApproachParams east;
    ApproachParams west;

    double timeStepDt{0.05};
};