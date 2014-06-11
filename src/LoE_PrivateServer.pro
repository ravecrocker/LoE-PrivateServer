#-------------------------------------------------
#
# Project created by QtCreator 2013-06-21T21:58:54
#
#-------------------------------------------------

QT       += core gui network widgets

TARGET = LoE_PrivateServer
TEMPLATE = app

SOURCES += main.cpp \
        widget.cpp \
    tcp.cpp \
    udp.cpp \
    messages.cpp \
    utils.cpp \
    pingTimeout.cpp \
    character.cpp \
    scene.cpp \
    dataType.cpp \
    sync.cpp \
    receiveMessage.cpp \
    sendMessage.cpp \
    serverCommands.cpp \
    quest.cpp \
    serialize.cpp \
    items.cpp \
    receiveAck.cpp \
    receiveChatMessage.cpp \
    mobsParser.cpp \
    mob.cpp \
    mobStats.cpp \
    skill.cpp \
    skillparser.cpp \
    widgetStartStopServer.cpp

HEADERS  += widget.h \
    character.h \
    message.h \
    utils.h \
    scene.h \
    dataType.h \
    sync.h \
    quest.h \
    serialize.h \
    items.h \
    sendMessage.h \
    receiveAck.h \
    receiveChatMessage.h \
    mobzone.h \
    sceneEntity.h \
    mobsParser.h \
    mob.h \
    mobsStats.h \
    packetloss.h \
    skill.h \
    skillparser.h \
    statsComponent.h

FORMS    += widget.ui

TRANSLATIONS = ../translations/fr.ts \
    ../translations/ru.ts

# include coreservices (required for timestamps) only on mac
macx {
    QMAKE_LFLAGS += -F /System/Library/Frameworks/CoreServices.framework/
    LIBS += -framework CoreServices
}

CONFIG += c++11
