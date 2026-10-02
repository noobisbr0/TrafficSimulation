#include "ControlPanel.h"
#include "presenter/SimulationPresenter.h"
#include <QHBoxLayout>
#include <QIcon>

ControlPanel::ControlPanel(SimulationPresenter* presenter, QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(0);

    m_btnStart = new QPushButton(this);
    m_btnStart->setIcon(QIcon(":/icons/play.svg"));
    m_btnStart->setToolTip("Старт симуляции");
    m_btnStart->setObjectName("btnGroupLeft");

    m_btnPause = new QPushButton(this);
    m_btnPause->setIcon(QIcon(":/icons/pause.svg"));
    m_btnPause->setToolTip("Пауза");
    m_btnPause->setObjectName("btnGroupMiddle");

    m_btnStep = new QPushButton(this);
    m_btnStep->setIcon(QIcon(":/icons/step_forward.svg"));
    m_btnStep->setToolTip("Шаг");
    m_btnStep->setObjectName("btnGroupMiddle");

    m_btnReset = new QPushButton(this);
    m_btnReset->setIcon(QIcon(":/icons/stop.svg"));
    m_btnReset->setToolTip("Сброс");
    m_btnReset->setObjectName("btnGroupRight");

    m_sbSpeed = new QDoubleSpinBox(this);
    m_sbSpeed->setRange(0.1, 5.0);
    m_sbSpeed->setSingleStep(0.1);
    m_sbSpeed->setValue(1.0);
    m_sbSpeed->setSuffix("x");

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(m_btnStart);
    layout->addWidget(m_btnPause);
    layout->addWidget(m_btnStep);
    layout->addWidget(m_btnReset);

    auto* rightLayout = new QHBoxLayout();
    rightLayout->setContentsMargins(10, 0, 0, 0);
    rightLayout->addWidget(m_sbSpeed);
    layout->addLayout(rightLayout);
    layout->addStretch();

    connect(m_btnStart, &QPushButton::clicked, presenter, &SimulationPresenter::onStartClicked);
    connect(m_btnPause, &QPushButton::clicked, presenter, &SimulationPresenter::onPauseClicked);
    connect(m_btnStep,  &QPushButton::clicked, presenter, &SimulationPresenter::onStepClicked);
    connect(m_btnReset, &QPushButton::clicked, presenter, &SimulationPresenter::onResetClicked);

    connect(m_sbSpeed, &QDoubleSpinBox::valueChanged, presenter, &SimulationPresenter::onSimulationSpeedMultiplierChanged);
}