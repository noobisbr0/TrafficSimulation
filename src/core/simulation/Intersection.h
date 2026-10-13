#pragma once

#include <vector>
#include <map>

#include "Vector2D.h"
#include "../entities/Lane.h"
#include "../entities/TrafficLight.h"

class Intersection {
public:
    Intersection(const Vector2D& center);

    Vector2D getCenter() const;

    void addLane(DirectionId direction, const Lane& lane);
    void addTrafficLight(const TrafficLight& trafficLight);

    const std::vector<Lane>& getLanes() const;
    std::vector<Lane*> getLanes(DirectionId direction) const;
    std::vector<TrafficLight>& getTrafficLights();
    const std::vector<TrafficLight>& getTrafficLights() const;

private:
    Vector2D m_center;
    std::vector<Lane> m_lanes;
    std::map<DirectionId, std::vector<size_t>> m_lanesByDirection;
    std::vector<TrafficLight> m_trafficLights;
};