#include "Lane.h"

Lane::Lane(int id, const Vector2D& start, const Vector2D& end,
           double stopLine)
    : m_id(id),
      m_start(start),
      m_end(end),
      m_stopLine(stopLine) {
}

int Lane::getId() const {
    return m_id;
}

Vector2D Lane::getStart() const {
    return m_start;
}

Vector2D Lane::getEnd() const {
    return m_end;
}

double Lane::getStopLine() const {
    return m_stopLine;
}

void Lane::addVehicle(Vehicle* vehicle) {
    if (vehicle == nullptr) {
        return;
    }

    m_vehicles.push_back(vehicle);
}

void Lane::removeVehicle(Vehicle* vehicle) {
    for (auto it = m_vehicles.begin(); it != m_vehicles.end(); ++it) {
        if (*it == vehicle) {
            m_vehicles.erase(it);
            return;
        }
    }
}

const std::vector<Vehicle*>& Lane::getVehicles() const {
    return m_vehicles;
}