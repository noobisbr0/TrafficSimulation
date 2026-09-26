#include "StatsPanel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <qlabel.h>

StatsPanel::StatsPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* gb = new QGroupBox("Статистика (В реальном времени)", this);
    auto* l = new QVBoxLayout(gb);

    lblWaitTime = new QLabel("Средняя задержка: 0.0 с");
    lblQueue = new QLabel("Текущая очередь: 0 авто");
    lblPassed = new QLabel("Машин проехало: 0");
    m_graph = new QueueGraphWidget(this); // Инициализация графика

    l->addWidget(lblWaitTime);
    l->addWidget(lblQueue);
    l->addWidget(lblPassed);

    l->addWidget(new QLabel("Динамика очередей (последние 60с):"));
    l->addWidget(m_graph); // Встраиваем график в верстку панели

    layout->addWidget(gb);
    layout->addStretch();
}

void StatsPanel::resetStats() {
    lblWaitTime->setText("Средняя задержка: 0.0 с");
    lblQueue->setText("Текущая очередь: 0 авто");
    lblPassed->setText("Машин проехало: 0");
    if (m_graph) {
        m_graph->clear();
    }
}

void StatsPanel::updateStats(const SimulationSnapshot& snap) {
    if (snap.stats.currentSimTimeSec == 0.0) {
        m_graph->clear();
    }

    lblWaitTime->setText(QString("Средняя задержка: %1 с").arg(snap.stats.averageWaitTimeSec, 0, 'f', 1));
    lblQueue->setText(QString("Текущая очередь: %1 авто").arg(snap.stats.currentCarsInQueue));
    lblPassed->setText(QString("Машин проехало: %1").arg(snap.stats.totalCarsPassed));

    m_graph->addData(snap.stats.currentSimTimeSec, snap.stats.currentCarsInQueue);
}