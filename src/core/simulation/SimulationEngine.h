#pragma once

#include <vector>
#include <array>

#include "ISimulationEngine.h"
#include "Intersection.h"
#include "TrafficGenerator.h"
#include "../entities/Vehicle.h"
#include "../entities/Pedestrian.h"
#include "../entities/Lane.h"
#include "../entities/TrafficLight.h"
#include "../traffic_control/StaticController.h"
#include "../traffic_control/DynamicController.h"
#include "../analytics/StatisticsCollector.h"

class SimulationEngine : public ISimulationEngine {
public:
    SimulationEngine();

    void start() override;
    void pause() override;
    void reset() override;
    void step(double dt) override;
    void updateConfig(const SimulationConfig& config) override;

    [[nodiscard]] SimulationSnapshot getSnapshot() const override;

private:
    void initializeIntersection();
    void updateTrafficLights(double dt);
    void updateVehicles(double dt);
    void removeVehiclesOutsideScene();
    void updatePedestrians(double dt);
    void removePedestriansOutsideScene();
    void updatePedestrianLights(
        std::vector<PedestrianTrafficLightRenderData>& lights) const;

    bool m_isRunning;
    double m_currentTime;
    double m_trafficLightPhaseTime;
    SimulationConfig m_config;

    Intersection m_intersection;
    TrafficGenerator m_trafficGenerator;
    StaticController m_staticController;
    DynamicController m_dynamicController;
    std::vector<Vehicle> m_vehicles;
    std::vector<Vehicle> m_pendingSpawns;
    std::vector<Pedestrian> m_pedestrians;
    std::array<double, 4> m_pedestrianTimeUntilNext;
    int m_nextPedestrianId;
    std::vector<Lane> m_lanes;
    std::vector<TrafficLight> m_trafficLights;
    StatisticsCollector m_statisticsCollector;
};