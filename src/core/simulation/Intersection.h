#pragma once

#include <vector>

#include "Vector2D.h"
#include "../entities/Lane.h"
#include "../entities/TrafficLight.h"

class Intersection {
public:
  Intersection(const Vector2D& center);

  Vector2D getCenter() const;

  void addLane(Lane* lane);
  void addTrafficLight(TrafficLight* trafficLight);

  const std::vector<Lane*>& getLanes() const;
  const std::vector<TrafficLight*>& getTrafficLights() const;

private:
  Vector2D m_center;
  std::vector<Lane*> m_lanes;
  std::vector<TrafficLight*> m_trafficLights;
};