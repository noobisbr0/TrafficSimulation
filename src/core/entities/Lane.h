#pragma once

#include <vector>

#include "Vector2D.h"
#include "Vehicle.h"

class Lane {
public:
    Lane(int id, const Vector2D& start, const Vector2D& end, double stopLine);

    int getId() const;
    Vector2D getStart() const;
    Vector2D getEnd() const;
    double getStopLine() const;

    void addVehicle(Vehicle* vehicle);
    void removeVehicle(Vehicle* vehicle);

    const std::vector<Vehicle*>& getVehicles() const;

private:
    int m_id;
    Vector2D m_start;
    Vector2D m_end;
    double m_stopLine;

    std::vector<Vehicle*> m_vehicles;
};