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

void MovementLogic::startTurn(Vehicle& vehicle) {
  if (vehicle.isTurning() || vehicle.isTurnCompleted()) {
    return;
  }

  TurnDirection turn = vehicle.getTurnDirection();

  if (turn == TurnDirection::Straight) {
    return;
  }

  constexpr double radius = 3.5;
  constexpr double pi = 3.14159265358979323846;

  double heading =
      vehicle.getAngleDeg() * pi / 180.0;

  Vector2D position =
      vehicle.getPosition();

  // Вектор вправо относительно направления машины.
  Vector2D rightNormal(
      std::sin(heading),
      -std::cos(heading));

  Vector2D center;

  if (turn == TurnDirection::Right) {
    // Для правого поворота центр дуги справа.
    center =
        position + rightNormal * radius;
  } else {
    // Для левого поворота центр дуги слева.
    center =
        position - rightNormal * radius;
  }

  double startAngle =
      std::atan2(
          position.y - center.y,
          position.x - center.x);

  vehicle.setTurnCenter(center);
  vehicle.setTurnStartAngle(startAngle);
  vehicle.setTurnProgress(0.0);
  vehicle.setTurning(true);
  vehicle.setTurnCompleted(false);

  // Перед поворотом не разгоняем машину.
  if (vehicle.getSpeed() > 20.0) {
    vehicle.setSpeed(20.0);
  }
}

void MovementLogic::processTurn(Vehicle& vehicle, double dt) {
  if (!vehicle.isTurning() || dt <= 0.0) {
    return;
  }

  constexpr double radius = 3.5;
  constexpr double turnSpeedKmh = 20.0;
  constexpr double pi = 3.14159265358979323846;

  const double speedMs =
      turnSpeedKmh / 3.6;

  const double angularSpeed =
      speedMs / radius;

  double progress =
      vehicle.getTurnProgress();

  progress +=
      angularSpeed * dt / (pi / 2.0);

  if (progress >= 1.0) {
    progress = 1.0;
  }

  const double startAngle =
      vehicle.getTurnStartAngle();

  const Vector2D center =
      vehicle.getTurnCenter();

  double angle;

  if (vehicle.getTurnDirection() == TurnDirection::Right) {
    // По часовой стрелке.
    angle =
        startAngle -
        progress * (pi / 2.0);
  } else {
    // Против часовой стрелки.
    angle =
        startAngle +
        progress * (pi / 2.0);
  }

  Vector2D newPosition(
      center.x +
          radius * std::cos(angle),

      center.y +
          radius * std::sin(angle));

  double headingAngle;

  if (vehicle.getTurnDirection() == TurnDirection::Right) {
    // Касательная к окружности при движении по часовой.
    headingAngle =
        angle - pi / 2.0;
  } else {
    // Касательная при движении против часовой.
    headingAngle =
        angle + pi / 2.0;
  }

  double headingDeg =
      headingAngle * 180.0 / pi;

  while (headingDeg <= -180.0) {
    headingDeg += 360.0;
  }

  while (headingDeg > 180.0) {
    headingDeg -= 360.0;
  }

  vehicle.setPosition(newPosition);
  vehicle.setAngleDeg(headingDeg);
  vehicle.setSpeed(turnSpeedKmh);
  vehicle.setTurnProgress(progress);

  if (progress >= 1.0) {
    vehicle.setTurning(false);
    vehicle.setTurnCompleted(true);

    vehicle.setBraking(false);
    vehicle.setWaitingInQueue(false);

    DirectionId target =
        vehicle.getTargetDirection();

    double finalAngle;

    if (target == DirectionId::North) {
      finalAngle = -90.0;
    } else if (target == DirectionId::South) {
      finalAngle = 90.0;
    } else if (target == DirectionId::East) {
      finalAngle = 180.0;
    } else {
      finalAngle = 0.0;
    }

    vehicle.setAngleDeg(finalAngle);
    vehicle.setApproachDirection(target);

    // Дальше IDM снова управляет разгоном.
    vehicle.setAcceleration(0.0);
  }
}