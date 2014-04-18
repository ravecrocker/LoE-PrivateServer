#ifndef RECEIVECHATMESSAGE_H
#define RECEIVECHATMESSAGE_H

/**
  This file handles the raw chat RPC messages we receive.
  In particular, it handles chat commands.
**/

#include <QByteArray>

class Player;

void receiveChatMessage(QByteArray msg, Player* player);

#endif // RECEIVECHATMESSAGE_H
