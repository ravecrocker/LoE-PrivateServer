#ifndef MOB_H
#define MOB_H

#include "sceneEntity.h"
#include <QString>

class Mobzone;

class Mob : public SceneEntity
{
private:
    enum class MobType
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
    void respawn(); ///< Resets the mob

private:
    static UVector getRandomPos(Mobzone* zone); ///< Returns a random position in this zone

private:
    MobType type;
    Mobzone* spawnZone;
    Mobzone* currentZone;
    unsigned health;
};

#endif // MOB_H
