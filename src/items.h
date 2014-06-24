#ifndef ITEMS_H
#define ITEMS_H

#include <QMap>
#include <QByteArray>
#include <cstdint>

void parseItemsXml(QByteArray data);
uint8_t wearablePositionsToSlot(uint32_t positions);
uint32_t slotToWearablePositions(uint8_t slot);

extern QMap<uint32_t, uint32_t> wearablePositionsMap; // Maps item IDs to their wearable positions
extern QMap<uint32_t, uint32_t> itemsPricesMap; // Maps item IDs to their prices

#endif // ITEMS_H
