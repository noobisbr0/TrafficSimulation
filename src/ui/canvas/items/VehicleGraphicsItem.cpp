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
    return QRectF(-2.3, -1.05, 4.6, 2.1);
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

    const QRectF bodyRect(-2.25, -0.9, 4.5, 1.8);
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(bodyRect, 0.5, 0.5);
    painter->fillPath(bodyPath, bodyColor);
    painter->setPen(QPen(bodyColor.darker(140), 0.08));
    painter->drawPath(bodyPath);

    painter->setBrush(bodyColor.darker(115));
    painter->setPen(QPen(bodyColor.darker(145), 0.04));
    painter->drawRoundedRect(QRectF(0.55, -1.02, 0.24, 0.12), 0.04, 0.04);
    painter->drawRoundedRect(QRectF(0.55, 0.90, 0.24, 0.12), 0.04, 0.04);

    const QColor glassColor("#151D28");
    const QPen glassPen(QColor("#0D121A"), 0.05);

    QPainterPath windshield;
    windshield.moveTo(0.25, -0.66);
    windshield.lineTo(0.70, -0.56);
    windshield.quadTo(0.80, 0.0, 0.70, 0.56);
    windshield.lineTo(0.25, 0.66);
    windshield.closeSubpath();

    painter->setPen(glassPen);
    painter->setBrush(glassColor);
    painter->drawPath(windshield);

    painter->setPen(QPen(QColor(255, 255, 255, 65), 0.08, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(0.38, -0.38), QPointF(0.58, -0.14));

    QPainterPath rearWindow;
    rearWindow.moveTo(-0.85, -0.66);
    rearWindow.lineTo(-1.30, -0.58);
    rearWindow.quadTo(-1.37, 0.0, -1.30, 0.58);
    rearWindow.lineTo(-0.85, 0.66);
    rearWindow.closeSubpath();

    painter->setPen(glassPen);
    painter->setBrush(glassColor);
    painter->drawPath(rearWindow);

    painter->setBrush(glassColor);
    painter->setPen(glassPen);
    painter->drawRect(QRectF(-0.82, -0.68, 1.04, 0.10));
    painter->drawRect(QRectF(-0.82, 0.58, 1.04, 0.10));

    painter->setBrush(bodyColor.lighter(106));
    painter->setPen(QPen(bodyColor.darker(125), 0.05));
    painter->drawRoundedRect(QRectF(-0.82, -0.58, 1.04, 1.16), 0.15, 0.15);

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

    painter->setBrush(QColor("#ECEFF1"));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(frontLeft, 0.15, 0.15);
    painter->drawRoundedRect(frontRight, 0.15, 0.15);

    painter->setBrush(m_data.isBraking ? QColor("#FF1744") : QColor("#80121D"));
    painter->drawRoundedRect(rearLeft, 0.15, 0.15);
    painter->drawRoundedRect(rearRight, 0.15, 0.15);

    const bool blinkOn = std::fmod(m_simTime, 0.66) < 0.33;
    if (blinkOn && m_data.turnDirection != TurnDirection::Straight) {
        painter->setBrush(QColor("#FFD600"));
        if (m_data.turnDirection == TurnDirection::Left) {
            painter->drawRoundedRect(frontLeft, 0.15, 0.15);
            painter->drawRoundedRect(rearLeft, 0.15, 0.15);
        } else if (m_data.turnDirection == TurnDirection::Right) {
            painter->drawRoundedRect(frontRight, 0.15, 0.15);
            painter->drawRoundedRect(rearRight, 0.15, 0.15);
        }
    }
}