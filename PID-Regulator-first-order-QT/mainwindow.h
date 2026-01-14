#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBtnStartClicked();
    void onBtnResetClicked();
    void updateSimulation();
    void on_btnSaveSettings_clicked();
    void on_btnLoadSettings_clicked();
    void on_btnSaveGraph_clicked();
    void onBtnSendClicked();

private:
    double t;            // aktuálny čas simulácie [s]
    bool running;        // indikácia, či simulácia beží

    Ui::MainWindow *ui;  // UI generované z Qt Designer
    QTimer timer;        // časovač zabezpečujúci diskrétne kroky simulácie

    double y;            // výstup systému (procesná veličina)
    double prevError;    // chyba z predchádzajúceho kroku
    double integral;     // integrál chyby (I z PID)
    double dt;           // krok simulácie

    double tau;          // časová konštanta 1. rádu
    double K;            // statické zosilnenie systému
    QTcpSocket *tcpClient;  // TCP klient
};

#endif // MAINWINDOW_H
