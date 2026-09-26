#include "SimulationView.h"
#include <QWheelEvent>
#include <algorithm>

SimulationView::SimulationView(QWidget* parent) : QGraphicsView(parent) {
    setRenderHints(QPainter::Antialiasing |
                   QPainter::TextAntialiasing |
                   QPainter::SmoothPixmapTransform);

    setDragMode(QGraphicsView::ScrollHandDrag);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    applyScale();
}

void SimulationView::wheelEvent(QWheelEvent* event) {
    constexpr double zoomStep = 1.15;
    double targetZoom = m_currentZoomFactor;

    if (event->angleDelta().y() > 0) {
        targetZoom *= zoomStep;
    } else if (event->angleDelta().y() < 0) {
        targetZoom /= zoomStep;
    } else {
        return;
    }

    targetZoom = std::clamp(targetZoom, kMinScaleFactor, kMaxScaleFactor);

    if (targetZoom != m_currentZoomFactor) {
        const double factor = targetZoom / m_currentZoomFactor;
        m_currentZoomFactor = targetZoom;
        scale(factor, factor);
    }

    event->accept();
}

void SimulationView::applyScale() {
    resetTransform();
    scale(kBaseScale * m_currentZoomFactor, kBaseScale * m_currentZoomFactor);
}