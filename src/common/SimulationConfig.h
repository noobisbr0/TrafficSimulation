#pragma once

#include "Types.h"

struct SimulationConfig {
    // Режим управления светофорами (Auto / Man)
    ControllerMode mode{ControllerMode::Static};

    // Параметры автомобильного потока
    double carSpawnIntervalMin{2.0}; // Интервал появления авто (P)
    double carSpawnIntervalMax{6.0};
    double minSpeedKmH{30.0};        // Диапазон возможных скоростей (30-120 км/ч)
    double maxSpeedKmH{90.0};
    double visibilityDistance{60.0}; // Дистанция видимости светофора (D)

    // Параметры таймингов светофоров
    double redDurationSec{20.0};     // Длительность красного (К)
    double greenDurationSec{25.0};   // Длительность зеленого (З)
    double totalCycleSec{60.0};      // Общая длина цикла фаз (Т)

    // Параметры пешеходов
    double pedestrianSpawnInterval{10.0}; // Поток пешеходов (П)
    double pedestrianGreenSec{15.0};      // Зеленый для пешеходов (ЗдП)

    // Шаг физического времени моделирования
    double timeStepDt{0.05};
};
