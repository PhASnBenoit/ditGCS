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

#define TIMEOUT 5000

typedef unsigned short US;
typedef unsigned char UC;

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
    void on_pbDepartAcqMes_clicked();
    void on_pbArretAcqMes_clicked();
    int emettre(char *trame, int nb);
    void on_pbEmettreOrdre_clicked();

private:
    US crc16(UC *tab,int nb);
    Ui::MainIhm *ui;
    QSerialPort *serial; // objet de gestion de la liaison série
    QByteArray qbaRep;  // réponse
    bool recPossible;  // flag de test reponse possible
    bool tempsEcoule;  // flag de réception voie série
};

#endif // MAINIHM_H
