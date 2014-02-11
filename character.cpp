#include <QFile>
#include <QDir>
#include "widget.h"
#include "character.h"
#include "message.h"
#include "utils.h"

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

// The single ugliest function I've ever written. OCCC is going to be jealous.
QByteArray Pony::ponyCodeToPonyData(QByteArray ponyCode)
{
    // The ponycode has 247 bits before the string, the remaining size is the string
    // You don't have the string length, but each char is 6 bit
    // Are we even sure that & has bigger priority than << and >> ?
    QByteArray data;
    data+= ponyCode[0] &0b11000000; // Race
    data+= ponyCode[22]&0b00000110; // Gender
    // CMs 3*4, start at 151=18*8+7, 10 bits each
    data+= (char)0;
    data+= (char)0;
    data+= (ponyCode[18]&0b00000001<<1) + (ponyCode[19]&0b10000000>>7);
    data+= (ponyCode[19]&0b01111111<<1) + (ponyCode[20]&0b10000000>>7);
    data+= (char)0;
    data+= (char)0;
    data+= (ponyCode[20]&0b01100000>>5);
    data+= (ponyCode[20]&0b00011111<<3) + (ponyCode[21]&0b11100000>>5);
    data+= (char)0;
    data+= (char)0;
    data+= (ponyCode[21]&0b00011000>>3);
    data+= (ponyCode[21]&0b00000111<<5) + (ponyCode[22]&0b11111000>>3);
    // hair0 3
    data+= (ponyCode[0] &0b00111111<<2) + (ponyCode[1] &0b11000000>>6);
    data+= (ponyCode[1] &0b00111111<<2) + (ponyCode[2] &0b11000000>>6);
    data+= (ponyCode[2] &0b00111111<<2) + (ponyCode[3] &0b11000000>>6);
    // hair1 3
    data+= (ponyCode[3] &0b00111111<<2) + (ponyCode[4] &0b11000000>>6);
    data+= (ponyCode[4] &0b00111111<<2) + (ponyCode[5] &0b11000000>>6);
    data+= (ponyCode[5] &0b00111111<<2) + (ponyCode[6] &0b11000000>>6);
    // body 3
    data+= (ponyCode[6] &0b00111111<<2) + (ponyCode[7] &0b11000000>>6);
    data+= (ponyCode[7] &0b00111111<<2) + (ponyCode[8] &0b11000000>>6);
    data+= (ponyCode[8] &0b00111111<<2) + (ponyCode[9] &0b11000000>>6);
    // eye 3
    data+= (ponyCode[9] &0b00111111<<2) + (ponyCode[10]&0b11000000>>6);
    data+= (ponyCode[10]&0b00111111<<2) + (ponyCode[11]&0b11000000>>6);
    data+= (ponyCode[11]&0b00111111<<2) + (ponyCode[12]&0b11000000>>6);
    // hoof 3
    data+= (ponyCode[12]&0b00111111<<2) + (ponyCode[13]&0b11000000>>6);
    data+= (ponyCode[13]&0b00111111<<2) + (ponyCode[14]&0b11000000>>6);
    data+= (ponyCode[14]&0b00111111<<2) + (ponyCode[15]&0b11000000>>6);
    // mane 2
    data+= (char)0;
    data+= (ponyCode[15]&0b00111111<<2) + (ponyCode[16]&0b11000000>>6);
    // tail 2
    data+= (char)0;
    data+= (ponyCode[16]&0b00111111<<2) + (ponyCode[17]&0b11000000>>6);
    // eye 2
    data+= (char)0;
    data+= (ponyCode[17]&0b00111111<<2) + (ponyCode[18]&0b11000000>>6);
    // hoof 2
    data+= (char)0;
    data+= (ponyCode[18]&0b00111110>>1);
    // bodySize float (4 bytes)
    data+= (ponyCode[22]&0b00000001)    + (ponyCode[23]&0b11111110);
    data+= (ponyCode[23]&0b00000001)    + (ponyCode[24]&0b11111110);
    data+= (ponyCode[24]&0b00000001)    + (ponyCode[25]&0b11111110);
    data+= (ponyCode[25]&0b00000001)    + (ponyCode[26]&0b11111110);
    // hordSize 2 (ranged single) (4 bytes)
    data+= (ponyCode[26]&0b00000001)    + (ponyCode[27]&0b11111110);
    data+= (ponyCode[27]&0b00000001)    + (ponyCode[28]&0b11111110);
    data+= (ponyCode[28]&0b00000001)    + (ponyCode[29]&0b11111110);
    data+= (ponyCode[29]&0b00000001)    + (ponyCode[30]&0b11111110);
    // Don't forget to add the name at the start of the ponyData
    unsigned remainingBits = ponyCode.size()*8 - 247;
    unsigned off1=7, off2;
    QByteArray name;
    for (unsigned i=0; i<=remainingBits-6; i+=6) // Read the string at the end
    {
        char byte=0;
        for(off2=2; off2 < 8; off2++) // Read 6 bits, cast into byte
        {
            byte |= (ponyCode[30+off1/8]&(1<<(7-(off1%8))))>>(7-(off1%8))<<(7-off2); // I'm so sorry
            off1++;
        }
        byte = convertChar(byte, false);
        name += byte;
    }
    win.logMessage("Name from ponyCode : "+QString(name));
    return stringToData(name) + data;
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
    inv.clear();
    worn.clear();
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
