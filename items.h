#ifndef ITEMS_H
#define ITEMS_H

#include <QMap>
#include <QByteArray>
#include <cstdint>

QMap<uint32_t, uint32_t> parseItemsXml(QByteArray data);

#endif // ITEMS_H
