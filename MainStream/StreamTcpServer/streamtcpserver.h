#ifndef STREAMTCPSERVER_H
#define STREAMTCPSERVER_H

#include "../Utils/ThreadSafeQueue.hpp"
#include "tcpserverclient.h"

#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <QThread>


namespace MainStream {

class StreamTcpServer : public QTcpServer {
    Q_OBJECT

public:
    explicit StreamTcpServer(int port, InThreadSafeQueue &inQueue, OutThreadSafeQueue &outQueue, QObject *parent = nullptr);

    ~StreamTcpServer();

    // Start listening for new clients
    bool startServer();

    // Stop listening and free resources
    void stopServer();

private:
    void incomingConnection(qintptr socketDescriptor);

private:
    qint16 port_;

    QList<QThread *> clientsThreads_;
    QList<TcpServerClient *> clients_;
    InThreadSafeQueue &inQueue_;
    OutThreadSafeQueue &outQueue_;
};

}

#endif // STREAMTCPSERVER_H
