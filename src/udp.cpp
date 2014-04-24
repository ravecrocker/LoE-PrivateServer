#include "widget.h"
#include "message.h"
#include "serialize.h"
#include "utils.h"

void Widget::udpProcessPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QHostAddress rAddr;
        quint16 rPort;
        QByteArray datagram;
        qint64 dataRead = 0;
        int datagramSize = udpSocket->pendingDatagramSize();
        datagram.resize(datagramSize);

        while(dataRead < datagram.size())
        {
            qint64 readNow = udpSocket->readDatagram(datagram.data()+dataRead, datagramSize, &rAddr, &rPort); // le +dataRead sur un tableau, ça décale le pointeur d'un offset de taille dataRead !
            if(readNow != -1)
            {
                dataRead += readNow; // Remember the total number of bytes read, so we know when to stop
                if (datagramSize > (datagram.size() - dataRead)) // Make sure we don't overflow
                    datagramSize = (datagram.size() - dataRead);
            }
            else
            {
                logStatusMessage(tr("Socket error : %1").arg(udpSocket->errorString()));
                return;
            }
        }

        // Add player on connection
        if ((unsigned char)datagram[0]==MsgConnect && (unsigned char)datagram[1]==0
                && (unsigned char)datagram[2]==0 && datagram.size()>=22)
        {
            QString name = dataToString(datagram.right(datagram.size()-22));
            int nameFullSize = getVUint32Size(datagram.right(datagram.size()-22))+name.size();
            QString sesskey = dataToString(datagram.right(datagram.size()-22-nameFullSize));
            //logMessage(QString("UDP: Connect detected with name : ")+name);
            //logMessage(QString("UDP: Connect detected with sesskey : ")+sesskey);
            //logMessage(QString("UDP: Datagram was : ")+datagram.toHex());

            bool is_sesskey_valid = true;

            if (enableSessKeyValidation) {
                is_sesskey_valid = QCryptographicHash::hash(QString(sesskey.right(40) + saltPassword).toLatin1(),
                                                            QCryptographicHash::Md5).toHex() == sesskey.left(32);
            }

            if (is_sesskey_valid)
            {
                //logMessage("Sesskey token accepted");

                // Create new player if needed, else just update player
                Player* newPlayer = Player::findPlayer(udpPlayers, rAddr.toString(),rPort);
                if (newPlayer->IP != rAddr.toString()) // IP:Port not found in player list
                {
                    newPlayer->resetNetwork();
                    //newPlayer->connected = true; // Not really connected until we finish the handshake
                    newPlayer->name = name;
                    newPlayer->IP = rAddr.toString();
                    newPlayer->port = rPort;

                    // Check if we have too many players connected
                    int n=0;
                    for (int i=0;i<udpPlayers.size();i++)
                        if (udpPlayers[i]->connected)
                            n++;
                    if (n>=maxConnected)
                    {
                        sendMessage(newPlayer, MsgDisconnect, "Error : Too many players connected. Try again later.");
                    }
                    else
                        // If not add the player
                        udpPlayers << newPlayer;
                }
                else  // IP:Port found in player list
                {
                    if (newPlayer->connected) // TODO: Error, player already connected
                    {
                        sendMessage(newPlayer, MsgDisconnect, "Error : Player already connected.");
                        return;
                    }

                    // Check if we have too many players connected
                    int n=0;
                    for (int i=0;i<udpPlayers.size();i++)
                        if (udpPlayers[i]->connected)
                            n++;
                    if (n>=maxConnected)
                    {
                        sendMessage(newPlayer, MsgDisconnect, "Error : Too many players connected. Try again later.");
                    }

                    newPlayer->resetNetwork();
                    newPlayer->name = name;
                    newPlayer->IP = rAddr.toString();
                    newPlayer->port = rPort;
                    //newPlayer->connected = true; // We're not really connected until we finish the handshake
                }
            }
            else
            {
                logMessage(tr("UDP: Sesskey rejected","The sesskey is a cryptographic hash, short for session key"));
                Player* newPlayer = new Player;
                newPlayer->IP = rAddr.toString();
                newPlayer->port = rPort;
                sendMessage(newPlayer, MsgDisconnect, "Error: Wrong server password. This server is protected with a salt password.");
                return;
            }
        }

        Player* player = Player::findPlayer(udpPlayers, rAddr.toString(), rPort);
        if (player->IP == rAddr.toString() && player->port == rPort)
        {
            // Acquire datas
            player->receivedDatas->append(datagram);

            // Process data
            receiveMessage(player);
        }
        else // You need to connect with TCP first
        {
            logMessage(tr("UDP: Request from unknown peer rejected : %1:%2").arg(rAddr.toString()).arg(rPort));
            // Send disconnect message manually
            QString data("You're not connected, please login first. (aka the server has no idea who the hell you are)");
            QByteArray msg(6,0);
            msg[0] = MsgDisconnect;
            msg[3] = (quint8)(((data.size()+1)*8)&0xFF);
            msg[4] = (quint8)((((data.size()+1)*8)>>8)&0xFF);
            msg[5] = (quint8)data.size();
            msg += data;
            win.udpSocket->writeDatagram(msg,rAddr,rPort);
        }
    }
}
