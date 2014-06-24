#include "sceneEntity.h"
#include "quest.h"
#include "mob.h"
#include "player.h"

int SceneEntity::lastNetviewId;
int SceneEntity::lastId;
QMutex SceneEntity::lastIdMutex; // Protects lastId and lastNetviewId
bool SceneEntity::usedids[65536];

int SceneEntity::getNewNetviewId()
{
    int i;

    for (i = 0; i < 65536; i++) {
        usedids[i] = false;
    }

    for (int c = 0; c < Quest::npcs.size(); c++) {
        usedids[Quest::npcs[c]->netviewId] = true;
    }
    for (int c = 0; c < Mob::mobs.size(); c++) {
        usedids[Mob::mobs[c]->netviewId] = true;
    }
    for (int c = 0; c < Player::udpPlayers.size(); c++) {
        usedids[Player::udpPlayers[c]->pony.netviewId] = true;
    }

    i = 0;
    while (usedids[i]) i++;

    return i;
}

int SceneEntity::getNewId()
{
    int i;

    for (i = 0; i < 65536; i++) {
        usedids[i] = false;
    }

    for (int c = 0; c < Quest::npcs.size(); c++) {
        usedids[Quest::npcs[c]->id] = true;
    }
    for (int c = 0; c < Mob::mobs.size(); c++) {
        usedids[Mob::mobs[c]->id] = true;
    }
    for (int c = 0; c < Player::udpPlayers.size(); c++) {
        usedids[Player::udpPlayers[c]->pony.id] = true;
    }

    i = 0;
    while (usedids[i]) i++;

    return i;
}
