#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
#include <algorithm>

class SimulationView : public QGraphicsView {
    // Базовый масштаб: 10 px = 1 метр
    const double m_baseScale{10.0};
    // Ограничения приближения по ТЗ (0.5x ... 3.0x от базового масштаба)
    const double m_minScaleFactor{0.5};
    const double m_maxScaleFactor{3.0};
    double m_currentZoomFactor{1.0};

public:
    explicit SimulationView(QWidget* parent = nullptr) : QGraphicsView(parent) {
        // Качественный рендеринг без размытия и ступенек
        setRenderHints(QPainter::Antialiasing |
                       QPainter::TextAntialiasing |
                       QPainter::SmoothPixmapTransform);

        // Панорамирование сцены зажатием левой кнопки мыши
        setDragMode(QGraphicsView::ScrollHandDrag);
        setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

        // Приближение строго относительно текущего положения курсора мыши
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorUnderMouse);

        // Устанавливаем исходный масштаб (1.0x)
        applyScale();
    }

protected:
    void wheelEvent(QWheelEvent* event) override {
        // Шаг зума за один щелчок колесика (15%)
        const double zoomStep = 1.15;
        double targetZoom = m_currentZoomFactor;

        if (event->angleDelta().y() > 0) {
            targetZoom *= zoomStep; // Приближение
        } else if (event->angleDelta().y() < 0) {
            targetZoom /= zoomStep; // Отдаление
        } else {
            return;
        }

        // Ограничиваем диапазон в пределах 0.5x ... 3.0x
        targetZoom = std::clamp(targetZoom, m_minScaleFactor, m_maxScaleFactor);

        if (targetZoom != m_currentZoomFactor) {
            // Вычисляем относительный коэффициент для текущего шага
            double factor = targetZoom / m_currentZoomFactor;
            m_currentZoomFactor = targetZoom;

            // Масштабируем строго симметрично по обеим осям, исключая искажения
            scale(factor, factor);
        }

        event->accept();
    }

private:
    void applyScale() {
        resetTransform();
        scale(m_baseScale * m_currentZoomFactor, m_baseScale * m_currentZoomFactor);
    }
};