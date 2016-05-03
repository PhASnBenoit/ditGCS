#include "mainihm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainIhm w;
    w.show();

    return a.exec();
}
