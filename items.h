#ifndef ITEMS_H
#define ITEMS_H

#include <QMap>
#include <QByteArray>
#include <cstdint>

QMap<uint32_t, uint32_t> parseItemsXml(QByteArray data);
uint8_t wearablePositionsToSlot(uint32_t positions);
uint32_t slotToWearablePositions(uint8_t slot);

#endif // ITEMS_H
