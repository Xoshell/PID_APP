#include "mainwindow.h"
#include <QApplication>

/*
 * Vstupný bod Qt aplikácie.
 * Inicializuje QApplication a zobrazí hlavné okno.
 */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
