#include "Pedestrian.h"

Pedestrian::Pedestrian(int id, const Vector2D& position)
    : m_id(id),
    m_position(position),
    m_speed(1.2),
    m_targetCrossing(DirectionId::North),
    m_moveDirection(DirectionId::North),
    m_isWaiting(true) {
}

int Pedestrian::getId() const {
    return m_id;
}

Vector2D Pedestrian::getPosition() const {
    return m_position;
}

double Pedestrian::getSpeed() const {
    return m_speed;
}

DirectionId Pedestrian::getTargetCrossing() const {
    return m_targetCrossing;
}

DirectionId Pedestrian::getMoveDirection() const {
    return m_moveDirection;
}

bool Pedestrian::isWaiting() const {
    return m_isWaiting;
}

void Pedestrian::setPosition(const Vector2D& position) {
    m_position = position;
}

void Pedestrian::setSpeed(double speed) {
    m_speed = speed;
}

void Pedestrian::setTargetCrossing(DirectionId crossing) {
    m_targetCrossing = crossing;
}

void Pedestrian::setMoveDirection(DirectionId direction) {
    m_moveDirection = direction;
}

void Pedestrian::setWaiting(bool waiting) {
    m_isWaiting = waiting;
}