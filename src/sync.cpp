#include "sync.h"
#include "scene.h"
#include "message.h"
#include "utils.h"
#include "serialize.h"

Sync::Sync(QObject *parent) : QObject(parent)
{
    syncTimer = new QTimer(this);
    connect(syncTimer, SIGNAL(timeout()), this, SLOT(doSync()));
}

void Sync::startSync(int syncInterval)
{
    syncTimer->start(syncInterval);
}

void Sync::stopSync()
{
    syncTimer->stop();
}

void Sync::doSync()
{
    for (int i=0; i<Scene::scenes.size(); i++)
    {
        if (Scene::scenes[i].players.size()<2)
            continue;
        for (int j=0; j<Scene::scenes[i].players.size(); j++)
            for (int k=0; k<Scene::scenes[i].players.size(); k++)
            {
                if (j==k)
                    continue;
                sendSyncMessage(Scene::scenes[i].players[j], Scene::scenes[i].players[k]);
            }
    }
}

void Sync::sendSyncMessage(Player* source, Player* dest)
{
    QByteArray data(2,0);
    data[0] = (quint8)(source->pony.netviewId&0xFF);
    data[1] = (quint8)((source->pony.netviewId>>8)&0xFF);
    data += floatToData(timestampNow());
    //data += rangedSingleToData(source.pony.pos.x, XMIN, XMAX, PosRSSize);
    //data += rangedSingleToData(source.pony.pos.y, YMIN, YMAX, PosRSSize);
    //data += rangedSingleToData(source.pony.pos.z, ZMIN, ZMAX, PosRSSize);
    data += floatToData(source->pony.pos.x);
    data += floatToData(source->pony.pos.y);
    data += floatToData(source->pony.pos.z);
    //data += rangedSingleToData(source.pony.rot.x, ROTMIN, ROTMAX, RotRSSize);
    data += rangedSingleToData(source->pony.rot.y, ROTMIN, ROTMAX, RotRSSize);
    //data += rangedSingleToData(source.pony.rot.z, ROTMIN, ROTMAX, RotRSSize);
    sendMessage(dest, MsgUserUnreliable, data);

    //app.logMessage("UDP: Syncing "+QString().setNum(source->pony.netviewId)+" to "+QString().setNum(dest->pony.netviewId));
}
// TODO: Test ranged singles on PonyVille with the bounds from the text assets
// Or maybe test the command that gives the bounds to the clients

void Sync::receiveSync(Player* player, QByteArray data) // Receives the 01 updates from each players
{
    if (player->inGame < 2) // A sync message while loading would teleport use to a wrong position
        return;
    //app.logMessage("Got sync from "+QString().setNum(player->pony.netviewId));

    // 5 and 6 are id and id>>8
    player->pony.pos.x = dataToFloat(data.mid(11));
    player->pony.pos.y = dataToFloat(data.mid(15));
    player->pony.pos.z = dataToFloat(data.mid(19));

    if (data.size() >= 23)
        player->pony.rot.y = dataToRangedSingle(ROTMIN, ROTMAX, RotRSSize, data.mid(23,1));
    if (data.size() >= 25)
    {
        player->pony.rot.x = dataToRangedSingle(ROTMIN, ROTMAX, RotRSSize, data.mid(24,1));
        player->pony.rot.z = dataToRangedSingle(ROTMIN, ROTMAX, RotRSSize, data.mid(25,1));
    }
}
