#include "websocketserverworker.h"


MainStream::WebSocketServerWorker::WebSocketServerWorker(int port, MainStream::InThreadSafeQueue &inQueue, MainStream::OutThreadSafeQueue &outQueue, QObject *parent)
    : QObject(parent)
    , port_(port)
    , inQueue_(inQueue)
    , outQueue_(outQueue)
{}

MainStream::WebSocketServerWorker::~WebSocketServerWorker()
{
    if(wsServer_){
        delete wsServer_.data();
    }
    if(clients_){
        qDeleteAll(clients_->begin(), clients_->end());
        clients_->clear();
        delete clients_;
        clients_ = nullptr;
    }
}

void MainStream::WebSocketServerWorker::startServer()
{
    clients_ = nullptr;
    wsServer_ = new QWebSocketServer("Server for streamming data", QWebSocketServer::SslMode::NonSecureMode, this);

    if (! wsServer_->listen(QHostAddress::Any, port_)) {
        qWarning() << "WsServer not started: " << wsServer_->errorString();
        emit sigClosed();
        return;
    }

    clients_ = new QList<QWebSocket *>();

    connect(wsServer_, &QWebSocketServer::newConnection, this, &WebSocketServerWorker::slotNewConnection, Qt::ConnectionType::DirectConnection);
    connect(wsServer_, &QWebSocketServer::closed, this, &WebSocketServerWorker::sigClosed, Qt::ConnectionType::DirectConnection);

    connect(this, &WebSocketServerWorker::sigCloseServer, this, &WebSocketServerWorker::slotCloseServer, Qt::ConnectionType::QueuedConnection);

    qDebug() << "WSServer started. WSServerThread[" << wsServer_->thread()
             << "]. Id[" << QThread::currentThreadId() << "]";

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &WebSocketServerWorker::inBuffChecker);
}

void MainStream::WebSocketServerWorker::slotStopServer()
{
    // We should emit sigCloseServer that connected to slotCloseServer as QueuedConnection
    // (DirectConnection will not work, as WebSocketServerWorker will working in separate thread)
    emit sigCloseServer(QPrivateSignal());
}

void MainStream::WebSocketServerWorker::slotNewConnection()
{
    QWebSocket *client = wsServer_->nextPendingConnection();

    connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServerWorker::slotTextMessage);
    connect(client, &QWebSocket::binaryMessageReceived, this, &WebSocketServerWorker::slotBinaryMessage);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServerWorker::slotSocketDisconnected);

    *clients_ << client;
    qDebug() << "Connected Client[" << client->peerAddress() << client->peerPort() << client
             << "]. CurrentThread[" << QThread::currentThreadId()
             << "].";
}

void MainStream::WebSocketServerWorker::slotTextMessage(const QString &message)
{
    QWebSocket *client = getCurrentClient();
    if (client) {
        FillQueueInSepThread(outQueue_, message.toUtf8());

        //qDebug() << "slotTextMessage Client[" << client->peerAddress() << client->peerPort() << client
        //         << "]. CurrentThread[" << QThread::currentThreadId()
        //         << "].";
    }
}

void MainStream::WebSocketServerWorker::slotBinaryMessage(const QByteArray &message)
{
    QWebSocket *client = getCurrentClient();
    if (client) {
        FillQueueInSepThread(outQueue_, message);

        //qDebug() << "slotBinaryMessage Client[" << client->peerAddress() << client->peerPort() << client
        //         << "]. CurrentThread[" << QThread::currentThreadId()
        //         << "].";
    }
}

void MainStream::WebSocketServerWorker::slotSocketDisconnected()
{
    QWebSocket *client = getCurrentClient();
    qDebug() << "socketDisconnected:" << client;
    if (client) {
        client->deleteLater();
        clients_->removeAll(client);
        qDebug() << "OnDisconnected Client[" << client->peerAddress() << client->peerPort() << client
                 << "]. CurrentThread[" << QThread::currentThreadId()
                 << "].";
    }
}

void MainStream::WebSocketServerWorker::slotCloseServer()
{
    if(wsServer_){
        wsServer_->close();
        wsServer_->deleteLater();
    }
    if(clients_){
        qDeleteAll(clients_->begin(), clients_->end());
        clients_->clear();
        delete clients_;
        clients_ = nullptr;;
    }
    emit sigClosed();
}

void MainStream::WebSocketServerWorker::inBuffChecker()
{
    // We check for 'isValid()' only ONE time, because this func will block Qt Event loop
    // and socket can't become Invalid untill this function return
    if (clients_ && (! clients_->isEmpty())) {
        InBuffChunk chunk;
        while (inQueue_.tryPop(chunk)) {
            sendToClients(chunk);
        }
    }

    QTimer::singleShot(1, Qt::TimerType::PreciseTimer, this, &WebSocketServerWorker::inBuffChecker);
}

QWebSocket *MainStream::WebSocketServerWorker::getCurrentClient()
{
    return qobject_cast<QWebSocket *>(sender());
}

void MainStream::WebSocketServerWorker::sendToClients(const InBuffChunk &chunk)
{
    for (const auto clientSocket : *clients_) {
        if (clientSocket->isValid()){
            sendToClient(clientSocket, chunk);
            clientSocket->flush(); // We must call this, because if there are a lot of chunks in outQueue_, the socket buffer can overflow
        }
    }
}

void MainStream::WebSocketServerWorker::sendToClient(const QPointer<QWebSocket> clientSocket, const MainStream::InBuffChunk &chunk)
{
    size_t sendedNum = clientSocket->sendBinaryMessage(QByteArray(chunk.data.data(), chunk.actualSize));

    // If we wrote to the clientSocket_ not all chunk...
    // This situation is very rare...
    if(sendedNum < chunk.actualSize) {
        size_t totalSended = sendedNum;
        while (totalSended < chunk.actualSize) {
            const char *leftDataPtr = (chunk.data.data() + totalSended);
            size_t leftDataSize = (chunk.actualSize - totalSended);
            sendedNum = clientSocket->sendBinaryMessage(QByteArray(leftDataPtr, leftDataSize));
            totalSended += sendedNum;
        }
    }
}
