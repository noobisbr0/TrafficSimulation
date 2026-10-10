#pragma once

#include "../entities/Vehicle.h"
#include "../entities/TrafficLight.h"

class Intersection;

class MovementLogic {
public:
    static double calculateIDMAcceleration(
        const Vehicle& vehicle,
        double gap,
        double leadSpeedKmh,
        double maxBrake = 4.5,
        double speedLimitKmh = 1e9);
    static void moveVehicle(Vehicle& vehicle, double dt);
    static void processTurn(Vehicle& vehicle, double dt, double idmAcceleration);
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