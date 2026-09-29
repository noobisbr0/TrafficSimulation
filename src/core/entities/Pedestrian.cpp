#include "Pedestrian.h"

Pedestrian::Pedestrian(int id, const Vector2D& position)
    : m_id(id),
      m_position(position),
      m_isWaiting(false) {
}

int Pedestrian::getId() const {
    return m_id;
}

Vector2D Pedestrian::getPosition() const {
    return m_position;
}

bool Pedestrian::isWaiting() const {
    return m_isWaiting;
}

void Pedestrian::setPosition(const Vector2D& position) {
    m_position = position;
}

void Pedestrian::setWaiting(bool waiting) {
    m_isWaiting = waiting;
}