#include "ParametersPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <qtimer.h>

QHBoxLayout* ParametersPanel::createSliderRow(const QString& title, int min, int max, int val, QSlider*& sl, QLabel*& lbl) {
    auto* h = new QHBoxLayout();
    sl = new QSlider(Qt::Horizontal);
    sl->setRange(min, max);
    sl->setValue(val);
    lbl = new QLabel(QString::number(val));
    lbl->setMinimumWidth(35);
    h->addWidget(new QLabel(title));
    h->addWidget(sl);
    h->addWidget(lbl);
    return h;
}

QWidget* ParametersPanel::createApproachTab(ApproachUI& uiElements) {
    auto* w = new QWidget();
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

    auto* scroll = new QScrollArea();
    auto* scrollWidget = new QWidget();
    auto* scrollLayout = new QVBoxLayout(scrollWidget);
    scroll->setWidgetResizable(true);
    scroll->setWidget(scrollWidget);
    mainLayout->addWidget(scroll);

    // 1. Органичный блок настроек
    auto* gbMode = new QGroupBox("Настройки перекрестка", this);
    auto* lMode = new QVBoxLayout(gbMode);

    auto* modeRow = new QHBoxLayout();
    rbStatic = new QRadioButton("Статический (Man)");
    rbDynamic = new QRadioButton("Адаптивный (Auto)");
    rbStatic->setChecked(true);
    modeRow->addWidget(rbStatic);
    modeRow->addWidget(rbDynamic);
    lMode->addLayout(modeRow);

    auto* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Топология:"));
    cbTopology = new QComboBox();
    cbTopology->addItems({"2+2", "2+3 (Асимметрия)", "3+3"});
    topRow->addWidget(cbTopology);
    lMode->addLayout(topRow);

    cbLeftTurn = new QCheckBox("ВСТР (Просачивание налево)");
    cbParallelPeds = new QCheckBox("Пешеходы в фазе с авто");

    cbLeftTurn->setChecked(true);
    cbParallelPeds->setChecked(false);
    cbParallelPeds->setEnabled(false); // Заблокировано, так как ВСТР включен

    lMode->addWidget(cbLeftTurn);
    lMode->addWidget(cbParallelPeds);
    scrollLayout->addWidget(gbMode);

    // 2. Вкладки направлений
    auto* gbApproaches = new QGroupBox("Светофоры (Тайминги)", this);
    auto* lApproaches = new QVBoxLayout(gbApproaches);
    tabWidget = new QTabWidget();
    tabWidget->addTab(createApproachTab(appr[0]), "Север");
    tabWidget->addTab(createApproachTab(appr[1]), "Юг");
    tabWidget->addTab(createApproachTab(appr[2]), "Восток");
    tabWidget->addTab(createApproachTab(appr[3]), "Запад");
    lApproaches->addWidget(tabWidget);
    scrollLayout->addWidget(gbApproaches);

    // 3. Глобальные параметры
    auto* gbGlobal = new QGroupBox("Глобальные параметры", this);
    auto* lGlobal = new QVBoxLayout(gbGlobal);
    lGlobal->addLayout(createSliderRow("Общий цикл T (с):", 30, 180, 90, slTotalT, lblTotalT));
    lGlobal->addLayout(createSliderRow("Видимость D (м):", 20, 150, 60, slDistD, lblDistD));
    lGlobal->addLayout(createSliderRow("Пешех. З_п (с):", 5, 60, 15, slPedZ, lblPedZ));
    lGlobal->addLayout(createSliderRow("Поток пеш-в П:", 50, 1000, 300, slPedFlow, lblPedFlow));

    lGlobal->addLayout(createSliderRow("V мин (км/ч):", 20, 140, 30, slMinSpeed, lblMinSpeed));
    lGlobal->addLayout(createSliderRow("V макс (км/ч):", 20, 140, 80, slMaxSpeed, lblMaxSpeed));

    scrollLayout->addWidget(gbGlobal);
    scrollLayout->addStretch();

    // Логика блокировки "Пешеходы в фазе с авто"
    connect(cbLeftTurn, &QCheckBox::toggled, this, [this](bool checked){
        if(checked) {
            cbParallelPeds->setChecked(false);
            cbParallelPeds->setEnabled(false);
        } else {
            cbParallelPeds->setEnabled(true);
        }
        onGlobalChanged();
    });

    auto triggerGlobal = [this]() { onGlobalChanged(); queueConfigUpdate(); }; // Было: emitConfig();    connect(rbStatic, &QRadioButton::toggled, this, triggerGlobal);
    connect(cbParallelPeds, &QCheckBox::toggled, this, triggerGlobal);
    connect(cbTopology, QOverload<int>::of(&QComboBox::currentIndexChanged), this, triggerGlobal);
    connect(slMinSpeed, &QSlider::valueChanged, this, triggerGlobal);
    connect(slMaxSpeed, &QSlider::valueChanged, this, triggerGlobal);
    connect(slTotalT, &QSlider::valueChanged, this, triggerGlobal);
    connect(slDistD, &QSlider::valueChanged, this, triggerGlobal);
    connect(slPedZ, &QSlider::valueChanged, this, triggerGlobal);
    connect(slPedFlow, &QSlider::valueChanged, this, triggerGlobal);

    onGlobalChanged();
}

