#pragma once

#include <QWidget>
#include <deque>
#include <vector>
#include <utility>
#include "common/ISimulationEngine.h"

class QLabel;
class QPushButton;
class QPaintEvent;

struct SimulationStatRecord {
    double timeSec{0.0};
    int carsInQueue{0};
    double averageWaitTimeSec{0.0};
    int totalCarsPassed{0};
};

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
    void exportToCsv();

private:
    QLabel* m_lblWaitTime{nullptr};
    QLabel* m_lblQueue{nullptr};
    QLabel* m_lblPassed{nullptr};
    QueueGraphWidget* m_graph{nullptr};
    QPushButton* m_btnExport{nullptr};

    std::vector<SimulationStatRecord> m_records;
};