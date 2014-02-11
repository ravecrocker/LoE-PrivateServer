#ifndef UTILS_H
#define UTILS_H

void saveResourceToDataFolder(QString resRelPath);
QByteArray removeHTTPHeader(QByteArray data,QString header);
char convertChar (char c, bool direction = true);

#endif // UTILS_H
