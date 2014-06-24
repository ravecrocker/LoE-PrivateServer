#include "log.h"
#include "widget.h"

void logMessage(QString msg)
{
    win.logMessage(msg);
}

void logStatusMessage(QString msg)
{
    win.logStatusMessage(msg);
}

void logError(QString msg)
{
    win.logError(msg);
}

void logStatusError(QString msg)
{
    win.logStatusError(msg);
}
