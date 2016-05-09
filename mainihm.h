#ifndef MAINIHM_H
#define MAINIHM_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QFile>
#include <unistd.h>

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
    void onReadyRead();

private:
    Ui::MainIhm *ui;
    QSerialPort *serial; // objet de gestion de la liaison série
//    QTimer *tRec;   // timer de limite de réception de la voie série
    QByteArray qbaRep;  // réponse
    bool recPossible;  // flag de test reponse possible
    bool tempsEcoule;  // flag de réception voie série
};

#endif // MAINIHM_H
