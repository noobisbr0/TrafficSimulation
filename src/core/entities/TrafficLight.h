#pragma once

#include "Types.h"

class TrafficLight {
public:
    TrafficLight(int id, DirectionId direction);

    int getId() const;
    DirectionId getDirection() const;

    LightColor getColor() const;
    bool hasLeftArrow() const;
    bool isLeftArrowGreen() const;
    void setLeftArrowGreen(bool green);
    bool hasRightArrow() const;
    bool isRightArrowGreen() const;
    void setRightArrowGreen(bool green);

    void setColor(LightColor color);
    void setLeftArrow(bool hasArrow, bool isGreen);
    void setRightArrow(bool hasArrow, bool isGreen);

private:
    int m_id;
    DirectionId m_direction;

    LightColor m_color;
    bool m_hasLeftArrow;
    bool m_leftArrowGreen;
    bool m_hasRightArrow;
    bool m_rightArrowGreen;
};