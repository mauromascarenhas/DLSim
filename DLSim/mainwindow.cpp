#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bindSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindSlots(){
    ui->statusbar->showMessage("Ready!");

    // Add event
    connect(ui->btAddLoad, &QPushButton::clicked, this, [this]{
        EventEditWindow* eew = new EventEditWindow(this, nullptr, ui->sbDevices->value());
        eew->setAttribute(Qt::WA_DeleteOnClose);
        connect(eew, &EventEditWindow::data, this, [this](const DLEvent &evt){
            int pos = 0;
            if (this->evl.size())
                while(pos < this->evl.size() && evt.startTransmissionTime() >= this->evl[pos].startTransmissionTime()) pos++;
            this->evl.insert(pos, evt);
            this->ui->lwEvents->insertItem(pos, QString("%1[t] - Device #%2 - %3 byte(s)")
                                            .arg(evt.startTransmissionTime(), 6, 10, QChar('0'))
                                            .arg(evt.deviceNumber(), 2, 10, QChar('0'))
                                            .arg(evt.dataSize()));
            this->ui->lwEvents->setCurrentRow(pos);
            this->ui->lwEvents->setFocus();
        });
        eew->show();
    });

    // Remove event
    connect(ui->btRemoveSel, &QPushButton::clicked, this, [this]{
        if (QMessageBox::question(this, "Remove event| DLSim", "You are about to remove the selected event\n"
                "This action cannot be undone. Proceed?")
                == QMessageBox::Yes){
            int cr = ui->lwEvents->currentRow();
            this->evl.removeAt(cr);
            delete ui->lwEvents->item(cr);
            ui->lwEvents->setCurrentRow(-1);
        }
    });

    // Update event
    connect(ui->btUpdateSel, &QPushButton::clicked, this, [this]{
        int cr = ui->lwEvents->currentRow();
        EventEditWindow* eew = new EventEditWindow(this, &this->evl[cr], ui->sbDevices->value());
        eew->setAttribute(Qt::WA_DeleteOnClose);
        connect(eew, &EventEditWindow::data, this, [this, cr](const DLEvent &evt){
            this->evl.removeAt(cr);
            delete ui->lwEvents->item(cr);

            int pos = 0;
            if (this->evl.size())
                while(pos < this->evl.size() && evt.startTransmissionTime() >= this->evl[pos].startTransmissionTime()) pos++;
            this->evl.insert(pos, evt);
            this->ui->lwEvents->insertItem(pos, QString("%1[t] - Device #%2 - %3 byte(s)")
                                            .arg(evt.startTransmissionTime(), 6, 10, QChar('0'))
                                            .arg(evt.deviceNumber(), 2, 10, QChar('0'))
                                            .arg(evt.dataSize()));
            this->ui->lwEvents->setCurrentRow(pos);
            this->ui->lwEvents->setFocus();
        });
        eew->show();
    });


    // New event selection
    connect(ui->lwEvents, &QListWidget::currentRowChanged, this, [this](int r){
        bool enable = r > -1;
        this->ui->btUpdateSel->setEnabled(enable);
        this->ui->btRemoveSel->setEnabled(enable);
    });

    // Updated number of devices
    connect(ui->sbDevices, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int nValue){
        int i = 0;
        QList<int> toRemove;
        for (DLEvent evnt : qAsConst(this->evl)){
            if (evnt.deviceNumber() > nValue) toRemove.append(i);
            i++;
        }

        if (toRemove.length()){
            if (QMessageBox::question(this, "Remove events| DLSim",
                                      QString("The number of devices have changed"
                                              "\nShould you proceed, a total of %1 events are going to be removed."
                                              "\nThis action cannot be undone. Continue?").arg(toRemove.length()))
                == QMessageBox::Yes){
                i = 0;
                for (int c : toRemove){
                    int cr = c - i++;
                    this->evl.removeAt(cr);
                    delete ui->lwEvents->item(cr);
                }
                ui->lwEvents->setCurrentRow(-1);
            }
        }
    });

    // Start simulation
    connect(ui->btStart, &QPushButton::clicked, this, [this]{
        if (!this->evl.length()){
            QMessageBox::information(this, "No event | DLSim",
                                     "You have not provided any event yet."
                                     "\nPlease, provide at least one event before proceeding.",
                                     QMessageBox::Ok);
            return;
        }

       ui->btStart->setEnabled(false);
       SimulationWindow* sw = new SimulationWindow(this, (SimulationWindow::SIM_ALGORITHM)this->ui->cbAlg->currentIndex(),
                                                   this->ui->sbDevices->value(), this->evl, this->ui->sbBER->value(),
                                                   this->ui->sbSeed->value());
       sw->setAttribute(Qt::WA_DeleteOnClose);
       connect(sw, &SimulationWindow::simulationFinished, this, [this]{ this->ui->btStart->setEnabled(true); });
       sw->show();
    });

    // Quit button click
    connect(ui->btQuit, &QPushButton::clicked, this, &QMainWindow::close);

    // About button click
    connect(ui->btAbout, &QPushButton::clicked, this, [this]{
        QMessageBox::about(this, "About | DLSim",
                           "DLSim v0.8.1"
                           "\n---------------------------------------------------"
                           "\nFrame size: 8bits (1byte)"
                           "\nCSMA fairness: No fairness (first transmission event)"
                           "\nCSMA backoff time: 0[t]"
                           "\nTransmission window size: 1 (Data link layer)"
                           "\nTransmission error retry attempts: infinity");
    });
}

void MainWindow::closeEvent(QCloseEvent *e){
    if (QMessageBox::question(this, "Quit | DLSim", "You are about to quit. Proceed?")
            == QMessageBox::Yes) e->accept();
    else e->ignore();
}

