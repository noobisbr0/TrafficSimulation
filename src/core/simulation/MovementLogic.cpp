#include "MovementLogic.h"

#include <cmath>
double MovementLogic::calculateIDMAcceleration(
    const Vehicle& vehicle,
    double gap,
    double leadSpeedKmh) {

  const double aMax = 1.5;
  const double comfortableDeceleration = 2.0;
  const double minimumGap = 2.0;
  const double timeHeadway = 1.2;

  double speed = vehicle.getSpeed() / 3.6;
  double desiredSpeed = vehicle.getDesiredSpeed() / 3.6;
  double leadSpeed = leadSpeedKmh / 3.6;

  if (desiredSpeed < 1.0) {
    desiredSpeed = 1.0;
  }

  if (gap < 0.5) {
    gap = 0.5;
  }

  double deltaSpeed = speed - leadSpeed;

  double desiredGap =
      minimumGap
      + speed * timeHeadway
      + (speed * deltaSpeed) /
        (2.0 * std::sqrt(aMax * comfortableDeceleration));

  double acceleration = aMax * (1.0 - std::pow(speed / desiredSpeed, 4.0) - std::pow(desiredGap / gap, 2.0));

  if (acceleration < -4.5) {
    acceleration = -4.5;
  }

  if (acceleration > aMax) {
    acceleration = aMax;
  }

  return acceleration;
}

void MovementLogic::moveVehicle(Vehicle& vehicle, double dt) {
  if (dt <= 0.0) {
    return;
  }

  double currentSpeed = vehicle.getSpeed() / 3.6;
  double acceleration = vehicle.getAcceleration();

  double newSpeed = currentSpeed + acceleration * dt;

  if (newSpeed < 0.0) {
    newSpeed = 0.0;
  }

  double distance =
      currentSpeed * dt +
      0.5 * acceleration * dt * dt;

  if (distance < 0.0) {
    distance = 0.0;
  }

  double angleRadians = vehicle.getAngleDeg() * std::acos(-1.0) / 180.0;
  Vector2D position = vehicle.getPosition();
  position.x += std::cos(angleRadians) * distance;
  position.y += std::sin(angleRadians) * distance;

  vehicle.setPosition(position);
  vehicle.setSpeed(newSpeed * 3.6);
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

void MovementLogic::updateTurn(Vehicle& vehicle, DirectionId direction) {
  if (vehicle.isTurning()) {
    return;
  }

  TurnDirection turn = vehicle.getTurnDirection();

  if (turn == TurnDirection::Straight) {
    return;
  }

  double targetAngle = vehicle.getAngleDeg();

  if (direction == DirectionId::North) {
    if (turn == TurnDirection::Left) {
      targetAngle = 0.0;
    } else if (turn == TurnDirection::Right) {
      targetAngle = 180.0;
    }
  } else if (direction == DirectionId::South) {
    if (turn == TurnDirection::Left) {
      targetAngle = 180.0;
    } else if (turn == TurnDirection::Right) {
      targetAngle = 0.0;
    }
  } else if (direction == DirectionId::East) {
    if (turn == TurnDirection::Left) {
      targetAngle = 90.0;
    } else if (turn == TurnDirection::Right) {
      targetAngle = -90.0;
    }
  } else if (direction == DirectionId::West) {
    if (turn == TurnDirection::Left) {
      targetAngle = -90.0;
    } else if (turn == TurnDirection::Right) {
      targetAngle = 90.0;
    }
  }

  vehicle.setTargetAngleDeg(targetAngle);
  vehicle.setTurnProgress(0.0);
  vehicle.setTurning(true);
}

void MovementLogic::processTurn(Vehicle& vehicle, double dt) {
  if (!vehicle.isTurning() || dt <= 0.0) {
    return;
  }
  double targetAngle = vehicle.getTargetAngleDeg();

  double progress = vehicle.getTurnProgress();

  double currentAngle = vehicle.getAngleDeg();
  double angleDifference = targetAngle - currentAngle;

  if (angleDifference > 180.0) {
      angleDifference -= 360.0;
  }

  if (angleDifference < -180.0) {
      angleDifference += 360.0;
  }

  vehicle.setAngleDeg(currentAngle + angleDifference * 1.5 * dt);

  progress += dt * 1.5;

  if (progress >= 1.0) {
    progress = 1.0;
    vehicle.setAngleDeg(targetAngle);
    vehicle.setTurnProgress(progress);
    vehicle.setTurning(false);
    return;
  }
  vehicle.setAngleDeg(currentAngle);
}