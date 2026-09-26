#pragma once

#include <QGraphicsItem>
#include "common/ISimulationEngine.h"

class PedestrianGraphicsItem : public QGraphicsItem {
public:
    explicit PedestrianGraphicsItem(const PedestrianRenderData& data, QGraphicsItem* parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    PedestrianRenderData m_data;
};