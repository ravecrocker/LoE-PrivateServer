#include "widget.h"
#include "ui_widget.h"

void saveResourceToDataFolder(QString resRelPath)
{
    win.logMessage(QString("Saving ")+resRelPath);
    QString dataPath = QDir::homePath()+"/AppData/LocalLow/LoE/Legends of Equestria/";
    QFile::remove(dataPath+resRelPath);
    QFile::copy(QString(":/gameFiles/")+resRelPath, dataPath+resRelPath);
    //SetFileAttributesA(QString(dataPath+resRelPath).toStdString().c_str(),FILE_ATTRIBUTE_NORMAL);
}

QByteArray removeHTTPHeader(QByteArray data,QString header)
{
    int i1 = data.indexOf(header);
    if (i1==-1) return data;
    int i2 = data.indexOf("\n", i1);
    if (i2==-1) return data;
    data.remove(i1, i2-i1+1);
    return data;
}

char convertChar (char c, bool direction = true)
{
    if (direction)
    {
        if (c >= 'a' && c <= 'z')
        {
            return c - 'a';
        }
        if (c >= 'A' && c <= 'Z')
        {
            return c - 'A' + '\x1a';  // \u001a
        }
        if (c >= '0' && c <= '9')
        {
            return c - '0' + '4';
        }
        return '>';
    }
    else
    {
        if (c >= '\0' && c <= '\x19')  // \u0019
        {
            return c + 'a';
        }
        if (c >= '\x1a' && c <= '3')  // \u001a
        {
            return c + 'A' - '\x1a';  // u001a
        }
        if (c >= '4' && c <= '=')
        {
            return c + '0' - '4';
        }
        return ' ';
    }
}
