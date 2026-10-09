#pragma once

#include "Vector2D.h"
#include "../../common/Types.h"

class Pedestrian {
public:
    Pedestrian(int id, const Vector2D& position);

    int getId() const;
    Vector2D getPosition() const;
    double getSpeed() const;
    DirectionId getTargetCrossing() const;
    DirectionId getMoveDirection() const;
    bool isWaiting() const;

    void setPosition(const Vector2D& position);
    void setSpeed(double speed);
    void setTargetCrossing(DirectionId crossing);
    void setMoveDirection(DirectionId direction);
    void setWaiting(bool waiting);

private:
    int m_id;
    Vector2D m_position;
    double m_speed;
    DirectionId m_targetCrossing;
    DirectionId m_moveDirection;
    bool m_isWaiting;
};