#ifndef SKILL_H
#define SKILL_H

#include <QMap>
#include <QVector>

class Animation;
class StatsComponent;

enum class SkillTargetStat
{
    Health,
    Tension
};

enum class SkillTarget
{
    None=0,
    Enemy=1,
    Friendly=2,
    Self=4
};

struct SkillTargetEffect
{
public:
    SkillTargetEffect();

public:
    SkillTargetStat stat;
    SkillTarget targets;
    float amount;
    bool isMultiplier; ///< is amount a multiplier or a set value. how to do percent based changes to attributes
    float chance; ///< [0,1], chance of the effect happenning (default:1)
    bool isDPS; ///< Is the skill in damage per second or fixed damage
    int skillId; ///< Id of a skill to affect. -1 means none.
    float duration;
};

struct SkillUpgrade
{
public:
    SkillUpgrade();

public:
    enum TargetShape
    {
        None=0, // Invalid !
        Sphere=1,
        Frustum=2,
        Target=4,
        Ray=8
    };

public:
    unsigned id;
    unsigned tier;
    unsigned energyCost;
    float targetDistance;
    TargetShape targetShapes;
    float cooldown;
    float castTime;
    float globalCooldown;
    QVector<SkillTargetEffect> targetEffects;
    QVector<SkillTargetEffect> splashEffects;
    int parentId;
    unsigned aggression;
    unsigned coneAngle;
    unsigned AoERadius;
    unsigned AoEDuration; ///< Time that the aoe sits on the ground ticking. Each time the aoe ticks (AoEDuration times), it will perform the splash effects
    unsigned maxSplashCount;
    unsigned trainingPointCost;
    unsigned minimulLevel;
    const Animation* casterAnimation;
};

struct Skill
{
public:
    Skill();

public:
    enum SkillRace
    {
        None = 0,
        EarthPony = 1,
        Unicorn = 2,
        Pegasus = 4,
        Moose = 8
    };

    enum class SkillDamageType
    {
        Physical,
        Magical
    };

public:
    // Does not handle "undocumented" effects, like teleport actually teleporting
    // If splashOnly, only the splash effects will be applied, not the target effects
    // Only handles changes to the Health stat at the moment
    // Returns false if the skill couldn't be applied, true otherwise
    static bool applySkill(unsigned skillId, StatsComponent& target, SkillTarget targetType,
                           unsigned upgradeId=0, bool splashOnly=false);

private:
    static void applySkillEffect(SkillTargetEffect& effect, StatsComponent& target, SkillTarget targetType);

public:
    unsigned id;
    unsigned maxLevel;
    SkillRace races;
    SkillDamageType damageType;
    QMap<unsigned,SkillUpgrade> upgrades; // Maps upgrade ids to upgrades

public:
    static QMap<unsigned, Skill> skills; // Maps skill ids to skills
};

#endif // SKILL_H
