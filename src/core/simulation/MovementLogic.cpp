#include "MovementLogic.h"
#include "Intersection.h"
#include <cmath>
#include <algorithm>

double MovementLogic::calculateIDMAcceleration(const Vehicle& vehicle, double gap, double leadSpeedKmh) {
    const double aMax = 1.5;
    const double comfortableDeceleration = 2.0;
    const double minimumGap = 2.0;
    const double timeHeadway = 1.2;

    double speed = vehicle.getSpeed() / 3.6;
    double desiredSpeed = vehicle.getDesiredSpeed() / 3.6;
    double leadSpeed = leadSpeedKmh / 3.6;

    if (desiredSpeed < 1.0) desiredSpeed = 1.0;
    if (gap < 0.5) gap = 0.5;

    double deltaSpeed = speed - leadSpeed;
    double desiredGap = minimumGap + speed * timeHeadway + (speed * deltaSpeed) / (2.0 * std::sqrt(aMax * comfortableDeceleration));
    double acceleration = aMax * (1.0 - std::pow(speed / desiredSpeed, 4.0) - std::pow(desiredGap / gap, 2.0));

    if (acceleration < -4.5) acceleration = -4.5;
    if (acceleration > aMax) acceleration = aMax;

    return acceleration;
}

void MovementLogic::moveVehicle(Vehicle& vehicle, double dt) {
    if (dt <= 0.0) return;

    double currentSpeed = vehicle.getSpeed() / 3.6;
    double acceleration = vehicle.getAcceleration();
    double newSpeed = currentSpeed + acceleration * dt;

    if (newSpeed < 0.0) newSpeed = 0.0;
    double distance = currentSpeed * dt + 0.5 * acceleration * dt * dt;
    if (distance < 0.0) distance = 0.0;

    double angleRadians = vehicle.getAngleDeg() * std::acos(-1.0) / 180.0;
    Vector2D position = vehicle.getPosition();
    position.x += std::cos(angleRadians) * distance;
    position.y += std::sin(angleRadians) * distance;

    vehicle.setPosition(position);
    vehicle.setSpeed(newSpeed * 3.6);
}

bool MovementLogic::shouldStopAtRedLight(const Vehicle& vehicle, const TrafficLight& trafficLight, double stopLine) {
    if (trafficLight.getColor() != LightColor::Red) return false;
    const double angle = vehicle.getAngleDeg();
    const Vector2D position = vehicle.getPosition();
    const DirectionId direction = trafficLight.getDirection();

    if (direction == DirectionId::North && angle == -90.0) return position.y >= stopLine;
    if (direction == DirectionId::South && angle == 90.0) return position.y <= -stopLine;
    if (direction == DirectionId::East && angle == 180.0) return position.x >= stopLine;
    if (direction == DirectionId::West && angle == 0.0) return position.x <= -stopLine;
    return false;
}

void MovementLogic::stopVehicleAtRedLight(Vehicle& vehicle, double dt, DirectionId direction, double stopLine) {
    if (dt <= 0.0) return;
    double speedMetersPerSecond = vehicle.getSpeed() / 3.6;
    double angleRadians = vehicle.getAngleDeg() * std::acos(-1.0) / 180.0;
    Vector2D position = vehicle.getPosition();

    double nextX = position.x + std::cos(angleRadians) * speedMetersPerSecond * dt;
    double nextY = position.y + std::sin(angleRadians) * speedMetersPerSecond * dt;

    if (direction == DirectionId::North && nextY < stopLine) nextY = stopLine;
    if (direction == DirectionId::South && nextY > -stopLine) nextY = -stopLine;
    if (direction == DirectionId::East && nextX < stopLine) nextX = stopLine;
    if (direction == DirectionId::West && nextX > -stopLine) nextX = -stopLine;

    position.x = nextX;
    position.y = nextY;
    vehicle.setPosition(position);
    vehicle.setBraking(true);
    vehicle.setWaitingInQueue(true);
}

