#ifndef MOB_H
#define MOB_H

#include "sceneEntity.h"
#include <QString>

class Mobzone;

class Mob : public SceneEntity
{
public:
    enum MobType : unsigned
    {
        birch_dryad,
        bunny,
        cockatrice,
        dragon,
        hornet,
        husky_diamond_dog,
        lantern_monster,
        timberwolf
    };

public:
    explicit Mob(Mobzone* zone);
    void setType(QString ModelName); ///< Don't change the SceneEntity model name directly
    void kill(); ///< Kills the mob. He'll respawn
    void respawn(); ///< Resets the mob
    void takeDamage(unsigned amount); ///< Remove health, update the client, may kill the mob

private:
    static UVector getRandomPos(Mobzone* zone); ///< Returns a random position in this zone

public:
    MobType type;
    float health;

private:
    Mobzone* spawnZone;
    Mobzone* currentZone;
};

#endif // MOB_H
