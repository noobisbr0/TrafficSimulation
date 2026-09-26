#include "PedestrianTrafficLightGraphicsItem.h"
#include <QPainter>

PedestrianTrafficLightGraphicsItem::PedestrianTrafficLightGraphicsItem(const PedestrianTrafficLightRenderData& data, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_data(data) {
    setPos(m_data.position.x, m_data.position.y);
    setZValue(95);
}

QRectF PedestrianTrafficLightGraphicsItem::boundingRect() const {
    return QRectF(-4.0, -4.0, 8.0, 8.0);
}

void PedestrianTrafficLightGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    const int c = m_data.corner;

    auto drawBox = [&](double cx, double cy, bool isHoriz, PedestrianLightSignal sig) {
        const double w = isHoriz ? 2.4 : 1.0;
        const double h = isHoriz ? 1.0 : 2.4;
        const QRectF rect(cx - w / 2.0, cy - h / 2.0, w, h);

        painter->setBrush(QColor("#111115"));
        painter->setPen(QPen(Qt::black, 0.1));
        painter->drawRoundedRect(rect, 0.3, 0.3);

        const bool isRed = (sig == PedestrianLightSignal::Red);
        const bool isGreen = (sig == PedestrianLightSignal::Green);

        const double rX = isHoriz ? cx - 0.6 : cx;
        const double rY = isHoriz ? cy : cy - 0.6;
        const double gX = isHoriz ? cx + 0.6 : cx;
        const double gY = isHoriz ? cy : cy + 0.6;
        constexpr double radius = 0.35;

        painter->setBrush(isRed ? QColor("#FF1744") : QColor("#2B2D42"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(rX, rY), radius, radius);

        painter->setBrush(isGreen ? QColor("#00E676") : QColor("#2B2D42"));
        painter->drawEllipse(QPointF(gX, gY), radius, radius);
    };

    double hBoxX = 0.0;
    double hBoxY = 0.0;
    double vBoxX = 0.0;
    double vBoxY = 0.0;

    if (c == 0) {
        hBoxX = 1.0;  hBoxY = -2.0;
        vBoxX = -2.0; vBoxY = 1.0;
    } else if (c == 1) {
        hBoxX = -1.0; hBoxY = -2.0;
        vBoxX = 2.0;  vBoxY = 1.0;
    } else if (c == 2) {
        hBoxX = 1.0;  hBoxY = 2.0;
        vBoxX = -2.0; vBoxY = -1.0;
    } else if (c == 3) {
        hBoxX = -1.0; hBoxY = 2.0;
        vBoxX = 2.0;  vBoxY = -1.0;
    }

    painter->setPen(QPen(QColor("#252836"), 0.3, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(0, 0), QPointF(hBoxX, hBoxY));
    painter->drawLine(QPointF(0, 0), QPointF(vBoxX, vBoxY));

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#3A3F55"));
    painter->drawEllipse(QPointF(0, 0), 0.4, 0.4);

    drawBox(hBoxX, hBoxY, true, m_data.signalEW);
    drawBox(vBoxX, vBoxY, false, m_data.signalNS);
}