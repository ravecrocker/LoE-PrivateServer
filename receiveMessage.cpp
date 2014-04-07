#include "message.h"
#include "character.h"
#include "widget.h"
#include "sync.h"
#include "utils.h"
#include "serialize.h"

void receiveMessage(Player* player)
{
    QByteArray msg = *(player->receivedDatas);
    int msgSize=5 + (((unsigned char)msg[3]) + (((unsigned char)msg[4]) << 8))/8;

    // Check the sequence (seq) of the received messag
    if ((unsigned char)msg[0] >= MsgUserReliableOrdered1 && (unsigned char)msg[0] <= MsgUserReliableOrdered32)
    {
        quint16 seq = (quint8)msg[1] + ((quint8)msg[2]<<8);
        quint8 channel = ((unsigned char)msg[0])-MsgUserReliableOrdered1;
        if (seq <= player->udpRecvSequenceNumbers[channel] && player->udpRecvSequenceNumbers[channel]!=0)
        {
            // If this is a missing packet, accept it
            MessageHead missingMsg;
            missingMsg.channel = channel;
            missingMsg.seq = seq;
            if (player->udpRecvMissing.contains(missingMsg))
            {
                win.logMessage("UDP: Processing retransmission (-"+QString().setNum(player->udpRecvSequenceNumbers[channel]-seq)
                               +") from "+QString().setNum(player->pony.netviewId));
                for (int i=0; i<player->udpRecvMissing.size(); i++)
                    if (player->udpRecvMissing[i] == missingMsg)
                        player->udpRecvMissing.remove(i);
            }
            else
            {
                // We already processed this packet, we should discard it
                win.logMessage("UDP: Discarding double message (-"+QString().setNum(player->udpRecvSequenceNumbers[channel]-seq)
                               +") from "+QString().setNum(player->pony.netviewId));
                win.logMessage("UDP: Message was : "+QString(player->receivedDatas->left(msgSize).toHex().data()));
                player->nReceivedDups++;
                if (player->nReceivedDups >= 100) // Kick the player if he's infinite-looping on us
                {
                    win.logMessage(QString("UDP: Kicking "+QString().setNum(player->pony.netviewId)+" : Too many packet dups."));
                    sendMessage(player,MsgDisconnect, "You were kicked for lagging the server, sorry. You can login again.");
                    Player::disconnectPlayerCleanup(player); // Save game and remove the player
                    return;
                }
                *(player->receivedDatas) = player->receivedDatas->mid(msgSize);

                // Ack if needed, so that the client knows to move on already.
                if ((unsigned char)msg[0] >= MsgUserReliableOrdered1 && (unsigned char)msg[0] <= MsgUserReliableOrdered32) // UserReliableOrdered
                {
                    //win.logMessage("UDP: ACKing discarded message");
                    QByteArray data(3,0);
                    data[0] = (quint8)(msg[0]); // ack type
                    data[1] = (quint8)(((quint8)msg[1])/2); // seq
                    data[2] = (quint8)(((quint8)msg[2])/2); // seq
                    sendMessage(player, MsgAcknowledge, data);
                }
                if (player->receivedDatas->size())
                    receiveMessage(player);
                return; // When we're done with the recursion, we still need to skip this message.
            }
        }
        else if (seq > player->udpRecvSequenceNumbers[channel]+2) // If a message was skipped, keep going.
        {
            win.logMessage("UDP: Unordered message (+"+QString().setNum(seq-player->udpRecvSequenceNumbers[channel])
                           +") received from "+QString().setNum(player->pony.netviewId));
            player->udpRecvSequenceNumbers[channel] = seq;

            // Mark the packets we skipped as missing
            for (int i=player->udpRecvSequenceNumbers[channel]+2; i<seq; i+=2)
            {
                MessageHead missing;
                missing.channel = channel;
                missing.seq = i;
                player->udpRecvMissing.append(missing);
            }
        }
        else
        {
            if (player->nReceivedDups > 0) // If he stopped sending dups, forgive him slowly.
                player->nReceivedDups--;
            //win.logMessage("UDP: Received message (="+QString().setNum(seq)
            //               +") from "+QString().setNum(player->pony.netviewId));
            player->udpRecvSequenceNumbers[channel] = seq;
        }
    }

    // Process the received message
    if ((unsigned char)msg[0] == MsgPing) // Ping
    {
        //win.logMessage("UDP: Ping received from "+player->IP+":"+QString().setNum(player->port)
        //        +" ("+QString().setNum((timestampNow() - player->lastPingTime))+"s)");
        player->lastPingNumber = (quint8)msg[5];
        player->lastPingTime = timestampNow();
        sendMessage(player,MsgPong);
    }
    else if ((unsigned char)msg[0] == MsgPong) // Pong
    {
        win.logMessage("UDP: Pong received");
    }
    else if ((unsigned char)msg[0] == MsgConnect) // Connect SYN
    {
        msg.resize(18); // Supprime le message LocalHail et le Timestamp
        msg = msg.right(13); // Supprime le Header

        win.logMessage(QString("UDP: Connecting ..."));

        for (int i=0; i<32; i++) // Reset sequence counters
            player->udpSequenceNumbers[i]=0;

        sendMessage(player, MsgConnectResponse, msg);
    }
    else if ((unsigned char)msg[0] == MsgConnectionEstablished) // Connect ACK
    {
        win.logMessage("UDP: Connected to client");
        player->connected=true;
        for (int i=0; i<32; i++) // Reset sequence counters
            player->udpSequenceNumbers[i]=0;

        // Start game
        win.logMessage(QString("UDP: Starting game"));

        // Set local player id
        win.lastIdMutex.lock();
        player->pony.id = win.getNewId();
        player->pony.netviewId = win.getNewNetviewId();
        win.lastIdMutex.unlock();

        win.logMessage("UDP: Set id request : " + QString().setNum(player->pony.id) + "/" + QString().setNum(player->pony.netviewId));

        // Set player Id request
        QByteArray id(3,0);
        id[0]=4;
        id[1]=(quint8)(player->pony.id&0xFF);
        id[2]=(quint8)((player->pony.id>>8)&0xFF);
        sendMessage(player,MsgUserReliableOrdered6,id); // Sends a 48

        // Load Characters screen requets
        QByteArray data(1,5);
        data += stringToData("Characters");
        sendMessage(player,MsgUserReliableOrdered6,data); // Sends a 48
    }
    else if ((unsigned char)msg[0] == MsgAcknowledge) // Acknowledge
    {
        //win.logMessage("ACK message : "+QString(msg.toHex()));
        // Number of messages ACK'd by this message
        int nAcks = ((quint8)msg[3] + (((quint16)(quint8)msg[4])<<8)) / 24;
        if (nAcks)
        {
            // Extract the heads (channel and seq) of the ack'd messages
            QVector<MessageHead> acks;
            for (int i=0; i<nAcks; i++)
            {
                MessageHead head;
                head.channel = (quint8)msg[3*i+5];
                head.seq = ((quint16)(quint8)msg[3*i+6] + (((quint16)(quint8)msg[3*i+7])<<8))*2;
                // If that's not a supported reliable message, there's no point in checking
                if (head.channel >= MsgUserReliableOrdered1 && head.channel <= MsgUserReliableOrdered32)
                    acks << head;
            }

            // Print list of ACK'd messages
            //if (acks.size())
            //{
                //QString ackMsg = "UDP: Messages acknoledged (";
                //for (int i=0; i<acks.size(); i++)
                //{
                //    if (i)
                //        ackMsg += "/";
                //    ackMsg+=QString(QByteArray().append(acks[i].channel).toHex())+":"+QString().setNum(acks[i].seq);
                //}
                //ackMsg += ")";
                //win.logMessage(ackMsg);
            //}

            if (player->udpSendReliableQueue.size() && acks.size()) // If there's nothing to check, do nothing
            {
                //win.logMessage("receiveMessage ACK locking");
                player->udpSendReliableMutex.lock();
                player->udpSendReliableTimer->stop();
                // Remove the ACK'd messages from the reliable send queue
                QByteArray qMsg = player->udpSendReliableQueue[0];
                //win.logMessage("About to remove ACKs from : "+QString(qMsg.toHex()));
                for (int i=0; i<acks.size(); i++)
                {
                    int pos=0;
                    while (pos < qMsg.size()) // Try to find a msg matching this ACK
                    {
                        quint16 seq = ((quint16)(quint8)qMsg[pos+1]) + (((quint16)(quint8)qMsg[pos+2])<<8);
                        quint16 qMsgSize = (((quint16)(quint8)qMsg[pos+3])+(((quint16)(quint8)qMsg[pos+4])<<8))/8+5;
                        if ((quint16)(quint8)qMsg[pos] == acks[i].channel && seq == acks[i].seq) // Remove the msg, now that it was ACK'd
                        {
                            //win.logMessage("Removed message : "+QString().setNum(seq)+" size is "+QString().setNum(qMsgSize));
                            qMsg = qMsg.left(pos) + qMsg.mid(pos+qMsgSize);
                            //win.logMessage("New message is "+QString(qMsg.toHex()));
                        }
                        else
                            pos += qMsgSize;
                    }
                }

                // Now update the reliable message queue
                if (qMsg.isEmpty())
                {
                    player->udpSendReliableQueue.remove(0); // The whole grouped msg was ACK'd, remove it
                    if (player->udpSendReliableQueue.size()) // If there's a next message in the queue, send it
                    {
                        win.logMessage("Sending next message in queue");
                        qMsg = player->udpSendReliableQueue[0];
                        if (win.udpSocket->writeDatagram(qMsg,QHostAddress(player->IP),player->port) != qMsg.size())
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
                        player->udpSendReliableTimer->start();
                        //win.logMessage("receiveMessage ACK unlocking");
                        player->udpSendReliableMutex.unlock();
                    }
                    else
                    {
                        //win.logMessage("receiveMessage ACK unlocking");
                        player->udpSendReliableMutex.unlock();
                    }
                }
                else
                {
                    player->udpSendReliableQueue[0] = qMsg;
                    player->udpSendReliableTimer->start(); // We're still waiting for some msgs to get ACK'd
                    //win.logMessage("receiveMessage ACK unlocking");
                    player->udpSendReliableMutex.unlock();
                }

            }
        }
    }
    else if ((unsigned char)msg[0] == MsgDisconnect) // Disconnect
    {
        win.logMessage("UDP: Client disconnected");
        Player::disconnectPlayerCleanup(player); // Save game and remove the player

        return; // We can't use Player& player anymore, it referes to free'd memory.
    }
    else if ((unsigned char)msg[0] >= MsgUserReliableOrdered1 && (unsigned char)msg[0] <= MsgUserReliableOrdered32) // UserReliableOrdered
    {
        //win.logMessage("UDP: Data received (hex) : ");
        //win.logMessage(player->receivedDatas->toHex().constData());

        QByteArray data(3,0);
        data[0] = (quint8)msg[0]; // ack type
        data[1] = (quint8)(((quint8)msg[1])/2); // seq
        data[2] = (quint8)(((quint8)msg[2])/2); // seq
        sendMessage(player, MsgAcknowledge, data);

        if ((unsigned char)msg[0]==MsgUserReliableOrdered6 && (unsigned char)msg[3]==8 && (unsigned char)msg[4]==0 && (unsigned char)msg[5]==6 ) // Prefab (player/mobs) list instantiate request
        {
            sendEntitiesList(player); // Called when the level was loaded on the client side
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered6 && (unsigned char)msg[3]==0x18 && (unsigned char)msg[4]==0 && (unsigned char)msg[5]==8 ) // Player game info (inv/ponyData/...) request
        {
            sendPonySave(player, msg); // Called when instantiate finished on the client side
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered4 && (unsigned char)msg[5]==0xf) // Chat
        {
            QString txt = dataToString(msg.mid(7));
            QString author = player->pony.name;
            //win.logMessage("Chat "+author+":"+txt);

            if (txt.startsWith("/stuck") || txt.startsWith("unstuck me"))
            {
                sendLoadSceneRPC(player, player->pony.sceneName);
            }
            else if (txt == ":commands")
            {
                sendChatMessage(player, "<span color=\"yellow\">List of Commands:</span><br /><em>:roll</em><br /><span color=\"yellow\">Rolls a random number between 00 and 99</span><br /><em>:msg player message</em><br /><span color=\"yellow\">Sends a private message to a player</span><br /><em>:names</em><br /><span color=\"yellow\">Lists all players on the server</span><br /><em>:me action</em><br /><span color=\"yellow\">States your current action</span><br /><em>:tp location</em><br /><span color=\"yellow\">Teleports your pony to the specified region</span>", "[Server]", ChatLocal);
            }
            else if (txt.startsWith(":msg"))
            {
                if(txt.count(" ") < 2)
                    sendChatMessage(player, ":msg<br /><span color=\"yellow\">Usage:</span><br /><em>:msg player message</em><br /><span color=\"yellow\">Player names are case-insensitive, ignore spaces and you do not need to type out their full name.</span>", author, ChatLocal);
                else
                {
                    for (int i=0; i<win.udpPlayers.size(); i++)
                        if (win.udpPlayers[i]->inGame>=2 && win.udpPlayers[i]->pony.name.toLower().remove(" ").startsWith(txt.toLower().section(" ", 1, 1)))
                        {
                            txt = txt.remove(0, txt.indexOf(" ", 5) + 1);
                            sendChatMessage(win.udpPlayers[i], "<span color=\"yellow\">[PM] </span>" + txt, author, ChatLocal);
                            sendChatMessage(player, "<span color=\"yellow\">[PM to " + win.udpPlayers[i]->pony.name + "] </span>" + txt, author, ChatLocal);
                        }
                }

            }
            else if (txt.startsWith(":names"))
            {
                QString namesmsg = "<span color=\"yellow\">Players currently in game:</span>";

                for (int i=0; i<win.udpPlayers.size(); i++)
                    if (win.udpPlayers[i]->inGame>=2)
                        namesmsg += "<br />#b" + win.udpPlayers[i]->pony.name + "#b<br /><span color=\"yellow\"> - in " + win.udpPlayers[i]->pony.sceneName + "</span>";

                sendChatMessage(player, namesmsg, "[Server]", ChatLocal);
            }
            else if (txt.startsWith(":tp"))
            {
                if (txt.count(" ") < 1)
                {
                  QString msgtosend = ":tp<br /><span color=\"yellow\">Usage:</span><br /><em>:tp location</em><br /><span color=\"yellow\">Available locations:</span><em>";

                    for (int i=0; i<win.scenes.size(); i++)
                        msgtosend += "<br />" + win.scenes[i].name;

                    sendChatMessage(player, msgtosend + "</em>", author, ChatLocal);
                }

                else
                    sendLoadSceneRPC(player, txt.remove(0, 4));
            }
            else if (txt == ":me")
            {
                sendChatMessage(player, ":me<br /><span color=\"yellow\">Usage:</span><br /><em>:me action</em>", author, ChatLocal);
            }
            else // Broadcast the message
            {
                int rollnum = -1;
                QString rollstr;
                bool actmsg = false;
                
                if (txt == ":roll")
                {
                    if (player->chatRollCooldownEnd < QDateTime::currentDateTime())
                    {
                        rollnum = qrand() % 100;
                        rollstr.sprintf("<span color=\"yellow\">#b%s#b rolls %02d</span>", author.toLocal8Bit().data(), rollnum);
                        player->chatRollCooldownEnd = QDateTime::currentDateTime().addSecs(10);
                    }
                }
                if (txt.startsWith(":me "))
                {
                    actmsg = true;
                    txt.remove(0, 3);
                    txt = "<em>#b* " + author + "#b" + txt + "</em>";
                }
                if ((quint8)msg[6] == 8) // Local chat only
                {
                    Scene* scene = findScene(player->pony.sceneName);
                    if (scene->name.isEmpty())
                        win.logMessage("UDP: Can't find the scene for chat message, aborting");
                    else
                        for (int i=0; i<scene->players.size(); i++)
                            if (scene->players[i]->inGame>=2)
                            {
                                if (rollnum > -1)
                                    sendChatMessage(scene->players[i], rollstr, "[Server]", ChatLocal);
                                else if (actmsg)
                                    sendChatMessage(scene->players[i], txt, "", ChatLocal);
                                else if (txt.startsWith(">"))
                                    sendChatMessage(scene->players[i], "<span color=\"green\">" + txt + "</span>", author, ChatLocal);
                                else
                                    sendChatMessage(scene->players[i], txt, author, ChatLocal);
                            }
                }
                else // Send globally
                    for (int i=0; i<win.udpPlayers.size(); i++)
                        if (win.udpPlayers[i]->inGame>=2)
                        {
                            if (rollnum > -1)
                                sendChatMessage(win.udpPlayers[i], rollstr, "[Server]", ChatGeneral);
                            else if (actmsg)
                                sendChatMessage(win.udpPlayers[i], txt, "", ChatGeneral);
                            else if (txt.startsWith(">"))
                                sendChatMessage(win.udpPlayers[i], "<span color=\"green\">" + txt + "</span>", author, ChatGeneral);
                            else
                                sendChatMessage(win.udpPlayers[i], txt, author, ChatGeneral);

                        }

            }
        }
        else if ((quint8)msg[0]==MsgUserReliableOrdered4 && (quint8)msg[5]==0x1 && player->inGame!=0) // Edit ponies request error (happens if you click play twice quicly, for example)
        {
            win.logMessage("UDP: Rejecting game start request from "+QString().setNum(player->pony.netviewId)
                           +" : player already in game");
            // Fix the buggy state we're now in
            // Reload to hide the "saving ponies" message box
            QByteArray data(1,5);
            data += stringToData(player->pony.sceneName);
            sendMessage(player,MsgUserReliableOrdered6,data);
            // Try to cancel the loading callbacks with inGame=1
            player->inGame = 1;
        }
        else if ((quint8)msg[0]==MsgUserReliableOrdered4 && (quint8)msg[5]==0x1 && player->inGame==0) // Edit ponies request
        {
            QList<Pony> ponies = Player::loadPonies(player);
            QByteArray ponyData = msg.right(msg.size()-10);
            Pony pony;
            if ((unsigned char)msg[6]==0xff && (unsigned char)msg[7]==0xff && (unsigned char)msg[8]==0xff && (unsigned char)msg[9]==0xff)
            {
                // Create the new pony for this player
                pony.ponyData = ponyData;
                pony.sceneName = "PonyVille";
                pony.pos = findVortex(pony.sceneName, 0).destPos;
                ponies += pony;
            }
            else
            {
                quint32 id = (quint8)msg[6] +((quint8)msg[7]<<8) + ((quint8)msg[8]<<16) + ((quint8)msg[9]<<24);
                if (ponies.size()<0 || (quint32)ponies.size() <= id)
                {
                    win.logMessage("UDP: Received invalid id in 'edit ponies' request. Disconnecting user.");
                    sendMessage(player,MsgDisconnect, "You were kicked for sending invalid data.");
                    Player::disconnectPlayerCleanup(player); // Save game and remove the player
                    return; // It's ok, since we just disconnected the player
                }
                ponies[id].ponyData = ponyData;
                pony = ponies[id];
            }
            pony.id = player->pony.id;
            pony.netviewId = player->pony.netviewId;
            player->pony = pony;

            Player::savePonies(player, ponies);

            player->pony.loadQuests(player);
            if (!player->pony.loadInventory(player)) // Create a default inventory if we can't find one saved
            {
                InventoryItem raincloudHat{0,73};
                InventoryItem goggles{1,17};
                InventoryItem hat{2,62};
                InventoryItem bag{3,60};
                player->pony.inv << raincloudHat << goggles << hat << bag;
                player->pony.nBits = 15;
                player->pony.saveInventory(player);
            }

            sendLoadSceneRPC(player, player->pony.sceneName, player->pony.pos);
            // Send instantiate to the players of the new scene
            Scene* scene = findScene(player->pony.sceneName);
            for (int i=0; i<scene->players.size(); i++)
                if (scene->players[i]->pony.netviewId!=player->pony.netviewId && scene->players[i]->inGame>=2)
                    sendNetviewInstantiate(&player->pony, scene->players[i]);

            //Send the 46s init messages
            win.logMessage(QString("UDP: Sending the 46 init messages"));
            sendMessage(player,MsgUserReliableOrdered4,QByteArray::fromHex("141500000000")); // Sends a 46, init friends
            sendMessage(player,MsgUserReliableOrdered4,QByteArray::fromHex("0e00000000")); // Sends a 46, init journal
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered20 && (unsigned char)msg[3]==0x18 && (unsigned char)msg[4]==0) // Vortex messages
        {
            if (player->inGame>=2)
            {
                quint8 id = (quint8)msg[5];
                Vortex vortex = findVortex(player->pony.sceneName, id);
                if (vortex.destName.isEmpty())
                {
                    win.logMessage("Can't find vortex "+QString().setNum(id)+" on map "+player->pony.sceneName);
                }
                else
                {
                    sendLoadSceneRPC(player, vortex.destName, vortex.destPos);
                }
            }
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered4 && (unsigned char)msg[5]==0x2) // Delete pony request
        {
            win.logMessage(QString("UDP: Deleting a character"));
            QList<Pony> ponies = Player::loadPonies(player);
            quint32 id = (quint8)msg[6] +((quint8)msg[7]<<8) + ((quint8)msg[8]<<16) + ((quint8)msg[9]<<24);
            ponies.removeAt(id);

            Player::savePonies(player,ponies);
            sendPonies(player);
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered12 && (unsigned char)msg[7]==0xCA) // Animation
        {
            //win.logMessage("UDP: Broadcasting animation");
            // Send to everyone
            Scene* scene = findScene(player->pony.sceneName);
            if (scene->name.isEmpty())
                win.logMessage("UDP: Can't find the scene for animation message, aborting");
            else
            {
                if (player->lastValidReceivedAnimation.isEmpty() ||
                    (quint8)player->lastValidReceivedAnimation[3] != (quint8)0x01 || (quint8)msg[5 + 3] == 0x00) {
                    // Don't accept invalid animation (0x01 Flying 0x00 Landing)
                    // XXX The game lets players send nonsense (dancing while sitting down), those should be filtered
                    player->lastValidReceivedAnimation = msg.mid(5, msgSize - 5);
                    for (int i=0; i<scene->players.size(); i++) {
                        if (scene->players[i] == player)
                            continue; // Don't send the animation to ourselves, it'll be played regardless
                        if (scene->players[i]->inGame>=2)
                            sendMessage(scene->players[i], MsgUserReliableOrdered12, player->lastValidReceivedAnimation); // Broadcast
                    }
                }
            }
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered11 && (unsigned char)msg[7]==0x3D) // Skill
        {
            //win.logMessage("UDP: Broadcasting skill");
            // Send to everyone
            Scene* scene = findScene(player->pony.sceneName);
            if (scene->name.isEmpty())
                win.logMessage("UDP: Can't find the scene for skill message, aborting");
            else
                for (int i=0; i<scene->players.size(); i++)
                    if (scene->players[i]->inGame>=2)
                        sendMessage(scene->players[i], MsgUserReliableOrdered11, msg.mid(5, msgSize - 5)); // Broadcast
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered11 && (unsigned char)msg[7]==0x08) // Wear request
        {
            quint8 index = msg[9];
            if (index < player->pony.inv.size() && !player->pony.worn.contains(player->pony.inv[index])) // Send the wear request to everyone in room
            {
                player->pony.worn << player->pony.inv[index];
                Scene* scene = findScene(player->pony.sceneName);
                if (scene->name.isEmpty())
                    win.logMessage("UDP: Can't find the scene for wear message, aborting");
                else
                    for (int i=0; i<scene->players.size(); i++)
                        if (scene->players[i]->inGame>=2)
                            sendWornRPC(&player->pony, scene->players[i], player->pony.worn);
            }
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered11 && (unsigned char)msg[7]==0x04) // Get worn items request
        {
            quint16 targetId = ((quint16)(quint8)msg[5]) + (((quint16)(quint8)msg[6])<<8);
            Player* target = Player::findPlayer(win.udpPlayers, targetId);
            if (target->pony.netviewId == targetId)
                sendWornRPC(&target->pony, player, target->pony.worn);
            else
                win.logMessage("Can't find netviewId "+QString().setNum(targetId)+" to send worn items");
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered11 && (unsigned char)msg[7]==0x31) // Run script (NPC) request
        {
            quint16 targetId = ((quint16)(quint8)msg[5]) + (((quint16)(quint8)msg[6])<<8);
            //win.logMessage("Quest "+QString().setNum(targetId)+" requested");
            for (int i=0; i<player->pony.quests.size(); i++)
                if (player->pony.quests[i].id == targetId)
                {
                    player->pony.lastQuest = i;
                    player->pony.quests[i].runScript();
                    break;
                }
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered4 && (unsigned char)msg[5]==0xB) // Continue dialog
        {
            //win.logMessage("Resuming script for quest "+QString().setNum(player->pony.lastQuest));
            player->pony.quests[player->pony.lastQuest].processAnswer();
        }
        else if ((unsigned char)msg[0]==MsgUserReliableOrdered4 && (unsigned char)msg[5]==0xC) // Continue dialog (with answer)
        {
            quint32 answer = ((quint32)(quint8)msg[6])
                            + (((quint32)(quint8)msg[7])<<8)
                            + (((quint32)(quint8)msg[8])<<16)
                            + (((quint32)(quint8)msg[9])<<24);
            //win.logMessage("Resuming script with answer "+QString().setNum(answer)
            //               +" for quest "+QString().setNum(player->pony.lastQuest));
            player->pony.quests[player->pony.lastQuest].processAnswer(answer);
        }
        else
        {
            // Display data
            quint32 unknownMsgSize =  (((quint16)(quint8)msg[3]) +(((quint16)(quint8)msg[4])<<8)) / 8;
            win.logMessage("UDP: Unknown message received : "
                           +QString(player->receivedDatas->left(unknownMsgSize+5).toHex().data()));
            *player->receivedDatas = player->receivedDatas->mid(unknownMsgSize+5);
            msgSize=0;
        }
    }
    else if ((unsigned char)msg[0]==MsgUserUnreliable) // Sync (position) update
    {
        if ((quint8)msg[5]==(quint8)player->pony.netviewId
                && (quint8)msg[6]==(quint8)((player->pony.netviewId>>8)&0xFF))
            Sync::receiveSync(player, msg);
    }
    else
    {
        // Display data
        win.logMessage("Unknown data received (UDP) (hex) : ");
        win.logMessage(QString(player->receivedDatas->toHex().data()));
        quint32 unknownMsgSize = (((quint16)(quint8)msg[3]) +(((quint16)(quint8)msg[4])<<8)) / 8;
        *player->receivedDatas = player->receivedDatas->mid(unknownMsgSize+5);
        msgSize=0;
    }

    *player->receivedDatas = player->receivedDatas->mid(msgSize);
    if (player->receivedDatas->size())
        receiveMessage(player);
}
