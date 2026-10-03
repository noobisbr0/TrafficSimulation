#include "MovementLogic.h"

#include <cmath>

void MovementLogic::moveVehicle(Vehicle& vehicle, double dt) {
  if (dt <= 0.0) {
      return;
  }

  double speedMetersPerSecond = vehicle.getSpeed() / 3.6;

  double angleRadians =
      vehicle.getAngleDeg() * std::acos(-1.0) / 180.0;

  Vector2D position = vehicle.getPosition();

  position.x += std::cos(angleRadians) * speedMetersPerSecond * dt;
  position.y += std::sin(angleRadians) * speedMetersPerSecond * dt;

  vehicle.setPosition(position);
}

bool MovementLogic::shouldStopAtRedLight(const Vehicle& vehicle, const TrafficLight& trafficLight,
double stopLine) {
  if (trafficLight.getColor() != LightColor::Red) {
    return false;
  }

  const double angle = vehicle.getAngleDeg();
  const Vector2D position = vehicle.getPosition();
  const DirectionId direction = trafficLight.getDirection();

  if (direction == DirectionId::North && angle == -90.0) {
    return position.y >= stopLine;
  }

  if (direction == DirectionId::South && angle == 90.0) {
    return position.y <= -stopLine;
  }

  if (direction == DirectionId::East && angle == 180.0) {
    return position.x >= stopLine;
  }

  if (direction == DirectionId::West && angle == 0.0) {
    return position.x <= -stopLine;
  }

  return false;
}

void MovementLogic::stopVehicleAtRedLight(Vehicle& vehicle, double dt, DirectionId direction, double stopLine) {
  if (dt <= 0.0) {
    return;
  }

  double speedMetersPerSecond = vehicle.getSpeed() / 3.6;

  double angleRadians = vehicle.getAngleDeg() * std::acos(-1.0) / 180.0;

  Vector2D position = vehicle.getPosition();

  double nextX = position.x + std::cos(angleRadians) * speedMetersPerSecond * dt;

  double nextY = position.y + std::sin(angleRadians) * speedMetersPerSecond * dt;

  if (direction == DirectionId::North && nextY < stopLine) {
    nextY = stopLine;
  }

  if (direction == DirectionId::South && nextY > -stopLine) {
    nextY = -stopLine;
  }

  if (direction == DirectionId::East && nextX < stopLine) {
    nextX = stopLine;
  }

  if (direction == DirectionId::West && nextX > -stopLine) {
    nextX = -stopLine;
  }

  position.x = nextX;
  position.y = nextY;

  vehicle.setPosition(position);
  vehicle.setBraking(true);
  vehicle.setWaitingInQueue(true);
}