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