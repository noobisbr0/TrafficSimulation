#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include "common/ISimulationEngine.h"

class TrafficLightGraphicsItem : public QGraphicsItem {
    TrafficLightRenderData m_data;
public:
    explicit TrafficLightGraphicsItem(const TrafficLightRenderData& data, QGraphicsItem* parent = nullptr)
        : QGraphicsItem(parent), m_data(data) {
        setZValue(100);
    }

    QRectF boundingRect() const override {
        // Выделяем место СЛЕВА под доп. секцию левого поворота
        double left = m_data.hasLeftArrow ? -3.5 : -1.5;
        return QRectF(left, -3.5, 1.5 - left, 7.0);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);

        // Основной корпус светофора
        painter->setPen(QPen(Qt::black, 0.1));
        painter->setBrush(QColor("#111115"));
        painter->drawRoundedRect(-1.2, -3.2, 2.4, 6.4, 0.5, 0.5);

        auto drawLens = [&](double x, double y, LightColor color, LightColor activeColor) {
            bool isActive = (m_data.mainColor == color || m_data.mainColor == LightColor::RedYellow);
            if (color == LightColor::Yellow && m_data.mainColor == LightColor::Yellow) isActive = true;

            QColor activeHex = (activeColor == LightColor::Red) ? QColor("#FF1744") :
                                   (activeColor == LightColor::Yellow) ? QColor("#FFEA00") : QColor("#00E676");
            painter->setBrush(isActive ? activeHex : QColor("#2B2D42"));
            painter->drawEllipse(QRectF(x - 0.8, y - 0.8, 1.6, 1.6));
        };

        drawLens(0, -2.0, LightColor::Red, LightColor::Red);
        drawLens(0, 0.0, LightColor::Yellow, LightColor::Yellow);
        drawLens(0, 2.0, LightColor::Green, LightColor::Green);

        // Дополнительная секция для левого поворота
        if (m_data.hasLeftArrow) {
            // Рисуем секцию СЛЕВА от основного корпуса
            painter->setBrush(QColor("#111115"));
            painter->drawRoundedRect(-3.2, 1.0, 2.0, 2.0, 0.3, 0.3);

            bool active = m_data.leftArrowGreen;
            painter->setBrush(active ? QColor("#00E676") : QColor("#2B2D42"));
            painter->drawEllipse(QRectF(-3.0, 1.2, 1.6, 1.6));

            // Рисуем контур стрелки налево
            painter->setPen(QPen(active ? Qt::black : QColor("#111115"), 0.25, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawLine(QPointF(-1.7, 2.0), QPointF(-2.5, 2.0));
            painter->drawLine(QPointF(-2.5, 2.0), QPointF(-2.2, 1.7));
            painter->drawLine(QPointF(-2.5, 2.0), QPointF(-2.2, 2.3));
        }
    }
};