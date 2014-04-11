#include "items.h"
#include "dataType.h"
#include "widget.h"
#include <QString>
#include <QStringList>

QMap<uint32_t, uint32_t> parseItemsXml(QByteArray data)
{
    QMap<uint32_t, uint32_t> map;

    while (1)
    {
        int idIndex = data.indexOf("<ID>")+4;
        int slotsIndex = data.indexOf("<WearableSlots>")+15;
        if (idIndex == -1 || slotsIndex == -1)
            break;
        int idIndexEnd = data.indexOf("</ID>", idIndex);
        int slotsIndexEnd = data.indexOf("</WearableSlots>", slotsIndex);
        if (idIndexEnd == -1 || slotsIndexEnd == -1)
            break;

        QString idStr = data.mid(idIndex, idIndexEnd - idIndex);
        QString slotsStr = data.mid(slotsIndex, slotsIndexEnd - slotsIndex);

        //win.logMessage("Found id "+idStr+", slots : "+slotsStr);

        bool ok;
        int id = idStr.toInt(&ok);
        if (!ok)
            break;

        QStringList slotsList = slotsStr.split(' ');
        if (slotsList.isEmpty())
            break;

        uint32_t itemSlots;
        for (QString slot : slotsList)
        {
            if (slot == "None")
            {
                itemSlots = 0;
                break;
            }
            else if (slot == "Tail")    itemSlots |= (uint32_t)WearablePositions::Tail;
            else if (slot == "Pants")   itemSlots |= (uint32_t)WearablePositions::Pants;
            else if (slot == "FrontSocks")   itemSlots |= (uint32_t)WearablePositions::FrontSocks;
            else if (slot == "BackSocks")   itemSlots |= (uint32_t)WearablePositions::BackSocks;
            else if (slot == "FrontShoes")   itemSlots |= (uint32_t)WearablePositions::FrontShoes;
            else if (slot == "BackShoes")   itemSlots |= (uint32_t)WearablePositions::BackShoes;
            else if (slot == "Saddle")   itemSlots |= (uint32_t)WearablePositions::Saddle;
            else if (slot == "Shirt")   itemSlots |= (uint32_t)WearablePositions::Shirt;
            else if (slot == "Necklace")   itemSlots |= (uint32_t)WearablePositions::Necklace;
            else if (slot == "Mouth")   itemSlots |= (uint32_t)WearablePositions::Mouth;
            else if (slot == "Mask")   itemSlots |= (uint32_t)WearablePositions::Mask;
            else if (slot == "Eyes")   itemSlots |= (uint32_t)WearablePositions::Eyes;
            else if (slot == "Ears")   itemSlots |= (uint32_t)WearablePositions::Ears;
            else if (slot == "FrontKnees")   itemSlots |= (uint32_t)WearablePositions::FrontKnees;
            else if (slot == "BackKnees")   itemSlots |= (uint32_t)WearablePositions::BackKnees;
            else if (slot == "SaddleBags")   itemSlots |= (uint32_t)WearablePositions::SaddleBags;
            else if (slot == "Hat")   itemSlots |= (uint32_t)WearablePositions::Hat;
            else
            {
                win.logMessage("Unknown wearable slots while parsing Items.xml");
            }
        }
        map[id] = itemSlots;
        data = data.mid(slotsIndexEnd);
    }

    return map;
}
