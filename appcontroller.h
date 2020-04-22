#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include "MainStream/streamcommunicationtype.hpp"
#include <MainStream/StreamTcpServer/streamtcpserver.h>
#include "MainStream/StreamTcpClient/streamtcpclient.h"
#include "MainStream/StreamWebsocketServer/streamwebsocketserver.h"

#include <QTimer>

using namespace MainStream;

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    ~AppController() override;

signals:
    void sigStreamReaded(QByteArray data);

public slots:
    void slotChangeConnectionInfo(const QString connectionInfo);

    void slotChangeConnector(const StreamCommunicationType type);

    void slotChangeWsPort(int port);

    void slotSendStream(QByteArray data);

    void quit();

protected:
    void timerEvent(QTimerEvent *) override;

private:
    StreamCommunicationType connectorType_;
    QPointer<StreamTcpServer> pcTcpServer_;
    QPointer<StreamTcpClient> pcTcpClient_;
    QUrl connectionUrl_;

    QPointer<StreamWebsocketServer> webServer_;

    InThreadSafeQueue inQueue_;
    OutThreadSafeQueue outQueue_;
};

#endif // APPCONTROLLER_H
