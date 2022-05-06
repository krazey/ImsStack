/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
    This file implements a container class using hash table internally.
*/

#ifndef _IMS_HASH_MAP_H_
#define _IMS_HASH_MAP_H_

#include "IMSTypeDef.h"

struct __IMSIterator
{
    IMS_UINT32 nDummy;
};

typedef struct __IMSIterator* IMSIterator;

class IMSHashMap
{
protected:
    class Element
    {
    public:
        Element(IN void* pvKey_, IN void* pvValue_) :
                pvKey(pvKey_),
                pvValue(pvValue_),
                pNext(IMS_NULL)
        {
        }
        ~Element() {}

    public:
        void* pvKey;
        void* pvValue;
        Element* pNext;
    };

public:
    IMSHashMap(IN IMS_UINT32 nHashTableSize_ = DEFAULT_SIZE);
    virtual ~IMSHashMap();

public:
    void InitHashTable(IN IMS_UINT32 nHashTableSize, IN IMS_BOOL bAlloc = IMS_TRUE);
    inline IMS_UINT32 GetHashTableSize() const { return nHashTableSize; }

    // Add new (key, value) pair or modify value
    void SetAt(IN void* pvKey, IN void* pvNewValue);
    void* GetValueAt(IN void* pvKey);

    // Search key and return value
    IMS_BOOL Lookup(IN void* pvKey, OUT void*& pvValue);
    IMS_BOOL Lookup(IN void* pvKey, OUT void*& pvOrigKey, OUT void*& pvValue);

    // Remove existing (key, ?) pair
    IMS_BOOL RemoveKey(IN void* pvKey);
    void RemoveAll();

    inline IMS_BOOL IsEmpty() const { return (nCount == 0); }
    inline IMS_UINT32 GetSize() const { return nCount; }

    IMSIterator GetStartPosition();
    void GetNext(IN_OUT IMSIterator& stIterator, OUT void*& pvKey, OUT void*& pvValue);

protected:
    virtual IMS_UINTP GetHashKey(IN void* pvKey);
    virtual Element* GetElementAt(IN void* pvKey, OUT IMS_UINT32& nBucket);

protected:
    IMS_UINT32 nHashTableSize;
    Element** ppHashTable;

private:
    enum
    {
        DEFAULT_SIZE = 5
    };

    IMS_UINT32 nCount;
};

#endif  // _IMS_HASH_MAP_H_
