#pragma once

#include <cstdint>
#include "Vector2D.h"

enum class ControllerMode {
    Static,
    Dynamic
};

enum class TurnDirection {
    Straight,
    Left,
    Right
};

enum class LightColor {
    Red,
    Yellow,
    Green,
    RedYellow,
    Off
};

enum class DirectionId {
    North,
    South,
    East,
    West
};

struct TrafficLightRenderData {
    DirectionId direction{DirectionId::North};
    LightColor mainColor{LightColor::Red};
    bool hasLeftArrow{false};
    bool leftArrowGreen{false};
    bool hasRightArrow{false};
    bool rightArrowGreen{false};
};

enum class PedestrianLightSignal {
    Red,
    Green
};

struct PedestrianTrafficLightRenderData {
    int id{0};
    Vector2D position;
    PedestrianLightSignal signalNS{PedestrianLightSignal::Red};
    PedestrianLightSignal signalEW{PedestrianLightSignal::Red};
    int corner{0};
};