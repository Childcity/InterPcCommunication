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

    qDebug() << "WSServer started. WSServerThread[" << wsServer_->thread() << "]. Id[" << QThread::currentThreadId() << "]";

    using namespace std::chrono;
    startTimer(1ms, Qt::TimerType::PreciseTimer);
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
    qDebug() << "Message received:" << message << client;
    QByteArray utf8Str = message.toUtf8();
    if (client) {
        for (int i = 0; i < message.size(); ++i) {
            outQueue_.push(utf8Str.at(i));
        }
        qDebug() << "OnMessege Client[" << client->peerAddress() << client->peerPort() << client
                 << "]. CurrentThread[" << QThread::currentThreadId()
                 << "].";
    }
}

void MainStream::WebSocketServerWorker::slotBinaryMessage(QByteArray message)
{
    QWebSocket *client = getCurrentClient();
    qDebug() << "Binary Message received:" << message;
    if (client) {
        client->sendBinaryMessage(message);
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

void MainStream::WebSocketServerWorker::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    char data;
    while (inQueue_.tryPop(data)) {
        sendToClients(data);
    }
}

QWebSocket *MainStream::WebSocketServerWorker::getCurrentClient()
{
    return qobject_cast<QWebSocket *>(sender());
}

void MainStream::WebSocketServerWorker::sendToClients(char data)
{
    for (const auto client : *clients_) {
        if (client->isValid()){
            client->sendTextMessage(QString(data));
        }
    }
}
