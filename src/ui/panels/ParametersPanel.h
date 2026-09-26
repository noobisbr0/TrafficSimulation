#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTabWidget>
#include <QTimer>
#include "common/SimulationConfig.h"

class QHBoxLayout;

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
        QSlider* slP{nullptr};
        QSlider* slZ{nullptr};
        QSlider* slK{nullptr};
        QLabel* lblP{nullptr};
        QLabel* lblZ{nullptr};
        QLabel* lblK{nullptr};
    };

    QWidget* createApproachTab(ApproachUI& uiElements);
    QHBoxLayout* createSliderRow(const QString& title, int min, int max, int val, QSlider*& sl, QLabel*& lbl);

    QTimer* m_debounceTimer{nullptr};
    QRadioButton* m_rbStatic{nullptr};
    QRadioButton* m_rbDynamic{nullptr};
    QComboBox* m_cbTopology{nullptr};
    QCheckBox* m_cbLeftTurn{nullptr};
    QCheckBox* m_cbParallelPeds{nullptr};

    QSlider* m_slTotalT{nullptr};
    QSlider* m_slDistD{nullptr};
    QSlider* m_slPedZ{nullptr};
    QSlider* m_slPedFlow{nullptr};

    QLabel* m_lblTotalT{nullptr};
    QLabel* m_lblDistD{nullptr};
    QLabel* m_lblPedZ{nullptr};
    QLabel* m_lblPedFlow{nullptr};

    QSlider* m_slMinSpeed{nullptr};
    QSlider* m_slMaxSpeed{nullptr};
    QLabel* m_lblMinSpeed{nullptr};
    QLabel* m_lblMaxSpeed{nullptr};

    QTabWidget* m_tabWidget{nullptr};
    ApproachUI m_approaches[4];
};