#ifndef DATATYPE_H
#define DATATYPE_H

#include <qglobal.h>

class UVector
{
public:
    UVector();
    UVector(float ux, float uy, float uz);

public:
    float x;
    float y;
    float z;
};
typedef struct UVector UVector;

class UQuaternion
{
public:
    UQuaternion();
    UQuaternion(float ux, float uy, float uz, float uw);

public:
    float x;
    float y;
    float z;
    float w;
};
typedef struct UQuaternion UQuaternion;

class WearableItem
{
public:
    WearableItem();
    WearableItem(quint8 Index, quint32 Id);
    bool operator==(const WearableItem& other);

public:
    quint8 index;
    quint32 id;
};

class InventoryItem : public WearableItem
{
public:
    InventoryItem();
    InventoryItem(quint8 Index, quint32 Id);
    InventoryItem(quint8 Index, quint32 Id, quint32 Amount);

public:
    quint32 amount;
};

struct MessageHead
{
    quint16 channel;
    quint16 seq;

    bool operator==(const MessageHead& other)
    {
        return (channel==other.channel && seq==other.seq);
    }
};

#endif // DATATYPE_H
