#pragma once

#include <QGraphicsItem>
#include "common/ISimulationEngine.h"

class VehicleGraphicsItem : public QGraphicsItem {
public:
    explicit VehicleGraphicsItem(const VehicleRenderData& data, double simTime, QGraphicsItem* parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    VehicleRenderData m_data;
    double m_simTime{0.0};
};