#ifndef MAINIHM_H
#define MAINIHM_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDateTime>
#include <QDebug>
#include <QFile>

namespace Ui {
class MainIhm;
}

class MainIhm : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainIhm(QWidget *parent = 0);
    ~MainIhm();

private slots:
    void on_pbTransfertDrone_clicked();
    void on_pbStopperMission_clicked();
    void on_pbNewMission_2_clicked();
    void on_pbTestData_clicked();

private:
    Ui::MainIhm *ui;
};

#endif // MAINIHM_H
