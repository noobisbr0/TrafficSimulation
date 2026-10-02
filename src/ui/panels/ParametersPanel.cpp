#include "ParametersPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFrame>
#include <QListView>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTimer>
#include <algorithm>

void ParametersPanel::addSliderRow(QFormLayout* layout, const QString& title, int min, int max, int val, QSlider*& sl, QSpinBox*& sb) {
    auto* h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(6);

    sl = new QSlider(Qt::Horizontal, this);
    sl->setRange(min, max);
    sl->setValue(val);

    sb = new QSpinBox(this);
    sb->setRange(min, max);
    sb->setValue(val);
    sb->setFixedWidth(60);

    connect(sl, &QSlider::valueChanged, sb, &QSpinBox::setValue);
    connect(sb, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), sl, &QSlider::setValue);

    h->addWidget(sl);
    h->addWidget(sb);
    layout->addRow(title, h);
}

int ParametersPanel::calculatePhaseCount() const {
    int phases = 2;
    if (!m_cbLeftTurn->isChecked()) {
        phases += 2;
    }
    if (!m_cbParallelPeds->isChecked()) {
        phases += 1;
    }
    return phases;
}

QWidget* ParametersPanel::createApproachTab(ApproachUI& uiElements) {
    auto* w = new QWidget(this);
    auto* l = new QFormLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setVerticalSpacing(2);
    l->setHorizontalSpacing(6);

    addSliderRow(l, "Поток P (авт/ч):", 100, 2000, 600, uiElements.slP, uiElements.sbP);
    addSliderRow(l, "Зеленый Z (с):", 5, 180, 30, uiElements.slZ, uiElements.sbZ);
    addSliderRow(l, "Красный K (с):", 5, 180, 60, uiElements.slK, uiElements.sbK);

    connect(uiElements.slP, &QSlider::valueChanged, this, &ParametersPanel::queueConfigUpdate);

    connect(uiElements.slZ, &QSlider::valueChanged, this, [this, uiElements](int val) {
        const int tPhase = m_slTotalT->value() / calculatePhaseCount();
        const int newK = tPhase - val;

        const QSignalBlocker bK(uiElements.slK), bKsb(uiElements.sbK);
        uiElements.slK->setValue(newK);
        uiElements.sbK->setValue(newK);

        queueConfigUpdate();
    });

    connect(uiElements.slK, &QSlider::valueChanged, this, [this, uiElements](int val) {
        const int tPhase = m_slTotalT->value() / calculatePhaseCount();
        const int newZ = tPhase - val;

        const QSignalBlocker bZ(uiElements.slZ), bZsb(uiElements.sbZ);
        uiElements.slZ->setValue(newZ);
        uiElements.sbZ->setValue(newZ);

        queueConfigUpdate();
    });

    return w;
}

void ParametersPanel::queueConfigUpdate() {
    m_debounceTimer->start();
}

ParametersPanel::ParametersPanel(QWidget* parent) : QWidget(parent) {
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(50);
    connect(m_debounceTimer, &QTimer::timeout, this, &ParametersPanel::emitConfig);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    setObjectName("scrollBg");

    auto* gbMode = new QGroupBox("Настройки перекрестка", this);
    auto* lMode = new QVBoxLayout(gbMode);

    auto* modeRow = new QHBoxLayout();
    m_rbStatic = new QRadioButton("Статический", this);
    m_rbDynamic = new QRadioButton("Адаптивный", this);
    m_rbStatic->setChecked(true);
    modeRow->addWidget(m_rbStatic);
    modeRow->addStretch();
    modeRow->addWidget(m_rbDynamic);
    modeRow->addSpacing(10);
    lMode->addLayout(modeRow);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    lMode->addWidget(line);

    auto* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Топология:", this));
    topRow->addStretch();
    m_cbTopology = new QComboBox(this);
    m_cbTopology->setView(new QListView(this));
    m_cbTopology->addItems({"2+2", "2+3 (Асимметрия)", "3+3"});
    m_cbTopology->setFixedWidth(160);
    topRow->addWidget(m_cbTopology);
    topRow->addSpacing(10);
    lMode->addLayout(topRow);

    m_cbLeftTurn = new QCheckBox("ВСТР (Просачивание налево)", this);
    m_cbParallelPeds = new QCheckBox("Пешеходы в фазе с авто", this);
    m_cbLeftTurn->setChecked(true);
    m_cbParallelPeds->setChecked(false);
    m_cbParallelPeds->setEnabled(false);

    lMode->addWidget(m_cbLeftTurn);
    lMode->addWidget(m_cbParallelPeds);
    mainLayout->addWidget(gbMode);

    auto* gbApproaches = new QGroupBox("Светофоры (Тайминги)", this);
    auto* lApproaches = new QVBoxLayout(gbApproaches);
    lApproaches->setSpacing(2);
    lApproaches->setContentsMargins(8, 4, 8, 6);

    auto addDirection = [&](int index, const QString& title) {
        if (index > 0) {
            lApproaches->addSpacing(8);
        }
        auto* lbl = new QLabel(title, this);
        lbl->setObjectName("approachTitle");
        lApproaches->addWidget(lbl);
        lApproaches->addWidget(createApproachTab(m_approaches[index]));
    };

    addDirection(0, "↑ Север");
    addDirection(1, "↓ Юг");
    addDirection(2, "→ Восток");
    addDirection(3, "← Запад");

    mainLayout->addWidget(gbApproaches);

    m_globalWidget = new QGroupBox("Глобальные параметры");
    auto* lGlobal = new QFormLayout(m_globalWidget);
    addSliderRow(lGlobal, "Общий цикл T (с):", 30, 180, 90, m_slTotalT, m_sbTotalT);
    addSliderRow(lGlobal, "Видимость D (м):", 20, 150, 60, m_slDistD, m_sbDistD);
    addSliderRow(lGlobal, "Пешех. З_п (с):", 5, 60, 15, m_slPedZ, m_sbPedZ);
    addSliderRow(lGlobal, "Поток пеш-в П:", 50, 1000, 300, m_slPedFlow, m_sbPedFlow);
    addSliderRow(lGlobal, "V мин (км/ч):", 20, 140, 30, m_slMinSpeed, m_sbMinSpeed);
    addSliderRow(lGlobal, "V макс (км/ч):", 20, 140, 80, m_slMaxSpeed, m_sbMaxSpeed);

    mainLayout->addStretch();

    connect(m_cbLeftTurn, &QCheckBox::toggled, this, [this](bool checked) {
        m_cbParallelPeds->setChecked(false);
        m_cbParallelPeds->setEnabled(!checked);
        onGlobalChanged();
    });

    auto triggerGlobal = [this]() {
        onGlobalChanged();
        queueConfigUpdate();
    };

    connect(m_rbStatic, &QRadioButton::toggled, this, triggerGlobal);
    connect(m_cbParallelPeds, &QCheckBox::toggled, this, triggerGlobal);
    connect(m_cbTopology, &QComboBox::currentIndexChanged, this, triggerGlobal);

    connect(m_slMinSpeed, &QSlider::valueChanged, this, [this](int val) {
        if (val > m_slMaxSpeed->value()) {
            m_slMaxSpeed->setValue(val);
        }
        onGlobalChanged();
        queueConfigUpdate();
    });

    connect(m_slMaxSpeed, &QSlider::valueChanged, this, [this](int val) {
        if (val < m_slMinSpeed->value()) {
            m_slMinSpeed->setValue(val);
        }
        onGlobalChanged();
        queueConfigUpdate();
    });

    connect(m_slTotalT, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slDistD, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slPedZ, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slPedFlow, &QSlider::valueChanged, this, triggerGlobal);

    onGlobalChanged();
}

