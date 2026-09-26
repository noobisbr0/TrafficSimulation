#include "mainwindow.h"
#include "canvas/SimulationView.h"
#include "canvas/SimulationScene.h"
#include "panels/ControlPanel.h"
#include "panels/ParametersPanel.h"
#include "panels/StatsPanel.h"
#include "presenter/SimulationPresenter.h"
#include "common/MockSimulationEngine.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDockWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Симулятор перекрестка (Frontend UI)");
    resize(1280, 720);

    auto engine = std::make_shared<MockSimulationEngine>();
    m_presenter = new SimulationPresenter(engine, this);

    auto* scene = new SimulationScene(this);
    auto* view = new SimulationView(this);
    view->setScene(scene);
    setCentralWidget(view);

    auto* paramPanel = new ParametersPanel(this);
    auto* ctrlPanel = new ControlPanel(m_presenter, this);
    auto* statsPanel = new StatsPanel(this);

    auto* rightDock = new QDockWidget("Параметры", this);
    rightDock->setWidget(paramPanel);
    rightDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    auto* leftDock = new QDockWidget("Управление и Аналитика", this);
    auto* leftWidget = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addWidget(ctrlPanel);
    leftLayout->addWidget(statsPanel);
    leftDock->setWidget(leftWidget);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    connect(m_presenter, &SimulationPresenter::snapshotUpdated, statsPanel, &StatsPanel::updateStats);
    connect(m_presenter, &SimulationPresenter::snapshotUpdated, scene, &SimulationScene::updateState);
    connect(paramPanel, &ParametersPanel::configChanged, m_presenter, &SimulationPresenter::onConfigChanged);
    connect(paramPanel, &ParametersPanel::configChanged, scene, &SimulationScene::updateConfig);
}

MainWindow::~MainWindow() = default;