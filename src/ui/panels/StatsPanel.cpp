#include "StatsPanel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

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