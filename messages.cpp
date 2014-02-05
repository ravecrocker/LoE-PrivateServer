#include "message.h"
#include "character.h"
#include "widget.h"

// File-global game-entering mutex (to prevent multiple instantiates)
static QMutex levelLoadMutex;

void sendPonies(Player* player)
{
    // The full request is like a normal sendPonies but with all the serialized ponies at the end
    QList<Pony> ponies = Player::loadPonies(player);
    quint32 poniesDataSize=0;
    for (int i=0;i<ponies.size();i++)
        poniesDataSize+=ponies[i].ponyData.size();

    QByteArray data(5,0);
    data[0] = 1; // Request number
    data[1] = ponies.size(); // Number of ponies
    data[2] = ponies.size()>>8; // Number of ponies
    data[3] = ponies.size()>>16; // Number of ponies
    data[4] = ponies.size()>>24; // Number of ponies

    for (int i=0;i<ponies.size();i++)
        data += ponies[i].ponyData;

    win.logMessage(QString("UDP: Sending characters data to ")+QString().setNum(player->pony.netviewId));
    sendMessage(player, MsgUserReliableOrdered4, data);
}

void sendEntitiesList(Player *player)
{
    if (!player->inGame) // Not yet in game, send player's ponies list
    {
        sendPonies(player);
        return;
    }

    // Loading finished, sending entities list
    if (player->inGame!=1) // Only instantiate once
    {
        win.logMessage(QString("UDP: Entities list already sent to ")+QString().setNum(player->pony.netviewId));
        return;
    }
    win.logMessage(QString("UDP: Sending entities list to ")+QString().setNum(player->pony.netviewId));
    Scene* scene = findScene(player->pony.sceneName); // Spawn all the players on the client
    for (int i=0; i<scene->players.size(); i++)
        sendNetviewInstantiate(scene->players[i], player);

    /// SEND DONUT STEEL (in front of the mirror)
    if (scene->name == "RaritysBoutique")
    {
        win.logMessage(QString("UDP: Sending Donut Steel to ")+QString().setNum(player->pony.netviewId));
        sendNetviewInstantiate(player,"PlayerBase",0,0,UVector(24.7333,-1.16802,-51.7106), UQuaternion(0,-0.25,0,1));
    }

    // Send stats of the client's pony
    sendSetMaxStatRPC(player, 0, 100);
    sendSetStatRPC(player, 0, 100);
    sendSetMaxStatRPC(player, 1, 100);
    sendSetStatRPC(player, 1, 100);
}

void sendPonySave(Player *player, QByteArray msg)
{
    quint16 netviewId = (quint8)msg[6] + ((quint8)msg[7]<<8);
    Player* refresh = Player::findPlayer(win.udpPlayers, netviewId);

    if (netviewId == player->pony.netviewId)
    {
        // Send the entities list if the game is starting (sends RPC calls to the view with id 0085 (the PlayerBase))
        win.logMessage(QString("UDP: Sending pony save for/to ")+QString().setNum(netviewId));

        // Set current/max stats
        sendSetMaxStatRPC(player, 0, 100);
        sendSetStatRPC(player, 0, 100);
        sendSetMaxStatRPC(player, 1, 100);
        sendSetStatRPC(player, 1, 100);

        sendPonyData(player);

        // Set inventory
        InventoryItem raincloudHat;
        raincloudHat.id=73;
        raincloudHat.index=0;
        InventoryItem goggles;
        goggles.id=17;
        goggles.index=1;
        InventoryItem hat;
        hat.id=62;
        hat.index=2;
        InventoryItem bag;
        bag.id=60;
        bag.index=3;
        QList<InventoryItem> inv;
        inv << raincloudHat << goggles << hat << bag;
        QList<WearableItem> worn;
        worn << goggles << bag;
        sendInventoryRPC(player, inv, worn, 10); // Start with 10 bits and no inventory, until we implement it correctly

        // Send skills
        QList<QPair<quint32, quint32> > skills;
        skills << QPair<quint32, quint32>(10, 0); // Ground pound
        if (player->pony.getType() == Pony::EarthPony)
            skills << QPair<quint32, quint32>(5, 0); // Seismic buck
        else if (player->pony.getType() == Pony::Pegasus)
            skills << QPair<quint32, quint32>(11, 0); // Tornado
        else if (player->pony.getType() == Pony::Unicorn)
            skills << QPair<quint32, quint32>(2, 0); // Teleport
        sendSkillsRPC(player, skills);

        // Set current/max stats again (that's what the official server does, not my idea !)
        sendSetStatRPC(player, 0, 100);
        sendSetMaxStatRPC(player, 0, 100);
        sendSetStatRPC(player, 1, 100);
        sendSetMaxStatRPC(player, 1, 100);

        /// SEND DONUT STEEL (he's a moose dark-as-my-soul OC)
        if (player->pony.sceneName == "RaritysBoutique")
        {
            win.logMessage("UDP: Sending pony save for Donut Steel to "+QString().setNum(player->pony.netviewId));
            Player donutSteel;
            donutSteel.pony.id = donutSteel.pony.netviewId = 0;
            donutSteel.pony.ponyData = QByteArray::fromHex("0b446f6e757420537465656c0402b70000000000000000000000ca1a0017ff00032e03ff050072ff000c000b0000000000cdcc8c3fff9f");
            sendPonyData(&donutSteel, player);
        }

        refresh->inGame = 2;
    }
    else if (!refresh->IP.isEmpty())
    {
        win.logMessage(QString("UDP: Sending pony save for ")+QString().setNum(refresh->pony.netviewId)
                       +" to "+QString().setNum(player->pony.netviewId));

        QList<WearableItem> worn;
        sendWornRPC(player, worn); // Wear nothing, until we implement it

        sendSetMaxStatRPC(player, 0, 100);
        sendSetStatRPC(player, 0, 100);
        sendSetMaxStatRPC(player, 1, 100);
        sendSetStatRPC(player, 1, 100);

        sendPonyData(refresh, player);

        if (!refresh->lastValidReceivedAnimation.isEmpty())
            sendMessage(player, MsgUserReliableOrdered12, refresh->lastValidReceivedAnimation);
    }
    else
    {
        win.logMessage("UDP: Error sending pony save : netviewId "+QString().setNum(netviewId)+" not found");
    }
}

