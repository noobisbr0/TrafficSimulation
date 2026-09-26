#pragma once
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTabWidget>
#include "common/SimulationConfig.h"

class ParametersPanel : public QWidget {
    Q_OBJECT
public:
    explicit ParametersPanel(QWidget* parent = nullptr);

signals:
    void configChanged(const SimulationConfig& config);

private slots:
    void queueConfigUpdate();
    void onGlobalChanged();
    void onTabSlidersChanged();
    void emitConfig();

private:
    struct ApproachUI {
        QSlider *slP, *slZ, *slK;
        QLabel *lblP, *lblZ, *lblK;
    };

    QTimer* m_debounceTimer;
    QRadioButton *rbStatic, *rbDynamic;
    QComboBox *cbTopology;
    QCheckBox *cbLeftTurn, *cbParallelPeds;

    QSlider *slTotalT, *slDistD, *slPedZ, *slPedFlow;
    QLabel *lblTotalT, *lblDistD, *lblPedZ, *lblPedFlow;

    // --- Слайдеры скоростей авто ---
    QSlider *slMinSpeed, *slMaxSpeed;
    QLabel *lblMinSpeed, *lblMaxSpeed;

    QTabWidget *tabWidget;
    ApproachUI appr[4]; // 0=Север, 1=Юг, 2=Восток, 3=Запад

    QWidget* createApproachTab(ApproachUI& uiElements);
    class QHBoxLayout* createSliderRow(const QString& title, int min, int max, int val, QSlider*& sl, QLabel*& lbl);
};