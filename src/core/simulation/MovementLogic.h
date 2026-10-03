#pragma once

#include "../entities/Vehicle.h"

class MovementLogic {
public:
    static void moveVehicle(Vehicle& vehicle, double dt);
};