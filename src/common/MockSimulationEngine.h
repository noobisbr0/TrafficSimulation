#pragma once

#include "ISimulationEngine.h"
#include <cmath>

class MockSimulationEngine : public ISimulationEngine {
public:
    MockSimulationEngine() {
        reset();
    }

    void start() override {
        m_isRunning = true;
    }

    void pause() override {
        m_isRunning = false;
    }

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

    void updateConfig(const SimulationConfig& config) override {
        m_config = config;
    }

    void step(double dt) override {
        if (!m_isRunning) {
            return;
        }
        m_simTime += dt;

        constexpr double kPi = 3.14159265358979323846;
        const double speed = 15.0 * dt;
        const int cycle = static_cast<int>(m_simTime) % 90;

        bool ewLeftAllowed = false;
        if (m_config.permitLeftTurnFilter) {
            ewLeftAllowed = (cycle >= 45 && cycle < 88);
        } else {
            ewLeftAllowed = (cycle >= 77 && cycle < 88);
        }

        const bool shouldStop = (!ewLeftAllowed && m_carPhase == 0 && m_carX >= -16.0 && m_carX <= -13.25);

        if (shouldStop) {
            m_carX = -13.25;
            m_carBraking = true;
        } else {
            m_carBraking = false;

            if (m_carPhase == 0) {
                m_carX += speed;
                m_carAngle = 0.0;
                if (m_carX >= -5.25) {
                    m_carX = -5.25;
                    m_carPhase = 1;
                }
            } else if (m_carPhase == 1) {
                const double arcLength = 7.0 * kPi / 2.0;
                m_turnProgress += (speed / arcLength);

                if (m_turnProgress >= 1.0) {
                    m_turnProgress = 1.0;
                    m_carPhase = 2;
                }

                const double a = kPi / 2.0 - (m_turnProgress * kPi / 2.0);
                m_carX = -5.25 + 7.0 * std::cos(a);
                m_carY = -5.25 + 7.0 * std::sin(a);
                m_carAngle = (a - kPi / 2.0) * 180.0 / kPi;
            } else if (m_carPhase == 2) {
                m_carY -= speed;
                m_carAngle = -90.0;
                if (m_carY < -50.0) {
                    resetCar();
                }
            }
        }

        m_pedY += m_pedDir * 2.5 * dt;
        if (m_pedY > 9.0) {
            m_pedY = 9.0;
            m_pedDir = -1;
        }
        if (m_pedY < -9.0) {
            m_pedY = -9.0;
            m_pedDir = 1;
        }
    }

    [[nodiscard]] SimulationSnapshot getSnapshot() const override {
        SimulationSnapshot snap;
        snap.stats.currentSimTimeSec = m_simTime;

        VehicleRenderData car;
        car.id = 1;
        car.position = Vector2D(m_carX, m_carY);
        car.angleDeg = m_carAngle;
        car.turnDirection = TurnDirection::Left;
        car.isBraking = m_carBraking;
        snap.vehicles.push_back(car);

        const double hw_EW = (m_config.topology != IntersectionTopology::Lanes_2x2) ? 10.5 : 7.0;
        const double hw_NS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;

        snap.pedestrians.push_back({101, Vector2D(-hw_EW - 2.0, m_pedY), false});
        snap.pedestrians.push_back({102, Vector2D(hw_EW + 2.0, -m_pedY), false});

        snap.pedestrians.push_back({201, Vector2D(-hw_NS - 1.5, -hw_EW - 4.5), true});
        snap.pedestrians.push_back({202, Vector2D(-hw_NS - 2.5, -hw_EW - 4.0), true});
        snap.pedestrians.push_back({203, Vector2D(hw_NS + 1.5, -hw_EW - 4.5), true});
        snap.pedestrians.push_back({204, Vector2D(-hw_NS - 1.5, hw_EW + 4.5), true});
        snap.pedestrians.push_back({205, Vector2D(hw_NS + 2.0, hw_EW + 4.5), true});

        const int cycle = static_cast<int>(m_simTime) % 90;

        LightColor nsColor = LightColor::Red;
        LightColor ewColor = LightColor::Red;
        bool nsLeft = false;
        bool ewLeft = false;

        PedestrianLightSignal pedsAcrossEW = PedestrianLightSignal::Red;
        PedestrianLightSignal pedsAcrossNS = PedestrianLightSignal::Red;

        if (m_config.permitLeftTurnFilter) {
            if (cycle < 43) {
                nsColor = LightColor::Green;
                nsLeft = true;
                pedsAcrossEW = PedestrianLightSignal::Green;
            } else if (cycle < 45) {
                nsColor = LightColor::Yellow;
            }

            if (cycle >= 45 && cycle < 88) {
                ewColor = LightColor::Green;
                ewLeft = true;
                pedsAcrossNS = PedestrianLightSignal::Green;
            } else if (cycle >= 88 && cycle < 90) {
                ewColor = LightColor::Yellow;
            }
        } else {
            if (cycle < 30) {
                nsColor = LightColor::Green;
                pedsAcrossEW = PedestrianLightSignal::Green;
            } else if (cycle < 32) {
                nsColor = LightColor::Yellow;
            } else if (cycle >= 32 && cycle < 43) {
                nsColor = LightColor::Red;
                nsLeft = true;
            }

            if (cycle >= 45 && cycle < 75) {
                ewColor = LightColor::Green;
                pedsAcrossNS = PedestrianLightSignal::Green;
            } else if (cycle >= 75 && cycle < 77) {
                ewColor = LightColor::Yellow;
            } else if (cycle >= 77 && cycle < 88) {
                ewColor = LightColor::Red;
                ewLeft = true;
            }
        }

        auto createTrafficLight = [&](DirectionId dir, LightColor color, bool leftGreen) {
            TrafficLightRenderData light;
            light.direction = dir;
            light.mainColor = color;
            light.hasLeftArrow = !m_config.permitLeftTurnFilter;
            light.leftArrowGreen = light.hasLeftArrow && leftGreen;
            return light;
        };

        snap.trafficLights.push_back(createTrafficLight(DirectionId::North, nsColor, nsLeft));
        snap.trafficLights.push_back(createTrafficLight(DirectionId::South, nsColor, nsLeft));
        snap.trafficLights.push_back(createTrafficLight(DirectionId::East, ewColor, ewLeft));
        snap.trafficLights.push_back(createTrafficLight(DirectionId::West, ewColor, ewLeft));

        snap.pedestrianLights.push_back({301, Vector2D(-hw_NS, -hw_EW), pedsAcrossEW, pedsAcrossNS, 0});
        snap.pedestrianLights.push_back({302, Vector2D(hw_NS, -hw_EW), pedsAcrossEW, pedsAcrossNS, 1});
        snap.pedestrianLights.push_back({303, Vector2D(-hw_NS, hw_EW), pedsAcrossEW, pedsAcrossNS, 2});
        snap.pedestrianLights.push_back({304, Vector2D(hw_NS, hw_EW), pedsAcrossEW, pedsAcrossNS, 3});

        return snap;
    }

private:
    SimulationConfig m_config;
    bool m_isRunning{false};
    double m_simTime{0.0};
    int m_carPhase{0};
    bool m_carBraking{false};
    double m_carX{-50.0};
    double m_carY{1.75};
    double m_carAngle{0.0};
    double m_turnProgress{0.0};
    double m_pedY{4.0};
    int m_pedDir{1};
};