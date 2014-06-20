#include "widget.h"
#include "ui_widget.h"
#include "skillparser.h"
#include "mobsParser.h"
#include "animationparser.h"
#include "animation.h"
#include "items.h"
#include "settings.h"
#include "sceneEntity.h"
#include "scene.h"
#include "quest.h"
#include "player.h"
#include "mob.h"
#include "sync.h"
#include "udp.h"
#include <QUdpSocket>
#include <QSettings>
#include <QDir>

#if defined _WIN32 || defined WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace Settings;

/// Reads the config file (server.ini) and start the server accordingly
void Widget::startServer()
{
    ui->retranslateUi(this);

    logStatusMessage(tr("Private server")+" v0.5.3-beta1");
#ifdef __APPLE__
    // this fixes the directory in OSX so we can use the relative CONFIGFILEPATH and etc properly
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);
    // the path we get is to the .app folder, so we go up after we chdir
    chdir(path);
    chdir("..");
#endif
    SceneEntity::lastNetviewId=0;
    SceneEntity::lastId=1;

    /// Read config
    logStatusMessage(tr("Reading config file ..."));
    QSettings config(CONFIGFILEPATH, QSettings::IniFormat);
    loginPort = config.value("loginPort", 1034).toInt();
    gamePort = config.value("gamePort", 1039).toInt();
    maxConnected = config.value("maxConnected",128).toInt();
    maxRegistered = config.value("maxRegistered",2048).toInt();
    pingTimeout = config.value("pingTimeout", 15).toInt();
    pingCheckInterval = config.value("pingCheckInterval", 5000).toInt();
    logInfos = config.value("logInfosMessages", true).toBool();
    saltPassword = config.value("saltPassword", "Change Me").toString();
    enableSessKeyValidation = config.value("enableSessKeyValidation", true).toBool();
    enableLoginServer = config.value("enableLoginServer", true).toBool();
    enableGameServer = config.value("enableGameServer", true).toBool();
    enableMultiplayer = config.value("enableMultiplayer", true).toBool();
    syncInterval = config.value("syncInterval",DEFAULT_SYNC_INTERVAL).toInt();
    remoteLoginIP = config.value("remoteLoginIP", "127.0.0.1").toString();
    remoteLoginPort = config.value("remoteLoginPort", 1034).toInt();
    remoteLoginTimeout = config.value("remoteLoginTimeout", 5000).toInt();
    useRemoteLogin = config.value("useRemoteLogin", false).toBool();
    enableGetlog = config.value("enableGetlog", true).toBool();
    enablePVP = config.value("enablePVP", true).toBool();

    /// Init servers
    tcpClientsList.clear();
#if defined _WIN32 || defined WIN32
    startTimestamp = GetTickCount();
#elif __APPLE__
    timeval time;
    gettimeofday(&time, NULL);
    startTimestamp = (time.tv_sec * 1000) + (time.tv_usec / 1000);
