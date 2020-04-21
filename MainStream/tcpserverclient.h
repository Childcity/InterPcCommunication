#ifndef TCPSERVERCLIENT_H
#define TCPSERVERCLIENT_H

#include "ThreadSafeQueue.hpp"
#include "common.hpp"

#include <QPointer>
#include <QTcpSocket>
#include <QThread>


namespace MainStream {

class TcpServerClient : public QObject {
    Q_OBJECT

public:
    explicit TcpServerClient(qintptr socketDescriptor, InThreadSafeQueue &inQueue, OutThreadSafeQueue &outQueue, QObject *parent = nullptr);

    ~TcpServerClient() override;

signals:
    // public
    void sigClientDisconnected();

    // private
    void sigStopClient(QPrivateSignal);

public slots:
    void slotStartClient();

    void slotStopClient();

private slots:
    void slotReadyRead();

    void slotSocketError(QAbstractSocket::SocketError error);

    void slotCloseClient();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    qintptr socketDescriptor_;
    QPointer<QTcpSocket> clientSocket_;
    std::array<char, READ_BUFSIZE> readBuf_;

    InThreadSafeQueue &inQueue_;
    OutThreadSafeQueue &outQueue_;
};

}

#endif // TCPSERVERCLIENT_H
