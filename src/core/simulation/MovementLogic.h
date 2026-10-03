#pragma once

#include "../entities/Vehicle.h"
#include "../entities/TrafficLight.h"

class MovementLogic {
public:
  static void moveVehicle(Vehicle& vehicle, double dt);

  static bool shouldStopAtRedLight(
    const Vehicle& vehicle,
    const TrafficLight& trafficLight,
    double stopLine
  );
  static void stopVehicleAtRedLight(
    Vehicle& vehicle,
    double dt,
    DirectionId direction,
    double stopLine
  );
};