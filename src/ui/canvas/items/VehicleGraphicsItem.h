#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include <cmath>
#include "common/ISimulationEngine.h"

class VehicleGraphicsItem : public QGraphicsItem {
    VehicleRenderData m_data;
    double m_simTime;

public:
    explicit VehicleGraphicsItem(const VehicleRenderData& data, double simTime, QGraphicsItem* parent = nullptr)
        : QGraphicsItem(parent), m_data(data), m_simTime(simTime) {
        setPos(m_data.position.x, m_data.position.y);
        setRotation(m_data.angleDeg);
        setZValue(50);
    }

    QRectF boundingRect() const override {
        return QRectF(-2.25, -0.9, 4.5, 1.8); // 4.5м x 1.8м
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);

        // Цветовая дифференциация по ТЗ
        // Определение цвета кузова
        QColor bodyColor;
        if (m_data.hasCustomColor) {
            bodyColor = QColor(m_data.customColor.r, m_data.customColor.g, m_data.customColor.b);
        } else {
            switch (m_data.turnDirection) {
            case TurnDirection::Straight: bodyColor = QColor("#2196F3"); break; // Синий
            case TurnDirection::Left:     bodyColor = QColor("#FF9800"); break; // Оранжевый
            case TurnDirection::Right:    bodyColor = QColor("#9C27B0"); break; // Пурпурный
            }
        }

        // Корпус авто со скруглением углов
        QPainterPath path;
        path.addRoundedRect(boundingRect(), 0.5, 0.5);
        painter->fillPath(path, bodyColor);
        painter->setPen(QPen(Qt::white, 0.1));
        painter->drawPath(path);

        // Геометрические границы фар и сигналов (симметрично углам)
        const double w = 0.35; // Длина фары вдоль оси X
        const double h = 0.35; // Ширина фары вдоль оси Y
        const double xRear  = -2.20; // С небольшим отступом от кормы
        const double xFront =  1.85; // Передний край (2.20 - 0.35)
        const double yLeft  = -0.85; // Верхний (левый по ходу движения) край
        const double yRight =  0.50; // Нижний (правый) край до 0.85

        QRectF rearLeft(xRear, yLeft, w, h);
        QRectF rearRight(xRear, yRight, w, h);
        QRectF frontLeft(xFront, yLeft, w, h);
        QRectF frontRight(xFront, yRight, w, h);

        // Стоп-сигналы на корме при торможении
        if (m_data.isBraking) {
            painter->setBrush(QColor("#FF1744"));
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(rearLeft, 0.15, 0.15);
            painter->drawRoundedRect(rearRight, 0.15, 0.15);
        }

        // Поворотники (пульсация 1.5 Гц)
        bool blinkOn = std::fmod(m_simTime, 0.66) < 0.33;
        if (blinkOn && m_data.turnDirection != TurnDirection::Straight) {
            painter->setBrush(QColor("#FFD600"));
            painter->setPen(Qt::NoPen);

            if (m_data.turnDirection == TurnDirection::Left) {
                // Передний и задний левый углы
                painter->drawRoundedRect(frontLeft, 0.15, 0.15);
                painter->drawRoundedRect(rearLeft, 0.15, 0.15);
            } else if (m_data.turnDirection == TurnDirection::Right) {
                // Передний и задний правый углы
                painter->drawRoundedRect(frontRight, 0.15, 0.15);
                painter->drawRoundedRect(rearRight, 0.15, 0.15);
            }
        }
    }
};