void receiveSync(Player* player, QByteArray data) // Receives the 01 updates from each players
{
    if (player->inGame!=2) // A sync message while loading would teleport use to a wrong position
        return;
    //win.logMessage("Got sync");

    // 5 and 6 are id and id>>8
    player->pony.pos.x = dataToFloat(data.right(data.size()  - 11));
    player->pony.pos.y = dataToFloat(data.right(data.size()  - 15));
    player->pony.pos.z = dataToFloat(data.right(data.size()  - 19));

    // TODO : Get the rotation and save it
    if (data.size() >= 23)
        player->pony.rot.y = dataToRangedSingle(ROTMIN, ROTMAX, RotRSSize, data.mid(23,1));
    if (data.size() >= 25)
    {
        player->pony.rot.x = dataToRangedSingle(ROTMIN, ROTMAX, RotRSSize, data.mid(24,1));
        player->pony.rot.z = dataToRangedSingle(ROTMIN, ROTMAX, RotRSSize, data.mid(25,1));
    }
}

void sendNetviewInstantiate(Player *player, QString key, quint16 ViewId, quint16 OwnerId, UVector pos, UQuaternion rot)
{
    QByteArray data(1,1);
    data += stringToData(key);
    QByteArray data2(4,0);
    data2[0]=ViewId;
    data2[1]=ViewId>>8;
    data2[2]=OwnerId;
    data2[3]=OwnerId>>8;
    data += data2;
    data += vectorToData(pos);
    data += quaternionToData(rot);
    sendMessage(player, MsgUserReliableOrdered6, data);
}

void sendNetviewInstantiate(Player *player)
{
    win.logMessage("UDP: Send instantiate for/to "+QString().setNum(player->pony.netviewId));
    QByteArray data(1,1);
    data += stringToData("PlayerBase");
    QByteArray data2(4,0);
    data2[0]=player->pony.netviewId;
    data2[1]=player->pony.netviewId>>8;
    data2[2]=player->pony.id;
    data2[3]=player->pony.id>>8;
    data += data2;
    data += vectorToData(player->pony.pos);
    data += quaternionToData(player->pony.rot);
    sendMessage(player, MsgUserReliableOrdered6, data);

    win.logMessage(QString("Instantiate at ")+QString().setNum(player->pony.pos.x)+" "
                   +QString().setNum(player->pony.pos.y)+" "
                   +QString().setNum(player->pony.pos.z));
}

void sendNetviewInstantiate(Player *src, Player *dst)
{
    win.logMessage("UDP: Send instantiate for "+QString().setNum(src->pony.netviewId)
                   +" to "+QString().setNum(dst->pony.netviewId));
    QByteArray data(1,1);
    data += stringToData("PlayerBase");
    QByteArray data2(4,0);
    data2[0]=src->pony.netviewId;
    data2[1]=src->pony.netviewId>>8;
    data2[2]=src->pony.id;
    data2[3]=src->pony.id>>8;
    data += data2;
    data += vectorToData(src->pony.pos);
    data += quaternionToData(src->pony.rot);
    sendMessage(dst, MsgUserReliableOrdered6, data);

   //win.logMessage(QString("Instantiate at ")+QString().setNum(rSrc.pony.pos.x)+" "
   //                +QString().setNum(rSrc.pony.pos.y)+" "
   //                +QString().setNum(rSrc.pony.pos.z));
}

