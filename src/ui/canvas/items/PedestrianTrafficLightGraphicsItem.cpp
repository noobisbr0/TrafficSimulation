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

    struct BoxOffsetPair {
        QPointF hBox;
        QPointF vBox;
    };

    static constexpr BoxOffsetPair kOffsets[4] = {
        { { 1.0, -2.0}, {-2.0,  1.0} },
        { {-1.0, -2.0}, { 2.0,  1.0} },
        { { 1.0,  2.0}, {-2.0, -1.0} },
        { {-1.0,  2.0}, { 2.0, -1.0} }
    };

    const int c = m_data.corner;
    const auto [hBox, vBox] = (c >= 0 && c < 4) ? kOffsets[c] : kOffsets[0];

    painter->setPen(QPen(QColor("#252836"), 0.3, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(0, 0), hBox);
    painter->drawLine(QPointF(0, 0), vBox);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#3A3F55"));
    painter->drawEllipse(QPointF(0, 0), 0.4, 0.4);

    drawBox(hBox.x(), hBox.y(), true, m_data.signalEW);
    drawBox(vBox.x(), vBox.y(), false, m_data.signalNS);
}