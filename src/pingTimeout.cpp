#include "widget.h"
#include "utils.h"
#include "message.h"
#include "player.h"
#include "settings.h"

using namespace Settings;

void Widget::checkPingTimeouts()
{
    for (int i=0;i<Player::udpPlayers.size();i++)
    {
        // Check ping timeouts
        int time = (timestampNow()-Player::udpPlayers[i]->lastPingTime);
        if (time > pingTimeout)
        {
            logMessage(tr("UDP: Ping timeout (%1s) for %2 (player %3)")
                       .arg((int)timestampNow()-Player::udpPlayers[i]->lastPingTime)
                       .arg(Player::udpPlayers[i]->pony.netviewId).arg(Player::udpPlayers[i]->name));
            Player::udpPlayers[i]->connected = false;
            sendMessage(Player::udpPlayers[i], MsgDisconnect, "You were kicked (Ping timeout)");
            Player::disconnectPlayerCleanup(Player::udpPlayers[i]);
        }
        else // Send a ping to prevent the client timing out on us
            sendMessage(Player::udpPlayers[i], MsgPing);
    }
}
