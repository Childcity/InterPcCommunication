#include "streamtcpclient.h"

MainStream::StreamTcpClient::StreamTcpClient(QUrl hostAddr, MainStream::InThreadSafeQueue &inQueue, MainStream::OutThreadSafeQueue &outQueue, QObject *parent)
    : QThread(parent)
    , hostAddr_(hostAddr)
    , inQueue_(inQueue)
    , outQueue_(outQueue)
{}

MainStream::StreamTcpClient::~StreamTcpClient()
{
    if(clientSocket_){
        clientSocket_->disconnect();
        //clientSocket_->close();
        delete clientSocket_.data();
    }

    qDebug() << "~StreamTcpClient";

    quit();
    wait();
}

void MainStream::StreamTcpClient::run()
{
    QTimer::singleShot(0, this, [=]{
        clientSocket_ = new QTcpSocket(this);

        // We will catch disconnected in error slot
        //connect(clientSocket_, &QTcpSocket::disconnected, this, &TcpServerClient::sigClientDisconnected, Qt::ConnectionType::QueuedConnection);
        connect(clientSocket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &StreamTcpClient::slotSocketError);
        connect(clientSocket_, &QTcpSocket::readyRead, this, &StreamTcpClient::slotReadyRead);

        connect(this, &StreamTcpClient::sigCloseClient, this, &StreamTcpClient::slotCloseClient, Qt::ConnectionType::QueuedConnection);
        connect(this, &StreamTcpClient::sigClientDisconnected, this, &QThread::quit, Qt::ConnectionType::DirectConnection);

        clientSocket_->connectToHost(hostAddr_.host(), hostAddr_.port(DEFAULT_TCP_PORT));

        QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &StreamTcpClient::outBuffChecker);
    });

    exec();
}

void MainStream::StreamTcpClient::slotStopClient()
{
    emit sigCloseClient(QPrivateSignal());
}

void MainStream::StreamTcpClient::slotReadyRead()
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

void MainStream::StreamTcpClient::slotSocketError(QAbstractSocket::SocketError error)
{
    QString errStr = QString("Error [%1] in client socket").arg(error);
    if(clientSocket_){
        errStr += QString(". Socket: %1. Description: %2: ").arg(clientSocket_->socketDescriptor()).arg(clientSocket_->errorString());
    }
    qDebug() << errStr;
    deleteLater();
    emit sigClientDisconnected();
}

void MainStream::StreamTcpClient::slotCloseClient()
{
    if(clientSocket_){
        clientSocket_->close();
        clientSocket_->deleteLater();
    }
    qDebug() << "slotStopClient";
    deleteLater();
    emit sigClientDisconnected();
}

void MainStream::StreamTcpClient::outBuffChecker()
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

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &StreamTcpClient::outBuffChecker);
}

void MainStream::StreamTcpClient::writeData(const MainStream::OutBuffChunk &chunk)
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
