#ifndef RECEIVEACK_H
#define RECEIVEACK_H

#include <QByteArray>

/**
 * This unit contains handlers to clean up the reliable message sending queue
 * Acknowledged messages are removed from the queue
 **/

class Player;

void onConnectAckReceived(Player* player);
void onAckReceived(QByteArray msg, Player* player);

#endif // RECEIVEACK_H
