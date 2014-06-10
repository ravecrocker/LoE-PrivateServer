#include "skillparser.h"
#include "widget.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

SkillParser::SkillParser(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw "SkillParser: Unable to open file";

    QByteArray data = file.readAll();

    // Remove comments
    while (true)
    {
        int start=data.indexOf("/*");
        if (start == -1)    break;
        int end = data.indexOf("*/");
        if (end < start+2)  break; // We would loop forever otherwise
        if (end != -1)      data.remove(start, end-start+2);
    }

    QJsonParseError jsonError;
    QJsonArray json = QJsonDocument::fromJson(data,&jsonError).array();
    if (jsonError.error != QJsonParseError::NoError)
        throw QString("SkillParser: Error: "+jsonError.errorString()+QString(" at %1").arg(jsonError.offset));

    for (QJsonValue skillValue : json)
    {
        if (!skillValue.isObject())
            throw "SkillParser: Invalid format (not an object)";

        QJsonObject skillObject = skillValue.toObject();
        win.skills.append(parseSkill(skillObject));
    }
}

Skill SkillParser::parseSkill(QJsonObject &skillObject)
{
    Skill skill;
    if (skillObject.contains("ID"))
        skill.id = skillObject["ID"].toDouble();
    if (skillObject.contains("MaxLevel"))
        skill.maxLevel = skillObject["MaxLevel"].toDouble();
    if (skillObject.contains("Races"))
        skill.races = parseRaces(skillObject["Races"].toArray());
    if (skillObject.contains("DamageType"))
        skill.damageType = parseDamageType(skillObject["DamageType"].toArray());
    if (skillObject.contains("Upgrades"))
        skill.upgrades = parseSkillUpgrades(skillObject["Upgrades"].toArray());
    return skill;
}

Skill::SkillRace SkillParser::parseRaces(QJsonArray jsonRaces)
{
    Skill::SkillRace races = Skill::None;
    for (QJsonValue raceVal : jsonRaces)
    {
        if (raceVal.toString() == "Earth")
            races = (Skill::SkillRace)(races|Skill::EarthPony);
        else if (raceVal.toString() == "Unicorn")
            races = (Skill::SkillRace)(races|Skill::Unicorn);
        else if (raceVal.toString() == "Pegasus")
            races = (Skill::SkillRace)(races|Skill::Pegasus);
        else
            throw "SkillParser::parseRaces: Invalid race";
    }
    return races;
}

Skill::SkillDamageType SkillParser::parseDamageType(QJsonArray jsonDT)
{
    if (jsonDT.size() > 1)
        throw "SkillParser::parseDamageType: multiple damage types !";
    Skill::SkillDamageType dt = Skill::SkillDamageType::Physical;
    for (QJsonValue dtVal : jsonDT)
    {
        if (dtVal.toString() == "Physical")
            dt = Skill::SkillDamageType::Physical;
        else if (dtVal.toString() == "Magical")
            dt = Skill::SkillDamageType::Magical;
        else
            throw "SkillParser::parseDamageType: Invalid damage type";
    }
    return dt;
}

QVector<SkillUpgrade> SkillParser::parseSkillUpgrades(QJsonArray jsonUpgrades)
{
    QVector<SkillUpgrade> skillUpgrades;
    for (QJsonValue jsonUpgrade : jsonUpgrades)
    {
        QJsonObject upgradeObject = jsonUpgrade.toObject();
        SkillUpgrade upgrade;
        if (upgradeObject.contains("ID"))
            upgrade.id = upgradeObject["ID"].toDouble();
        if (upgradeObject.contains("Tier"))
            upgrade.tier = upgradeObject["Tier"].toDouble();
        if (upgradeObject.contains("EnergyCost"))
            upgrade.energyCost = upgradeObject["EnergyCost"].toDouble();
        if (upgradeObject.contains("TargetDistance"))
            upgrade.targetDistance = upgradeObject["TargetDistance"].toDouble();
        if (upgradeObject.contains("Cooldown"))
            upgrade.cooldown = upgradeObject["Cooldown"].toDouble();
        if (upgradeObject.contains("CastTime"))
            upgrade.castTime = upgradeObject["CastTime"].toDouble();
        if (upgradeObject.contains("GlobalCooldown"))
            upgrade.globalCooldown = upgradeObject["GlobalCooldown"].toDouble();
        if (upgradeObject.contains("ParentID"))
            upgrade.parentId = upgradeObject["ParentID"].toDouble();
        if (upgradeObject.contains("Aggression"))
            upgrade.aggression = upgradeObject["Aggression"].toDouble();
        if (upgradeObject.contains("ConeAngle"))
            upgrade.coneAngle = upgradeObject["ConeAngle"].toDouble();
        if (upgradeObject.contains("AoERadius"))
            upgrade.AoERadius = upgradeObject["AoERadius"].toDouble();
        if (upgradeObject.contains("AoEDuration"))
            upgrade.AoEDuration = upgradeObject["AoEDuration"].toDouble();
        if (upgradeObject.contains("MaxSplashCount"))
            upgrade.maxSplashCount = upgradeObject["MaxSplashCount"].toDouble();
        if (upgradeObject.contains("TrainingPointCost"))
            upgrade.trainingPointCost = upgradeObject["TrainingPointCost"].toDouble();
        if (upgradeObject.contains("MinimumLevel"))
            upgrade.minimulLevel = upgradeObject["MinimumLevel"].toDouble();
        if (upgradeObject.contains("TargetShapes"))
            upgrade.targetShapes = parseTargetShapes(upgradeObject["TargetShapes"].toString());
        if (upgradeObject.contains("TargetEffects"))
            upgrade.targetEffects = parseTargetEffects(upgradeObject["TargetEffects"].toArray());
        if (upgradeObject.contains("SplashEffects"))
            upgrade.splashEffects = parseTargetEffects(upgradeObject["SplashEffects"].toArray());
        skillUpgrades.append(upgrade);
    }
    return skillUpgrades;
}

