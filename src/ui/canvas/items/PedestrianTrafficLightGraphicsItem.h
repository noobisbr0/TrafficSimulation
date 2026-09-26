#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include "common/Types.h"

class PedestrianTrafficLightGraphicsItem : public QGraphicsItem {
    PedestrianTrafficLightRenderData m_data;

public:
    explicit PedestrianTrafficLightGraphicsItem(const PedestrianTrafficLightRenderData& data, QGraphicsItem* parent = nullptr)
        : QGraphicsItem(parent), m_data(data) {
        setPos(m_data.position.x, m_data.position.y);
        setZValue(95); // Выше зебры и пешеходов, ниже авто-светофоров
    }

    QRectF boundingRect() const override {
        // Гарантированный запас для отрисовки всех секций вокруг угла
        return QRectF(-4.0, -4.0, 8.0, 8.0);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);

        int c = m_data.corner; // 0: Верх-Лево, 1: Верх-Право, 2: Низ-Лево, 3: Низ-Право

        // Лямбда для отрисовки чистого блока светофора (без лишних translate)
        auto drawBox = [&](double cx, double cy, bool isHoriz, PedestrianLightSignal sig) {
            double w = isHoriz ? 2.4 : 1.0;
            double h = isHoriz ? 1.0 : 2.4;
            QRectF rect(cx - w / 2.0, cy - h / 2.0, w, h);

            // Темный корпус
            painter->setBrush(QColor("#111115"));
            painter->setPen(QPen(Qt::black, 0.1));
            painter->drawRoundedRect(rect, 0.3, 0.3);

            bool isRed = (sig == PedestrianLightSignal::Red);
            bool isGreen = (sig == PedestrianLightSignal::Green);

            // Координаты центров линз
            double rX = isHoriz ? cx - 0.6 : cx;
            double rY = isHoriz ? cy : cy - 0.6;
            double gX = isHoriz ? cx + 0.6 : cx;
            double gY = isHoriz ? cy : cy + 0.6;
            double radius = 0.35;

            // Красная линза
            painter->setBrush(isRed ? QColor("#FF1744") : QColor("#2B2D42"));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(rX, rY), radius, radius);

            // Зеленая линза
            painter->setBrush(isGreen ? QColor("#00E676") : QColor("#2B2D42"));
            painter->drawEllipse(QPointF(gX, gY), radius, radius);
        };

        // Координаты размещения секций относительно угла (0,0)
        double hBoxX = 0, hBoxY = 0; // Горизонтальная секция (для вертикального пешеходного потока)
        double vBoxX = 0, vBoxY = 0; // Вертикальная секция (для горизонтального пешеходного потока)

        // Центры зебр находятся на расстоянии ~2.0 от геометрического угла перекрестка
        if (c == 0) { // Верхний-Левый
            hBoxX = 1.0;  hBoxY = -2.0;
            vBoxX = -2.0; vBoxY = 1.0;
        } else if (c == 1) { // Верхний-Правый
            hBoxX = -1.0; hBoxY = -2.0;
            vBoxX = 2.0;  vBoxY = 1.0;
        } else if (c == 2) { // Нижний-Левый
            hBoxX = 1.0;  hBoxY = 2.0;
            vBoxX = -2.0; vBoxY = -1.0;
        } else if (c == 3) { // Нижний-Правый
            hBoxX = -1.0; hBoxY = 2.0;
            vBoxX = 2.0;  vBoxY = -1.0;
        }

        // 1. Отрисовка креплений (темные балки к секциям)
        painter->setPen(QPen(QColor("#252836"), 0.3, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(0, 0), QPointF(hBoxX, hBoxY));
        painter->drawLine(QPointF(0, 0), QPointF(vBoxX, vBoxY));

        // 2. Центральный опорный столб на углу
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#3A3F55"));
        painter->drawEllipse(QPointF(0, 0), 0.4, 0.4);

        // 3. Отрисовка самих светофорных блоков поверх креплений
        drawBox(hBoxX, hBoxY, true, m_data.signalEW);
        drawBox(vBoxX, vBoxY, false, m_data.signalNS);
    }
};