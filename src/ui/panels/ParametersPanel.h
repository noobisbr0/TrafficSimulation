#pragma once

#include <QWidget>
#include <QSlider>
#include <QSpinBox> // Изменение QLabel на QSpinBox[cite: 2]
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTabWidget>
#include <QTimer>
#include <QFormLayout> // Использование QFormLayout[cite: 2]
#include <qgroupbox.h>
#include "common/SimulationConfig.h"

class QHBoxLayout;

class ParametersPanel : public QWidget {
    Q_OBJECT
public:
    explicit ParametersPanel(QWidget* parent = nullptr);
    QGroupBox* getGlobalWidget() const { return m_globalWidget; } // ДОБАВЛЕНО

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

        QSpinBox* sbP{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]
        QSpinBox* sbZ{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]
        QSpinBox* sbK{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]

    };

    QWidget* createApproachTab(ApproachUI& uiElements);


    void addSliderRow(QFormLayout* layout, const QString& title, int min, int max, int val, QSlider*& sl, QSpinBox*& sb); // Использование QFormLayout[cite: 2]


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

    QSpinBox* m_sbTotalT{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]
    QSpinBox* m_sbDistD{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]
    QSpinBox* m_sbPedZ{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]
    QSpinBox* m_sbPedFlow{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]

    QSlider* m_slMinSpeed{nullptr};
    QSlider* m_slMaxSpeed{nullptr};
    QSpinBox* m_sbMinSpeed{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]
    QSpinBox* m_sbMaxSpeed{nullptr}; // Изменение QLabel на QSpinBox[cite: 2]

    QGroupBox* m_globalWidget{nullptr}; // ДОБАВЛЕНО
    // QTabWidget* m_tabWidget{nullptr};
    ApproachUI m_approaches[4];

};
