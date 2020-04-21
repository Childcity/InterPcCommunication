#include "streamtcpserver.h"


MainStream::StreamTcpServer::StreamTcpServer(int port, MainStream::InThreadSafeQueue &inQueue, MainStream::OutThreadSafeQueue &outQueue, QObject *parent)
    : QTcpServer(parent)
    , port_(port)
    , inQueue_(inQueue)
    , outQueue_(outQueue)
{}

MainStream::StreamTcpServer::~StreamTcpServer()
{
    qDebug() << "~StreamTcpServer";
}

bool MainStream::StreamTcpServer::startServer()
{
    if (! listen(QHostAddress::Any, port_)) {
        qWarning() << "Server not started: " << errorString();
        return false;
    }

    return true;
}

void MainStream::StreamTcpServer::stopServer()
{
    close();

    qDebug() << "There are unclosed clients: " << clients_.size() << "and threads" <<clientsThreads_.size();

    for (auto cl : clients_) {
        cl->slotStopClient();
    }

    for(auto th : clientsThreads_){
        th->quit();
        th->wait();
    }

    clients_.clear();
    clientsThreads_.clear();
}

void MainStream::StreamTcpServer::incomingConnection(qintptr socketDescriptor)
{
    QThread *clientThread = new QThread(this);

    // Creat Client and move to another Thread
    TcpServerClient *serverClient = new TcpServerClient(socketDescriptor, inQueue_, outQueue_, nullptr);
    serverClient->moveToThread(clientThread);

    // we need to instanciate all fields in New Thread
    // for this purpose we initDispenserObject() after workerThread_.start() and
    // initDispenserObject will be executed in New Thread
    connect(clientThread, &QThread::started, serverClient, &TcpServerClient::slotStartClient);

    // Connection to proper delition of Thread from memory after delition of 'client'
    connect(serverClient, &QObject::destroyed, clientThread, &QThread::quit);
    connect(serverClient, &QObject::destroyed, this, [=]{qDebug() << "serverClient QObject::destroyed!";});
    connect(clientThread, &QThread::finished, clientThread, &QObject::deleteLater);
    connect(clientThread, &QThread::finished, this, [=]{qDebug() << "ClientThread Finished properly!";});
    connect(serverClient, &TcpServerClient::sigClientDisconnected, this, [=]{
        clientsThreads_.removeAll(clientThread);
        clients_.removeAll(serverClient);
        serverClient->deleteLater();
        qDebug() << "Catch sigClientDisconnected from: " << socketDescriptor;
    }, Qt::ConnectionType::QueuedConnection);

    clientsThreads_.append(clientThread);
    clients_.append(serverClient);

    clientThread->start();

    qDebug() << "Connected Client[" << socketDescriptor
             << "]. CurrentThread[" << QThread::currentThreadId()
             << "].";
}
