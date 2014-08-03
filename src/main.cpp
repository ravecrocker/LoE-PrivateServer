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
        "QToolButton { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1," +
            "stop: 0 #f6f7fa, stop: 1 #dadbde); padding: 2px 3px; border-radius: 4px;" +
            "border: 1px solid rgba(20, 20, 30, 110); }" +
        "QToolButton:pressed { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1," +
            "stop: 0 #e6e7ea, stop: 1 #cacbce); }");
    #endif

    app.show();
#endif
    a.processEvents();
    app.startup();

    return a.exec(); // win's dtor will quick_exit (we won't run the atexits)
}
