#ifndef STREAMWEBSOCKETSERVER_H
#define STREAMWEBSOCKETSERVER_H

#include "ThreadSafeQueue.hpp"
#include "common.hpp"
#include "websocketserverworker.h"

#include <QPointer>
#include <QThread>
#include <QWebSocket>
#include <QWebSocketServer>


namespace MainStream {

class StreamWebsocketServer : public QObject {
    Q_OBJECT

public:
    StreamWebsocketServer(int port, InThreadSafeQueue &inQueue, OutThreadSafeQueue &outQueue, QObject *parent = nullptr);

    ~StreamWebsocketServer() override;

public slots:
    void slotStopServer();

private:
    QPointer<WebSocketServerWorker> wsServer_;
    QPointer<QThread> serverThread_;
};

}

#endif // STREAMWEBSOCKETSERVER_H
