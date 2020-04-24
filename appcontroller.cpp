#include "appcontroller.h"

#include <QtConcurrent>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , connectionUrl_("tcp://127.0.0.1:"+QString::number(DEFAULT_TCP_PORT))
{
    connectorType_ = StreamCommunicationType::TcpServer;
    pcTcpServer_ = new StreamTcpServer(connectionUrl_.port(), inQueue_, outQueue_, this);
    //webServer_ = new StreamWebsocketServer(DEFAULT_WEBSOCKET_PORT, inQueue_, outQueue_, this);

    QTimer::singleShot(0, this, [=]{
        if(pcTcpServer_->startServer()){
            qDebug() << "Server has been started on port: " << connectionUrl_.port();
        }
        inBuffChecker();
    });
}

AppController::~AppController()
{
    qDebug() << "~AppController";
}

void AppController::slotChangeConnectionInfo(const QString connectionInfo)
{
    connectionUrl_ = connectionInfo;
    slotChangeConnector(connectorType_);
}

void AppController::slotChangeConnector(const StreamCommunicationType type)
{
    // delete existing connector
    switch (connectorType_) {
    case StreamCommunicationType::TcpServer:
        if(pcTcpServer_){
            pcTcpServer_->stopServer();
            pcTcpServer_->deleteLater();
        }
        break;
    case StreamCommunicationType::TcpClient:
        if(pcTcpClient_){
            pcTcpClient_->slotStopClient();
            // pcTcpClient_ will be autodeleted
        }
        break;
    }

    // create new connector
    switch (type) {
    case StreamCommunicationType::TcpServer:
        pcTcpServer_ = new StreamTcpServer(connectionUrl_.port(), inQueue_, outQueue_, this);
        if(pcTcpServer_->startServer()){
            qDebug() << "Server has been started on port: " << connectionUrl_.port();
        }
        break;
    case StreamCommunicationType::TcpClient:
        pcTcpClient_ = new StreamTcpClient(connectionUrl_, inQueue_, outQueue_, this);
        pcTcpClient_->start();
        break;
    }

    connectorType_ = type;
}

void AppController::slotChangeWsPort(int port)
{
    if(webServer_){
        webServer_->slotStopServer();
        webServer_->deleteLater();
    }

    webServer_ = new StreamWebsocketServer(port, inQueue_, outQueue_, this);
}

void AppController::slotSendStream(const QByteArray &data)
{
    QFuture<void> future = QtConcurrent::run([=](const QByteArray &data){
        for (const auto ch : data) {
            outQueue_.push(ch);
        }
    }, data);
}

void AppController::quit()
{
    if(pcTcpServer_){
        pcTcpServer_->stopServer();
        // will be autodeleted
    }

    if(pcTcpClient_){
        pcTcpClient_->slotStopClient();
        // will be autodeleted
    }

    if(webServer_){
        webServer_->slotStopServer();
        // will be autodeleted
    }
}

QElapsedTimer timer;
std::uint64_t totalBytesNum = 0;
void AppController::inBuffChecker()
{

    /*
         *  // If uncomment next lines, all received text will showing on MainWindow form.
         *
         *  QByteArray ba;
         *
         *  char data;
         *  while (inQueue_.tryPop(data)) {
         *      ba.append(data);
         *  }
         *
         *  if(! ba.isEmpty()){
         *      emit sigStreamReaded(ba);
         *  }
         */

    std::uint64_t currentBytesNum = 0;

    char data = 0;
    while (inQueue_.tryPop(data)) {
        if(data == char('S')) { // start a timer, when we receive char 'S'
            timer.start();
        }
        totalBytesNum++;
        currentBytesNum++;
    }

    if(data == char('E')) {// log a timer.eapsed, when we receive char 'S'
        double mbS = (totalBytesNum/1048576.)/(timer.elapsed()/1000.);
        qDebug() << "Total received: " << totalBytesNum <<"(~" << totalBytesNum/1048576. << "MB)"
                 << "Time: " << timer.elapsed()/1000. << "s  ("  << timer.elapsed() <<"ms)"
                 << "\tSpeed: " << QString::number(mbS, 'f', 2).toDouble() << "MB/s" << "==" << QString::number(mbS*8., 'f', 2).toDouble() << "Mbit/s";
        totalBytesNum = 0;
    }

    // for debug reason
    //if (totalBytesNum > 0 && (totalBytesNum % 1000) == 0) {
    //    qDebug() << "Total received: " << totalBytesNum << "Currently received: " << currentBytesNum <<data;
    //}

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &AppController::inBuffChecker);
}
