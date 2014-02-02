#include <QtWidgets/QApplication>
#include "widget.h"

int argc=0;

QApplication a(argc,(char**)0);
Widget win;

int main(int, char**)
{
    // Windows DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");

    win.show();
    win.startServer();

    return a.exec();
}

/**
  TODO :
  - Reuse a disconnected player's Id
  - If you create another pony with the same name as an existing one, overwrite the existing one
  - Read the items in Infos.txt
**/
