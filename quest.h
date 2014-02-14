#ifndef QUEST_H
#define QUEST_H

#include <QString>
#include <QList>
#include "character.h"

// Quests are bound to one NPC
// They define the NPC, what he say, and the reactions.
struct Quest
{
public:
    Quest(QString path, Player* Owner);
    void runScript(); // Runs the script until the end or the next dialog
    bool doCommand(int eip); // Runs the command at eip. Returns false if we should stop running the script (e.g because we're waiting for an answer)
    void processAnswer(int answer); // Called when a client answers or clicks on a dialog
    int findLabel(QString label); // Returns the eip of this label

public:
    QList<QList<QString> >* commands; // List of commands and their arguments, parsed from the quest file.
    quint16 eip; // Instruction pointer inside the commands.
    quint16 state; // State (progress) of the quest.
    quint16 id; // Unique id of the quest.
    QString* name; // Name of the quest.
    QString* descr; // Description of the quest.
    Pony* npc; // NPC of this quest's script
    Player* owner; // Player who owns this quest save / runs the script
};

#endif // QUEST_H
