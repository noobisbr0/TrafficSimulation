#pragma once
#include "ISimulationEngine.h"
#include <cmath>

class MockSimulationEngine : public ISimulationEngine {
public:
    MockSimulationEngine() { reset(); }

    void start() override { m_isRunning = true; }
    void pause() override { m_isRunning = false; }

    void resetCar() {
        m_carX = -50.0;
        m_carY = 1.75;
        m_carAngle = 0.0;
        m_carPhase = 0;
        m_turnProgress = 0.0;
        m_carBraking = false;
    }

    void reset() override {
        m_simTime = 0.0;
        resetCar();
    }

    void updateConfig(const SimulationConfig& config) override { m_config = config; }

    void step(double dt) override {
        if (!m_isRunning) return;
        m_simTime += dt;

        double speed = 15.0 * dt;
        int cycle = static_cast<int>(m_simTime) % 90;

        // Определяем, разрешен ли сейчас левый поворот для направления Восток-Запад (машина едет с Запада)
        bool ewLeftAllowed = false;
        if (m_config.permitLeftTurnFilter) {
            // Если ВСТР включен (просачивание), едем на основной зеленый свет (45 - 88 сек)
            ewLeftAllowed = (cycle >= 45 && cycle < 88);
        } else {
            // Если ВСТР отключен, ждем выделенную фазу только для левого поворота (77 - 88 сек)
            ewLeftAllowed = (cycle >= 77 && cycle < 88);
        }

        // Логика остановки перед перекрестком (если поворот налево сейчас запрещен)
        // Логика остановки перед перекрестком
        // Стоп-линия теперь находится на x = -11.0 (при hw_NS = 7.0).
        // Чтобы передний бампер (выступает на 2.25м) не пересекал линию, центр авто должен быть на x = -13.25.
        bool shouldStop = (!ewLeftAllowed && m_carPhase == 0 && m_carX >= -16.0 && m_carX <= -13.25);

        if (shouldStop) {
            m_carX = -13.25; // Удержание ровно перед новой стоп-линией
            m_carBraking = true;
        } else {
            m_carBraking = false;

            // Плавная дуга левого поворота (без выезда на встречку)
            // Центр поворота: (-5.25, -5.25), Радиус: 7.0
            if (m_carPhase == 0) {
                m_carX += speed;
                m_carAngle = 0.0;
                // Автомобиль выезжает на середину перекрестка перед поворотом
                if (m_carX >= -5.25) {
                    m_carX = -5.25;
                    m_carPhase = 1;
                }
            } else if (m_carPhase == 1) {
                double arcLength = 7.0 * M_PI / 2.0;
                m_turnProgress += (speed / arcLength);

                if (m_turnProgress >= 1.0) {
                    m_turnProgress = 1.0;
                    m_carPhase = 2;
                }

                // Угол изменяется от PI/2 до 0
                double a = M_PI / 2.0 - (m_turnProgress * M_PI / 2.0);
                m_carX = -5.25 + 7.0 * std::cos(a);
                m_carY = -5.25 + 7.0 * std::sin(a);
                m_carAngle = (a - M_PI / 2.0) * 180.0 / M_PI;
            } else if (m_carPhase == 2) {
                m_carY -= speed;
                m_carAngle = -90.0;
                if (m_carY < -50.0) resetCar();
            }
        }

        // Анимация пешеходов
        m_pedY += m_pedDir * 2.5 * dt;
        if (m_pedY > 9.0) { m_pedY = 9.0; m_pedDir = -1; }
        if (m_pedY < -9.0) { m_pedY = -9.0; m_pedDir = 1; }
    }

