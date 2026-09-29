#pragma once

#include "Vector2D.h"

class Pedestrian {
public:
  Pedestrian(int id, const Vector2D& position);

  int getId() const;
  Vector2D getPosition() const;

  bool isWaiting() const;

  void setPosition(const Vector2D& position);
  void setWaiting(bool waiting);

private:
  int m_id;
  Vector2D m_position;
  bool m_isWaiting;
};