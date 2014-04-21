#include "mob.h"
#include "mobzone.h"
#include "widget.h"
#include "mobsStats.h"
#include "message.h"

Mob::Mob(Mobzone* zone)
{
    spawnZone = currentZone = zone;
    sceneName = zone->sceneName;

    id = win.getNewId();
    netviewId = win.getNewNetviewId();

    pos = getRandomPos(zone);

    rot = {0, (float)(rand()%4-2), 0, 1};

    health = 0; // We need to know the type to know the default healt
}

UVector Mob::getRandomPos(Mobzone* zone)
{
    UVector pos;
    // Pick a random x/z point on the plane
    int xmod = floor(abs(zone->end.x-zone->start.x));
    if (xmod == 0)      pos.x = zone->start.x;
    else if (xmod <0)   pos.x = -(rand()%(xmod))+zone->start.x;
    else                pos.x = (rand()%(xmod))+zone->start.x;

    int zmod = floor(abs(zone->end.z-zone->start.z));
    if (zmod == 0)      pos.z = zone->start.z;
    else if (zmod < 0)  pos.z = -(rand()%(zmod))+zone->start.z;
    else                pos.z = (rand()%(zmod))+zone->start.z;

    // Deduce y (height)

    // Vector of the diagonal
    float diagVectX = zone->end.x - zone->start.x;
    float diagVectZ = zone->end.z - zone->start.z;

    // Coords of the pos projected on the diagonal
    float projectedX = (diagVectX*diagVectX*pos.x + diagVectX*diagVectZ*pos.z)
                        / (diagVectX*diagVectX + diagVectZ*diagVectZ);
    float projectedZ = (diagVectX*diagVectZ*pos.x + diagVectZ*diagVectZ*pos.z)
                        / (diagVectX*diagVectX + diagVectZ*diagVectZ);

    // Compare the lengths
    float diagLength = sqrt(diagVectX*diagVectX + diagVectZ*diagVectZ);
    float projectedLength = sqrt(projectedX*projectedX + projectedZ * projectedZ);

    pos.y = zone->start.y + projectedLength/diagLength * (zone->end.y - zone->start.y); /// TODO: Really deduce y
    return pos;
}

void Mob::setType(QString ModelName)
{
    ModelName = ModelName.toLower();
    modelName = ModelName;
    if (modelName == "mobs/birch dryad")
        type = MobType::birch_dryad;
    else if (modelName == "mobs/bunny")
        type = MobType::bunny;
    else if (modelName == "mobs/cockatrice")
        type = MobType::cockatrice;
    else if (modelName == "mobs/dragon")
        type = MobType::dragon;
    else if (modelName == "mobs/hornet")
        type = MobType::hornet;
    else if (modelName == "mobs/husky diamond dog")
        type = MobType::husky_diamond_dog;
    else if (modelName == "mobs/lantern monster")
        type = MobType::lantern_monster;
    else if (modelName == "mobs/timberwolf")
        type = MobType::timberwolf;
    else
        throw QString("Mob::setType(): Error, unknown type "+modelName);

    health = defaultMaxHealth[type];
}

void Mob::takeDamage(unsigned amount)
{
    if (health <= amount)
        kill();
    else
    {
        health -= amount;
        Scene* scene = findScene(sceneName);
        for (Player* player : scene->players)
        {
            sendSetStatRPC(player, netviewId, 1, health);
        }
    }
}

void Mob::kill()
{
    currentZone = spawnZone;
    health = 0;

    Scene* scene = findScene(sceneName);
    for (Player* player : scene->players)
    {
        sendNetviewRemove(player, netviewId);
    }
}

void Mob::respawn()
{
    currentZone = spawnZone;
    pos = getRandomPos(spawnZone);
    rot = {0, (float)(rand()%4-2), 0, 1};

    health = defaultMaxHealth[type];

    Scene* scene = findScene(sceneName);
    for (Player* player : scene->players)
    {
        sendNetviewInstantiate(player, modelName, netviewId, id, pos, rot);
    }
}
