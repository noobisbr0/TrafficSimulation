#include "TrafficLight.h"

TrafficLight::TrafficLight(int id, DirectionId direction)
    : m_id(id),
    m_direction(direction),
    m_color(LightColor::Red),
    m_hasLeftArrow(false),
    m_leftArrowGreen(false),
    m_allowsParallelPeds(false),
    m_parallelPedsGreen(false) {
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

void TrafficLight::setLeftArrowGreen(bool green) {
    m_leftArrowGreen = green;
}

bool TrafficLight::allowsParallelPeds() const {
    return m_allowsParallelPeds;
}

bool TrafficLight::isParallelPedsGreen() const {
    return m_parallelPedsGreen;
}

void TrafficLight::setParallelPedsGreen(bool green) {
    m_parallelPedsGreen = green;
}

void TrafficLight::setColor(LightColor color) {
    m_color = color;
}

void TrafficLight::setLeftArrow(bool hasArrow, bool isGreen) {
    m_hasLeftArrow = hasArrow;
    m_leftArrowGreen = isGreen;
}

void TrafficLight::setParallelPeds(bool hasArrow, bool isGreen) {
    m_allowsParallelPeds = hasArrow;
    m_parallelPedsGreen = isGreen;
}