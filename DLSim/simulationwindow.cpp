#include "simulationwindow.h"
#include "ui_simulationwindow.h"

SimulationWindow::SimulationWindow(QWidget *parent, SIM_ALGORITHM algorithm, int devices,
                                   const QList<DLEvent> &events, double bitErrorRate, int rSeed) :
    QMainWindow(parent),
    ui(new Ui::SimulationWindow)
{
    ui->setupUi(this);

    this->metaSim = QMetaEnum::fromType<SIM_ALGORITHM>();
    this->metaNWEvent = QMetaEnum::fromType<NW_EVENT_TYPE>();

    this->random = (rSeed != -1) ? QRandomGenerator(rSeed) : QRandomGenerator::securelySeeded();

    this->m_ber = bitErrorRate;
    this->m_rSeed = rSeed;
    this->m_events = QList<DLEvent>(events);
    this->m_devices = devices;
    this->m_algorithm = algorithm;
    this->m_multable_events = QList<DLEvent>(events);

    this->sTimer.setInterval(0);

    cTime = 0;
    cData = tData = 0;
    lastEvent = NW_EVENT_TYPE::T_FREE_MEDIA;
    allocatedToEvent = -1;
    for (const DLEvent &evt: qAsConst(this->m_events)) tData += evt.dataSize();
    ui->pbSimProgress->setMaximum(tData);
    ui->pbSimProgress->setFormat(QString("%p% - %v/%1").arg(tData));

    ui->twEvents->resizeColumnsToContents();

    ui->twEvents->setSortingEnabled(false);
    bindSlots();
}

SimulationWindow::~SimulationWindow()
{
    delete ui;
}

void SimulationWindow::bindSlots(){
    // End of simulation
    connect(this, &SimulationWindow::simulationFinished, this, [this]{
       this->ui->btInterrupt->setEnabled(false);
       this->ui->btExportResults->setEnabled(true);
    });

    // Cancel button
    connect(ui->btInterrupt, &QPushButton::clicked, this, [this]{
        if (QMessageBox::question(this, "Cancel simulation | DLSim",
                                  "You are about to cancel the current simulation."
                                  "\nThis action cannot be undone. Continue?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes){
            this->sTimer.stop();
            emit simulationFinished();
        }
    });

    // Export results button
    connect(ui->btExportResults, &QPushButton::clicked, this, &SimulationWindow::exportResults);
    // Export settings button
    connect(ui->btExportSettings, &QPushButton::clicked, this, &SimulationWindow::exportSettings);

    // Specifies algorithm
    if (this->m_algorithm == SIM_ALGORITHM::TDMA) connect(&this->sTimer, &QTimer::timeout, this, &SimulationWindow::step_TDMA);
    else connect(&this->sTimer, &QTimer::timeout, this, &SimulationWindow::step_CSMA_CA);

    // Waits 2 seconds before starting simulation
    QTimer::singleShot(2000, this, [this]{ this->sTimer.start(); });
}

void SimulationWindow::exportSettings(){
    QString fPath = QFileDialog::getSaveFileName(this, "Export simulation settings | DLSim",
                                                 QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0),
                                                "JSON files (*.json)");
    if (fPath.length()){
        if (QFile::exists(fPath)) QFile::remove(fPath);
        QFile cFile(fPath);
        if (cFile.open(QIODevice::WriteOnly)){
            QJsonObject json;
            json["BER"] = m_ber;
            json["algorithm"] = metaSim.valueToKey(m_algorithm);
            json["randomSeed"] = m_rSeed;
            json["numberOfDevices"] = m_devices;

            QJsonArray events;
            for (const DLEvent &evt: qAsConst(this->m_events)){
                QJsonObject cEvent;
                cEvent["device"] = evt.deviceNumber();
                cEvent["dataSize"] = evt.dataSize();
                cEvent["startTime"] = evt.startTransmissionTime();
                events.append(cEvent);
            }
            json["events"] = events;

            cFile.write(QJsonDocument(json).toJson());
            cFile.close();
        }
    }
}

