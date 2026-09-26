#pragma once

#include <QGraphicsScene>
#include <QList>
#include "common/ISimulationEngine.h"
#include "common/SimulationConfig.h"

class SimulationScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit SimulationScene(QObject* parent = nullptr);

    void drawRoadInfrastructure();
    void drawZebra(double x, double y, double width, double height, bool verticalStripes);

public slots:
    void updateState(const SimulationSnapshot& snapshot);
    void updateConfig(const SimulationConfig& config);

private:
    SimulationConfig m_config;
    QList<QGraphicsItem*> m_dynamicItems;
};