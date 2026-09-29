#include "TrafficLight.h"

TrafficLight::TrafficLight(int id, DirectionId direction)
    : m_id(id),
      m_direction(direction),
      m_color(LightColor::Red),
      m_hasLeftArrow(false),
      m_leftArrowGreen(false),
      m_hasRightArrow(false),
      m_rightArrowGreen(false) {
}

int TrafficLight::getId() const {
    return m_id;
}

DirectionId TrafficLight::getDirection() const {
    return m_direction;
}

LightColor TrafficLight::getColor() const {
    return m_color;
}

bool TrafficLight::hasLeftArrow() const {
    return m_hasLeftArrow;
}

bool TrafficLight::isLeftArrowGreen() const {
    return m_leftArrowGreen;
}

bool TrafficLight::hasRightArrow() const {
    return m_hasRightArrow;
}

bool TrafficLight::isRightArrowGreen() const {
    return m_rightArrowGreen;
}

void TrafficLight::setColor(LightColor color) {
    m_color = color;
}

void TrafficLight::setLeftArrow(bool hasArrow, bool isGreen) {
    m_hasLeftArrow = hasArrow;
    m_leftArrowGreen = isGreen;
}

void TrafficLight::setRightArrow(bool hasArrow, bool isGreen) {
    m_hasRightArrow = hasArrow;
    m_rightArrowGreen = isGreen;
}