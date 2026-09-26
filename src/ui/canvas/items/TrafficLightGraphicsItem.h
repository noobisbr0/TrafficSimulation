#pragma once

#include <QGraphicsItem>
#include "common/ISimulationEngine.h"

class TrafficLightGraphicsItem : public QGraphicsItem {
public:
    explicit TrafficLightGraphicsItem(const TrafficLightRenderData& data, QGraphicsItem* parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    TrafficLightRenderData m_data;
};