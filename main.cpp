#include <QApplication>

#include "Gui/mainwindow.h"
#include "appcontroller.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AppController appController;
    MainWindow w;

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &appController, &AppController::quit);
    QObject::connect(&w, &MainWindow::sigCloseEvent, &app, &QCoreApplication::quit);

    QObject::connect(&w, &MainWindow::sigConnectionTypeChanged, &appController, &AppController::slotChangeConnector);
    QObject::connect(&w, &MainWindow::sigConnectionInfoChanged, &appController, &AppController::slotChangeConnectionInfo);
    QObject::connect(&w, &MainWindow::sigWsServerPortChanged, &appController, &AppController::slotChangeWsPort);
    QObject::connect(&w, &MainWindow::sigNewStreamData, &appController, &AppController::slotSendStream);
    QObject::connect(&appController, &AppController::sigStreamReaded, &w, &MainWindow::slotNewStreamData);


    w.show();

    return app.exec();
}