SkillUpgrade::TargetShape SkillParser::parseTargetShapes(QString jsonShapes)
{
    SkillUpgrade::TargetShape shapes = SkillUpgrade::TargetShape::None;
    if (jsonShapes.contains("Sphere"))
        shapes = (SkillUpgrade::TargetShape)(shapes|SkillUpgrade::Sphere);
    if (jsonShapes.contains("Frustum"))
        shapes = (SkillUpgrade::TargetShape)(shapes|SkillUpgrade::Frustum);
    if (jsonShapes.contains("Target"))
        shapes = (SkillUpgrade::TargetShape)(shapes|SkillUpgrade::Target);
    if (jsonShapes.contains("Ray"))
        shapes = (SkillUpgrade::TargetShape)(shapes|SkillUpgrade::Ray);
    if (shapes == SkillUpgrade::TargetShape::None)
        throw "SkillParser::parseTargetShapes: No target !";

    return shapes;
}

QVector<SkillTargetEffect> SkillParser::parseTargetEffects(QJsonArray jsonEffects)
{
    QVector<SkillTargetEffect> effects;
    for (QJsonValue jsonEffect : jsonEffects)
    {
        QJsonObject effectObject = jsonEffect.toObject();
        SkillTargetEffect effect;
        if (effectObject.contains("Stat"))
            effect.stat = parseStat(effectObject["Stat"].toArray());
        if (effectObject.contains("Targets"))
            effect.targets = parseTargets(effectObject["Targets"].toArray());
        if (effectObject.contains("Amount"))
            effect.amount = effectObject["Amount"].toDouble();
        if (effectObject.contains("IsMultiplier"))
            effect.isMultiplier = effectObject["IsMultiplier"].toBool();
        if (effectObject.contains("Chance"))
            effect.chance = effectObject["Chance"].toDouble();
        if (effectObject.contains("IsDSP"))
            effect.isDSP = effectObject["IsDSP"].toBool();
        if (effectObject.contains("SkillId"))
            effect.skillId = effectObject["SkillId"].toDouble();
        effects.append(effect);
    }
    return effects;
}

SkillTarget SkillParser::parseTargets(QJsonArray jsonTargets)
{
    SkillTarget targets = SkillTarget::None;
    for (QJsonValue targetVal : jsonTargets)
    {
        if (targetVal.toString() == "None")
            continue;
        else if (targetVal.toString() == "Enemy")
            targets = (SkillTarget)((int)targets|(int)SkillTarget::Enemy);
        else if (targetVal.toString() == "Friendly")
            targets = (SkillTarget)((int)targets|(int)SkillTarget::Friendly);
        else if (targetVal.toString() == "Self")
            targets = (SkillTarget)((int)targets|(int)SkillTarget::Self);
        else
            throw "SkillParser::parseTargets: Invalid target";
    }
    return targets;
}

SkillTargetStat SkillParser::parseStat(QJsonArray jsonStat)
{
    if (jsonStat.size() > 1)
        throw "SkillParser::parseStat: multiple stats !";
    SkillTargetStat stat = SkillTargetStat::Health;
    for (QJsonValue statVal : jsonStat)
    {
        if (statVal.toString() == "Health")
            stat = SkillTargetStat::Health;
        else if (statVal.toString() == "Tension")
            stat = SkillTargetStat::Tension;
        else
            throw "SkillParser::parseStat: Invalid stat";
    }
    return stat;
}
