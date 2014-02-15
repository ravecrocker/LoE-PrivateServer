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

Quest::~Quest()
{
    // Don't delete the commands, since they are shared between copies of the quest. We could use a share pointer.
    delete name;
    delete descr;
}

QString Quest::concatAfter(QList<QString> list, int id)
{
    QString result;
    if (list.size() <= id)
        return result;
    result = list[id];
    for (int i=id+1; i<list.size(); i++)
        result += " " + list[i];
    return result;
}

int Quest::findLabel(QString label)
{
    for (int i=0; i<commands->size(); i++)
        if ((*commands)[i].size()==2 && (*commands)[i][0] == "label" && (*commands)[i][1]==label)
            return i;
    return -1;
}

void Quest::logError(QString message)
{
    win.logMessage("Error running quest script "+QString().setNum(id)
                   +", eip="+QString().setNum(eip)+" : "+message);
}

void Quest::setOwner(Player* Owner)
{
    owner = Owner;
}

void Quest::runScript()
{
    runScript(0);
}

void Quest::runScript(int Eip)
{
    for (eip=Eip; eip<commands->size();)
        if (doCommand(eip))
            eip++;
        else
            break;
}

bool Quest::doCommand(int eip)
{
    if (!owner)
    {
        win.logMessage("Quest::doCommand called with no owner");
        return false;
    }

    //win.logMessage("Executing command "+QString().setNum(eip));

    QStringList command = (*commands)[eip];

    if (command[0] == "label")
        return true;
    else if (command[0] == "goto")
    {
        if (command.size() != 2)
        {
            logError("goto takes exactly one argument");
            return false;
        }
        int newEip = findLabel(command[1]);
        if (newEip == -1)
        {
            logError("label not found");
            return false;
        }
        eip = newEip;
    }
    else if (command[0] == "end")
    {
        sendEndDialog(owner);
    }
    else if (command[0] == "say")
    {
        QString msg, npcName;
        QList<QString> answers;
        quint16 iconId=0;
        if (command.size() >= 2)
        {
            msg = concatAfter(command, 1);
            npcName = npc->name;
        }
        else
        {
            logError("say takes 2 arguments");
            return false;
        }

        // Parse answers, icon, and name
        for (int i=eip+1; i<commands->size();i++)
        {
            if ((*commands)[i][0] == "answer")
                answers << concatAfter((*commands)[i], 2);
            else if ((*commands)[i][0] == "sayName")
                npcName = concatAfter((*commands)[i], 1);
            else if ((*commands)[i][0] == "sayIcon")
            {
                bool ok;
                int id = (*commands)[i][1].toInt(&ok);
                if (!ok || id < 0)
                {
                    logError("invalid icon id");
                    return false;
                }
                iconId = id;
            }
            else
                break;
        }

        sendBeginDialog(owner);
        sendDialogMessage(owner, msg, npcName);
        sendDialogMessage(owner, msg, npcName);
        sendDialogOptions(owner, answers);
        return false; // We stop the script until we get a reply
    }
    else if (command[0] == "answer") // can only be ran after a say, sayName, sayIcon, or another answer
    {
        logError("trying to run answer command by itself");
        return false;
    }
    else if (command[0] == "sayName") // can only be ran after a say, sayIcon, answer, or another sayName
    {
        logError("trying to run sayName command by itself");
        return false;
    }
    else if (command[0] == "sayIcon") // Answer can only be ran after a say, sayName, answer, or another sayIcon
    {
        logError("trying to run sayIcon command by itself");
        return false;
    }
    else
    {
        logError("unknown command");
        return false;
    }
    return true;
}

void Quest::processAnswer(int answer)
{
    int curAnswer = 0;
    for (int i=eip+1; i<commands->size();i++)
    {
        if ((*commands)[i][0] == "answer")
        {
            if (curAnswer == answer)
            {
                int newEip = findLabel((*commands)[i][1]);
                if (newEip == -1)
                {
                    logError("label not found for answer "+QString().setNum(answer));
                    sendEndDialog(owner);
                    return;
                }
                eip = newEip;
                runScript(eip);
                return;
            }
            else
                curAnswer++;
        }
        else if ((*commands)[i][0] == "sayName")
            continue;
        else if ((*commands)[i][0] == "sayIcon")
            continue;
        else
        {
            logError("answer "+QString().setNum(answer)+" not found");
            sendEndDialog(owner);
            return;
        }
    }
}

void Quest::processAnswer()
{
    for (int i=eip+1; i<commands->size();i++)
    {
        if ((*commands)[i][0] == "answer")
            continue;
        else if ((*commands)[i][0] == "sayName")
            continue;
        else if ((*commands)[i][0] == "sayIcon")
            continue;
        else
        {
            eip = i;
            runScript(eip);
            return;
        }
    }
}
