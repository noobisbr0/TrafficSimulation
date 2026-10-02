#pragma once

#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTabWidget>
#include <QTimer>
#include <QFormLayout>
#include <qgroupbox.h>
#include "common/SimulationConfig.h"

class ParametersPanel : public QWidget {
    Q_OBJECT
public:
    explicit ParametersPanel(QWidget* parent = nullptr);
    QGroupBox* getGlobalWidget() const { return m_globalWidget; }

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
        QSpinBox* sbP{nullptr};
        QSpinBox* sbZ{nullptr};
        QSpinBox* sbK{nullptr};
    };

    QWidget* createApproachTab(ApproachUI& uiElements);
    void addSliderRow(QFormLayout* layout, const QString& title, int min, int max, int val, QSlider*& sl, QSpinBox*& sb);

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

    QSpinBox* m_sbTotalT{nullptr};
    QSpinBox* m_sbDistD{nullptr};
    QSpinBox* m_sbPedZ{nullptr};
    QSpinBox* m_sbPedFlow{nullptr};

    QSlider* m_slMinSpeed{nullptr};
    QSlider* m_slMaxSpeed{nullptr};
    QSpinBox* m_sbMinSpeed{nullptr};
    QSpinBox* m_sbMaxSpeed{nullptr};

    QGroupBox* m_globalWidget{nullptr};
    ApproachUI m_approaches[4];
};