#include "Intersection.h"

Intersection::Intersection(const Vector2D& center)
    : m_center(center) {
}

Vector2D Intersection::getCenter() const {
    return m_center;
}

void Intersection::addLane(DirectionId direction, const Lane& lane) {
    m_lanes.push_back(lane);
    size_t index = m_lanes.size() - 1;
    m_lanesByDirection[direction].push_back(index);
}

void Intersection::addTrafficLight(const TrafficLight& trafficLight) {
    m_trafficLights.push_back(trafficLight);
}

const std::vector<Lane>& Intersection::getLanes() const {
    return m_lanes;
}

std::vector<Lane*> Intersection::getLanes(DirectionId direction) const {
    std::vector<Lane*> lanes;
    auto it = m_lanesByDirection.find(direction);
    if (it == m_lanesByDirection.end()) {
        return lanes;
    }

    for (size_t index : it->second) {
        lanes.push_back(const_cast<Lane*>(&m_lanes[index]));
    }
    return lanes;
}

std::vector<TrafficLight>& Intersection::getTrafficLights() {
    return m_trafficLights;
}

const std::vector<TrafficLight>& Intersection::getTrafficLights() const {
    return m_trafficLights;
}