void ParametersPanel::onGlobalChanged() {
    const QSignalBlocker b1(m_sbTotalT), b2(m_sbDistD), b3(m_sbPedZ), b4(m_sbPedFlow);
    m_sbTotalT->setValue(m_slTotalT->value());
    m_sbDistD->setValue(m_slDistD->value());
    m_sbPedZ->setValue(m_slPedZ->value());
    m_sbPedFlow->setValue(m_slPedFlow->value());

    const QSignalBlocker b5(m_sbMinSpeed), b6(m_sbMaxSpeed);
    m_sbMinSpeed->setValue(m_slMinSpeed->value());
    m_sbMaxSpeed->setValue(m_slMaxSpeed->value());

    const bool isStatic = m_rbStatic->isChecked();
    m_slTotalT->setEnabled(isStatic);
    m_sbTotalT->setEnabled(isStatic);
    m_slPedZ->setEnabled(isStatic && !m_cbParallelPeds->isChecked());
    m_sbPedZ->setEnabled(isStatic && !m_cbParallelPeds->isChecked());

    for (auto& app : m_approaches) {
        app.slZ->setEnabled(isStatic);
        app.sbZ->setEnabled(isStatic);
        app.slK->setEnabled(isStatic);
        app.sbK->setEnabled(isStatic);
    }
    onTabSlidersChanged();
}

void ParametersPanel::onTabSlidersChanged() {
    const int tPhase = m_slTotalT->value() / calculatePhaseCount();
    const int maxVal = std::max(5, tPhase - 5);

    for (auto& app : m_approaches) {
        const QSignalBlocker bZ(app.slZ), bZsb(app.sbZ);
        const QSignalBlocker bK(app.slK), bKsb(app.sbK);

        app.slZ->setRange(5, maxVal);
        app.sbZ->setRange(5, maxVal);
        app.slK->setRange(5, maxVal);
        app.sbK->setRange(5, maxVal);

        const int currentZ = app.slZ->value();
        const int currentK = tPhase - currentZ;

        app.slK->setValue(currentK);
        app.sbK->setValue(currentK);
    }
    queueConfigUpdate();
}

void ParametersPanel::emitConfig() {
    SimulationConfig config;
    config.mode = m_rbStatic->isChecked() ? ControllerMode::Static : ControllerMode::Dynamic;
    config.topology = static_cast<IntersectionTopology>(m_cbTopology->currentIndex());
    config.permitLeftTurnFilter = m_cbLeftTurn->isChecked();
    config.hasRightTurnArrow = m_cbParallelPeds->isChecked();

    config.totalCycleSec = m_slTotalT->value();
    config.visibilityDistance = m_slDistD->value();
    config.pedestrianGreenSec = m_slPedZ->value();
    config.pedestrianFlow = m_slPedFlow->value();

    config.minSpeedKmh = m_slMinSpeed->value();
    config.maxSpeedKmh = m_slMaxSpeed->value();

    const std::array<ApproachParams*, 4> targets = {&config.north, &config.south, &config.east, &config.west};
    for (size_t i = 0; i < targets.size(); ++i) {
        targets[i]->flowP = m_approaches[i].slP->value();
        targets[i]->greenZ = m_approaches[i].slZ->value();
        targets[i]->redK = m_approaches[i].slK->value();
    }

    emit configChanged(config);
}