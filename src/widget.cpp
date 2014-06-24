#include "widget.h"
#include "ui_widget.h"
#include "player.h"
#include "message.h"
#include "utils.h"
#include "mob.h"
#include "sync.h"
#include "quest.h"
#include "settings.h"
#include "udp.h"
#include <QUdpSocket>

using namespace Settings;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    cmdPeer(new Player()),
    sync{new Sync()}
{
    tcpServer = new QTcpServer(this);
    udpSocket = new QUdpSocket(this);
    tcpReceivedDatas = new QByteArray();
    ui->setupUi(this);

    pingTimer = new QTimer(this);

    qsrand(QDateTime::currentMSecsSinceEpoch());
    srand(QDateTime::currentMSecsSinceEpoch());
}

/// Adds the message in the log, and sets it as the status message
void Widget::logStatusMessage(QString msg)
{
    ui->log->appendPlainText(msg);
    ui->status->setText(msg);
    ui->log->repaint();
    ui->status->repaint();
}

/// Adds the message to the log
void Widget::logMessage(QString msg)
{
    if (!logInfos)
        return;
    ui->log->appendPlainText(msg);
    ui->log->repaint();
}

/// Adds the error in the log, and sets it as the status message
void Widget::logStatusError(QString msg)
{
    static QTextCharFormat defaultFormat, redFormat;
    defaultFormat.setForeground(QBrush(Qt::black));
    redFormat.setForeground(QBrush(Qt::red));
    ui->log->setCurrentCharFormat(redFormat);
    ui->log->appendPlainText(msg);
    ui->status->setText(msg);
    ui->log->repaint();
    ui->status->repaint();
    ui->log->setCurrentCharFormat(defaultFormat);
}

/// Adds the error to the log
void Widget::logError(QString msg)
{
    if (!logInfos)
        return;
    static QTextCharFormat defaultFormat, redFormat;
    defaultFormat.setForeground(QBrush(Qt::black));
    redFormat.setForeground(QBrush(Qt::red));
    ui->log->setCurrentCharFormat(redFormat);
    ui->log->appendPlainText(msg);
    ui->log->repaint();
    ui->log->setCurrentCharFormat(defaultFormat);
}

// Disconnect players, free the sockets, and exit quickly
// Does NOT run the atexits
Widget::~Widget()
{
    logInfos=false; // logMessage while we're trying to destroy would crash.
    //logMessage(tr("UDP: Disconnecting all players"));
    while (Player::udpPlayers.size())
    {
        Player* player = Player::udpPlayers[0];
        sendMessage(player, MsgDisconnect, "Server closed by the admin");

        // Save the pony
        QList<Pony> ponies = Player::loadPonies(player);
        for (int i=0; i<ponies.size(); i++)
            if (ponies[i].ponyData == player->pony.ponyData)
                ponies[i] = player->pony;
        Player::savePonies(player, ponies);
        player->pony.saveInventory();
        player->pony.saveQuests();

        // Free
        delete player;
        Player::udpPlayers.removeFirst();
    }

    for (int i=0; i<Quest::quests.size(); i++)
    {
        delete Quest::quests[i].commands;
        delete Quest::quests[i].name;
        delete Quest::quests[i].descr;
    }

    stopServer(false);
    delete tcpServer;
    delete tcpReceivedDatas;
    delete udpSocket;
    delete pingTimer;
    delete cmdPeer;

    delete ui;

    // We freed everything that was important, so don't waste time in atexits
#if defined WIN32 || defined _WIN32 || defined __APPLE__
    _exit(EXIT_SUCCESS);
#else
    quick_exit(EXIT_SUCCESS);
#endif
}
