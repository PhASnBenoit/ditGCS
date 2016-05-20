#include "mainihm.h"
#include "ui_mainihm.h"

MainIhm::MainIhm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainIhm)
{
    ui->setupUi(this);
    ui->tabWidget->setTabEnabled(0,true);
    ui->tabWidget->setTabEnabled(1,false);
    ui->tabWidget->setTabEnabled(2,false);
    ui->tabWidget->setCurrentWidget(0);


    // init de la liste des capteurs suivant capteurs.csv
    // situé dans le dossier des sources
    QList<QByteArray> parties;
    QFile file("capteurs.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug() << "Erreur ouverture du fichier capteurs.csv !";
    } // if fichier pas bon
    while (!file.atEnd()) {         // lecture des lignes du fichier
         QByteArray line = file.readLine();
         line = line.mid(0, line.size()-1);
         if ( (line[0]>='0') && (line[0]<='9') ) {    // si la ligne décrit un capteur
             qDebug() << "MainIhm:capteurs.csv: " << line;
             parties = line.split(';'); // extrait chaque partie de la ligne
             ui->lwCapteurs->addItem(QString(line));
             ui->lwCapteurs2->addItem(QString(line));
             ui->lwCapteurs->selectAll();
             ui->lwCapteurs2->selectAll();
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

    // liste des ports série disponibles
    serial = new QSerialPort(this);
    QList<QSerialPortInfo> listeSerialPort = QSerialPortInfo::availablePorts();
    for(int i=0 ; i<listeSerialPort.size() ; i++) {
        qDebug() << "MainIhm::MainIhm : " << listeSerialPort.at(i).portName() << " " << listeSerialPort.at(i).description();
        ui->cbListePortSerie->addItem(listeSerialPort.at(i).portName());
    } // for port série

    // position initiale sur avant mission
    ui->pbTransfertDrone->setEnabled(false);
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
    char trame[500]={0}, ordre[500]={0};
    char chCrc16[5]={0};
    US crc16c;

    // s'assurer de la connexion de la liaison DATA
    qDebug() << "envoi de la commande [TT] TEST...";
    bool res=emettre((char *)"\x02[TT]\x03",6);

    if (res > 0) {  // test ok
        // envoi des informations de configuration vers le drone.
        qDebug() << "envoi de la commande [03] DATA CONFIG MISSION...";
        sprintf(ordre,"%s;%s", ui->leNomMission->text().toStdString().c_str(), (ui->cbMesure->isChecked()?"1":"0"));
        crc16c = crc16((unsigned char *)ordre,strlen(ordre));
        sprintf(chCrc16,"%04x",crc16c);
        sprintf(trame,"\x02[03]%s;%s\x03", ordre, chCrc16);
        emettre(trame,strlen(trame));

        // envoi des informations d'incrustation
        qDebug() << "envoi de la commande [04] DATA INCRUSTATION DEPART MISSION...";
        // ouvrir le fichier config.ini contenant les capteurs
        QFile *file = new QFile("capteurs.csv");
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
            qDebug() << "Erreur ouverture du fichier config.ini";
        // lire les lignes et former la trame
        bzero(ordre, sizeof(ordre));
        while (!file->atEnd()) {         // lecture des lignes du fichier
             QByteArray line = file->readLine();
             //line.at(line.size()-1) = 0;
             if (isdigit(line[0])) {    // si le premier car de la ligne est 0-9
                 qDebug() << "CONFIG.INI: " << line;
                 QList<QByteArray> parties = line.split(';'); // extrait chaque partie de la ligne
                 for (int j=0 ; j<parties.size() ; j++) {
                     strcat(ordre, parties.at(j).toStdString().c_str());
                     strcat(ordre,";");
                 } // for j
                 ordre[strlen(ordre)-1]=0;  // enleve le dernier ;
                 strcat(ordre,"@");
             } // if
        } // while
        file->close();
        delete file;
        crc16c = crc16((unsigned char *)ordre,strlen(ordre));
        sprintf(chCrc16,"%04x",crc16c);
        sprintf(trame, "\x02[04]%s;%s\x03", ordre, chCrc16);
        emettre(trame,strlen(trame));

        // Mise à jour de la date et heure de l'EDD
        qDebug() << "envoi de la commande [09] Mise à jour de la date/heure de l'EDD...";
        QDateTime dt = QDateTime::currentDateTime();
        dt.addSecs(5);  // pour compenser le temps de transfert vers l'EDD
        bzero(ordre, sizeof(ordre));
        sprintf(ordre,"%s;%s",dt.date().toString("yy;MM;dd").toStdString().c_str(), dt.time().toString("HH;mm;ss").toStdString().c_str());
        crc16c = crc16((unsigned char *)ordre,strlen(ordre));
        sprintf(chCrc16,"%04x",crc16c);
        sprintf(trame,"\x02[09]%s;%s\x03", ordre, chCrc16);
        emettre(trame,strlen(trame));

        // envoi du départ mission
        qDebug() << "envoi de la commande [00] START MISSION...";
        emettre((char *)"\x02[00]\x03",6);
        ui->tabWidget->setTabEnabled(0,false);
        ui->tabWidget->setTabEnabled(1,true);
        ui->tabWidget->setTabEnabled(2,false);
        ui->tabWidget->setCurrentIndex(1);
    } // if test ok
} // on_pbTransfertDrone_clicked

void MainIhm::on_pbStopperMission_clicked()
{
    qDebug() << "STOPPER MISSION !";
    on_pbArretAcqMes_clicked();
    qDebug() << "envoi de la commande [99] STOP MISSION...";
    emettre((char *)"\x02[99]\x03",6);
    ui->tabWidget->setTabEnabled(0,false);
    ui->tabWidget->setTabEnabled(1,false);
    ui->tabWidget->setTabEnabled(2,true);
    ui->tabWidget->setCurrentIndex(2);
} // on_pbStopperMission_clicked

void MainIhm::on_pbNewMission_2_clicked()
{
    ui->tabWidget->setTabEnabled(0,true);
    ui->tabWidget->setTabEnabled(1,false);
    ui->tabWidget->setTabEnabled(2,false);
    ui->tabWidget->setCurrentIndex(0);
}

void MainIhm::on_pbTestData_clicked()
{
    qDebug() << "envoi de la commande [TT] TEST...";
    int res = emettre((char *)"\x02[TT]\x03",6);
    if (res == 1) {
        ui->pbTransfertDrone->setEnabled(true);
        qDebug() << "MainIhm::on_pbTestData_clicked: Test positif !!!";
    }else {
        qDebug() << "MainIhm::on_pbTestData_clicked: Test négatif !!!";
    } // else
} // on_pbTestData_clicked

void MainIhm::onReadyRead()
{
  qDebug() << "Des cars arrivent...";
} // onReadyRead


void MainIhm::on_pbDepartAcqMes_clicked()
{
    char trame[255];
    char chCrc16[10];
    char chTimer[10];
    US crc16c;
    qDebug() << "envoi de la commande [01] START MESURES...";

    int timer = ui->leTimer->text().toInt();
    sprintf(chTimer, "%05d", timer);

    crc16c = crc16((unsigned char *)chTimer,5);
    sprintf(chCrc16,"%04x",crc16c);

    sprintf(trame,"\x02[01]%s;%s\x03",chTimer, chCrc16);
    emettre(trame, strlen(trame));
} // on_pbDepartAcqMes_clicked


void MainIhm::on_pbArretAcqMes_clicked()
{
    qDebug() << "envoi de la commande [02] ARRET MESURES...";
    emettre((char *)"\x02[02]\x03",6);
} // on_pbArretAcqMes_clicked


void MainIhm::on_pbEmettreOrdre_clicked()
{
    US crc16c;
    char trame[255];
    char ordre[50];
    char chCrc16[5];
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

    sprintf(trame, "\x02[05]%s;%s\x03", ordre, chCrc16);
    emettre(trame, strlen(trame));
} // on_pbEmettreOrdre_clicked



void MainIhm::on_cbListePortSerie_activated(const QString &arg1)
{
    initVoieSerie(arg1);
}


int MainIhm::emettre(char *trame, int nb)
{
    int nbe=3;  // 3 essais de transmission max
    while(nbe>0) {
        nbe--; // essai de transmission
        int ret = serial->write(trame,nb);  // envoi  trame
        if (ret != nb) {
            ui->pbTestData->setText("Erreur W !, Retry !");
            qDebug() << "MainIhm:on_pbTestData_clicked: Impossible d'écrire dans le port série.";
        } else {
            recPossible=false;
            do {
                recPossible=serial->waitForReadyRead(TIMEOUT);
                if (recPossible && (serial->bytesAvailable()>5) ) {
                    QByteArray qbaRep = serial->readAll();
                    if (qbaRep == "\x02[AA]\x03") {
                        ui->pbTestData->setText("test OK !, Retry !");
                        nbe=-1; // pas de nouveau essai de transmission
                    } else
                        ui->pbTestData->setText("test KO !, Retry !");
                    qDebug() << "Test liaison DATA : Réponse : " << qbaRep;
                    break;
                } // if
                if (recPossible && serial->bytesAvailable() < 6) {
                    qDebug() << "Réception incomplète de la réponse";
                } // if
            } while(recPossible);  // while
        } // else ret
    } // while nb essai 3 fois de communiquer si erreur
    if (nbe != -1)
        return -1;
    else
        return 1;
} // on_pbArretAcqMes_clicked

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
}

int MainIhm::initVoieSerie(QString nomVs)
{
    // objet voie série
    if (serial->isOpen()) {
        qDebug() << "MainIhm::initVoieSerie: Disconnected to " << nomVs;
        serial->close();
    } // if open
    serial->setPortName(nomVs);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        serial->setRequestToSend(false);
        qDebug() << "MainIhm::initVoieSerie: Connected to " << nomVs;
        connect(serial, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        return 1;
    } else {
        qDebug() << "MainIhm::initVoieSerie: Impossible to connect to " << nomVs;
        return -1;
    }

} // crc16