void sendNetviewRemove(Player *player, quint16 netviewId)
{
    win.logMessage("UDP: Removing netview "+QString().setNum(netviewId)+" to "+QString().setNum(player->pony.netviewId));

    QByteArray data(3,2);
    data[1] = netviewId;
    data[2] = netviewId>>8;
    sendMessage(player, MsgUserReliableOrdered6, data);
}

void sendSetStatRPC(Player *player, quint8 statId, float value)
{
    QByteArray data(4,50);
    data[0] = player->pony.netviewId;
    data[1] = player->pony.netviewId>>8;
    data[3] = statId;
    data += floatToData(value);
    sendMessage(player, MsgUserReliableOrdered18, data);
}

void sendSetMaxStatRPC(Player* player, quint8 statId, float value)
{
    QByteArray data(4,51);
    data[0] = player->pony.netviewId;
    data[1] = player->pony.netviewId>>8;
    data[3] = statId;
    data += floatToData(value);
    sendMessage(player, MsgUserReliableOrdered18, data);
}

void sendWornRPC(Player *player, QList<WearableItem> &worn)
{
    QByteArray data(3, 4);
    data[0] = player->pony.netviewId;
    data[1] = player->pony.netviewId>>8;
    data += 32; // Max Worn Items
    data += worn.size();
    for (int i=0;i<worn.size();i++)
    {
        data += worn[i].index;
        data += worn[i].id;
        data += worn[i].id>>8;
        data += worn[i].id>>16;
        data += worn[i].id>>24;
    }
    sendMessage(player, MsgUserReliableOrdered18, data);
}

void sendInventoryRPC(Player *player, QList<InventoryItem>& inv, QList<WearableItem>& worn, quint32 nBits)
{
    QByteArray data(5, 5);
    data[0] = player->pony.netviewId;
    data[1] = player->pony.netviewId>>8;
    data[3] = 12; // Max Inventory Size
    data[4] = inv.size();
    for (int i=0;i<inv.size();i++)
    {
        data += inv[i].index;
        data += inv[i].id;
        data += inv[i].id>>8;
        data += inv[i].id>>16;
        data += inv[i].id>>24;
        data += inv[i].amount;
        data += inv[i].amount>>8;
        data += inv[i].amount>>16;
        data += inv[i].amount>>24;
    }
    data += 32; // Max Worn Items
    data += worn.size();
    for (int i=0;i<worn.size();i++)
    {
        data += worn[i].index;
        data += worn[i].id;
        data += worn[i].id>>8;
        data += worn[i].id>>16;
        data += worn[i].id>>24;
    }
    data += nBits;
    data += nBits>>8;
    data += nBits>>16;
    data += nBits>>24;
    sendMessage(player, MsgUserReliableOrdered18, data);
}

void sendSkillsRPC(Player* player, QList<QPair<quint32, quint32> > &skills)
{
    QByteArray data(8, 0xC3);
    data[0] = player->pony.netviewId;
    data[1] = player->pony.netviewId>>8;
    data[3] = 0x00; // Use dictionnary flag
    data[4] = skills.size();
    data[5] = skills.size()>>8;
    data[6] = skills.size()>>16;
    data[7] = skills.size()>>24;
    for (int i=0;i<skills.size();i++)
    {
        data += skills[i].first;
        data += skills[i].first>>8;
        data += skills[i].first>>16;
        data += skills[i].first>>24;
        data += skills[i].second;
        data += skills[i].second>>8;
        data += skills[i].second>>16;
        data += skills[i].second>>24;
    }
    sendMessage(player, MsgUserReliableOrdered18, data);
}

void sendPonyData(Player *player)
{
    // Sends the ponyData
    win.logMessage(QString("UDP: Sending the ponyData for/to "+QString().setNum(player->pony.netviewId)));
    QByteArray data(3,0xC8);
    data[0] = player->pony.netviewId;
    data[1] = player->pony.netviewId>>8;
    data += player->pony.ponyData;
    sendMessage(player, MsgUserReliableOrdered18, data);
}

void sendPonyData(Player *src, Player *dst)
{
    // Sends the ponyData
    //win.logMessage(QString("UDP: Sending the ponyData for "+QString().setNum(src->pony.netviewId)
    //                       +" to "+QString().setNum(dst->pony.netviewId)));
    QByteArray data(3,0xC8);
    data[0] = src->pony.netviewId;
    data[1] = src->pony.netviewId>>8;
    data += src->pony.ponyData;
    sendMessage(dst, MsgUserReliableOrdered18, data);
}

