#pragma once

#include "../entities/Vehicle.h"
#include "../entities/TrafficLight.h"

class Intersection;

class MovementLogic {
public:
    static double calculateIDMAcceleration(
        const Vehicle& vehicle,
        double gap,
        double leadSpeedKmh);
    static void moveVehicle(Vehicle& vehicle, double dt);

    static void processTurn(Vehicle& vehicle, double dt);

    static void startTurn(Vehicle& vehicle, const Intersection& intersection);

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