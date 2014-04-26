#include "widget.h"
#include "utils.h"
#include "message.h"

void Widget::checkPingTimeouts()
{
    for (int i=0;i<udpPlayers.size();i++)
    {
        // Check ping timeouts
        int time = (timestampNow()-udpPlayers[i]->lastPingTime);
        if (time > pingTimeout)
        {
            logMessage(tr("UDP: Ping timeout (%1s) for %2 (player %3)")
                       .arg((int)timestampNow()-udpPlayers[i]->lastPingTime)
                       .arg(udpPlayers[i]->pony.netviewId).arg(udpPlayers[i]->name));
            udpPlayers[i]->connected = false;
            sendMessage(udpPlayers[i], MsgDisconnect, "You were kicked (Ping timeout)");
            Player::disconnectPlayerCleanup(udpPlayers[i]);
        }

        // Send a ping to prevent the client timing out on us
        sendMessage(udpPlayers[i], MsgPing);
    }
}
