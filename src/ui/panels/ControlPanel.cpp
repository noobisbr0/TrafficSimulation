#include "ControlPanel.h"
#include <QHBoxLayout>

ControlPanel::ControlPanel(SimulationPresenter* presenter, QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);

    btnStart = new QPushButton("▶ Старт");
    btnPause = new QPushButton("⏸ Пауза");
    btnStep = new QPushButton("⏭ Шаг");
    btnReset = new QPushButton("⏹ Сброс");
    cbSpeed = new QComboBox();

    cbSpeed->addItems({"0.5x", "1.0x", "2.0x", "5.0x"});
    cbSpeed->setCurrentIndex(1);

    layout->addWidget(btnStart);
    layout->addWidget(btnPause);
    layout->addWidget(btnStep);
    layout->addWidget(btnReset);
    layout->addWidget(cbSpeed);

    connect(btnStart, &QPushButton::clicked, presenter, &SimulationPresenter::onStartClicked);
    connect(btnPause, &QPushButton::clicked, presenter, &SimulationPresenter::onPauseClicked);
    connect(btnStep,  &QPushButton::clicked, presenter, &SimulationPresenter::onStepClicked);
    connect(btnReset, &QPushButton::clicked, presenter, &SimulationPresenter::onResetClicked);

    connect(cbSpeed, QOverload<int>::of(&QComboBox::currentIndexChanged), [presenter, this](int index) {
        QString text = cbSpeed->itemText(index);
        double multiplier = text.remove("x").toDouble();
        presenter->onSimulationSpeedMultiplierChanged(multiplier);
    });
}