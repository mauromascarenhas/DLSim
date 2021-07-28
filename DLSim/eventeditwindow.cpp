#include "eventeditwindow.h"
#include "ui_eventeditwindow.h"

EventEditWindow::EventEditWindow(QWidget *parent, DLEvent* evt, int devices) :
    QMainWindow(parent),
    ui(new Ui::EventEditWindow)
{
    ui->setupUi(this);
    ui->sbDevice->setMaximum(devices);
    if (evt){
        ui->sbSize->setValue(evt->dataSize());
        ui->sbTime->setValue(evt->startTransmissionTime());
        ui->sbDevice->setValue(evt->deviceNumber());
    }
    bindSlots();
}

EventEditWindow::~EventEditWindow()
{
    delete ui;
}

void EventEditWindow::bindSlots(){
    connect(ui->btSave, &QPushButton::clicked, this, [this]{
        emit data(DLEvent(ui->sbDevice->value(), ui->sbTime->value(), ui->sbSize->value()));
        this->close();
    });
}