void ParametersPanel::onGlobalChanged() {
    lblTotalT->setText(QString::number(slTotalT->value()));
    lblDistD->setText(QString::number(slDistD->value()));
    lblPedZ->setText(QString::number(slPedZ->value()));
    lblPedFlow->setText(QString::number(slPedFlow->value()));

    // Защита от пересечения v_min > v_max
    if (slMinSpeed->value() > slMaxSpeed->value()) {
        slMaxSpeed->blockSignals(true);
        slMaxSpeed->setValue(slMinSpeed->value());
        slMaxSpeed->blockSignals(false);
    }
    lblMinSpeed->setText(QString::number(slMinSpeed->value()));
    lblMaxSpeed->setText(QString::number(slMaxSpeed->value()));

    bool isStatic = rbStatic->isChecked();
    slTotalT->setEnabled(isStatic);

    bool pedEnabled = isStatic && !cbParallelPeds->isChecked();
    slPedZ->setEnabled(pedEnabled);

    for (int i = 0; i < 4; ++i) appr[i].slZ->setEnabled(isStatic);
    onTabSlidersChanged();
}

void ParametersPanel::onTabSlidersChanged() {
    int T = slTotalT->value();

    int nPhases = 2;
    if (!cbLeftTurn->isChecked()) nPhases += 2;
    // Если пешеходы НЕ идут вместе с авто, значит для них выделяется ЭКСКЛЮЗИВНАЯ фаза
    if (!cbParallelPeds->isChecked()) nPhases += 1;

    int maxZ = (T / nPhases) - 5;
    if (maxZ < 10) maxZ = 10;

    for (int i = 0; i < 4; ++i) {
        appr[i].slZ->blockSignals(true);
        appr[i].slZ->setMaximum(maxZ);
        if (appr[i].slZ->value() > maxZ) appr[i].slZ->setValue(maxZ);
        appr[i].slZ->blockSignals(false);

        appr[i].lblP->setText(QString::number(appr[i].slP->value()));
        appr[i].lblZ->setText(QString::number(appr[i].slZ->value()));

        int Z = appr[i].slZ->value();
        int K = std::max(5, (T / nPhases) - Z);

        appr[i].slK->blockSignals(true);
        appr[i].slK->setMaximum(T);
        appr[i].slK->setValue(K);
        appr[i].lblK->setText(QString::number(K) + " (авто)");
        appr[i].slK->blockSignals(false);
    }
    queueConfigUpdate();
}

void ParametersPanel::emitConfig() {
    SimulationConfig config;
    config.mode = rbStatic->isChecked() ? ControllerMode::Static : ControllerMode::Dynamic;
    config.topology = static_cast<IntersectionTopology>(cbTopology->currentIndex());
    config.permitLeftTurnFilter = cbLeftTurn->isChecked();
    config.hasRightTurnArrow = cbParallelPeds->isChecked();

    config.totalCycleSec = slTotalT->value();
    config.visibilityDistance = slDistD->value();
    config.pedestrianGreenSec = slPedZ->value();
    config.pedestrianFlow = slPedFlow->value();

    // Передаем значения диапазона начальной скорости
    config.minSpeedKmh = slMinSpeed->value();
    config.maxSpeedKmh = slMaxSpeed->value();

    for(int i = 0; i < 4; i++) {
        ApproachParams ap;
        ap.flowP = appr[i].slP->value();
        ap.greenZ = appr[i].slZ->value();
        ap.redK = appr[i].slK->value();
        if(i==0) config.north = ap;
        else if(i==1) config.south = ap;
        else if(i==2) config.east = ap;
        else if(i==3) config.west = ap;
    }
    emit configChanged(config);
}