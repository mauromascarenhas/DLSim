#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QSpinBox>
#include <QCloseEvent>
#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QMessageBox>
#include <QListWidgetItem>

#include "dlevent/dlevent.h"
#include "eventeditwindow.h"
#include "simulationwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QList<DLEvent> evl;

    void bindSlots();

protected:
    void closeEvent(QCloseEvent* e);
};
#endif // MAINWINDOW_H
