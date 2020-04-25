#include "tcpserverclient.h"

#include <QTimer>


MainStream::TcpServerClient::TcpServerClient(qintptr socketDescriptor, MainStream::InThreadSafeQueue &inQueue, MainStream::OutThreadSafeQueue &outQueue, QObject *parent)
    : QObject(parent)
    , socketDescriptor_(socketDescriptor)
    , inQueue_(inQueue)
    , outQueue_(outQueue)
{}

MainStream::TcpServerClient::~TcpServerClient()
{
    if(clientSocket_){
        // We mustn't do next lines, because destructor will run in main thread
        // and socket must be edited only in thread in which creatd!
        // clientSocket_->disconnect();
        // clientSocket_->close();
        delete  clientSocket_.data();
    }
    qDebug() << "~TcpServerClient";
}

void MainStream::TcpServerClient::slotStartClient()
{
    clientSocket_ = new QTcpSocket(this);

    // We will catch disconnected in error slot
    // connect(clientSocket_, &QTcpSocket::disconnected, this, &TcpServerClient::sigClientDisconnected, Qt::ConnectionType::QueuedConnection);
    connect(clientSocket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &TcpServerClient::slotSocketError);
    connect(clientSocket_, &QTcpSocket::readyRead, this, &TcpServerClient::slotReadyRead);

    connect(this, &TcpServerClient::sigStopClient, this, &TcpServerClient::slotCloseClient, Qt::ConnectionType::QueuedConnection);

    if(! clientSocket_->setSocketDescriptor(socketDescriptor_))
    {
        qWarning() << "Thread[" << QThread::currentThreadId()
                   << "]. Client[" << socketDescriptor_
                   << "]. Error[ " << clientSocket_->error()
                   << "] of setting socket descriptor to new client socket: " << clientSocket_->errorString();
        emit sigClientDisconnected();
    }

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &TcpServerClient::outBuffChecker);
}

void MainStream::TcpServerClient::slotStopClient()
{
    emit sigStopClient(QPrivateSignal());
}

void MainStream::TcpServerClient::slotReadyRead()
{
    while (clientSocket_->bytesAvailable() > 0) {
        InBuffChunk chunk;
        chunk.actualSize = clientSocket_->read(chunk.data.data(), chunk.data.size());

        //qDebug() << "Thread[" << QThread::currentThreadId()
        //         << "]. Client[" << clientSocket_->socketDescriptor()
        //         << "]. Read: " << QByteArray(chunk.data.data(), chunk.actualSize);

        if (chunk.actualSize > 0) {
            inQueue_.push(std::move(chunk));
        }
    }
}

void MainStream::TcpServerClient::slotSocketError(QAbstractSocket::SocketError error)
{
    QString errStr = QString("Error [%1] in client socket").arg(error);
    if(clientSocket_){
        errStr += QString(". Socket: %1. Description: %2: ").arg(clientSocket_->socketDescriptor()).arg(clientSocket_->errorString());
    }
    qDebug() << errStr;
    emit sigClientDisconnected();
}

void MainStream::TcpServerClient::slotCloseClient()
{
    clientSocket_->close();
    clientSocket_->deleteLater();
    emit sigClientDisconnected();
    qDebug() << "slotStopClient";
}

void MainStream::TcpServerClient::outBuffChecker()
{
    // We check for 'isValid()' only ONE time, because this func will block Qt Event loop
    // and socket can't become Invalid untill this function return
    if (clientSocket_->isValid()) {
        OutBuffChunk chunk;
        while (outQueue_.tryPop(chunk)) {
            writeData(chunk);
            clientSocket_->flush(); // We must call this, because if there are a lot of chunks in outQueue_, the socket buffer can overflow
        }
    }

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &TcpServerClient::outBuffChecker);
}

void MainStream::TcpServerClient::writeData(const MainStream::OutBuffChunk &chunk)
{
    size_t sendedNum = clientSocket_->write(chunk.data.data(), chunk.actualSize);

    // If we wrote to the clientSocket_ not all chunk...
    // This situation is very rare...
    if(sendedNum < chunk.actualSize) {
        size_t totalSended = sendedNum;
        while (totalSended < chunk.actualSize) {
            const char *leftDataPtr = (chunk.data.data() + totalSended);
            size_t leftDataSize = (chunk.actualSize - totalSended);
            sendedNum = clientSocket_->write(leftDataPtr, leftDataSize);
            totalSended += sendedNum;
        }
    }
}
