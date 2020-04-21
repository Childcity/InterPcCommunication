QT       += core gui serialport websockets network widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings depend on your compiler).
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    MainStream/icommunicator.cpp \
    MainStream/streamtcpclient.cpp \
    MainStream/streamtcpserver.cpp \
    MainStream/streamwebsocketserver.cpp \
    MainStream/tcpserverclient.cpp \
    MainStream/websocketserverworker.cpp \
    main.cpp \
    Gui/mainwindow.cpp \
    appcontroller.cpp

HEADERS += \
    Gui/mainwindow.h \
    MainStream/ThreadSafeQueue.hpp \
    MainStream/bufferchunk.hpp \
    MainStream/common.hpp \
    MainStream/constants.h \
    MainStream/icommunicator.h \
    MainStream/streamcommunicationtype.hpp \
    MainStream/streamtcpclient.h \
    MainStream/streamtcpserver.h \
    MainStream/streamwebsocketserver.h \
    MainStream/tcpserverclient.h \
    MainStream/websocketserverworker.h \
    appcontroller.h

FORMS += \
    Gui/mainwindow.ui

DESTDIR = build/
OBJECTS_DIR = obj/
MOC_DIR     = moc/
UI_DIR      = ui/
