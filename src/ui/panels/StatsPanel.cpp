#include "StatsPanel.h"
#include <QVBoxLayout>
#include <QGridLayout> // Изменение макета на QGridLayout[cite: 2]
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient> // Использование градиента для графика[cite: 2]
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

    // Отрисовка бледной сетки[cite: 2]
    p.setPen(QPen(QColor(255, 255, 255, 25), 1));
    for (int i = 1; i < 4; ++i) {
        int yLine = height() * i / 4;
        p.drawLine(0, yLine, width(), yLine);
    }
    for (int i = 1; i < 6; ++i) {
        int xLine = width() * i / 6;
        p.drawLine(xLine, 0, xLine, height());
    }

    if (m_history.size() < 2) return;


    const double minT = m_history.front().first;
    const double maxT = m_history.back().first;
    int maxQ = 5;
    for (const auto& pt : m_history) {
        maxQ = std::max(maxQ, pt.second);
    }

    // Текст с указанием оси Y[cite: 2]
    p.setPen(QColor("#90A4AE"));
    p.drawText(5, 15, QString("Max Y: %1").arg(maxQ));

    QPainterPath path;
    QPainterPath fillPath;

    fillPath.moveTo(0, height());


    for (size_t i = 0; i < m_history.size(); ++i) {
        const double x = width() * (m_history[i].first - minT) / std::max(1.0, maxT - minT);
        const double y = height() - (height() * m_history[i].second / (maxQ * 1.2));
        if (i == 0) {
            path.moveTo(x, y);

            fillPath.lineTo(x, y);
        } else {
            path.lineTo(x, y);
            fillPath.lineTo(x, y);
        }
    }

    fillPath.lineTo(width(), height());
    fillPath.closeSubpath();

    // Градиент заливки[cite: 2]
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor(61, 90, 254, 100)); // #3D5AFE alpha 100
    gradient.setColorAt(1.0, QColor(61, 90, 254, 0));
    p.fillPath(fillPath, gradient);

    p.setPen(QPen(QColor("#3D5AFE"), 2));
    p.drawPath(path);
}



StatsPanel::StatsPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* gb = new QGroupBox("Статистика (В реальном времени)", this);
    auto* l = new QVBoxLayout(gb);

    // Дашборд сеткой[cite: 2]
    auto* grid = new QGridLayout();
    grid->setSpacing(5);

    auto setupLabel = [](const QString& text, QColor color, int fontSize, bool bold) {
        auto* lbl = new QLabel(text);
        QString weight = bold ? "bold" : "normal";
        lbl->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: %3;").arg(color.name()).arg(fontSize).arg(weight));
        return lbl;
    };

    grid->addWidget(setupLabel("Средняя задержка", QColor("#90A4AE"), 11, false), 0, 0);
    grid->addWidget(setupLabel("Очередь (авто)", QColor("#90A4AE"), 11, false), 0, 1);

    m_lblWaitTime = setupLabel("0.0 с", Qt::white, 16, false);
    m_lblQueue = setupLabel("0", Qt::white, 16, false);

    grid->addWidget(m_lblWaitTime, 1, 0);
    grid->addWidget(m_lblQueue, 1, 1);

    grid->addWidget(setupLabel("Проехало машин", QColor("#90A4AE"), 11, false), 2, 0, 1, 2);
    m_lblPassed = setupLabel("0", Qt::white, 16, false);
    grid->addWidget(m_lblPassed, 3, 0, 1, 2);

    m_graph = new QueueGraphWidget(this);

    l->addLayout(grid);
    l->addSpacing(10);
    l->addWidget(new QLabel("Динамика очередей (последние 60с):", this));
    l->addWidget(m_graph);



    layout->addWidget(gb);
    layout->addStretch();
}

void StatsPanel::resetStats() {

    m_lblWaitTime->setText("0.0 с");
    m_lblQueue->setText("0");
    m_lblPassed->setText("0");
    if (m_graph) m_graph->clear();
}

void StatsPanel::updateStats(const SimulationSnapshot& snap) {
    if (snap.stats.currentSimTimeSec == 0.0 && m_graph) m_graph->clear();

    m_lblWaitTime->setText(QString("%1 с").arg(snap.stats.averageWaitTimeSec, 0, 'f', 1));
    m_lblQueue->setText(QString::number(snap.stats.currentCarsInQueue));
    m_lblPassed->setText(QString::number(snap.stats.totalCarsPassed));

    m_graph->addData(snap.stats.currentSimTimeSec, snap.stats.currentCarsInQueue);
}
