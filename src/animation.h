#ifndef ANIMATION_H
#define ANIMATION_H

#include <QString>
#include <QMap>

struct Animation
{
public:
    Animation();

public:
    unsigned id;
    QString name;

public:
    static QMap<QString, Animation> animations;
};

#endif // ANIMATION_H
