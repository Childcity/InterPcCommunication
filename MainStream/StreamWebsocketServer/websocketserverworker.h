#ifndef WEBSOCKETSERVERWORKER_H
#define WEBSOCKETSERVERWORKER_H

#include "../Utils/ThreadSafeQueue.hpp"
#include "../common.hpp"

#include <QPointer>
#include <QThread>
#include <QWebSocket>
#include <QWebSocketServer>


namespace MainStream {

class WebSocketServerWorker : public QObject {
    Q_OBJECT

public:
    WebSocketServerWorker(int port, InThreadSafeQueue &inQueue, OutThreadSafeQueue &outQueue, QObject *parent = nullptr);

    ~WebSocketServerWorker() override;

signals:
    // public signals
    void sigClosed();

    // private signals
    void sigCloseServer(QPrivateSignal);

public slots:
    void startServer();

    void slotStopServer();

private slots:
    void slotNewConnection();

    void slotTextMessage(const QString &message);

    void slotBinaryMessage(const QByteArray &message);

    void slotSocketDisconnected();

    void slotCloseServer();

private:
    void inBuffChecker();

    void sendToClients(const InBuffChunk &chunk);

    void sendToClient(const QPointer<QWebSocket> clientSocket, const InBuffChunk &chunk);

    QWebSocket *getCurrentClient();

private:
    qint16 port_;
    QPointer<QWebSocketServer> wsServer_;
    QList<QWebSocket *> *clients_;

    InThreadSafeQueue &inQueue_;
    OutThreadSafeQueue &outQueue_;
};

}

#endif // WEBSOCKETSERVERWORKER_H
