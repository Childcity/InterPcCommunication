#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "../Utils/ThreadSafeQueue.hpp"
#include "../common.hpp"

#include <QPointer>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QUrl>


namespace MainStream {

class StreamTcpClient : public QThread {
    Q_OBJECT

public:
    explicit StreamTcpClient(QUrl hostAddr, InThreadSafeQueue &inQueue, OutThreadSafeQueue &outQueue, QObject *parent = nullptr);

    ~StreamTcpClient() override;

    void run() override;

signals:
    // public signals
    void sigClientDisconnected();

    // private signals
    void sigCloseClient(QPrivateSignal);

public slots:
    void slotStopClient();

private slots:
    void slotReadyRead();

    void slotSocketError(QAbstractSocket::SocketError);

    void slotCloseClient();

private:
    void outBuffChecker();

    void writeData(const OutBuffChunk &);

private:
    QUrl hostAddr_;
    QPointer<QTcpSocket> clientSocket_;
    std::array<char, READ_BUFSIZE> readBuf_;
    std::array<char, WRITE_BUFSIZE> writeBuf_;

    InThreadSafeQueue &inQueue_;
    OutThreadSafeQueue &outQueue_;
};

}

#endif // TCPCLIENT_H
