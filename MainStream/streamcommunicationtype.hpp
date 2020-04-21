#ifndef STREMCOMMUNICATIONTYPE_H
#define STREMCOMMUNICATIONTYPE_H

#include <QObject>

namespace MainStream {
Q_NAMESPACE

enum StreamCommunicationType {
    TcpServer,
    TcpClient
};
Q_ENUM_NS(StreamCommunicationType)

}

#endif // STREMCOMMUNICATIONTYPE_H
