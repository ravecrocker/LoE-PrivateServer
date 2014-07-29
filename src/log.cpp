#include "log.h"
#include "app.h"

void logMessage(QString msg)
{
    app.logMessage(msg);
}

void logStatusMessage(QString msg)
{
    app.logStatusMessage(msg);
}

void logError(QString msg)
{
    app.logError(msg);
}

void logStatusError(QString msg)
{
    app.logStatusError(msg);
}
