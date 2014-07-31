#include "app.h"
#include "player.h"
#include "message.h"
#include "utils.h"
#include "mob.h"
#include "sync.h"
#include "quest.h"
#include "settings.h"
#include "udp.h"
#include <QUdpSocket>

using namespace Settings;

#ifdef USE_CONSOLE
App::App() :
#else
App::App(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::App),
#endif
    cmdPeer(new Player()),
    sync{new Sync()}
{
    tcpServer = new QTcpServer(this);
    udpSocket = new QUdpSocket(this);
    tcpReceivedDatas = new QByteArray();
#ifdef USE_GUI
    ui->setupUi(this);
#else
    cin_notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
#endif

    pingTimer = new QTimer(this);

    qsrand(QDateTime::currentMSecsSinceEpoch());
    srand(QDateTime::currentMSecsSinceEpoch());
}

/// Adds the message in the log, and sets it as the status message
void App::logStatusMessage(QString msg)
{
#ifdef USE_GUI
    ui->log->appendPlainText(msg);
    ui->status->setText(msg);
    ui->log->repaint();
    ui->status->repaint();
#else
    cout << msg << endl;
#endif
}

/// Adds the message to the log
void App::logMessage(QString msg)
{
    if (!logInfos)
        return;
#ifdef USE_GUI
    ui->log->appendPlainText(msg);
    ui->log->repaint();
#else
    cout << msg << endl;
#endif
}

/// Adds the error in the log, and sets it as the status message
void App::logStatusError(QString msg)
{
#ifdef USE_GUI
    static QTextCharFormat defaultFormat, redFormat;
    defaultFormat.setForeground(QBrush(Qt::black));
    redFormat.setForeground(QBrush(Qt::red));
    ui->log->setCurrentCharFormat(redFormat);
    ui->log->appendPlainText(msg);
    ui->status->setText(msg);
    ui->log->repaint();
    ui->status->repaint();
    ui->log->setCurrentCharFormat(defaultFormat);
#else
    cout << "ERROR: " << msg << endl;
#endif
}

/// Adds the error to the log
void App::logError(QString msg)
{
    if (!logInfos)
        return;
#ifdef USE_GUI
    static QTextCharFormat defaultFormat, redFormat;
    defaultFormat.setForeground(QBrush(Qt::black));
    redFormat.setForeground(QBrush(Qt::red));
    ui->log->setCurrentCharFormat(redFormat);
    ui->log->appendPlainText(msg);
    ui->log->repaint();
    ui->log->setCurrentCharFormat(defaultFormat);
#else
    cout << "ERROR: " << msg << endl;
#endif
}

// Disconnect players, free the sockets, and exit quickly
// Does NOT run the atexits
App::~App()
{
    logInfos=false; // logMessage while we're trying to destroy would crash.
    //logMessage(tr("UDP: Disconnecting all players"));
    while (Player::udpPlayers.size())
    {
        Player* player = Player::udpPlayers[0];
        sendMessage(player, MsgDisconnect, "Server closed by the admin");

        // Save the pony
        QList<Pony> ponies = Player::loadPonies(player);
        for (int i=0; i<ponies.size(); i++)
            if (ponies[i].ponyData == player->pony.ponyData)
                ponies[i] = player->pony;
        Player::savePonies(player, ponies);
        player->pony.saveInventory();
        player->pony.saveQuests();

        // Free
        delete player;
        Player::udpPlayers.removeFirst();
    }

    for (int i=0; i<Quest::quests.size(); i++)
    {
        delete Quest::quests[i].commands;
        delete Quest::quests[i].name;
        delete Quest::quests[i].descr;
    }

    stopServer(false);
    delete tcpServer;
    delete tcpReceivedDatas;
    delete udpSocket;
    delete pingTimer;
    delete cmdPeer;

#ifdef USE_GUI
    delete ui;
#else
    delete cin_notifier;
#endif

    // We freed everything that was important, so don't waste time in atexits
#if defined WIN32 || defined _WIN32 || defined __APPLE__
    _exit(EXIT_SUCCESS);
#else
    quick_exit(EXIT_SUCCESS);
#endif
}

#ifdef USE_GUI
void App::on_clearLogButton_clicked()
{
    ui->log->clear();
}

void App::on_copyLogButton_clicked()
{
    // Can't just do log->copy() because that only copies highlighted text
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->log->toPlainText());
}

void App::on_saveLogButton_clicked()
{
    // Save server log to a file
    QString logFilename = QFileDialog::getSaveFileName(this, tr("Save Log"), "./log.txt", tr("Log files (*.txt)"));
    QFile logFile(logFilename);

    if (logFile.open(QIODevice::WriteOnly)) {
        QTextStream logFileStream(&logFile);
        logFileStream << ui->log->toPlainText() << "\n";

        logFile.close();
    }
    else
    {
        logMessage(tr("Failed to open log file '%1' for saving").arg(logFilename));
        return;
    }
}
#endif
