QT       += core gui serialport websockets network concurrent widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings depend on your compiler).
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    MainStream/StreamTcpClient/streamtcpclient.cpp \
    MainStream/StreamTcpServer/streamtcpserver.cpp \
    MainStream/StreamTcpServer/tcpserverclient.cpp \
    MainStream/StreamWebsocketServer/streamwebsocketserver.cpp \
    MainStream/StreamWebsocketServer/websocketserverworker.cpp \
    MainStream/icommunicator.cpp \
    main.cpp \
    Gui/mainwindow.cpp \
    appcontroller.cpp

HEADERS += \
    Gui/mainwindow.h \
    MainStream/StreamTcpClient/streamtcpclient.h \
    MainStream/StreamTcpServer/streamtcpserver.h \
    MainStream/StreamTcpServer/tcpserverclient.h \
    MainStream/StreamWebsocketServer/streamwebsocketserver.h \
    MainStream/StreamWebsocketServer/websocketserverworker.h \
    Utils/ThreadSafeQueue.hpp \
    MainStream/bufferchunk.hpp \
    MainStream/common.hpp \
    MainStream/constants.h \
    MainStream/icommunicator.h \
    MainStream/streamcommunicationtype.hpp \
    appcontroller.h

FORMS += \
    Gui/mainwindow.ui

DESTDIR = build/
OBJECTS_DIR = obj/
MOC_DIR     = moc/
UI_DIR      = ui/