void SimulationWindow::exportResults(){
    QString fPath = QFileDialog::getSaveFileName(this, "Export simulation results | DLSim",
                                                 QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0),
                                                "JSON files (*.json)");
    if (fPath.length()){
        if (QFile::exists(fPath)) QFile::remove(fPath);
        QFile cFile(fPath);
        if (cFile.open(QIODevice::WriteOnly)){
            QJsonArray results;

            int rc = ui->twEvents->rowCount();
            for(int i = 0; i < rc; ++i){
                QJsonObject cRow;
                cRow["instant"] = ui->twEvents->item(i, 0)->text().toLongLong();
                cRow["totalTransmitedData"] = ui->twEvents->item(i, 0)->data(Qt::UserRole).toLongLong();
                cRow["eventIndex"] = ui->twEvents->item(i, 1)->text().toInt();
                cRow["device"] = ui->twEvents->item(i, 2)->text().toInt();
                cRow["networkEvent"] = ui->twEvents->item(i, 3)->text();
                cRow["transmitedDataC"] = ui->twEvents->item(i, 4)->text().toInt();
                cRow["transmitedDataAcc"] = ui->twEvents->item(i, 5)->text().toInt();
                cRow["dataToTransmit"] = ui->twEvents->item(i, 6)->text().toInt();
                results.append(cRow);
            }

            cFile.write(QJsonDocument(results).toJson());
            cFile.close();
        }
    }
}

void SimulationWindow::step_TDMA(){
    int cIdx = 0;
    int cDevice = (cTime % this->m_devices) + 1; // Time starts at "t = 0"

    lastEvent = NW_EVENT_TYPE::T_FREE_MEDIA;

    QList<DLEvent>::iterator iter = this->m_multable_events.begin();
    while (iter != this->m_multable_events.end()){
        if (iter->deviceNumber() == cDevice && iter->dataSize() > iter->transmitedData()
                && cTime >= iter->startTransmissionTime()){
            if (random.bounded(1.0) < m_ber) lastEvent = NW_EVENT_TYPE::T_DATA_ERROR;
            else {
                iter->setTransmitedData(iter->transmitedData() + 1);
                lastEvent = NW_EVENT_TYPE::T_DATA_SUCCESS;
                cData++;
            }
            break;
        }
        cIdx++;
        iter++;
    }

    int rc = ui->twEvents->rowCount();
    ui->twEvents->insertRow(rc);
    QTableWidgetItem* item = new QTableWidgetItem(QString::number(cTime));
    item->setData(Qt::UserRole, cData);
    ui->twEvents->setItem(rc, 0, item);
    ui->twEvents->setItem(rc, 2, new QTableWidgetItem(QString::number(cDevice)));
    ui->twEvents->setItem(rc, 3, new QTableWidgetItem(metaNWEvent.valueToKey(lastEvent)));
    switch (lastEvent) {
    case T_DATA_SUCCESS:
        ui->twEvents->setItem(rc, 1, new QTableWidgetItem(QString::number(cIdx)));
        ui->twEvents->setItem(rc, 4, new QTableWidgetItem("1"));
        ui->twEvents->setItem(rc, 5, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).transmitedData())));
        ui->twEvents->setItem(rc, 6, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).dataSize())));
        break;
    case T_DATA_ERROR:
        ui->twEvents->setItem(rc, 1, new QTableWidgetItem(QString::number(cIdx)));
        ui->twEvents->setItem(rc, 4, new QTableWidgetItem("0"));
        ui->twEvents->setItem(rc, 5, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).transmitedData())));
        ui->twEvents->setItem(rc, 6, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).dataSize())));
        break;
    case T_FREE_MEDIA:
        ui->twEvents->setItem(rc, 1, new QTableWidgetItem("-1"));
        ui->twEvents->setItem(rc, 4, new QTableWidgetItem("0"));
        ui->twEvents->setItem(rc, 5, new QTableWidgetItem("0"));
        ui->twEvents->setItem(rc, 6, new QTableWidgetItem("0"));
        break;
    default: break;
    }

    ui->pbSimProgress->setValue(cData);
    cTime++;

    if (cData == tData){
        this->sTimer.stop();
        emit simulationFinished();
        QMessageBox::information(this, "Finished | DLSim", "The current simulation has been finished!",
                                 QMessageBox::Ok);
        this->raise();
    }
}

