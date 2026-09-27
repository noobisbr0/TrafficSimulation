#pragma once

#include <QWidget>
#include <deque>
#include <utility>
#include "common/ISimulationEngine.h"

class QLabel;
class QPaintEvent;

class QueueGraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit QueueGraphWidget(QWidget* parent = nullptr);

    void clear();
    void addData(double timeSec, int queueSize);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::deque<std::pair<double, int>> m_history;
};

class StatsPanel : public QWidget {
    Q_OBJECT
public:
    explicit StatsPanel(QWidget* parent = nullptr);

public slots:
    void updateStats(const SimulationSnapshot& snap);
    void resetStats();

private:
    QLabel* m_lblWaitTime{nullptr};
    QLabel* m_lblQueue{nullptr};
    QLabel* m_lblPassed{nullptr};
    QueueGraphWidget* m_graph{nullptr};
};