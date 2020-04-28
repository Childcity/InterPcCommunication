#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include "MainStream/streamcommunicationtype.hpp"
#include "MainStream/StreamTcpServer/streamtcpserver.h"
#include "MainStream/StreamTcpClient/streamtcpclient.h"
#include "MainStream/StreamWebsocketServer/streamwebsocketserver.h"

#include <QElapsedTimer>
#include <QTimer>

using namespace MainStream;

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    ~AppController() override;

    void setIsTestInterPcMode(bool isTestInterPcMode);

signals:
    void sigStreamReaded(const QByteArray &);

public slots:
    void slotChangeConnectionInfo(const QString);

    void slotChangeConnector(const MainStream::StreamCommunicationType);

    void slotChangeWsPort(int port);

    void slotSendStream(const QByteArray &);

    void slotSetTestMode(bool);

    void quit();

private:
    void inBuffChecker();

private:
    StreamCommunicationType connectorType_;
    QPointer<StreamTcpServer> pcTcpServer_;
    QPointer<StreamTcpClient> pcTcpClient_;
    QUrl connectionUrl_;

    QPointer<StreamWebsocketServer> webServer_;

    InThreadSafeQueue inQueue_;
    OutThreadSafeQueue outQueue_;

    bool isTestInterPcMode_;
    QElapsedTimer timer;
    std::uint64_t totalBytesNum = 0;
};

#endif // APPCONTROLLER_H
