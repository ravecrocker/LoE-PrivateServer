#include <QFile>
#include <QDir>
#include "widget.h"
#include "character.h"
#include "message.h"

SceneEntity::SceneEntity()
{
    modelName = QString();
    id = 0;
    netviewId = 0;
    pos=UVector(0,0,0);
    rot=UQuaternion(0,0,0,0);
    sceneName = QString();
}

Pony::Pony() : SceneEntity()
{
    modelName = "PlayerBase";
    name = "";
}

Pony::type Pony::getType()
{
    // Variable UInt32
    unsigned char num3;
    int num = 0;
    int num2 = 0;
    int i=0;
    do
    {
        num3 = ponyData[i]; i++;
        num |= (num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);
    unsigned off = (uint) num  + i;
    return (type)(quint8)ponyData[off];
}

Player::Player()
{
    connected=false;
    inGame=0;
    nReceivedDups=0;
    lastPingNumber=0;
    lastPingTime=timestampNow();
    port=0;
    IP=QString();
    receivedDatas = new QByteArray;
    pony = Pony();
    for (int i=0;i<33;i++)
        udpSequenceNumbers[i]=0;
    for (int i=0;i<33;i++)
        udpRecvSequenceNumbers[i]=0;
    udpRecvMissing.clear();

    // Prepare timers
    udpSendReliableTimer =  new QTimer;
    udpSendReliableTimer->setInterval(UDP_RESEND_TIMEOUT);
    udpSendReliableTimer->setSingleShot(true);
    QObject::connect(udpSendReliableTimer, SIGNAL(timeout()), this, SLOT(udpResendLast()));
    udpSendReliableGroupTimer =  new QTimer;
    udpSendReliableGroupTimer->setInterval(UDP_GROUPING_TIMEOUT);
    udpSendReliableGroupTimer->setSingleShot(true);
    connect(udpSendReliableGroupTimer, SIGNAL(timeout()), this, SLOT(udpDelayedSend()));
}

void Player::reset()
{
    name.clear();
    connected=false;
    inGame=0;
    nReceivedDups=0;
    lastPingNumber=0;
    lastPingTime=timestampNow();
    port=0;
    IP.clear();
    receivedDatas->clear();
    lastValidReceivedAnimation.clear();
    pony = Pony();
    for (int i=0;i<33;i++)
        udpSequenceNumbers[i]=0;
    for (int i=0;i<33;i++)
        udpRecvSequenceNumbers[i]=0;
    udpRecvMissing.clear();
}

void Player::resetNetwork()
{
    connected=false;
    nReceivedDups=0;
    lastPingNumber=0;
    lastPingTime=timestampNow();
    port=0;
    IP.clear();
    receivedDatas->clear();
    for (int i=0;i<33;i++)
        udpSequenceNumbers[i]=0;
    for (int i=0;i<33;i++)
        udpRecvSequenceNumbers[i]=0;
    udpRecvMissing.clear();
}

bool Player::savePlayers(QList<Player*>& playersData)
{
    QFile playersFile("data/players/players.dat");
    if (!playersFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        win.logStatusMessage("Error saving players database");
        win.stopServer();
        return false;
    }

    for (int i=0;i<playersData.size();i++)
    {
        playersFile.write(playersData[i]->name.toLatin1());
        playersFile.write("\31");
        playersFile.write(playersData[i]->passhash.toLatin1());
        if (i+1!=playersData.size())
            playersFile.write("\n");
    }
    return true;
}

QList<Player*>& Player::loadPlayers()
{
    QList<Player*>& players = *(new QList<Player*>);
    QFile playersFile("data/players/players.dat");
    if (!playersFile.open(QIODevice::ReadOnly))
    {
        win.logStatusMessage("Error reading players database");
        win.stopServer();
        return players;
    }
    QList<QByteArray> data = playersFile.readAll().split('\n');
    if (data.size()==1 && data[0].isEmpty())
    {
        win.logMessage("Player database is empty. Continuing happily");
        return players;
    }
    for (int i=0;i<data.size();i++)
    {
        QList<QByteArray> line = data[i].split('\31');
        if (line.size()!=2)
        {
            win.logStatusMessage("Error reading players database");
            win.stopServer();
            return players;
        }
        Player* newPlayer = new Player;
        newPlayer->name = line[0];
        newPlayer->passhash = line[1];
        players << newPlayer;
    }
    win.logMessage(QString("Got ")+QString().setNum(players.size())+" players in database");
    return players;
}

Player* Player::findPlayer(QList<Player*>& players, QString uname)
{
    for (int i=0; i<players.size(); i++)
    {
        if (players[i]->name == uname)
            return players[i];
    }

    Player* emptyPlayer = new Player();
    return emptyPlayer;
}

Player* Player::findPlayer(QList<Player*>& players, QString uIP, quint16 uport)
{
    for (int i=0; i<players.size(); i++)
    {
        if (players[i]->IP == uIP && players[i]->port == uport)
            return players[i];
    }

    Player* emptyPlayer = new Player();
    return emptyPlayer;
}

Player* Player::findPlayer(QList<Player*>& players, quint16 netviewId)
{
    for (int i=0; i<players.size(); i++)
    {
        if (players[i]->pony.netviewId == netviewId)
            return players[i];
    }

    Player* emptyPlayer = new Player();
    return emptyPlayer;
}

void Player::savePonies(Player *player, QList<Pony> ponies)
{
    win.logMessage("UDP: Saving ponies for "+QString().setNum(player->pony.netviewId)+" ("+player->name+")");

    QDir playerPath(QDir::currentPath());
    playerPath.cd("data");
    playerPath.cd("players");
    playerPath.mkdir(player->name.toLatin1());

    QFile file(QDir::currentPath()+"/data/players/"+player->name.toLatin1()+"/ponies.dat");
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    for (int i=0; i<ponies.size(); i++)
    {
        file.write(ponies[i].ponyData);
        file.write(vectorToData(ponies[i].pos));
        file.write(stringToData(ponies[i].sceneName));
    }
}

QList<Pony> Player::loadPonies(Player* player)
{
    QList<Pony> ponies;
    QFile file(QDir::currentPath()+"/data/players/"+player->name.toLatin1()+"/ponies.dat");
    if (!file.open(QIODevice::ReadOnly))
        return ponies;

    QByteArray data = file.readAll();

    int i=0;
    while (i<data.size())
    {
        Pony pony;
        // Read the ponyData
        unsigned strlen;
        unsigned lensize=0;
        {
            unsigned char num3; int num=0, num2=0;
            do {
                num3 = data[i+lensize]; lensize++;
                num |= (num3 & 0x7f) << num2;
                num2 += 7;
            } while ((num3 & 0x80) != 0);
            strlen = (uint) num;
        }
        int ponyDataSize = strlen+lensize+43;
        pony.ponyData = data.mid(i,ponyDataSize);
        pony.name = dataToString(pony.ponyData); // The name is the first elem
        //win.logMessage("Found pony : "+pony.name);
        i+=ponyDataSize;

        // Read pos
        UVector pos = dataToVector(data.mid(i,12));
        pony.pos = pos;
        i+=12;

        // Read sceneName
        unsigned strlen2;
        unsigned lensize2=0;
        {
            unsigned char num3; int num=0, num2=0;
            do {
                num3 = data[i+lensize2]; lensize2++;
                num |= (num3 & 0x7f) << num2;
                num2 += 7;
            } while ((num3 & 0x80) != 0);
            strlen2 = (uint) num;
        }
        pony.sceneName = data.mid(i+lensize2, strlen2);
        i+=strlen2+lensize2;

        // Create quests
        for (int i=0; i<win.quests.size(); i++)
        {
            Quest quest = win.quests[i];
            quest.setOwner(player);
            pony.quests << quest;
        }

        ponies << pony;
    }

    return ponies;
}

void Player::removePlayer(QList<Player*>& players, QString uIP, quint16 uport)
{
    for (int i=0; i<players.size(); i++)
    {
        if (players[i]->IP == uIP && players[i]->port == uport)
            players.removeAt(i);
    }
}

void Player::disconnectPlayerCleanup(Player* player)
{
    static QMutex playerCleanupMutex;

    // Save the pony
    QList<Pony> ponies = loadPonies(player);
    for (int i=0; i<ponies.size(); i++)
        if (ponies[i].ponyData == player->pony.ponyData)
            ponies[i] = player->pony;
    savePonies(player, ponies);

    QString uIP = player->IP;
    quint16 uPort = player->port;

    Scene* scene = findScene(player->pony.sceneName);
    if (scene->name.isEmpty())
        win.logMessage("UDP: Can't find scene for player cleanup");

    //win.logMessage("playerCleanup locking");
    playerCleanupMutex.lock();
    removePlayer(scene->players, uIP, uPort);
    for (int i=0; i<scene->players.size(); i++)
        sendNetviewRemove(scene->players[i], player->pony.netviewId);
    player->udpDelayedSend(); // We're about to remove the player, we can't delay the send
    player->udpSendReliableTimer->stop();
    player->udpSendReliableGroupTimer->stop();
    removePlayer(win.udpPlayers, uIP, uPort);
    delete player;
    //win.logMessage("playerCleanup unlocking");
    playerCleanupMutex.unlock();
}

void Player::udpResendLast()
{
    //win.logMessage("udpResendLast locking");
    if (!udpSendReliableMutex.tryLock())
    {
        win.logMessage("udpResendLast failed to lock.");
        return; // Avoid deadlock if sendMessage just locked but didn't have the time to stop the timers
    }
    //udpSendReliableMutex.lock();
    QByteArray msg = udpSendReliableQueue.first();
    //win.logMessage("Resending message : "+QString(msg.toHex().data()));

    if (win.udpSocket->writeDatagram(msg,QHostAddress(IP),port) != msg.size())
    {
        win.logMessage("UDP: Error sending last message");
        win.logStatusMessage("Restarting UDP server ...");
        win.udpSocket->close();
        if (!win.udpSocket->bind(win.gamePort, QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress))
        {
            win.logStatusMessage("UDP: Unable to start server on port "+QString().setNum(win.gamePort));
            win.stopServer();
            return;
        }
    }

    udpSendReliableTimer->start();

    //win.logMessage("udpResendLast unlocking");
    udpSendReliableMutex.unlock();
}

void Player::udpDelayedSend()
{
    //win.logMessage("udpDelayedSend locking");
    if (!udpSendReliableMutex.tryLock())
    {
        win.logMessage("udpDelayedSend failed to lock.");
        return; // Avoid deadlock if sendMessage just locked but didn't have the time to stop the timers
    }
    //udpSendReliableMutex.lock();
    //win.logMessage("Sending delayed grouped message : "+QString(udpSendReliableGroupBuffer.toHex()));

    // Move the grouped message to the reliable queue
    udpSendReliableQueue.append(udpSendReliableGroupBuffer);

    // If this is the only message queued, send it now
    // If it isn't, we need to wait until the previous one was ACK'd
    if (udpSendReliableQueue.size() >= 1)
    {
        if (win.udpSocket->writeDatagram(udpSendReliableGroupBuffer,QHostAddress(IP),port) != udpSendReliableGroupBuffer.size())
        {
            win.logMessage("UDP: Error sending last message");
            win.logStatusMessage("Restarting UDP server ...");
            win.udpSocket->close();
            if (!win.udpSocket->bind(win.gamePort, QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress))
            {
                win.logStatusMessage("UDP: Unable to start server on port "+QString().setNum(win.gamePort));
                win.stopServer();
                return;
            }
        }
    }

    udpSendReliableGroupBuffer.clear();

    if (!udpSendReliableTimer->isActive())
        udpSendReliableTimer->start();

    //win.logMessage("udpDelayedSend unlocking");
    udpSendReliableMutex.unlock();
}
