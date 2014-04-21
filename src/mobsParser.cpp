#include <QStringList>
#include <QString>
#include "mob.h"
#include "mobzone.h"
#include "mobsParser.h"
#include "widget.h"

void parseMobzoneData(QByteArray data)
{
    QStringList lines = QString(data).replace('\r',"").split('\n');

    Mobzone* zone = new Mobzone;

    bool gotStart = false, gotEnd = false; // True when we have the start/end bounds

    for (QString line : lines)
    {
        // Skip empty lines and comments
        if (!line.size() || line[0]=='#')
            continue;

        if (line.startsWith("startPos "))
        {
            QStringList coords = line.mid(9).split(' ');
            if (coords.size() != 3)
                throw QString("parseMobzoneData(): error reading startPos");

            bool ok1, ok2, ok3;
            float x = coords[0].toFloat(&ok1);
            float y = coords[1].toFloat(&ok2);
            float z = coords[2].toFloat(&ok3);
            if (!ok1 || !ok2 || !ok3)
                throw QString("parseMobzoneData(): error reading startPos");

            zone->start = UVector{x,y,z};

            gotStart = true;
            if (gotEnd)
            {
                // "Normalize" the bounds. The start gets the smaller coordinates, the end gets the bigger
                UVector newStart = {std::min(zone->start.x, zone->end.x),
                                    std::min(zone->start.y, zone->end.y),
                                    std::min(zone->start.z, zone->end.z)};
                UVector newEnd = {std::max(zone->start.x, zone->end.x),
                                  std::max(zone->start.y, zone->end.y),
                                  std::max(zone->start.z, zone->end.z)};
                zone->start = newStart;
                zone->end = newEnd;
            }
        }
        else if (line.startsWith("endPos "))
        {
            QStringList coords = line.mid(7).split(' ');
            if (coords.size() != 3)
                throw QString("parseMobzoneData(): error reading endPos");

            bool ok1, ok2, ok3;
            float x = coords[0].toFloat(&ok1);
            float y = coords[1].toFloat(&ok2);
            float z = coords[2].toFloat(&ok3);
            if (!ok1 || !ok2 || !ok3)
                throw QString("parseMobzoneData(): error reading endPos");

            zone->end = UVector{x,y,z};

            gotEnd = true;
            if (gotStart)
            {
                // "Normalize" the bounds. The start gets the smaller coordinates, the end gets the bigger
                UVector newStart = {std::min(zone->start.x, zone->end.x),
                                    std::min(zone->start.y, zone->end.y),
                                    std::min(zone->start.z, zone->end.z)};
                UVector newEnd = {std::max(zone->start.x, zone->end.x),
                                  std::max(zone->start.y, zone->end.y),
                                  std::max(zone->start.z, zone->end.z)};
                zone->start = newStart;
                zone->end = newEnd;
            }
        }
        else if (line.startsWith("scene "))
        {
            line = line.mid(6);
            zone->sceneName = line;
        }
        else if (line.startsWith("mob "))
        {
            line = line.mid(4);
            Mob* mob = new Mob(zone);
            QStringList args = line.split(',');
            for (QString arg : args)
            {
                QStringList keyValuePair = arg.split('=');
                if (keyValuePair.size() != 2)
                    throw QString("parseMobzoneData(): error reading mob key/value pair");
                QString key = keyValuePair[0].trimmed();
                QString value = keyValuePair[1].trimmed();

                if (key=="type")
                    mob->setType("mobs/"+value);
                else
                    throw QString("parseMobzoneData(): error reading mob arg, unknown arg "+key);
                win.mobs << mob;
            }
        }
        else
        {
            throw QString("parseMobzoneData(): error, unknown statement "+line);
        }
    }

    win.mobzones << zone;
}
