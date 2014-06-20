#ifndef LOG_H
#define LOG_H

#include <QString>

// Those functions will forward to the Widget singleton
// Most files will want a log, but we don't want to include widget.h everywhere
void logMessage(QString msg);
void logStatusMessage(QString msg);
void logError(QString msg);
void logStatusError(QString msg);

#endif // LOG_H
