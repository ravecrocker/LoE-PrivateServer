Legends of Equestria Private Server
===================================

This is a Private Server, to play LoE even when the official servers are closed.
The official release is for Windows x86. The server should work on Linux and Mac too, but you'll need to compile it yourself.

<b>The current release of the server DOES NOT work with the latest version of the game.</b><br/>
It will work if you compile it from source yourself, but it might be buggy.
Expect a working release at the end of the open weekend.

<h3>How to use</h3>
Extract in the Legends of Equestria folder, start PrivateServer.exe and the game. 
In the game pick a name/password (no need to register first)
Then use either :
- the Local server, for singleplayer or a multiplayer LAN in your house
- or the Online server, for multiplayer (but it's often offline, since I can't host it myself)
- You can also add your own servers in data/serversList.cfg

<h3>Still in Beta</h3>
The private server is still in beta, and some important features are lacking :
- Some places (Tartarus, Twilight's Library,..) are not accessible without using the server commands.
- No skills (no unicorn magic) except flying for pegasi.
- No monsters or NPCs without server commands.
- No quests.

<h3>Server commands</h3>
You don't need any of those commands to play, but they might be usefull.
setPeer is used to select a client. Most commands will only act on the selected client.
For example if you're stuck, do setPeer with your IP and port, then do for example "load PonyVille".
- clear : Clears the server's log.
- listPeers : Give the list of all the clients (= other players) connected
- setPeer : If there's only 1 client, select him for the other commands. If there's more than 1, use "setPeer IP port".
- setPeer IP port : Select the client at this IP and port. The other commands will act on the selected client.
- disconnect : Kick the player with the message "Connection closed by the server admin". Does not ban.
- load scene : Loads a scene (PonyVille, Cloudsdale, ...) and teleport to it's spawn. See the list of scenes below.
- getPos : Get the position (x, y, z) of the player. Often used with "move x y z".
- move x y z : Instantly teleport the player to the given position. Not a spell. You just move.
- setStat statId value : Set the given stat (health, mana, ..) to the given value. 
- setMaxStat statId value : Set the max of the given stat (health, mana, ..) to the given value. 
- error message : Sends a message-box-scroll-thingy to the player with the title "Error" and the given message. Doesn't disconnect.

<br/>The following commands are for debugging only. You really don't need them, and most of the time you don't want to use them.
- sync : Syncs the position of all the clients now. Doesn't need setPeer to work.
- sendPonies : For debugging only.
- sendPonyData : For debugging only.
- sendUtils3 : For debugging only.
- setPlayerId id : For debugging only. Change the player's netview id.
- remove id : For debugging only. Remove the entity with this netview id from the selected player's point of view.
- instantiate : For debugging only. Spawns the player's body. Will clone the body if used repeatedly. Will lag. Oh god the lag.
- instantiate key viewId ownerId : Will spawn key (PlayerBase for a pony). If the Ids are already taken, bad things will happen.
- instantiate key viewId ownerId x y z : Same than above but spawn at the given position.
- instantiate key viewId ownerId x y z rx ry rz: Same than above but spawn at the given position and rotation.
- beginDialog : For debugging only.
- endDialog : For debugging only.
- setDialogMsg message : For debugging only.

<h3>List of scenes</h3>
To use with the command "load scene".
PM-Lvl1 is the Pony Muncher game (Pac Man).
If you land in the void after loading a scene, try "move 0 0 0" (and either add an issue on GitHub or complain at mlpfightingismagic@gmail.com)
- PonyVille
- SugarCubeCorner
- GemMines
- Appaloosa
- SweetAppleAcres
- Everfree1
- Zecoras
- Everfree3
- Tartarus
- RaritysBoutique
- Canterlot
- Cottage
- Cloudsdale
- Ponyville Library 1st floor
- Ponyville Library 2nd floor
- minigameLoader
- PM-Lvl1

<img src="https://f.cloud.github.com/assets/5155966/1389911/6503cd02-3be3-11e3-987f-98611a94a106.jpg"/>
