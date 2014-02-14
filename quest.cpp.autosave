#include "quest.h"
#include "widget.h"
#include <QFile>

Quest::Quest(QString path, Player *Owner)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        win.logMessage("Error reading quest DB.");
        win.stopServer();
        throw std::exception();
    }

    QList<QString> lines = QString(file.readAll().replace('\r',"")).split('\n');

    owner = Owner;
    commands = new QList<QList<QString> >;
    name = new QString();
    descr = new QString();
    npc = new Pony();
    npc->id = 0;
    npc->netviewId = 0;
    id = 0;
    state = 0;
    eip = 0;

    try
    {
        // Parse the metadata, add everything else as quest commands
        for (int i=0; i<lines.size(); i++)
        {
            QList<QString> line = lines[i].split(" ", QString::SkipEmptyParts);
            if (!line.size() || lines[i][0]=='#')
                continue;
            if (line[0] == "name")
                if (line.size()>=2)
                    npc->name = lines[i].mid(line[0].size()+1);
                else throw QString("Quest::Quest: Error reading name");
            else if (line[0] >= "scene")
                if (line.size()==2)
                    npc->sceneName = lines[i].mid(line[0].size()+1);
                else throw QString("Quest::Quest: Error reading scene");
            else if (line[0] == "ponyData")
                if (line.size()==2)
                    npc->ponyData = QByteArray::fromBase64(line[1].toLatin1());
                else throw QString("Quest::Quest: Error reading ponyData");
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
            else if (line[0] == "questId")
                if (line.size()==2)
                {
                    id = line[1].toInt();
                    win.lastIdMutex.lock();
                    npc->id = 0;
                    npc->netviewId = id;
                    if (win.lastNetviewId <= id)
                        win.lastNetviewId = id+1;
                    win.lastIdMutex.unlock();
                }
                else throw QString("Quest::Quest: Error reading questId");
            else if (line[0] == "questName")
                if (line.size()>=2)
                    *name = lines[i].mid(line[0].size()+1);
                else throw QString("Quest::Quest: Error reading questName");
            else if (line[0] == "questDescr")
                if (line.size()>=2)
                    *descr = lines[i].mid(line[0].size()+1);
                else throw QString("Quest::Quest: Error reading questDescr");
            else
                commands->append(line);
        }
    }
    catch (QString error)
    {
        win.logMessage(error);
        win.stopServer();
    }
}

void Quest::runScript()
{
    for (eip=0; eip<commands->size(); eip++)
        if (!doCommand(eip))
            break;
}

bool Quest::doCommand(int eip)
{
    QStringList command = (*commands)[eip];

    if (command[0] == "label")
        return true;
    else if (command[0] == "goto")
    {
        if (command.size() != 2)
        {
            win.logStatusMessage("Error running quest script : goto takes exactly one argument");
            return false;
        }
        int newEip = findLabel(command[1]);
        if (newEip == -1)
        {
            win.logStatusMessage("Error running quest script : unknown label in argument to goto");
            return false;
        }
        eip = newEip;
        return true;
    }
    else if (command[0] == "end")
    {
        sendEndDialog(owner);
    }
    else if (command[0] == "say")
    {
        // TODO: Parse the say, parse the eventual answers, if everything is fine, send the messages
        if (command.size() == 2)
        {

        }
        else if (command.size() == 3)
        {

        }
        else
        {
            win.logMessage("Error running quest script : say takes 2 or 3 arguments");
            return false;
        }
    }
    else if (command[0] == "answer") // Answer can only be ran after a say, or another answer
    {
        win.logMessage("Error running quest script : trying to run answer command by itself");
        return false;
    }
}

int Quest::findLabel(QString label)
{
    for (int i=0; i<commands->size(); i++)
        if ((*commands)[i].size()==2 && (*commands)[i][0] == "label" && (*commands)[i][1]==label)
            return i;
    return -1;
}
