#include "StatsPanel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

QueueGraphWidget::QueueGraphWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(120);
}

void QueueGraphWidget::clear() {
    m_history.clear();
    update();
}

void QueueGraphWidget::addData(double timeSec, int queueSize) {
    if (!m_history.empty() && timeSec < m_history.back().first) {
        m_history.clear();
    }

    m_history.emplace_back(timeSec, queueSize);

    while (!m_history.empty() && (timeSec - m_history.front().first) > 60.0) {
        m_history.pop_front();
    }
    update();
}

void QueueGraphWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#1E1E24"));
    p.setPen(QPen(QColor("#3A3F55"), 1));
    p.drawRect(0, 0, width() - 1, height() - 1);

    if (m_history.size() < 2) {
        return;
    }

    const double minT = m_history.front().first;
    const double maxT = m_history.back().first;
    int maxQ = 5;
    for (const auto& pt : m_history) {
        maxQ = std::max(maxQ, pt.second);
    }

    QPainterPath path;
    for (size_t i = 0; i < m_history.size(); ++i) {
        const double x = width() * (m_history[i].first - minT) / std::max(1.0, maxT - minT);
        const double y = height() - (height() * m_history[i].second / (maxQ * 1.2));
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    p.setPen(QPen(QColor("#3D5AFE"), 2));
    p.drawPath(path);
}

StatsPanel::StatsPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* gb = new QGroupBox("Статистика (В реальном времени)", this);
    auto* l = new QVBoxLayout(gb);

    m_lblWaitTime = new QLabel("Средняя задержка: 0.0 с", this);
    m_lblQueue = new QLabel("Текущая очередь: 0 авто", this);
    m_lblPassed = new QLabel("Машин проехало: 0", this);
    m_graph = new QueueGraphWidget(this);

    l->addWidget(m_lblWaitTime);
    l->addWidget(m_lblQueue);
    l->addWidget(m_lblPassed);
    l->addWidget(new QLabel("Динамика очередей (последние 60с):", this));
    l->addWidget(m_graph);

    layout->addWidget(gb);
    layout->addStretch();
}

void StatsPanel::resetStats() {
    m_lblWaitTime->setText("Средняя задержка: 0.0 с");
    m_lblQueue->setText("Текущая очередь: 0 авто");
    m_lblPassed->setText("Машин проехало: 0");
    if (m_graph) {
        m_graph->clear();
    }
}

void StatsPanel::updateStats(const SimulationSnapshot& snap) {
    if (snap.stats.currentSimTimeSec == 0.0 && m_graph) {
        m_graph->clear();
    }

    m_lblWaitTime->setText(QString("Средняя задержка: %1 с").arg(snap.stats.averageWaitTimeSec, 0, 'f', 1));
    m_lblQueue->setText(QString("Текущая очередь: %1 авто").arg(snap.stats.currentCarsInQueue));
    m_lblPassed->setText(QString("Машин проехало: %1").arg(snap.stats.totalCarsPassed));

    if (m_graph) {
        m_graph->addData(snap.stats.currentSimTimeSec, snap.stats.currentCarsInQueue);
    }
}