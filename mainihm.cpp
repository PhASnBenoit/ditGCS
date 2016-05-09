#include "mainihm.h"
#include "ui_mainihm.h"

MainIhm::MainIhm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainIhm)
{
    ui->setupUi(this);
    // init de la liste des capteurs suivant capteurs.csv
    // situé dans le dossier des sources
    QList<QByteArray> parties;
    QFile file("capteurs.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug() << "Erreur ouverture du fichier capteurs.csv !";
    } // if fichier pas bon
    while (!file.atEnd()) {         // lecture des lignes du fichier
         QByteArray line = file.readLine();
         if (line[0]!= '#') {    // si le premier car de la ligne pas un #
             qDebug() << "MainIhm:capteurs.csv: " << line;
             parties = line.split(';'); // extrait chaque partie de la ligne
             ui->lwCapteurs->addItem(QString(parties.at(0)));
             ui->lwCapteurs2->addItem(QString(parties.at(0)));
         } // if
    } // while
    file.close();

    // nom de la mission
    QString chDate=QString::number(QDateTime::currentDateTime().date().year());
    chDate += QString::number(QDateTime::currentDateTime().date().month());
    chDate += QString::number(QDateTime::currentDateTime().date().day());
    chDate += QString::number(QDateTime::currentDateTime().time().hour());
    chDate += QString::number(QDateTime::currentDateTime().time().minute());
    chDate += QString::number(QDateTime::currentDateTime().time().second());
    ui->leNomMission->setText("Mission"+chDate);

    // objet voie série
    serial = new QSerialPort(this);
    serial->setPortName("ttyUSB0");
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        serial->setRequestToSend(false);
        qDebug() << "Connected to ttyUSB0";
        connect(serial, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    } else
        qDebug() << "Impossible to connect to ttyUSB0";
/*
    // liste des ports séries
    QList<QSerialPortInfo> lps = QSerialPortInfo::availablePorts();
    int nbps = lps.size();
    if (nbps==0)
        ui->cbListePortSerie->addItem("Pas de port série !");
    for(int i=0 ; i<nbps ; i++) {
        ui->cbListePortSerie->addItem(lps.at(i).portName());
    } // for
*/
    // position initiale sur avant mission
    ui->tabWidget->setCurrentIndex(0);
}

MainIhm::~MainIhm()
{
    delete ui;
    serial->close();
    delete serial;
    //delete tRec;
}

void MainIhm::on_pbTransfertDrone_clicked()
{
    // s'assurer de la connexion de la liaison DATA
    // envoi des informations de configuration vers le drone.

    ui->tabWidget->setCurrentIndex(1);
}

void MainIhm::on_pbStopperMission_clicked()
{
    ui->tabWidget->setCurrentIndex(2);
}

void MainIhm::on_pbNewMission_2_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
}

void MainIhm::on_pbTestData_clicked()
{
    qDebug() << "envoi de la commande...";
    int ret = serial->write("[TT]");  // envoi une trame de test
    if (ret != 4) {
        ui->pbTestData->setText("Erreur W !, Retry !");
        qDebug() << "MainIhm:on_pbTestData_clicked: Impossible d'écrire dans le port série.";
    } else {
        recPossible=false;
        do {
            recPossible=serial->waitForReadyRead(5000);
            if (recPossible && serial->bytesAvailable() > 3) {
                QByteArray qbaRep = serial->readAll();
                ui->pbTestData->setText("test OK !, Retry !");
                qDebug() << "Test liaison DATA : Réponse : " << qbaRep;
                break;
            } // if
            if (recPossible && serial->bytesAvailable() < 4) {
               qDebug() << "Réception incomplète de la réponse";
            } // if
        } while(recPossible);  // while
    } // else ret

}

void MainIhm::onReadyRead()
{
  qDebug() << "Des cars arrivent...";
}

