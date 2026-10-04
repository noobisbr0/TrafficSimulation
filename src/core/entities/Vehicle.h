#pragma once

#include "Vector2D.h"
#include "Types.h"

class Vehicle {
public:
  Vehicle(int id, const Vector2D& position, double speed, TurnDirection turnDirection);
  
  int getId() const;
  Vector2D getPosition() const;
  double getSpeed() const;
  double getDesiredSpeed() const;
  double getAcceleration() const;
  double getAngleDeg() const;
  TurnDirection getTurnDirection() const;
  DirectionId getApproachDirection() const;
  int getLaneId() const;
  
  bool isTurning() const;
  double getTargetAngleDeg() const;

  void setTurning(bool turning);
  void setTargetAngleDeg(double angleDeg);
  void setTurnProgress(double progress);
  double getTurnProgress() const;

  bool isBraking() const;
  bool isWaitingInQueue() const;

  void setPosition(const Vector2D& position);
  void setSpeed(double speed);
  void setDesiredSpeed(double speed);
  void setAcceleration(double acceleration);
  void setAngleDeg(double angleDeg);
  void setApproachDirection(DirectionId direction);
  void setLaneId(int laneId);
  void setBraking(bool braking);
  void setWaitingInQueue(bool waiting);

private:
  int m_id;
  Vector2D m_position;
  double m_speed;
  double m_desiredSpeedKmh{0.0};
  double m_acceleration{0.0};
  double m_angleDeg;
  TurnDirection m_turnDirection;
  bool m_isTurning;
  double m_targetAngleDeg;
  double m_turnProgress;
  DirectionId m_approachDirection;
  int m_laneId;

  bool m_isBraking;
  bool m_isWaitingInQueue;
};