#else
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    startTimestamp = tp.tv_sec*1000 + tp.tv_nsec/1000/1000;
#endif

    /// Read vortex DB
    if (enableGameServer)
    {
        bool corrupted=false;
        QDir vortexDir("data/vortex/");
        QStringList files = vortexDir.entryList(QDir::Files);
        int nVortex=0;
        for (int i=0; i<files.size(); i++) // For each vortex file
        {
            // Each file is a scene
            Scene scene(files[i].split('.')[0]);

            QFile file("data/vortex/"+files[i]);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                logStatusMessage(tr("Error reading vortex DB"));
                return;
            }
            QByteArray data = file.readAll();
            data.replace('\r', "");
            QList<QByteArray> lines = data.split('\n');

            // Each line is a vortex
            for (int j=0; j<lines.size(); j++)
            {
                if (lines[j].size() == 0) // Skip empty lines
                    continue;
                nVortex++;
                Vortex vortex;
                bool ok1, ok2, ok3, ok4;
                QList<QByteArray> elems = lines[j].split(' ');
                if (elems.size() < 5)
                {
                    logStatusMessage(tr("Vortex DB is corrupted. Incorrect line (%1 elems), file %2")
                                        .arg(elems.size()).arg(files[i]));
                    corrupted=true;
                    break;
                }
                vortex.id = elems[0].toInt(&ok1, 16);
                vortex.destName = elems[1];
                for (int j=2; j<elems.size() - 3;j++) // Concatenate the string between id and poss
                    vortex.destName += " "+elems[j];
                vortex.destPos.x = elems[elems.size()-3].toFloat(&ok2);
                vortex.destPos.y = elems[elems.size()-2].toFloat(&ok3);
                vortex.destPos.z = elems[elems.size()-1].toFloat(&ok4);
                if (!(ok1&&ok2&&ok3&&ok4))
                {
                    logStatusMessage(tr("Vortex DB is corrupted. Conversion failed, file %1").arg(files[i]));
                    corrupted=true;
                    break;
                }
                scene.vortexes << vortex;
                //win.logMessage("Add vortex "+QString().setNum(vortex.id)+" to "+vortex.destName+" "
                //               +QString().setNum(vortex.destPos.x)+" "
                //               +QString().setNum(vortex.destPos.y)+" "
                //               +QString().setNum(vortex.destPos.z));
            }
            Scene::scenes << scene;
        }

        if (corrupted)
        {
            stopServer();
            return;
        }

        logMessage(tr("Loaded %1 vortexes in %2 scenes").arg(nVortex).arg(Scene::scenes.size()));
    }

    /// Read/parse Items.xml
    if (enableGameServer)
    {
        QFile itemsFile("data/data/Items.xml");
        if (itemsFile.open(QIODevice::ReadOnly))
        {
            QByteArray data = itemsFile.readAll();
            wearablePositionsMap = parseItemsXml(data);
            win.logMessage(tr("Loaded %1 items").arg(wearablePositionsMap.size()));
        }
        else
        {
            win.logMessage(tr("Couln't open Items.xml"));
            stopServer();
            return;
        }
    }

    /// Read NPC/Quests DB
    if (enableGameServer)
    {
        try
        {
            unsigned nQuests = 0;
            QDir npcsDir("data/npcs/");
            QStringList files = npcsDir.entryList(QDir::Files);
            for (int i=0; i<files.size(); i++, nQuests++) // For each vortex file
            {
                try
                {
                    Quest quest("data/npcs/"+files[i], NULL);
                    for (const Quest& q : Quest::quests)
                        if (q.id == quest.id)
                            logMessage(tr("Error, two quests are using the same id (%1) !").arg(quest.id));
                    Quest::quests << quest;
                    Quest::npcs << quest.npc;
                }
                catch (QString& error)
                {
                    win.logMessage(error);
                    win.stopServer();
                    throw error;
                }
            }
            logMessage(tr("Loaded %1 quests/npcs").arg(nQuests));
        }
        catch (QString& e)
        {
            enableGameServer = false;
        }
    }

    /// Read/parse mob zones
    if (enableGameServer)
    {
        try
        {
            QDir mobsDir(MOBSPATH);
            QStringList files = mobsDir.entryList(QDir::Files);
            for (int i=0; i<files.size(); i++) // For each mobzone file
            {
                QFile file(MOBSPATH+files[i]);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    logStatusMessage(tr("Error reading mob zones"));
                    return;
                }
                QByteArray data = file.readAll();
                file.close();
                try {
                    parseMobzoneData(data); // Will fill our Mobzone and Mobs list
                }
                catch (QString& error)
                {
                    logMessage(error);
                    stopServer();
                    throw error;
                }
            }
            logMessage(tr("Loaded %1 mobs in %2 zones").arg(Mob::mobs.size()).arg(Mob::mobzones.size()));
        }
        catch (...) {}
    }

    // Parse animations
    if (enableGameServer)
    {
        try
        {
            AnimationParser(GAMEDATAPATH+QString("Animations.json"));
            logMessage(tr("Loaded %1 animations").arg(Animation::animations.size()));
        }
        catch (const QString& e)
        {
            logMessage(tr("Error parsing animations: ")+e);
            win.stopServer();
        }
        catch (const char* e)
        {
            logMessage(tr("Error parsing animations: ")+e);
            win.stopServer();
        }
    }

    // Parse skills
    if (enableGameServer)
    {
        try
        {
            SkillParser(GAMEDATAPATH+QString("Skills.json"));
            logMessage(tr("Loaded %1 skills").arg(Skill::skills.size()));
        }
        catch (const QString& e)
        {
            logMessage(tr("Error parsing skills: ")+e);
            win.stopServer();
        }
        catch (const char* e)
        {
            logMessage(tr("Error parsing skills: ")+e);
            stopServer();
        }
    }

    if (enableLoginServer)
    {
//      logStatusMessage(tr("Loading players database ..."));
        Player::tcpPlayers = Player::loadPlayers();
    }

    // TCP server
    if (enableLoginServer)
    {
        logStatusMessage(tr("Starting TCP login server on port %1...").arg(loginPort));
        if (!tcpServer->listen(QHostAddress::Any,loginPort))
        {
            logStatusMessage(tr("TCP: Unable to start server on port %1 : %2").arg(loginPort).arg(tcpServer->errorString()));
            stopServer();
            return;
        }

        // If we use a remote login server, try to open a connection preventively.
        if (useRemoteLogin)
            remoteLoginSock.connectToHost(remoteLoginIP, remoteLoginPort);
    }

    // UDP server
    if (enableGameServer)
    {
        logStatusMessage(tr("Starting UDP game server on port %1...").arg(gamePort));
        if (!udpSocket->bind(gamePort, QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress))
        {
            logStatusMessage(tr("UDP: Unable to start server on port %1").arg(gamePort));
            stopServer();
            return;
        }
    }

    if (enableGameServer)
    {
        // Start ping timeout timer
        pingTimer->start(pingCheckInterval);
    }

    if (enableMultiplayer)
        sync->startSync(syncInterval);

    if (enableLoginServer || enableGameServer)
        logStatusMessage(tr("Server started"));

    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendCmdLine()));
    if (enableLoginServer)
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(tcpConnectClient()));
    if (enableGameServer)
    {
        connect(udpSocket, &QUdpSocket::readyRead, &::udpProcessPendingDatagrams);
        connect(pingTimer, SIGNAL(timeout()), this, SLOT(checkPingTimeouts()));
    }
}

void Widget::stopServer()
{
    stopServer(true);
}

void Widget::stopServer(bool log)
{
    if (log)
        logStatusMessage(tr("Stopping all server operations"));
    pingTimer->stop();
    tcpServer->close();
    for (int i=0;i<tcpClientsList.size();i++)
        tcpClientsList[i].first->close();
    udpSocket->close();

    sync->stopSync();

    enableLoginServer = false;
    enableGameServer = false;

    disconnect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendCmdLine()));
    disconnect(udpSocket);
    disconnect(tcpServer, SIGNAL(newConnection()), this, SLOT(tcpConnectClient()));
    disconnect(pingTimer, SIGNAL(timeout()), this, SLOT(checkPingTimeouts()));
    disconnect(this);
}
