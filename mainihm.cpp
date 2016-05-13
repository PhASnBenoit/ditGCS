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
    // envoi des informations d'incrustation
    // envoi du départ mission
    qDebug() << "envoi de la commande [00] START MISSION...";
    emettre("[00]§",5);
    ui->tabWidget->setCurrentIndex(1);
}

void MainIhm::on_pbStopperMission_clicked()
{
    qDebug() << "STOPPER MISSION !";
    on_pbArretAcqMes_clicked();
    qDebug() << "envoi de la commande [99] STOP MISSION...";
    emettre("[99]§",5);
    ui->tabWidget->setCurrentIndex(2);
} // on_pbStopperMission_clicked

void MainIhm::on_pbNewMission_2_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
}

void MainIhm::on_pbTestData_clicked()
{
    qDebug() << "envoi de la commande [TT] TEST...";
    emettre("[TT]§",5);
} // on_pbTestData_clicked

void MainIhm::onReadyRead()
{
  qDebug() << "Des cars arrivent...";
} // onReadyRead


US MainIhm::crc16(UC *tab,int nb)
{
    UC nbDec,         // indique le nombre de décalage de l'octet */
       yaUn,          // booleen si bit = 1 alors =1
       ind;           // indique l'indice dans la chaine
    US crc;  // contient le crc16

    crc = 0xFFFF;
    ind = 0;

    do {
        crc ^= (US)tab[ind];
        nbDec = 0;
        do {
            if ((crc & 0x0001) == 1)
                yaUn = 1;
            else
                yaUn = 0;
            crc >>= 1;
            if (yaUn)
                crc ^= 0xA001;
            nbDec++;
        } while (nbDec < 8);
        ind++;
    } while (ind < nb);
    return(crc);
} // crc16


void MainIhm::on_pbDepartAcqMes_clicked()
{
    char trame[255];
    char chCrc16[10];
    char chTimer[10];
    US crc16c;
    qDebug() << "envoi de la commande [01] START MESURES...";

    int timer = ui->leTimer->text().toInt();
    sprintf(chTimer, "%05d", timer);

    crc16c = crc16((unsigned char *)tabTimer,5);
    sprintf(chCrc16,"%04x",crc16c);

    sprintf(trame,"[01]%s;%s§",tabTimer, chCrc16);
    emettre(trame, strlen(trame);
} // on_pbDepartAcqMes_clicked


void MainIhm::on_pbArretAcqMes_clicked()
{
    qDebug() << "envoi de la commande [02] ARRET MESURES...";
    emettre("[02]§",5);
} // on_pbArretAcqMes_clicked


void MainIhm::on_pbEmettreOrdre_clicked()
{
    US crc16c;
    char trame[255];
    char ordre[50];
    qDebug() << "envoi de la commande [05] ORDRE CAMERA...";

    strcpy(ordre,"GET /");
    strcat(ordre, ui->cbRep->currentText().toStdString().c_str());
    strcat(ordre, "/");
    int pos = ui->cbApp->currentText().indexOf(' ');
    strcat(ordre, ui->cbApp->currentText().left(pos).toStdString().c_str());
    strcat(ordre,"?t=goprohero&p=%");
    strcat(ordre, ui->cbVal->currentText().toStdString().c_str());
    strcat(ordre, " HTTP/1.1\r\n\r\n"); // fin d'entête requête HTTP

    crc16c = crc16((unsigned char *)ordre,strlen(ordre));
    sprintf(chCrc16,"%04x",crc16c);

    sprintf(trame, "[05]%s;%s§", ordre, chCrc16);
    emettre(trame, strlen(trame));
} // on_pbEmettreOrdre_clicked


int MainIhm::emettre(char *trame, int nb)
{
    int nb=3;  // 3 essais de transmission max
    while(nb>0) {
        nb--; // essai de transmission
        int ret = serial->write(trame,nb);  // envoi  trame
        if (ret != nb) {
            ui->pbTestData->setText("Erreur W !, Retry !");
            qDebug() << "MainIhm:on_pbTestData_clicked: Impossible d'écrire dans le port série.";
        } else {
            recPossible=false;
            do {
                recPossible=serial->waitForReadyRead(TIMEOUT);
                if (recPossible && (serial->bytesAvailable()>3) ) {
                    QByteArray qbaRep = serial->readAll();
                    if (qbaRep == "[AA]") {
                        ui->pbTestData->setText("test OK !, Retry !");
                        nb=0; // pas de nouveau essai de transmission
                    } else
                        ui->pbTestData->setText("test KO !, Retry !");
                    qDebug() << "Test liaison DATA : Réponse : " << qbaRep;
                    break;
                } // if
                if (recPossible && serial->bytesAvailable() < 4) {
                    qDebug() << "Réception incomplète de la réponse";
                } // if
            } while(recPossible);  // while
        } // else ret
    } // while nb essai 3 fois de communiquer si erreur
} // on_pbArretAcqMes_clicked

