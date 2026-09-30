#include "Vehicle.h"

Vehicle::Vehicle(int id, const Vector2D& position, double speed, TurnDirection turnDirection)
    : m_id(id),
      m_position(position),
      m_speed(speed),
      m_angleDeg(0.0),
      m_turnDirection(turnDirection),
      m_isBraking(false),
      m_isWaitingInQueue(false) {
}

int Vehicle::getId() const {
    return m_id;
}

Vector2D Vehicle::getPosition() const {
    return m_position;
}

double Vehicle::getSpeed() const {
    return m_speed;
}

double Vehicle::getAngleDeg() const {
    return m_angleDeg;
}

TurnDirection Vehicle::getTurnDirection() const {
    return m_turnDirection;
}

bool Vehicle::isBraking() const {
    return m_isBraking;
}

bool Vehicle::isWaitingInQueue() const {
    return m_isWaitingInQueue;
}

void Vehicle::setPosition(const Vector2D& position) {
    m_position = position;
}

void Vehicle::setSpeed(double speed) {
    m_speed = speed;
}

void Vehicle::setAngleDeg(double angleDeg) {
    m_angleDeg = angleDeg;
}

void Vehicle::setBraking(bool braking) {
    m_isBraking = braking;
}

void Vehicle::setWaitingInQueue(bool waiting) {
    m_isWaitingInQueue = waiting;
}