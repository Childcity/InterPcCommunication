#include "streamwebsocketserver.h"


MainStream::StreamWebsocketServer::StreamWebsocketServer(int port, MainStream::InThreadSafeQueue &inQueue, MainStream::OutThreadSafeQueue &outQueue, QObject *parent)
    : QObject(parent)
{
    serverThread_ = new QThread(this);
    wsServer_ = new WebSocketServerWorker(port, inQueue, outQueue, nullptr);
    wsServer_->moveToThread(serverThread_);

    // We need to instanciate all fields in New Thread
    // for this purpose we initDispenserObject() after workerThread_.start() and
    // initDispenserObject will be executed in New Thread
    connect(serverThread_, &QThread::started, wsServer_, &WebSocketServerWorker::startServer);

    // Connection to proper delition of Thread from memory after delition of 'client'
    connect(wsServer_, &QObject::destroyed, serverThread_, &QThread::quit);
    connect(wsServer_, &QObject::destroyed, this, [=]{qDebug() << "wsServer_ QObject::destroyed!";});
    connect(serverThread_, &QThread::finished, serverThread_, &QObject::deleteLater);
    connect(serverThread_, &QThread::finished, this, [=]{qDebug() << "StreamWebsocketServer Thread Finished properly!";});
    connect(wsServer_, &WebSocketServerWorker::sigClosed, this, [=]{
        qDebug() << "Catch sigServerStoped from: " << wsServer_;
        wsServer_->deleteLater();
    }, Qt::ConnectionType::QueuedConnection);

    serverThread_->start();

    qDebug() << "CurrentThread[" << QThread::currentThreadId()
             << "].";
}

MainStream::StreamWebsocketServer::~StreamWebsocketServer()
{
    qDebug() << "~StreamWebsocketServer";

    if(serverThread_){
        serverThread_->quit();
        serverThread_->wait();
        delete serverThread_.data();
    }
}

void MainStream::StreamWebsocketServer::slotStopServer()
{
    if(wsServer_)
        wsServer_->slotStopServer();
}
