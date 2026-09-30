#include "SimulationScene.h"
#include "items/VehicleGraphicsItem.h"
#include "items/TrafficLightGraphicsItem.h"
#include "items/PedestrianGraphicsItem.h"

SimulationScene::SimulationScene(QObject* parent) : QGraphicsScene(parent) {
    setSceneRect(-100, -100, 200, 200);
    drawRoadInfrastructure();
}

void SimulationScene::updateConfig(const SimulationConfig& config) {
    m_config = config;
    drawRoadInfrastructure();
}

void SimulationScene::drawZebra(double x, double y, double width, double height, bool verticalStripes) {
    QPainterPath path;
    if (verticalStripes) {
        for (double i = x; i < x + width; i += 1.0) path.addRect(i + 0.2, y, 0.4, height);
    } else {
        for (double i = y; i < y + height; i += 1.0) path.addRect(x, i + 0.2, width, 0.4);
    }
    addPath(path, Qt::NoPen, QColor("#D0D0D0"));
}

void SimulationScene::drawRoadInfrastructure() {
    clear();
    m_dynamicItems.clear();
    setBackgroundBrush(QColor("#1E1E24")); // Темный органичный фон окружения

    double hw_NS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
    double hw_EW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;

    // 1. Широкие площадки тротуаров
    QPainterPath sidewalks;
    double cr = 3.5;
    sidewalks.addRoundedRect(-100, -100, 100 - hw_NS, 100 - hw_EW, cr, cr);
    sidewalks.addRoundedRect(hw_NS, -100, 100 - hw_NS, 100 - hw_EW, cr, cr);
    sidewalks.addRoundedRect(-100, hw_EW, 100 - hw_NS, 100 - hw_EW, cr, cr);
    sidewalks.addRoundedRect(hw_NS, hw_EW, 100 - hw_NS, 100 - hw_EW, cr, cr);

    QPen curbPen(QColor("#3A3F55"), 0.8);
    QBrush sidewalkBrush(QColor("#252836")); // Чуть светлее фона, но не режет глаза
    addPath(sidewalks, curbPen, sidewalkBrush);

    // 2. Асфальт проезжей части
    QPainterPath roads;
    roads.setFillRule(Qt::WindingFill); // Гарантирует сплошную заливку центра перекрестка
    roads.addRect(-100, -hw_EW, 200, hw_EW * 2);
    roads.addRect(-hw_NS, -100, hw_NS * 2, 200);
    addPath(roads, Qt::NoPen, QColor("#2B2D42"));

    QPen centerPen(QColor("#F4D03F"), 0.15, Qt::SolidLine);
    QPen dashedPen(QColor("#FFFFFF"), 0.15, Qt::DashLine);
    QPen stopPen(QColor("#FFFFFF"), 0.15, Qt::SolidLine); // Тонкая стоп-линия

    // Осевые линии (обрезаны до стоп-линий с отступом 4.0)
    addLine(-100, 0, -hw_NS - 4.0, 0, centerPen);
    addLine(hw_NS + 4.0, 0, 100, 0, centerPen);
    addLine(0, -100, 0, -hw_EW - 4.0, centerPen);
    addLine(0, hw_EW + 4.0, 0, 100, centerPen);

    // Прерывистые линии между полосами (горизонтальные дороги)
    for (double y = 3.5; y < hw_EW; y += 3.5) {
        addLine(-100, y, -hw_NS - 4.0, y, dashedPen);
        addLine(hw_NS + 4.0, y, 100, y, dashedPen);
        addLine(-100, -y, -hw_NS - 4.0, -y, dashedPen);
        addLine(hw_NS + 4.0, -y, 100, -y, dashedPen);
    }
    // Прерывистые линии между полосами (вертикальные дороги)
    for (double x = 3.5; x < hw_NS; x += 3.5) {
        addLine(x, -100, x, -hw_EW - 4.0, dashedPen);
        addLine(x, hw_EW + 4.0, x, 100, dashedPen);
        addLine(-x, -100, -x, -hw_EW - 4.0, dashedPen);
        addLine(-x, hw_EW + 4.0, -x, 100, dashedPen);
    }

    // Стоп-линии с отступом перед зеброй
    addLine(-hw_NS, -hw_EW - 4.0, hw_NS, -hw_EW - 4.0, stopPen);
    addLine(-hw_NS, hw_EW + 4.0, hw_NS, hw_EW + 4.0, stopPen);
    addLine(-hw_NS - 4.0, -hw_EW, -hw_NS - 4.0, hw_EW, stopPen);
    addLine(hw_NS + 4.0, -hw_EW, hw_NS + 4.0, hw_EW, stopPen);

    drawZebra(-hw_NS, -hw_EW - 3.5, hw_NS * 2, 3.0, true);
    drawZebra(-hw_NS, hw_EW + 0.5, hw_NS * 2, 3.0, true);
    drawZebra(-hw_NS - 3.5, -hw_EW, 3.0, hw_EW * 2, false);
    drawZebra(hw_NS + 0.5, -hw_EW, 3.0, hw_EW * 2, false);
}

void SimulationScene::updateState(const SimulationSnapshot& snapshot) {
    for (auto* item : m_dynamicItems) {
        removeItem(item);
        delete item;
    }
    m_dynamicItems.clear();

    // 2. Добавляем новые элементы
    double hw_NS = (m_config.topology == IntersectionTopology::Lanes_3x3) ? 10.5 : 7.0;
    double hw_EW = (m_config.topology == IntersectionTopology::Lanes_2x2) ? 7.0 : 10.5;

    for (const auto& tl : snapshot.trafficLights) {
        auto* item = new TrafficLightGraphicsItem(tl);

        // Отключение поворотов (светофоры всегда направлены вертикально)
        item->setRotation(0);

        // Позиционирование на правой обочине с отступом от зон ожидания пешеходов
        switch (tl.direction) {
        case DirectionId::North: // Подъезд с севера (автомобили едут вниз)
            item->setPos(-hw_NS - 4.0, -hw_EW - 9.0);
            break;
        case DirectionId::South: // Подъезд с юга (автомобили едут вверх)
            item->setPos(hw_NS + 4.0, hw_EW + 9.0);
            break;
        case DirectionId::West:  // Подъезд с запада (автомобили едут вправо)
            item->setPos(-hw_NS - 9.0, hw_EW + 4.0);
            break;
        case DirectionId::East:  // Подъезд с востока (автомобили едут влево)
            item->setPos(hw_NS + 9.0, -hw_EW - 4.0);
            break;
        }
        addItem(item);
        m_dynamicItems.append(item); // Сохраняем указатель
    }

    for (const auto& v : snapshot.vehicles) {
        auto* item = new VehicleGraphicsItem(v, snapshot.stats.currentSimTimeSec); // Передаем время для поворотников
        addItem(item);
        m_dynamicItems.append(item);
    }

    for (const auto& p : snapshot.pedestrians) {
        auto* item = new PedestrianGraphicsItem(p);
        addItem(item);
        m_dynamicItems.append(item);
    }
}