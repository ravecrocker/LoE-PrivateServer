#include "skill.h"
#include "statsComponent.h"
#include "widget.h"
#include "animation.h"
#include <QTimer>
#include <QObject>

QMap<unsigned, Skill> Skill::skills;

SkillTargetEffect::SkillTargetEffect()
    : stat{SkillTargetStat::Health}, targets{SkillTarget::Enemy},
      amount{0}, isMultiplier{true}, chance{1}, isDPS{false}, skillId{-1}, duration{0}
{
}

SkillUpgrade::SkillUpgrade()
    : id{0}, tier{1}, energyCost{0}, targetDistance{1.5}, targetShapes{TargetShape::Target},
      cooldown{2.5}, castTime{0.5}, globalCooldown{1}, targetEffects{}, splashEffects{},
      parentId{-1}, aggression{1}, coneAngle{0}, AoERadius{0}, AoEDuration{0},
      maxSplashCount{0}, trainingPointCost{0}, minimulLevel{0}, casterAnimation(&Animation::animations["ground"])
{
}

Skill::Skill()
    : id{0}, maxLevel{50}, races{SkillRace::None}, damageType{SkillDamageType::Physical}, upgrades{}
{
}

bool Skill::applySkill(unsigned skillId, StatsComponent& target, SkillTarget targetType, unsigned upgradeId, bool splashOnly)
{
    // Skill/upgrade resolution
    if (!skills.contains(skillId))
    {
        win.logMessage(QObject::tr("Skill::applySkill: No skill with ID %1").arg(skillId));
        return false;
    }
    Skill& skill = skills[skillId];
    if (!skill.upgrades.contains(upgradeId))
    {
        win.logMessage(QObject::tr("Skill::applySkill: Skill with ID %1 has no upgrade %2").arg(skillId).arg(upgradeId));
        return false;
    }
    SkillUpgrade& upgrade(skill.upgrades[upgradeId]);

    // Apply effects
    for (SkillTargetEffect& effect : upgrade.splashEffects)
        applySkillEffect(effect, target, targetType);
    if (!splashOnly)
        for (SkillTargetEffect& effect : upgrade.targetEffects)
            applySkillEffect(effect, target, targetType);

    return true;
}

void Skill::applySkillEffect(SkillTargetEffect& effect, StatsComponent& target, SkillTarget targetType)
{
    if (!((int)effect.targets & (int)targetType))
        return;
    if (effect.isDPS)
    {
        QTimer* dps = new QTimer;
        dps->setInterval(1000);
        dps->setSingleShot(true);
        float *duration = new float{effect.duration};
        QObject::connect(dps, &QTimer::timeout, [effect, dps, duration, &target](){
            if (effect.stat == SkillTargetStat::Health)
                target.takeDamage(effect.amount);

            *duration -= 1;
            if (*duration > 0)
                dps->start();
            else
            {
                delete dps;
                delete duration;
            }
        });
        dps->start();
    }
    else
    {
        if (effect.stat == SkillTargetStat::Health)
            target.takeDamage(effect.amount);
    }
}
