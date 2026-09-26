#include "ControlPanel.h"
#include "presenter/SimulationPresenter.h"
#include <QHBoxLayout>

ControlPanel::ControlPanel(SimulationPresenter* presenter, QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);

    m_btnStart = new QPushButton("▶ Старт", this);
    m_btnPause = new QPushButton("⏸ Пауза", this);
    m_btnStep = new QPushButton("⏭ Шаг", this);
    m_btnReset = new QPushButton("⏹ Сброс", this);
    m_cbSpeed = new QComboBox(this);

    m_cbSpeed->addItems({"0.5x", "1.0x", "2.0x", "5.0x"});
    m_cbSpeed->setCurrentIndex(1);

    layout->addWidget(m_btnStart);
    layout->addWidget(m_btnPause);
    layout->addWidget(m_btnStep);
    layout->addWidget(m_btnReset);
    layout->addWidget(m_cbSpeed);

    connect(m_btnStart, &QPushButton::clicked, presenter, &SimulationPresenter::onStartClicked);
    connect(m_btnPause, &QPushButton::clicked, presenter, &SimulationPresenter::onPauseClicked);
    connect(m_btnStep,  &QPushButton::clicked, presenter, &SimulationPresenter::onStepClicked);
    connect(m_btnReset, &QPushButton::clicked, presenter, &SimulationPresenter::onResetClicked);

    connect(m_cbSpeed, QOverload<int>::of(&QComboBox::currentIndexChanged), [presenter, this](int index) {
        QString text = m_cbSpeed->itemText(index);
        const double multiplier = text.remove("x").toDouble();
        presenter->onSimulationSpeedMultiplierChanged(multiplier);
    });
}