#pragma once

#include "Vector2D.h"
#include "Types.h"

class Vehicle {
public:
  Vehicle(int id, const Vector2D& position, double speed, TurnDirection turnDirection);
  
  int getId() const;
  Vector2D getPosition() const;
  double getSpeed() const;
  double getAngleDeg() const;
  TurnDirection getTurnDirection() const;

  bool isBraking() const;
  bool isWaitingInQueue() const;

  void setPosition(const Vector2D& position);
  void setSpeed(double speed);
  void setAngleDeg(double angleDeg);
  void setBraking(bool braking);
  void setWaitingInQueue(bool waiting);

private:
  int m_id;
  Vector2D m_position;
  double m_speed;
  double m_angleDeg;
  TurnDirection m_turnDirection;

  bool m_isBraking;
  bool m_isWaitingInQueue;
};