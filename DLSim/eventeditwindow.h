#ifndef EVENTEDITWINDOW_H
#define EVENTEDITWINDOW_H

#include <QSpinBox>
#include <QPushButton>
#include <QMainWindow>

#include "dlevent/dlevent.h"

namespace Ui {
class EventEditWindow;
}

class EventEditWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EventEditWindow(QWidget *parent = nullptr, DLEvent *evt = nullptr, int devices = 1);
    ~EventEditWindow();

private:
    Ui::EventEditWindow *ui;
    void bindSlots();

signals:
    void data(const DLEvent &evt);
};

#endif // EVENTEDITWINDOW_H
