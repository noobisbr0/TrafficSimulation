#pragma once

#include <QGraphicsItem>
#include "common/Types.h"

class PedestrianTrafficLightGraphicsItem : public QGraphicsItem {
public:
    explicit PedestrianTrafficLightGraphicsItem(const PedestrianTrafficLightRenderData& data, QGraphicsItem* parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    PedestrianTrafficLightRenderData m_data;
};