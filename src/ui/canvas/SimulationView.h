#pragma once

#include <QGraphicsView>

class SimulationView : public QGraphicsView {
    Q_OBJECT
public:
    explicit SimulationView(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    void applyScale();

    static constexpr double kBaseScale{10.0};
    static constexpr double kMinScaleFactor{0.5};
    static constexpr double kMaxScaleFactor{3.0};
    double m_currentZoomFactor{1.0};
};