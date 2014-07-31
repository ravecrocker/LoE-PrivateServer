#ifndef APP_H
#define APP_H

#ifdef USE_GUI
    #include <QtWidgets/QApplication>
    #include <QtWidgets/QWidget>
    #include <QClipboard>
    #include <QFileDialog>
    #include "ui_app.h"
    #define QAPP_TYPE QApplication
    #define APP_CLASS QWidget
#else // USE_CONSOLE
    #include <QCoreApplication>
    #include <QSocketNotifier>
    #include <QTextStream>
    #define QAPP_TYPE QCoreApplication
    #define APP_CLASS QObject
#endif

#include <QFile>
#include <QTimer>
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
#ifdef USE_GUI
namespace Ui {class App;}
#endif

class App : public APP_CLASS
{
    Q_OBJECT

    /// Main functions
public slots:
    void sendCmdLine();
    void checkPingTimeouts();
public:
#ifdef USE_GUI
    explicit App(QWidget *parent = 0);
#else
    explicit App();
#endif
    ~App();
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

#ifdef USE_GUI
private slots:
    void on_clearLogButton_clicked();
    void on_copyLogButton_clicked();
    void on_saveLogButton_clicked();
#endif

#ifdef USE_GUI
public:
    Ui::App* ui;
#endif
private:
#ifdef USE_CONSOLE
    QSocketNotifier *cin_notifier;
#endif
    QTcpServer* tcpServer;
    QList<QPair<QTcpSocket*, QByteArray*>> tcpClientsList;
    QTcpSocket remoteLoginSock; // Socket to the remote login server, if we use one
    QByteArray* tcpReceivedDatas;
    Player* cmdPeer; // Player selected for the server commands
    QTimer* pingTimer;
    std::unique_ptr<Sync> sync;
};

#ifdef USE_CONSOLE
extern QTextStream cin;
extern QTextStream cout;
#endif

extern App app; // Defined in main

#endif // APP_H
