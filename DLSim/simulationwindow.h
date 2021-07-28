#ifndef SIMULATIONWINDOW_H
#define SIMULATIONWINDOW_H

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QMetaEnum>
#include <QCheckBox>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileDialog>
#include <QJsonObject>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QTableWidgetItem>

#include "dlevent/dlevent.h"

namespace Ui {
class SimulationWindow;
}

class SimulationWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum SIM_ALGORITHM{
        TDMA = 0,
        CSMA_CA = 1
    };
    Q_ENUM(SIM_ALGORITHM);

    enum NW_EVENT_TYPE{
        TRANSMIT_RTS,
        RECEIVE_CTS,
        T_FREE_MEDIA,
        T_DATA_ERROR,
        T_DATA_SUCCESS
    };
    Q_ENUM(NW_EVENT_TYPE);

    explicit SimulationWindow(QWidget *parent = nullptr, SIM_ALGORITHM algorithm = SIM_ALGORITHM::TDMA, int devices = 1,
                              const QList<DLEvent> &events = QList<DLEvent>(), double bitErrorRate = 0, int rSeed = -1);
    ~SimulationWindow();

    inline void setRSeed(int rSeed) { m_rSeed = rSeed; }
    inline void setEvents(const QList<DLEvent> &events){ m_events = events; }
    inline void setDevices(int devices){ m_devices = devices; }
    inline void setAlgorithm(SIM_ALGORITHM algorithm){ m_algorithm = algorithm; }
    inline void setBitErrorRate(double bitErrorRate){ m_ber = bitErrorRate; }

private:
    Ui::SimulationWindow *ui;

    int m_rSeed;
    int m_devices;

    int lastIndex;
    int allocatedToEvent;

    double m_ber;

    long long cTime;
    long long cData;
    long long tData;

    QList<DLEvent> m_events;
    QList<DLEvent> m_multable_events;

    QMetaEnum metaSim;
    SIM_ALGORITHM m_algorithm;

    QMetaEnum metaNWEvent;
    NW_EVENT_TYPE lastEvent;

    QTimer sTimer;
    QJsonObject simulationData;
    QRandomGenerator random;

    void bindSlots();

private slots:
    void step_TDMA();
    void step_CSMA_CA();

    void exportResults();
    void exportSettings();

signals:
    void simulationFinished();
};

#endif // SIMULATIONWINDOW_H
