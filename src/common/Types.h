#pragma once

#include <cstdint>

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
