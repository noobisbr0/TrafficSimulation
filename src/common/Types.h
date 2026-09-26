#pragma once

#include <cstdint>
#include "Vector2D.h"

// Режим работы контроллера светофоров
enum class ControllerMode {
    Static,   // Статический режим (фиксированные тайминги)
    Dynamic   // Динамический / адаптивный режим по датчикам очередей
};

// Направления проезда перекрестка автомобилем
enum class TurnDirection {
    Straight, // Прямо
    Left,     // Налево
    Right     // Направо
};

// Сигналы светофора
enum class LightColor {
    Red,
    Yellow,
    Green,
    RedYellow,
    Off
};

// Идентификаторы светофоров и направлений на перекрестке
enum class DirectionId {
    North,
    South,
    East,
    West
};

struct TrafficLightRenderData {
    DirectionId direction;
    LightColor mainColor;
    bool hasLeftArrow{false};
    bool leftArrowGreen{false};
    bool hasRightArrow{false};
    bool rightArrowGreen{false};
};

// --- Светофор для пешеходов ---
enum class PedestrianLightSignal {
    Red,   // Стой
    Green  // Иди
};

struct PedestrianTrafficLightRenderData {
    int id{0};
    Vector2D position; // Координата самого угла перекрестка
    PedestrianLightSignal signalNS{PedestrianLightSignal::Red}; // Сигнал для перехода Север-Юг
    PedestrianLightSignal signalEW{PedestrianLightSignal::Red}; // Сигнал для перехода Восток-Запад
    int corner{0}; // 0 - Верхний-Левый, 1 - Верхний-Правый, 2 - Нижний-Левый, 3 - Нижний-Правый
};