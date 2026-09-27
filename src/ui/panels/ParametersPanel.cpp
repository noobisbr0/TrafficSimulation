#include "ParametersPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <algorithm>
#include <array>

QHBoxLayout* ParametersPanel::createSliderRow(const QString& title, int min, int max, int val, QSlider*& sl, QLabel*& lbl) {
    auto* h = new QHBoxLayout();
    sl = new QSlider(Qt::Horizontal, this);
    sl->setRange(min, max);
    sl->setValue(val);
    lbl = new QLabel(QString::number(val), this);
    lbl->setMinimumWidth(35);
    h->addWidget(new QLabel(title, this));
    h->addWidget(sl);
    h->addWidget(lbl);
    return h;
}

QWidget* ParametersPanel::createApproachTab(ApproachUI& uiElements) {
    auto* w = new QWidget(this);
    auto* l = new QVBoxLayout(w);
    l->addLayout(createSliderRow("Поток P (авт/ч):", 100, 2000, 600, uiElements.slP, uiElements.lblP));
    l->addLayout(createSliderRow("Зеленый Z (с):", 10, 120, 30, uiElements.slZ, uiElements.lblZ));
    l->addLayout(createSliderRow("Красный K (с):", 10, 120, 60, uiElements.slK, uiElements.lblK));
    uiElements.slK->setEnabled(false);

    connect(uiElements.slP, &QSlider::valueChanged, this, &ParametersPanel::onTabSlidersChanged);
    connect(uiElements.slZ, &QSlider::valueChanged, this, &ParametersPanel::onTabSlidersChanged);
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
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea(this);
    auto* scrollWidget = new QWidget();
    auto* scrollLayout = new QVBoxLayout(scrollWidget);
    scroll->setWidgetResizable(true);
    scroll->setWidget(scrollWidget);
    mainLayout->addWidget(scroll);

    auto* gbMode = new QGroupBox("Настройки перекрестка", this);
    auto* lMode = new QVBoxLayout(gbMode);

    auto* modeRow = new QHBoxLayout();
    m_rbStatic = new QRadioButton("Статический (Man)", this);
    m_rbDynamic = new QRadioButton("Адаптивный (Auto)", this);
    m_rbStatic->setChecked(true);
    modeRow->addWidget(m_rbStatic);
    modeRow->addWidget(m_rbDynamic);
    lMode->addLayout(modeRow);

    auto* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Топология:", this));
    m_cbTopology = new QComboBox(this);
    m_cbTopology->addItems({"2+2", "2+3 (Асимметрия)", "3+3"});
    topRow->addWidget(m_cbTopology);
    lMode->addLayout(topRow);

    m_cbLeftTurn = new QCheckBox("ВСТР (Просачивание налево)", this);
    m_cbParallelPeds = new QCheckBox("Пешеходы в фазе с авто", this);

    m_cbLeftTurn->setChecked(true);
    m_cbParallelPeds->setChecked(false);
    m_cbParallelPeds->setEnabled(false);

    lMode->addWidget(m_cbLeftTurn);
    lMode->addWidget(m_cbParallelPeds);
    scrollLayout->addWidget(gbMode);

    auto* gbApproaches = new QGroupBox("Светофоры (Тайминги)", this);
    auto* lApproaches = new QVBoxLayout(gbApproaches);
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createApproachTab(m_approaches[0]), "Север");
    m_tabWidget->addTab(createApproachTab(m_approaches[1]), "Юг");
    m_tabWidget->addTab(createApproachTab(m_approaches[2]), "Восток");
    m_tabWidget->addTab(createApproachTab(m_approaches[3]), "Запад");
    lApproaches->addWidget(m_tabWidget);
    scrollLayout->addWidget(gbApproaches);

    auto* gbGlobal = new QGroupBox("Глобальные параметры", this);
    auto* lGlobal = new QVBoxLayout(gbGlobal);
    lGlobal->addLayout(createSliderRow("Общий цикл T (с):", 30, 180, 90, m_slTotalT, m_lblTotalT));
    lGlobal->addLayout(createSliderRow("Видимость D (м):", 20, 150, 60, m_slDistD, m_lblDistD));
    lGlobal->addLayout(createSliderRow("Пешех. З_п (с):", 5, 60, 15, m_slPedZ, m_lblPedZ));
    lGlobal->addLayout(createSliderRow("Поток пеш-в П:", 50, 1000, 300, m_slPedFlow, m_lblPedFlow));

    lGlobal->addLayout(createSliderRow("V мин (км/ч):", 20, 140, 30, m_slMinSpeed, m_lblMinSpeed));
    lGlobal->addLayout(createSliderRow("V макс (км/ч):", 20, 140, 80, m_slMaxSpeed, m_lblMaxSpeed));

    scrollLayout->addWidget(gbGlobal);
    scrollLayout->addStretch();

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
    connect(m_slMinSpeed, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slMaxSpeed, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slTotalT, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slDistD, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slPedZ, &QSlider::valueChanged, this, triggerGlobal);
    connect(m_slPedFlow, &QSlider::valueChanged, this, triggerGlobal);

    onGlobalChanged();
}

