#include "widget.h"
#include "ui_widget.h"
#include "character.h"
#include "message.h"
#include "utils.h"
#include "mob.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    cmdPeer(new Player()),
    usedids(new bool[65536])
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

int Widget::getNewNetviewId()
{
    int i;

    for (i = 0; i < 65536; i++) {
        usedids[i] = false;
    }

    for (int c = 0; c < npcs.size(); c++) {
        usedids[npcs[c]->netviewId] = true;
    }
    for (int c = 0; c < mobs.size(); c++) {
        usedids[mobs[c]->netviewId] = true;
    }
    for (int c = 0; c < udpPlayers.size(); c++) {
        usedids[udpPlayers[c]->pony.netviewId] = true;
    }

    i = 0;
    while (usedids[i]) i++;

    return i;
}

int Widget::getNewId()
{
    int i;

    for (i = 0; i < 65536; i++) {
        usedids[i] = false;
    }

    for (int c = 0; c < npcs.size(); c++) {
        usedids[npcs[c]->id] = true;
    }
    for (int c = 0; c < mobs.size(); c++) {
        usedids[mobs[c]->id] = true;
    }
    for (int c = 0; c < udpPlayers.size(); c++) {
        usedids[udpPlayers[c]->pony.id] = true;
    }

    i = 0;
    while (usedids[i]) i++;

    return i;
}

// Disconnect players, free the sockets, and exit quickly
// Does NOT run the atexits
Widget::~Widget()
{
    logInfos=false; // logMessage while we're trying to destroy would crash.
    //logMessage(tr("UDP: Disconnecting all players"));
    for (;udpPlayers.size();)
    {
        Player* player = udpPlayers[0];
        sendMessage(player, MsgDisconnect, "Connection closed by the server admin");

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
        udpPlayers.removeFirst();
    }

    for (int i=0; i<quests.size(); i++)
    {
        delete quests[i].commands;
        delete quests[i].name;
        delete quests[i].descr;
    }

    stopServer(false);
    delete tcpServer;
    delete tcpReceivedDatas;
    delete udpSocket;
    delete pingTimer;
    delete cmdPeer;
    delete[] usedids;

    delete ui;

    // We freed everything that was important, so don't waste time in atexits
#if defined WIN32 || defined _WIN32 || defined __APPLE__
    _exit(EXIT_SUCCESS);
#else
    quick_exit(EXIT_SUCCESS);
#endif
}
