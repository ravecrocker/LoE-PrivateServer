#ifndef MOB_H
#define MOB_H

#include "sceneEntity.h"
#include "statsComponent.h"
#include <QString>

class Mobzone;

class Mob : public SceneEntity, public StatsComponent
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
    virtual void kill() override; ///< Kills the mob. He'll respawn
    virtual void respawn() override; ///< Resets the mob
    virtual void takeDamage(unsigned amount) override; ///< Remove health, update the client, may kill the mob

private:
    static UVector getRandomPos(Mobzone* zone); ///< Returns a random position in this zone

public:
    MobType type;

private:
    Mobzone* spawnZone;
    Mobzone* currentZone;

public:
    static QList<Mob*> mobs;
    static QList<Mobzone*> mobzones;
};

#endif // MOB_H