void sendLoadSceneRPC(Player* player, QString sceneName) // Loads a scene and send to the default spawn
{
    win.logMessage(QString("UDP: Loading scene \"") + sceneName + "\" on "+QString().setNum(player->pony.netviewId));
    Vortex vortex = findVortex(sceneName, 0);
    if (vortex.destName.isEmpty())
    {
        win.logMessage("UDP: Scene not in vortex DB. Aborting scene load.");
        return;
    }

    player->inGame=1;

    Scene* scene = findScene(sceneName);
    Scene* oldScene = findScene(player->pony.sceneName);
    if (scene->name.isEmpty() || oldScene->name.isEmpty())
    {
        win.logMessage("UDP: Can't find the scene, aborting");
        return;
    }

    // Update scene players
    //win.logMessage("sendLoadSceneRPC: locking");
    levelLoadMutex.lock();
    player->pony.pos = vortex.destPos;
    player->pony.sceneName = sceneName;
    player->lastValidReceivedAnimation.clear(); // Changing scenes resets animations
    Player::removePlayer(oldScene->players, player->IP, player->port);
    if (oldScene->name != sceneName)
    {
        // Send remove RPC to the other players of the old scene
        for (int i=0; i<oldScene->players.size(); i++)
            sendNetviewRemove(oldScene->players[i], player->pony.netviewId);

        // Send instantiate to the players of the new scene
        for (int i=0; i<scene->players.size(); i++)
            if (scene->players[i]->inGame==2)
                sendNetviewInstantiate(player, scene->players[i]);
    }
    scene->players << player;

    QByteArray data(1,5);
    data += stringToData(sceneName);
    sendMessage(player,MsgUserReliableOrdered6,data); // Sends a 48
    //win.logMessage("sendLoadSceneRPC: unlocking");
    levelLoadMutex.unlock();
}

void sendLoadSceneRPC(Player* player, QString sceneName, UVector pos) // Loads a scene and send to the given pos
{
    win.logMessage(QString(QString("UDP: Loading scene \"")+sceneName
                           +"\" to "+QString().setNum(player->pony.netviewId)
                           +" at "+QString().setNum(pos.x)+" "
                           +QString().setNum(pos.y)+" "
                           +QString().setNum(pos.z)));

    player->inGame=1;

    Scene* scene = findScene(sceneName);
    Scene* oldScene = findScene(player->pony.sceneName);
    if (scene->name.isEmpty() || oldScene->name.isEmpty())
    {
        win.logMessage("UDP: Can't find the scene, aborting");
        return;
    }

    // Update scene players
    //win.logMessage("sendLoadSceneRPC pos: locking");
    levelLoadMutex.lock();
    player->pony.pos = pos;
    player->pony.sceneName = sceneName;
    player->lastValidReceivedAnimation.clear(); // Changing scenes resets animations
    Player::removePlayer(oldScene->players, player->IP, player->port);
    if (oldScene->name != sceneName)
    {
        // Send remove RPC to the other players of the old scene
        for (int i=0; i<oldScene->players.size(); i++)
            sendNetviewRemove(oldScene->players[i], player->pony.netviewId);

        // Send instantiate to the players of the new scene
        for (int i=0; i<scene->players.size(); i++)
            if (scene->players[i]->inGame==2)
                sendNetviewInstantiate(player, scene->players[i]);
    }
    scene->players << player;

    QByteArray data(1,5);
    data += stringToData(sceneName);
    sendMessage(player,MsgUserReliableOrdered6,data); // Sends a 48
    //win.logMessage("sendLoadSceneRPC pos: unlocking");
    levelLoadMutex.unlock();
}

void sendChatMessage(Player* player, QString message, QString author)
{
    QByteArray idAndAccess(5,0);
    idAndAccess[0] = player->pony.netviewId;
    idAndAccess[1] = player->pony.netviewId << 8;
    idAndAccess[2] = player->pony.netviewId << 16;
    idAndAccess[3] = player->pony.netviewId << 24;
    idAndAccess[4] = 0x0; // Access level
    QByteArray data(2,0);
    data[0] = 0xf;
    data[1] = 0x4;
    data += stringToData(author);
    data += stringToData(message);
    data += idAndAccess;

    sendMessage(player,MsgUserReliableOrdered4,data); // Sends a 46
}

void sendMove(Player* player, float x, float y, float z)
{
    QByteArray data(1,0);
    data[0] = 0xce; // Request number
    data += floatToData(x);
    data += floatToData(y);
    data += floatToData(z);
    win.logMessage(QString("UDP: Moving character"));
    sendMessage(player,MsgUserReliableOrdered4, data);
}
