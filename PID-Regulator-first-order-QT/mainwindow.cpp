#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QFileDialog>

/*
 * Konštruktor hlavného okna
 * Inicializuje GUI, stav simulácie a graf
 */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    y(0),
    prevError(0),
    integral(0),
    dt(0.01),
    tau(1.0),
    K(1.0)
{
    ui->setupUi(this);

    dt = 0.01;  // krok simulácie

    // nastavenie počiatočných hodnôt
    t = 0.0;
    y = 0.0;
    integral = 0.0;
    prevError = 0.0;
    running = false;

    // Setup QCustomPlot
    ui->customPlot->addGraph(); //graf vystup systemu
    ui->customPlot->xAxis->setLabel("Time [s]");
    ui->customPlot->yAxis->setLabel("Output");

    ui->customPlot->addGraph(); // graf pre ziadana hodnota
    ui->customPlot->graph(1)->setPen(QPen(Qt::DashLine));

    // nastavenie farieb tlacidiel
    ui->btnStart->setStyleSheet("background-color: green");
    ui->btnReset->setStyleSheet("background-color: red");
    ui->lblError->setStyleSheet("color: red;");

    //load image
    ui->lblEquation->setPixmap(QPixmap("C:/Users/lukas/Documents/pid/equationLATEX/equation.png"));
    ui->lblEquation->setScaledContents(true); // scales nicely


    connect(&timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onBtnStartClicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::onBtnResetClicked);
    connect(ui->btnSendServer, &QPushButton::clicked, this, &MainWindow::onBtnSendClicked);


    tcpClient = new QTcpSocket(this); //incializacia klienta tcp
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onBtnStartClicked()
{
    if (!running) {
        timer.start(int(dt * 1000));
        running = true;
        ui->btnStart->setText("Pause");
        ui->btnStart->setStyleSheet("background-color: orange"); // running
    } else {
        timer.stop();
        running = false;
        ui->btnStart->setText("Start");
        ui->btnStart->setStyleSheet("background-color: green"); // stopped/ready
    }

    ui->customPlot->xAxis->setRange(0, ui->spinMaxTime->value());  //set x limit to max_time set
    double setpoint = ui->spinSetpoint->value();
    ui->customPlot->yAxis->setRange(0, setpoint * 1.2); //set x limit to 120 % of setpoint

}

void MainWindow::on_btnSaveSettings_clicked()
{
    QJsonObject obj;


    obj["K"] = ui->spinGain->value();
    obj["Kd"] = ui->spinKd->value();
    obj["Ki"] = ui->spinKi->value();
    obj["Kp"] = ui->spinKp->value();
    obj["setpoint"] = ui->spinSetpoint->value();
    obj["tau"] = ui->spinTau->value();
    obj["tmax"] = ui->spinMaxTime->value();

    QJsonDocument doc(obj);

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save PID setup",
        "",
        "JSON (*.json)"
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
        file.write(doc.toJson(QJsonDocument::Indented));
}


void MainWindow::on_btnLoadSettings_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load PID setup",
        "",
        "JSON (*.json)"
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    ui->spinTau->setValue(obj["tau"].toDouble());
    ui->spinGain->setValue(obj["K"].toDouble());
    ui->spinSetpoint->setValue(obj["setpoint"].toDouble());

    ui->spinKp->setValue(obj["Kp"].toDouble());
    ui->spinKi->setValue(obj["Ki"].toDouble());
    ui->spinKd->setValue(obj["Kd"].toDouble());

    ui->spinMaxTime->setValue(obj["tmax"].toDouble());
}


void MainWindow::on_btnSaveGraph_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save graph",
        "",
        "PNG Image (*.png)"
        );

    if (!fileName.isEmpty())
        ui->customPlot->savePng(fileName, 1200, 800);
}

void MainWindow::onBtnSendClicked()
{
    if (tcpClient->state() != QAbstractSocket::ConnectedState) {
        tcpClient->connectToHost("127.0.0.1", 12345);
        if (!tcpClient->waitForConnected(1000)) {
            qDebug() << "Cannot connect to server";
            return;
        }
    }

    QJsonObject obj;
    obj["tau"] = ui->spinTau->value();
    obj["K"] = ui->spinGain->value();
    obj["setpoint"] = ui->spinSetpoint->value();
    obj["Kp"] = ui->spinKp->value();
    obj["Ki"] = ui->spinKi->value();
    obj["Kd"] = ui->spinKd->value();
    obj["tmax"] = ui->spinMaxTime->value();

    QJsonDocument doc(obj);
    tcpClient->write(doc.toJson(QJsonDocument::Compact) + "\n"); // \n pre server
    tcpClient->flush();
}


void MainWindow::onBtnResetClicked()
{
    timer.stop();
    running = false;

    t = 0.0;         // <-- reset simulation time
    y = 0.0;
    integral = 0.0;
    prevError = 0.0;

    ui->lblError->setText(QString::number(0, 'f', 2));  //reset error value

    ui->customPlot->graph(0)->data()->clear(); // clear output
    ui->customPlot->graph(1)->data()->clear(); // clear setpoint

    ui->customPlot->replot();

    ui->btnStart->setText("Start");
    ui->btnStart->setStyleSheet("background-color: green"); // stopped/ready

    // Re-enable max time spin box so user can change it
    ui->spinMaxTime->setEnabled(true);
}



void MainWindow::updateSimulation()
{
    double setpoint = ui->spinSetpoint->value();

    double maxTime = ui->spinMaxTime->value();
    if (t >= maxTime) {
        timer.stop();
        running = false;
        ui->btnStart->setText("Start");
    }



    double error = setpoint - y;
    integral += error * dt;
    double derivative = (error - prevError) / dt;
    double u = ui->spinKp->value() * error
               + ui->spinKi->value() * integral
               + ui->spinKd->value() * derivative;
    prevError = error;

    ui->lblError->setText(QString::number(error, 'f', 2));

    double tau = ui->spinTau->value();
    double K   = ui->spinGain->value();
    y += dt * (-y + K * u) / tau;

    ui->spinMaxTime->setEnabled(!running); //prevent changing time while running

    // add points using the **current t**
    ui->customPlot->graph(0)->addData(t, y);
    if (ui->customPlot->graphCount() > 1)
        ui->customPlot->graph(1)->addData(t, setpoint);


    t += dt;                     // <-- advance time

    ui->customPlot->replot();
}

