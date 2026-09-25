#pragma once

#include "ISimulationEngine.h"
#include <cmath>

class MockSimulationEngine : public ISimulationEngine {
public:
    MockSimulationEngine() {
        reset();
    }

    void start() override { m_isRunning = true; }
    void pause() override { m_isRunning = false; }
    
    void reset() override {
        m_simTime = 0.0;
        m_carX = -200.0;
        m_carY = 0.0;
    }

    void updateConfig(const SimulationConfig& config) override {
        m_config = config;
    }

    void step(double dt) override {
        if (!m_isRunning) return;
        m_simTime += dt;

        // Эмуляция движения тестовой машины слева направо
        m_carX += 60.0 * dt;
        if (m_carX > 250.0) {
            m_carX = -250.0;
        }
    }

    [[nodiscard]] SimulationSnapshot getSnapshot() const override {
        SimulationSnapshot snap;
        snap.stats.currentSimTimeSec = m_simTime;
        snap.stats.averageWaitTimeSec = 14.5;
        snap.stats.currentCarsInQueue = 3;
        snap.stats.totalCarsPassed = static_cast<int>(m_simTime / 5.0);

        // Тестовый автомобиль
        VehicleRenderData car;
        car.id = 1;
        car.position = Vector2D(m_carX, 0.0);
        car.angleDeg = 0.0;
        car.speed = 45.0;
        car.turnDirection = TurnDirection::Straight;
        car.isBraking = (m_carX > -50.0 && m_carX < -20.0);
        snap.vehicles.push_back(car);

        // Тестовый светофор (переключается раз в 5 секунд)
        TrafficLightRenderData light;
        light.direction = DirectionId::North;
        bool isGreen = (static_cast<int>(m_simTime / 5.0) % 2 == 0);
        light.mainColor = isGreen ? LightColor::Green : LightColor::Red;
        light.rightArrowGreen = isGreen;
        snap.trafficLights.push_back(light);

        return snap;
    }

private:
    SimulationConfig m_config;
    bool m_isRunning{false};
    double m_simTime{0.0};
    double m_carX{-200.0};
    double m_carY{0.0};
};
