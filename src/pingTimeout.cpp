#include "widget.h"
#include "utils.h"
#include "message.h"

void Widget::checkPingTimeouts()
{
    for (int i=0;i<udpPlayers.size();i++)
    {
        // Check ping timeout
        int time = (timestampNow()-udpPlayers[i]->lastPingTime);
        //win.logMessage(QString().setNum(time)+"s");
        if (time > pingTimeout)
        {
            logMessage("UDP: Ping timeout ("+QString().setNum(((int)timestampNow()-udpPlayers[i]->lastPingTime))+"s) for "
                       +QString().setNum(udpPlayers[i]->pony.netviewId)+" (player "+udpPlayers[i]->name+")");
            udpPlayers[i]->connected = false;
            sendMessage(udpPlayers[i], MsgDisconnect, "You were kicked (Ping timeout)");
            Player::disconnectPlayerCleanup(udpPlayers[i]);
        }

        // Send a ping to prevent the client timing out on us
        sendMessage(udpPlayers[i], MsgPing);
    }
}
