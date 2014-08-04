#include <QTranslator>
#include <QDir>
#include "app.h"

int argc = 0;

#ifdef USE_CONSOLE
QTextStream cout(stdout);
QTextStream cin(stdin);
#endif

QAPP_TYPE a(argc,(char**)0);
App app;

int main(int, char**)
{
    // Windows DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");

    // Translation
    QString locale = QLocale::system().name().section('_', 0, 0);

    QTranslator translator;
    translator.load("translations/"+locale);
    a.installTranslator(&translator);

    // Running the server
#ifdef USE_GUI
    // Hacky OSX Stylesheet Fixing
    #ifdef __APPLE__
    app.ui->BaseAppFrame->setStyleSheet(app.ui->BaseAppFrame->styleSheet() +
        "QToolButton { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1," +
            "stop:0 rgba(255, 255, 255, 255), stop:0.450 rgba(243, 243, 243, 255)," +
            "stop:0.550 rgba(236, 236, 236, 255), stop:1 rgba(242, 242, 242, 255));" +
            "padding: 2px 3px; border-radius: 4px; border: 1px solid #9a9a9a; }" +
        "QToolButton:pressed { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1," +
            "stop:0 rgb(188, 215, 237), stop:0.450 rgb(107, 164, 228)," +
            "stop:0.550 rgb(69, 148, 227), stop:1 rgb(167, 212, 237));" +
            "border: 1px solid rgb(75, 78, 143);" +
            "border-top: 1px solid rgb(80, 87, 168);" +
            "border-bottom: 1px solid rgb(69, 74, 112); }");

        // OSX Menubar
        // This is required so we don't crash when we quit
        QMenuBar *menuBar = new QMenuBar(0);
        QMenu *menu = menuBar->addMenu(a.tr("&File"));

        QAction *quitAction = menu->addAction(a.tr("&Quit"));
        quitAction->setStatusTip(a.tr("Shutdown private server and quit"));
        a.connect(quitAction, SIGNAL(triggered()), &app, SLOT(shutdown()));
    #endif

    app.show();
#endif
    a.processEvents();
    app.startup();

    return a.exec(); // win's dtor will quick_exit (we won't run the atexits)
}
