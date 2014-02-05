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
    bool operator==(const WearableItem& other);

public:
    quint8 index;
    quint32 id;
};

class InventoryItem : public WearableItem
{
public:
    InventoryItem();

public:
    quint32 amount;
};

#endif // DATATYPE_H