void SimulationWindow::step_CSMA_CA(){
    int cIdx = 0;
    int cDevice = -1;
    if (allocatedToEvent == -1){
        QList<DLEvent>::iterator iter = this->m_multable_events.begin();
        while (iter != this->m_multable_events.end()){
            if (iter->dataSize() > iter->transmitedData() && cTime >= iter->startTransmissionTime()){
                cDevice = iter->deviceNumber();
                lastEvent = NW_EVENT_TYPE::TRANSMIT_RTS;
                allocatedToEvent = cIdx;
                break;
            }
            cIdx++;
            iter++;
        }
        if (allocatedToEvent == -1) lastEvent = NW_EVENT_TYPE::T_FREE_MEDIA;
    }
    else {
        cIdx = allocatedToEvent;
        cDevice = this->m_multable_events.at(allocatedToEvent).deviceNumber();
        if (lastEvent == NW_EVENT_TYPE::TRANSMIT_RTS) lastEvent = NW_EVENT_TYPE::RECEIVE_CTS;
        else {
            if (random.bounded(1.0) < m_ber) lastEvent = NW_EVENT_TYPE::T_DATA_ERROR;
            else {
                DLEvent *evt = &this->m_multable_events[allocatedToEvent];
                evt->setTransmitedData(evt->transmitedData() + 1);
                lastEvent = NW_EVENT_TYPE::T_DATA_SUCCESS;
                cData++;
            }
            allocatedToEvent = -1;
        }
    }

    int rc = ui->twEvents->rowCount();
    ui->twEvents->insertRow(rc);
    QTableWidgetItem* item = new QTableWidgetItem(QString::number(cTime));
    item->setData(Qt::UserRole, cData);
    ui->twEvents->setItem(rc, 0, item);
    ui->twEvents->setItem(rc, 2, new QTableWidgetItem(QString::number(cDevice)));
    ui->twEvents->setItem(rc, 3, new QTableWidgetItem(metaNWEvent.valueToKey(lastEvent)));
    switch (lastEvent) {
    case RECEIVE_CTS:
    case TRANSMIT_RTS:
    case T_DATA_ERROR:
        ui->twEvents->setItem(rc, 1, new QTableWidgetItem(QString::number(cIdx)));
        ui->twEvents->setItem(rc, 4, new QTableWidgetItem("0"));
        ui->twEvents->setItem(rc, 5, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).transmitedData())));
        ui->twEvents->setItem(rc, 6, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).dataSize())));
        break;
    case T_DATA_SUCCESS:
        ui->twEvents->setItem(rc, 1, new QTableWidgetItem(QString::number(cIdx)));
        ui->twEvents->setItem(rc, 4, new QTableWidgetItem("1"));
        ui->twEvents->setItem(rc, 5, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).transmitedData())));
        ui->twEvents->setItem(rc, 6, new QTableWidgetItem(QString::number(this->m_multable_events.at(cIdx).dataSize())));
        break;
    case T_FREE_MEDIA:
        ui->twEvents->setItem(rc, 1, new QTableWidgetItem("-1"));
        ui->twEvents->setItem(rc, 4, new QTableWidgetItem("0"));
        ui->twEvents->setItem(rc, 5, new QTableWidgetItem("0"));
        ui->twEvents->setItem(rc, 6, new QTableWidgetItem("0"));
        break;
    default: break;
    }

    ui->pbSimProgress->setValue(cData);
    cTime++;

    if (cData == tData){
        this->sTimer.stop();
        emit simulationFinished();
        if (ui->cbWarning->isChecked())
            QMessageBox::information(this, "Finished | DLSim", "The current simulation has been finished!",
                                     QMessageBox::Ok);
        this->raise();
    }
}
