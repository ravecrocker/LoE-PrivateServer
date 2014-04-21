#ifndef MOBZONE_H
#define MOBZONE_H

#include "dataType.h"
#include <QMap>
#include <QPair>

struct Mobzone
{
    UVector start, end; ///< Bounds of the mobzone
    QString sceneName;
    QMap<Mobzone, QPair<UVector, UVector>> adjacents; ///< Map of adjacent mobzones, and the intersection line
};

#endif // MOBZONE_H
