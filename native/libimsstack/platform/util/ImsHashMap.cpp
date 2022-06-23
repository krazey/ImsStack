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

#include "ServiceMemory.h"
#include "ImsHashMap.h"

PUBLIC
IMSHashMap::IMSHashMap(IN IMS_UINT32 nHashTableSize_ /* = DEFAULT_SIZE */) :
        nHashTableSize(nHashTableSize_),
        ppHashTable(IMS_NULL),
        nCount(0)
{
}

PUBLIC
IMSHashMap::~IMSHashMap()
{
    RemoveAll();
}

// Initialize a hash table
PUBLIC
void IMSHashMap::InitHashTable(IN IMS_UINT32 nHashTableSize, IN IMS_BOOL bAlloc /* = IMS_TRUE */)
{
    this->nHashTableSize = nHashTableSize;

    if (this->nHashTableSize == 0)
    {
        this->nHashTableSize = DEFAULT_SIZE;
    }

    if (bAlloc == IMS_TRUE)
    {
        ppHashTable = (Element**)new Element*[this->nHashTableSize];

        if (ppHashTable != IMS_NULL)
        {
            for (IMS_UINT32 i = 0; i < this->nHashTableSize; ++i)
            {
                ppHashTable[i] = IMS_NULL;
            }
        }
    }
}

// Add new (key, value) pair or modify value
PUBLIC
void IMSHashMap::SetAt(IN void* pvKey, IN void* pvNewValue)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    // add new pair
    if (pElement == IMS_NULL)
    {
        if (ppHashTable == IMS_NULL)
        {
            InitHashTable(nHashTableSize);
        }

        if (ppHashTable != NULL)
        {
            pElement = new Element(pvKey, pvNewValue);

            pElement->pNext = ppHashTable[nBucket];
            ppHashTable[nBucket] = pElement;

            nCount++;
        }
    }
    // modify value field
    else
    {
        pElement->pvValue = pvNewValue;
    }
}

// Get a value which has the same key
PUBLIC
void* IMSHashMap::GetValueAt(IN void* pvKey)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    if (pElement == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pElement->pvValue;
}

// Search key and return value
PUBLIC
IMS_BOOL IMSHashMap::Lookup(IN void* pvKey, OUT void*& pvValue)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    if (pElement == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pvValue = pElement->pvValue;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IMSHashMap::Lookup(IN void* pvKey, OUT void*& pvOrigKey, OUT void*& pvValue)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    if (pElement == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pvOrigKey = pElement->pvKey;
    pvValue = pElement->pvValue;

    return IMS_TRUE;
}

// Remove existing (key, ?) pair
PUBLIC
IMS_BOOL IMSHashMap::RemoveKey(IN void* pvKey)
{
    IMS_UINTP nHashKey;
    IMS_UINT32 nBucket;
    Element* pElement;
    Element** ppPrevElement;

    if (ppHashTable == IMS_NULL)
    {
        return IMS_FALSE;
    }

    nHashKey = GetHashKey(pvKey);
    nBucket = static_cast<IMS_UINT32>(nHashKey % nHashTableSize);

    ppPrevElement = &(ppHashTable[nBucket]);

    for (pElement = (*ppPrevElement); pElement != IMS_NULL; pElement = pElement->pNext)
    {
        if (pElement->pvKey == pvKey)
        {
            (*ppPrevElement) = pElement->pNext;

            delete pElement;

            nCount--;

            return IMS_TRUE;
        }

        ppPrevElement = &(pElement->pNext);
    }

    return IMS_FALSE;
}

// Remove all the elements
PUBLIC
void IMSHashMap::RemoveAll()
{
    if (ppHashTable != IMS_NULL)
    {
        IMSIterator stIterator;
        Element* pElement;
        void* pvKey;
        void* pvValue;

        stIterator = GetStartPosition();
        while (stIterator != IMS_NULL)
        {
            pElement = reinterpret_cast<Element*>(stIterator);

            GetNext(stIterator, pvKey, pvValue);

            delete pElement;
        }

        delete[] ppHashTable;
        ppHashTable = IMS_NULL;
    }

    nCount = 0;
}

// Get a start position from the hash table
PUBLIC
IMSIterator IMSHashMap::GetStartPosition()
{
    IMS_UINT32 nBucket;
    Element* pElement = IMS_NULL;

    if (nCount == 0)
    {
        return IMS_NULL;
    }

    for (nBucket = 0; nBucket < nHashTableSize; ++nBucket)
    {
        if ((pElement = ppHashTable[nBucket]) != IMS_NULL)
        {
            break;
        }
    }

    if (nBucket == nHashTableSize)
    {
        return IMS_NULL;
    }
    else
    {
        return reinterpret_cast<IMSIterator>(pElement);
    }
}

// Get a next position and return pair(key, value)
PUBLIC
void IMSHashMap::GetNext(IN_OUT IMSIterator& stIterator, OUT void*& pvKey, OUT void*& pvValue)
{
    Element* pElement = reinterpret_cast<Element*>(stIterator);
    Element* pNextElement;

    if (pElement == IMS_NULL)
    {
        return;
    }

    if ((pNextElement = pElement->pNext) == IMS_NULL)
    {
        // Goes to next bucket
        IMS_UINTP nHashKey = GetHashKey(pElement->pvKey);
        IMS_UINT32 nBucket = static_cast<IMS_UINT32>(nHashKey % nHashTableSize) + 1;

        for (; nBucket < nHashTableSize; ++nBucket)
        {
            if ((pNextElement = ppHashTable[nBucket]) != IMS_NULL)
            {
                break;
            }
        }
    }

    stIterator = reinterpret_cast<IMSIterator>(pNextElement);

    pvKey = pElement->pvKey;
    pvValue = pElement->pvValue;
}

// Get a hash key
PROTECTED VIRTUAL IMS_UINTP IMSHashMap::GetHashKey(IN void* pvKey)
{
    IMS_UINTP nTemp = reinterpret_cast<IMS_UINTP>(pvKey);

    return static_cast<IMS_UINTP>(nTemp >> 2);
}

// Find an element using the key
PROTECTED VIRTUAL IMSHashMap::Element* IMSHashMap::GetElementAt(
        IN void* pvKey, OUT IMS_UINT32& nBucket)
{
    IMS_UINTP nHashKey = GetHashKey(pvKey);

    nBucket = static_cast<IMS_UINT32>(nHashKey % nHashTableSize);

    if (ppHashTable == IMS_NULL)
    {
        return IMS_NULL;
    }

    Element* pElement;

    // Find if the key exists

    for (pElement = ppHashTable[nBucket]; pElement != IMS_NULL; pElement = pElement->pNext)
    {
        // string comparison (??)
        if (pElement->pvKey == pvKey)
        {
            return pElement;
        }
    }

    return IMS_NULL;
}
