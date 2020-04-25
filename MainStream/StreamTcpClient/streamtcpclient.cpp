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
        //qint64 rlen = clientSocket_->read(readBuf_.data(), readBuf_.size());
        //qDebug() << "Thread[" << QThread::currentThreadId()
        //         << "]. Client[" << clientSocket_->socketDescriptor()
        //         << "]. Read: " << QByteArray(readBuf_.data(), rlen);
        //for (int i = 0; i < rlen; ++i) {
        //    inQueue_.push(readBuf_[i]);
        //}

        InBuffChunk chunk;
        chunk.actualSize = clientSocket_->read(chunk.data.data(), chunk.data.size());
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
    qWarning() << errStr;
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
    //char data; int sended = 0;
    //while (sended < 8192 && outQueue_.tryPop(data)) {
    //    writeBuf_[sended] = data;
    //    sended++;
    //}
    //
    //int wr = clientSocket_->write(writeBuf_.data(), sended);
    //if(wr < sended){
    //    qDebug() <<"wr:"<<wr<<"sendd:"<<sended<<"     DENGEROUS. I suppose, that this is never logged:)      ddddddddddddddddddddddddddddddddddddddddddddddddddddd";
    //}

    OutBuffChunk data;
    while (outQueue_.tryPop(data)) {
        writeData(data);
        clientSocket_->flush();
    }

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &StreamTcpClient::outBuffChecker);
}

void MainStream::StreamTcpClient::writeData(const MainStream::OutBuffChunk &data)
{
    std::size_t sendedNum = clientSocket_->write(data.data.data(), data.actualSize);

    if(sendedNum < data.actualSize){
        std::size_t totalSended = sendedNum;
        qDebug() <<"data.actualSize:"<<data.actualSize<<"sended:"<<sendedNum<<"     DENGEROUS. I suppose, that this is never logged:)      ddddddddddddddddddddddddddddddddddddddddddddddddddddd";
        qDebug() << "All buffer to be sended:" << QByteArray(data.data.data(), data.actualSize);
        while (totalSended < data.actualSize) {
            qDebug() << "sendedNum:" << sendedNum << "totalSended:" << totalSended << "actualSize - sendedNum =" << data.actualSize - sendedNum;
            qDebug() << "*(data.data.data()+sendedNum) = " << *(data.data.data()+sendedNum);
            qDebug() << "To be sended next:" << QByteArray((data.data.data()+sendedNum), data.actualSize - sendedNum);
            sendedNum = clientSocket_->write((data.data.data()+sendedNum), data.actualSize - sendedNum);
        }
    }
}
