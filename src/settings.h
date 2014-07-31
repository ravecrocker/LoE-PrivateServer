#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

#define DEFAULT_SYNC_INTERVAL 250

namespace Settings
{

extern int loginPort; // Port for the login server
extern int gamePort; // Port for the game server
extern QString remoteLoginIP; // IP of the remote login server
extern int remoteLoginPort; // Port of the remote login server
extern int remoteLoginTimeout; // Time before we give up connecting to the remote login server
extern bool useRemoteLogin; // Whether or not to use the remote login server
extern int maxConnected; // Max number of players connected at the same time, can deny login
extern int maxRegistered; // Max number of registered players in database, can deny registration
extern int pingTimeout; // Max time between recdption of pings, can disconnect player
extern int pingCheckInterval; // Time between ping timeout checks
extern bool logInfos; // Can disable logMessage, but doesn't affect logStatusMessage
extern QString saltPassword; // Used to check passwords between login and game servers, must be the same on all the servers involved
extern bool enableSessKeyValidation; // Enable Session Key Validation
extern bool enableLoginServer; // Starts a login server
extern bool enableGameServer; // Starts a game server
extern bool enableMultiplayer; // Sync players' positions
extern bool enableGetlog; // Enable GET /log requests
extern bool enablePVP; // Enables player versus player fights

}

#endif // SETTINGS_H
