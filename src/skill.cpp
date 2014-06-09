#include "skill.h"

SkillTargetEffect::SkillTargetEffect()
    : stat{SkillTargetStat::Health}, targets{SkillTarget::Enemy},
    amount{0}, isMultiplier{true}, chance{1}, isDSP{false}, skillId{-1}
{
}

SkillUpgrade::SkillUpgrade()
    : id{0}, tier{1}, energyCost{0}, targetDistance{1.5}, targetShapes{TargetShape::Target},
      cooldown{2.5}, castTime{0.5}, globalCooldown{1}, targetEffects{}, splashEffects{},
      parentId{-1}, aggression{1}, coneAngle{0}, AoERadius{0}, AoEDuration{0},
      maxSplashCount{0}, trainingPointCost{0}, minimulLevel{0}
{
}

Skill::Skill()
    : id{0}, maxLevel{50}, races{SkillRace::None}, damageType{SkillDamageType::Physical}, upgrades{}
{
}
