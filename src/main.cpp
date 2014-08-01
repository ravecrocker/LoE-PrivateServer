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
    app.show();
#endif
    a.processEvents();
    app.startup();

    return a.exec(); // win's dtor will quick_exit (we won't run the atexits)
}
