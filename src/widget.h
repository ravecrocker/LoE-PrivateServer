#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets/QWidget>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <memory>

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#define GAMEDATAPATH "data/data/"
#define PLAYERSPATH "data/players/"
#define NETDATAPATH "data/netData/"
#define MOBSPATH "data/mobZones/"
#define CONFIGFILEPATH "data/server.ini"
#define SERVERSLISTFILEPATH "data/serversList.cfg"

class Mobzone;
class Mob;
class Sync;
class Player;
namespace Ui {class Widget;}

class Widget : public QWidget
{
    Q_OBJECT

    /// Main functions
public slots:
    void sendCmdLine();
    void checkPingTimeouts();
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void logMessage(QString msg);
    void logStatusMessage(QString msg);
    void logError(QString msg);
    void logStatusError(QString msg);
    void startServer();
    void stopServer(); // Calls stopServer(true)
    void stopServer(bool log);

    /// UDP/TCP
public slots:
    void tcpConnectClient();
    void tcpDisconnectClient();
    void tcpProcessPendingDatagrams();
public:
    void tcpProcessData(QByteArray data, QTcpSocket *socket);

public:
    float startTimestamp;
    int syncInterval;

private:
    Ui::Widget* ui;
    QTcpServer* tcpServer;
    QList<QPair<QTcpSocket*, QByteArray*>> tcpClientsList;
    QTcpSocket remoteLoginSock; // Socket to the remote login server, if we use one
    QByteArray* tcpReceivedDatas;
    Player* cmdPeer; // Player selected for the server commands
    QTimer* pingTimer;
    std::unique_ptr<Sync> sync;
};

extern Widget win; // Defined in main

#endif // WIDGET_H
