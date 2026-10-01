#include "Intersection.h"

Intersection::Intersection(const Vector2D& center)
    : m_center(center) {
}

Vector2D Intersection::getCenter() const {
    return m_center;
}

void Intersection::addLane(DirectionId direction, Lane* lane) {
    if (lane == nullptr) {
        return;
    }

    m_lanes.push_back(lane);
    m_lanesByDirection[direction].push_back(lane);
}

void Intersection::addTrafficLight(TrafficLight* trafficLight) {
    if (trafficLight == nullptr) {
        return;
    }

    m_trafficLights.push_back(trafficLight);
}

const std::vector<Lane*>& Intersection::getLanes() const {
    return m_lanes;
}

const std::vector<Lane*>& Intersection::getLanes(DirectionId direction) const {
    static const std::vector<Lane*> emptyLanes;

    auto it = m_lanesByDirection.find(direction);

    if (it == m_lanesByDirection.end()) {
        return emptyLanes;
    }

    return it->second;
}

const std::vector<TrafficLight*>& Intersection::getTrafficLights() const {
    return m_trafficLights;
}