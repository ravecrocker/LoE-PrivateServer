#ifndef SKILLPARSER_H
#define SKILLPARSER_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include "skill.h"

class SkillParser
{
public:
    SkillParser(QString filepath);

private:
    Skill parseSkill(QJsonObject& skillObject);
    Skill::SkillRace parseRaces(QJsonArray jsonRaces);
    Skill::SkillDamageType parseDamageType(QJsonArray jsonDT);
    QMap<unsigned,SkillUpgrade> parseSkillUpgrades(QJsonArray jsonUpgrades);
    SkillUpgrade::TargetShape parseTargetShapes(QString jsonShapes);
    QVector<SkillTargetEffect> parseTargetEffects(QJsonArray jsonEffects);
    SkillTarget parseTargets(QJsonArray jsonTargets);
    SkillTargetStat parseStat(QJsonArray jsonStat);
};

#endif // SKILLPARSER_H
