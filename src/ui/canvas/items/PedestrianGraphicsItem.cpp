#include "PedestrianGraphicsItem.h"
#include <QPainter>

PedestrianGraphicsItem::PedestrianGraphicsItem(const PedestrianRenderData& data, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_data(data) {
    setPos(m_data.position.x, m_data.position.y);
    setZValue(90);
}

QRectF PedestrianGraphicsItem::boundingRect() const {
    return QRectF(-0.6, -0.6, 1.2, 1.2);
}

void PedestrianGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::white, 0.15));

    QColor pedColor;
    if (m_data.hasCustomColor) {
        pedColor = QColor(m_data.customColor.r, m_data.customColor.g, m_data.customColor.b);
    } else {
        pedColor = m_data.isWaiting ? QColor("#FFA000") : QColor("#00E676");
    }

    painter->setBrush(pedColor);
    painter->drawEllipse(boundingRect());
}