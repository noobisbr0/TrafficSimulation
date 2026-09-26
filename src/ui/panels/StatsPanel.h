#pragma once
#include <QWidget>
#include <QLabel>
#include <deque>
#include <QPainter>
#include <QPainterPath>
#include "common/ISimulationEngine.h"

class QueueGraphWidget : public QWidget {
    Q_OBJECT
    std::deque<std::pair<double, int>> m_history;
public:
    explicit QueueGraphWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(120);
    }

    void clear() {
        m_history.clear();
        update();
    }

    void addData(double timeSec, int queueSize) {
        // Если время сбросили или откатили назад — очищаем историю
        if (!m_history.empty() && timeSec < m_history.back().first) {
            m_history.clear();
        }

        m_history.push_back({timeSec, queueSize});

        // Удаляем точки старше 60 секунд
        while (!m_history.empty() && timeSec - m_history.front().first > 60.0) {
            m_history.pop_front();
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#1E1E24"));
        p.setPen(QPen(QColor("#3A3F55"), 1));
        p.drawRect(0, 0, width() - 1, height() - 1);

        if (m_history.size() < 2) return;

        double minT = m_history.front().first;
        double maxT = m_history.back().first;
        int maxQ = 5;
        for (const auto& pt : m_history) maxQ = std::max(maxQ, pt.second);

        QPainterPath path;
        for (size_t i = 0; i < m_history.size(); ++i) {
            double x = width() * (m_history[i].first - minT) / std::max(1.0, maxT - minT);
            double y = height() - (height() * m_history[i].second / (maxQ * 1.2));
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }

        p.setPen(QPen(QColor("#3D5AFE"), 2));
        p.drawPath(path);
    }
};

class StatsPanel : public QWidget {
    Q_OBJECT
    QLabel *lblWaitTime, *lblQueue, *lblPassed;
    QueueGraphWidget *m_graph;
public:
    explicit StatsPanel(QWidget* parent = nullptr);

public slots:
    void updateStats(const SimulationSnapshot& snap);
    void resetStats(); // Новый слот сброса статистики
};