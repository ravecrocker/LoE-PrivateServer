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

    for (QString line : lines)
    {
        // Skip empty lines and comments
        if (!line.size() || line[0]=='#')
            continue;

        if (line.startsWith("startPos "))
        {
            QStringList coords = line.mid(9).split(' ');
            if (coords.size() != 3)
                throw QString(QObject::tr("parseMobzoneData(): error reading startPos"));

            bool ok1, ok2, ok3;
            float x = coords[0].toFloat(&ok1);
            float y = coords[1].toFloat(&ok2);
            float z = coords[2].toFloat(&ok3);
            if (!ok1 || !ok2 || !ok3)
                throw QString(QObject::tr("parseMobzoneData(): error reading startPos"));

            zone->start = UVector{x,y,z};
        }
        else if (line.startsWith("endPos "))
        {
            QStringList coords = line.mid(7).split(' ');
            if (coords.size() != 3)
                throw QString(QObject::tr("parseMobzoneData(): error reading endPos"));

            bool ok1, ok2, ok3;
            float x = coords[0].toFloat(&ok1);
            float y = coords[1].toFloat(&ok2);
            float z = coords[2].toFloat(&ok3);
            if (!ok1 || !ok2 || !ok3)
                throw QString(QObject::tr("parseMobzoneData(): error reading endPos"));

            zone->end = UVector{x,y,z};
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
                    throw QString(QObject::tr("parseMobzoneData(): error reading mob key/value pair"));
                QString key = keyValuePair[0].trimmed();
                QString value = keyValuePair[1].trimmed();

                if (key=="type")
                    mob->setType("mobs/"+value);
                else
                    throw QString(QObject::tr("parseMobzoneData(): error reading mob arg, unknown arg %1").arg(key));
                win.mobs << mob;
            }
        }
        else
        {
            throw QString(QObject::tr("parseMobzoneData(): error, unknown statement %1").arg(line));
        }
    }

    win.mobzones << zone;
}