    [[nodiscard]] SimulationSnapshot getSnapshot() const override {
        SimulationSnapshot snap;
        snap.stats.currentSimTimeSec = m_simTime;

        // Генерация машины с дефолтным маневром налево
        VehicleRenderData car;
        car.id = 1;
        car.position = Vector2D(m_carX, m_carY);
        car.angleDeg = m_carAngle;
        car.turnDirection = TurnDirection::Left; // Налево по умолчанию
        car.isBraking = m_carBraking;
        snap.vehicles.push_back(car);

        double hw_EW = (m_config.topology != IntersectionTopology::Lanes_2x2) ? 10.5 : 7.0;
        double hw_NS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;

        // Движущиеся пешеходы
        snap.pedestrians.push_back({101, Vector2D(-hw_EW - 2.0, m_pedY), false});
        snap.pedestrians.push_back({102, Vector2D(hw_EW + 2.0, -m_pedY), false});

        // Ожидающие пешеходы
        snap.pedestrians.push_back({201, Vector2D(-hw_NS - 1.5, -hw_EW - 4.5), true});
        snap.pedestrians.push_back({202, Vector2D(-hw_NS - 2.5, -hw_EW - 4.0), true});
        snap.pedestrians.push_back({203, Vector2D(hw_NS + 1.5, -hw_EW - 4.5), true});
        snap.pedestrians.push_back({204, Vector2D(-hw_NS - 1.5, hw_EW + 4.5), true});
        snap.pedestrians.push_back({205, Vector2D(hw_NS + 2.0, hw_EW + 4.5), true});

        // Цикл светофоров 90 сек.
        int cycle = static_cast<int>(m_simTime) % 90;

        LightColor nsColor = LightColor::Red;
        LightColor ewColor = LightColor::Red;
        bool nsLeft = false;
        bool ewLeft = false;

        if (m_config.permitLeftTurnFilter) {
            // ВСТР включен: основной зеленый горит вместе со стрелками (стрелок физически нет)
            if (cycle < 43) { nsColor = LightColor::Green; nsLeft = true; }
            else if (cycle < 45) { nsColor = LightColor::Yellow; }

            if (cycle >= 45 && cycle < 88) { ewColor = LightColor::Green; ewLeft = true; }
            else if (cycle >= 88 && cycle < 90) { ewColor = LightColor::Yellow; }
        } else {
            // ВСТР выключен (добавлены отдельные фазы, когда едут только поворачивающие налево)
            if (cycle < 30) { nsColor = LightColor::Green; }
            else if (cycle < 32) { nsColor = LightColor::Yellow; }
            else if (cycle >= 32 && cycle < 43) { nsColor = LightColor::Red; nsLeft = true; } // Только налево С-Ю

            if (cycle >= 45 && cycle < 75) { ewColor = LightColor::Green; }
            else if (cycle >= 75 && cycle < 77) { ewColor = LightColor::Yellow; }
            else if (cycle >= 77 && cycle < 88) { ewColor = LightColor::Red; ewLeft = true; } // Только налево В-З
        }

        auto createTL = [&](DirectionId dir, LightColor color, bool leftGreen) {
            TrafficLightRenderData light;
            light.direction = dir;
            light.mainColor = color;
            light.hasLeftArrow = !m_config.permitLeftTurnFilter; // Стрелки появляются только когда ВСТР выкл
            light.leftArrowGreen = light.hasLeftArrow && leftGreen;
            return light;
        };

        snap.trafficLights.push_back(createTL(DirectionId::North, nsColor, nsLeft));
        snap.trafficLights.push_back(createTL(DirectionId::South, nsColor, nsLeft));
        snap.trafficLights.push_back(createTL(DirectionId::East, ewColor, ewLeft));
        snap.trafficLights.push_back(createTL(DirectionId::West, ewColor, ewLeft));

        return snap;
    }

private:
    SimulationConfig m_config;
    bool m_isRunning{false};
    double m_simTime{0.0};
    int m_carPhase{0};
    bool m_carBraking{false};
    double m_carX{-50.0}, m_carY{1.75}, m_carAngle{0.0}, m_turnProgress{0.0};
    double m_pedY{4.0};
    int m_pedDir{1};
};