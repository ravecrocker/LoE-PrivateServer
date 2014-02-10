#include "quest.h"
#include "widget.h"
#include <QFile>

Quest::Quest(QString path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        win.logMessage("Error reading quest DB.");
        win.stopServer();
        return;
    }

    QList<QString> lines = QString(file.readAll().replace('\r',"")).split('\n');

    Pony* npc = new Pony;

    try
    {
        // Parse the npc commands, add everything else as quest commands
        // This is UGLY AS HELL, but I'm tired and it works.
        for (int i=0; i<lines.size(); i++)
        {
            QList<QString> line = lines[i].split(" ", QString::SkipEmptyParts);
            if (line[0] == "name")
                if (line.size()==2)
                    npc->name = line[1];
                else throw QString("Quest::Quest: Error reading name");
            else if (line[0] == "scene")
                if (line.size()==2)
                    npc->sceneName = line[1];
                else throw QString("Quest::Quest: Error reading scene");
            else if (line[0] == "ponyCode")
                if (line.size()==2)
                    npc->ponyData = QByteArray::fromBase64(line[1].toLatin1());
                else throw QString("Quest::Quest: Error reading ponyCode");
            else if (line[0] == "pos")
                if (line.size()==4)
                {
                    bool ok1, ok2, ok3;
                    npc->pos = UVector(line[1].toFloat(&ok1), line[2].toFloat(&ok2),
                                        line[3].toFloat(&ok3));
                    if (!(ok1 && ok2 && ok3))
                        throw QString("Quest::Quest: Error reading pos");
                }
                else throw QString("Quest::Quest: Error reading pos");
            else if (line[0] == "rot")
                if (line.size()==5)
                {
                    bool ok1, ok2, ok3, ok4;
                    npc->rot = UQuaternion(line[1].toFloat(&ok1), line[2].toFloat(&ok2),
                                            line[3].toFloat(&ok3), line[4].toFloat(&ok4));
                    if (!(ok1 && ok2 && ok3 && ok4))
                        throw QString("Quest::Quest: Error reading rot");
                }
                else throw QString("Quest::Quest: Error reading rot");
            else
                commands.append(line);
        }
    }
    catch (QString error)
    {
        win.logMessage(error);
        win.stopServer();
    }
}
