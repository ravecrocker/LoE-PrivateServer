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
    messageUtils.cpp \
    serverCommands.cpp

HEADERS  += widget.h \
	character.h \
	message.h \
	utils.h \
	scene.h \
	dataType.h \
	sync.h

FORMS    += widget.ui