void ParametersPanel::onGlobalChanged() {
    m_lblTotalT->setText(QString::number(m_slTotalT->value()));
    m_lblDistD->setText(QString::number(m_slDistD->value()));
    m_lblPedZ->setText(QString::number(m_slPedZ->value()));
    m_lblPedFlow->setText(QString::number(m_slPedFlow->value()));

    if (m_slMinSpeed->value() > m_slMaxSpeed->value()) {
        const QSignalBlocker blocker(m_slMaxSpeed);
        m_slMaxSpeed->setValue(m_slMinSpeed->value());
    }
    m_lblMinSpeed->setText(QString::number(m_slMinSpeed->value()));
    m_lblMaxSpeed->setText(QString::number(m_slMaxSpeed->value()));

    const bool isStatic = m_rbStatic->isChecked();
    m_slTotalT->setEnabled(isStatic);
    m_slPedZ->setEnabled(isStatic && !m_cbParallelPeds->isChecked());

    for (int i = 0; i < 4; ++i) {
        m_approaches[i].slZ->setEnabled(isStatic);
    }
    onTabSlidersChanged();
}

void ParametersPanel::onTabSlidersChanged() {
    const int totalT = m_slTotalT->value();

    int nPhases = 2;
    if (!m_cbLeftTurn->isChecked()) {
        nPhases += 2;
    }
    if (!m_cbParallelPeds->isChecked()) {
        nPhases += 1;
    }

    const int maxZ = std::max(10, (totalT / nPhases) - 5);

    for (int i = 0; i < 4; ++i) {
        {
            const QSignalBlocker blocker(m_approaches[i].slZ);
            m_approaches[i].slZ->setMaximum(maxZ);
            if (m_approaches[i].slZ->value() > maxZ) {
                m_approaches[i].slZ->setValue(maxZ);
            }
        }

        m_approaches[i].lblP->setText(QString::number(m_approaches[i].slP->value()));
        m_approaches[i].lblZ->setText(QString::number(m_approaches[i].slZ->value()));

        const int greenZ = m_approaches[i].slZ->value();
        const int redK = std::max(5, (totalT / nPhases) - greenZ);

        {
            const QSignalBlocker blocker(m_approaches[i].slK);
            m_approaches[i].slK->setMaximum(totalT);
            m_approaches[i].slK->setValue(redK);
        }
        m_approaches[i].lblK->setText(QString::number(redK) + " (авто)");
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

    std::array<ApproachParams*, 4> targets = {&config.north, &config.south, &config.east, &config.west};
    for (size_t i = 0; i < targets.size(); ++i) {
        targets[i]->flowP = m_approaches[i].slP->value();
        targets[i]->greenZ = m_approaches[i].slZ->value();
        targets[i]->redK = m_approaches[i].slK->value();
    }

    emit configChanged(config);
}