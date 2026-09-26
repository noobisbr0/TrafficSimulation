#include "VehicleGraphicsItem.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

VehicleGraphicsItem::VehicleGraphicsItem(const VehicleRenderData& data, double simTime, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_data(data), m_simTime(simTime) {
    setPos(m_data.position.x, m_data.position.y);
    setRotation(m_data.angleDeg);
    setZValue(50);
}

QRectF VehicleGraphicsItem::boundingRect() const {
    return QRectF(-2.25, -0.9, 4.5, 1.8);
}

void VehicleGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    QColor bodyColor;
    if (m_data.hasCustomColor) {
        bodyColor = QColor(m_data.customColor.r, m_data.customColor.g, m_data.customColor.b);
    } else {
        switch (m_data.turnDirection) {
        case TurnDirection::Straight:
            bodyColor = QColor("#2196F3");
            break;
        case TurnDirection::Left:
            bodyColor = QColor("#FF9800");
            break;
        case TurnDirection::Right:
            bodyColor = QColor("#9C27B0");
            break;
        }
    }

    QPainterPath path;
    path.addRoundedRect(boundingRect(), 0.5, 0.5);
    painter->fillPath(path, bodyColor);
    painter->setPen(QPen(Qt::white, 0.1));
    painter->drawPath(path);

    constexpr double w = 0.35;
    constexpr double h = 0.35;
    constexpr double xRear = -2.20;
    constexpr double xFront = 1.85;
    constexpr double yLeft = -0.85;
    constexpr double yRight = 0.50;

    const QRectF rearLeft(xRear, yLeft, w, h);
    const QRectF rearRight(xRear, yRight, w, h);
    const QRectF frontLeft(xFront, yLeft, w, h);
    const QRectF frontRight(xFront, yRight, w, h);

    if (m_data.isBraking) {
        painter->setBrush(QColor("#FF1744"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rearLeft, 0.15, 0.15);
        painter->drawRoundedRect(rearRight, 0.15, 0.15);
    }

    const bool blinkOn = std::fmod(m_simTime, 0.66) < 0.33;
    if (blinkOn && m_data.turnDirection != TurnDirection::Straight) {
        painter->setBrush(QColor("#FFD600"));
        painter->setPen(Qt::NoPen);

        if (m_data.turnDirection == TurnDirection::Left) {
            painter->drawRoundedRect(frontLeft, 0.15, 0.15);
            painter->drawRoundedRect(rearLeft, 0.15, 0.15);
        } else if (m_data.turnDirection == TurnDirection::Right) {
            painter->drawRoundedRect(frontRight, 0.15, 0.15);
            painter->drawRoundedRect(rearRight, 0.15, 0.15);
        }
    }
}