void MovementLogic::startTurn(Vehicle& vehicle, const Intersection& intersection) {
    if (vehicle.isTurning() || vehicle.isTurnCompleted()) return;
    TurnDirection turn = vehicle.getTurnDirection();
    if (turn == TurnDirection::Straight) return;

    DirectionId targetDir = vehicle.getTargetDirection();
    std::vector<Lane*> targetLanes = intersection.getLanes(targetDir);
    if (targetLanes.empty()) return;

    Lane* targetLane = (turn == TurnDirection::Right) ? targetLanes.back() : targetLanes.front();
    vehicle.setTargetLaneId(targetLane->getId());

    constexpr double pi = 3.14159265358979323846;
    double heading = vehicle.getAngleDeg() * pi / 180.0;
    Vector2D position = vehicle.getPosition();

    double radius = 3.5;
    if (targetDir == DirectionId::North || targetDir == DirectionId::South) {
        radius = std::abs(position.x - targetLane->getStart().x);
    } else {
        radius = std::abs(position.y - targetLane->getStart().y);
    }
    if (radius < 1.0) radius = 1.0;

    Vector2D center;
    if (turn == TurnDirection::Right) {
        Vector2D rightNormal(-std::sin(heading), std::cos(heading));
        center = position + rightNormal * radius;
    } else {
        Vector2D leftNormal(std::sin(heading), -std::cos(heading));
        center = position + leftNormal * radius;
    }

    double startAngle = std::atan2(position.y - center.y, position.x - center.x);

    vehicle.setTurnCenter(center);
    vehicle.setTurnStartAngle(startAngle);
    vehicle.setTurnProgress(0.0);
    vehicle.setTurning(true);
    vehicle.setTurnCompleted(false);
    vehicle.setTurnRadius(radius);

    // Замедляем машину для входа в поворот в зависимости от радиуса
    double targetTurnSpeed = std::sqrt(3.0 * radius) * 3.6;
    if (targetTurnSpeed < 15.0) targetTurnSpeed = 15.0;
    if (targetTurnSpeed > 40.0) targetTurnSpeed = 40.0;

    if (vehicle.getSpeed() > targetTurnSpeed) {
        vehicle.setSpeed(targetTurnSpeed);
    }
}

void MovementLogic::processTurn(Vehicle& vehicle, double dt) {
    if (!vehicle.isTurning() || dt <= 0.0) return;

    double radius = vehicle.getTurnRadius();
    constexpr double pi = 3.14159265358979323846;

    double currentSpeedMs = vehicle.getSpeed() / 3.6;
    double progress = vehicle.getTurnProgress();

    // Динамическая целевая скорость поворота
    double targetSpeedMs = std::sqrt(3.0 * radius);
    if (targetSpeedMs < 4.0) targetSpeedMs = 4.0;
    if (targetSpeedMs > 11.0) targetSpeedMs = 11.0;

    // Плавный разгон на выходе из поворота (когда пройдено > 50%)
    if (progress > 0.5) {
        targetSpeedMs = vehicle.getDesiredSpeed() / 3.6;
    }

    if (currentSpeedMs < targetSpeedMs) {
        currentSpeedMs += 1.5 * dt;
        if (currentSpeedMs > targetSpeedMs) currentSpeedMs = targetSpeedMs;
    } else if (currentSpeedMs > targetSpeedMs) {
        currentSpeedMs -= 2.0 * dt;
        if (currentSpeedMs < targetSpeedMs) currentSpeedMs = targetSpeedMs;
    }

    if (currentSpeedMs < 1.0) currentSpeedMs = 1.0;

    double angularSpeed = currentSpeedMs / radius;
    progress += angularSpeed * dt / (pi / 2.0);
    if (progress >= 1.0) progress = 1.0;

    const double startAngle = vehicle.getTurnStartAngle();
    const Vector2D center = vehicle.getTurnCenter();
    double angle;
    double headingAngle;

    if (vehicle.getTurnDirection() == TurnDirection::Right) {
        angle = startAngle + progress * (pi / 2.0);
        headingAngle = angle + pi / 2.0;
    } else {
        angle = startAngle - progress * (pi / 2.0);
        headingAngle = angle - pi / 2.0;
    }

    Vector2D newPosition(
        center.x + radius * std::cos(angle),
        center.y + radius * std::sin(angle));

    double headingDeg = headingAngle * 180.0 / pi;
    while (headingDeg <= -180.0) headingDeg += 360.0;
    while (headingDeg > 180.0) headingDeg -= 360.0;

    vehicle.setPosition(newPosition);
    vehicle.setAngleDeg(headingDeg);
    vehicle.setSpeed(currentSpeedMs * 3.6);
    vehicle.setTurnProgress(progress);

    if (progress >= 1.0) {
        vehicle.setTurning(false);
        vehicle.setTurnCompleted(true);
        vehicle.setBraking(false);
        vehicle.setWaitingInQueue(false);

        // ВЫКЛЮЧЕНИЕ ПОВОРОТНИКА
        vehicle.setTurnDirection(TurnDirection::Straight);

        if (vehicle.getTargetLaneId() != -1) {
            vehicle.setLaneId(vehicle.getTargetLaneId());
        }

        DirectionId target = vehicle.getTargetDirection();
        double finalAngle;
        if (target == DirectionId::North) finalAngle = -90.0;
        else if (target == DirectionId::South) finalAngle = 90.0;
        else if (target == DirectionId::East) finalAngle = 180.0;
        else finalAngle = 0.0;

        vehicle.setAngleDeg(finalAngle);
        vehicle.setApproachDirection(target);
        vehicle.setAcceleration(0.0);
    }
}