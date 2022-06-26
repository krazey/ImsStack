/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ImsHashMap.h"
#include "ServiceMemory.h"

PUBLIC
ImsHashMap::ImsHashMap(IN IMS_UINT32 nHashTableSize /*= DEFAULT_SIZE*/) :
        m_nHashTableSize(nHashTableSize),
        m_ppHashTable(IMS_NULL),
        m_nCount(0)
{
}

PUBLIC
ImsHashMap::~ImsHashMap()
{
    RemoveAll();
}

// Initialize a hash table
PUBLIC
void ImsHashMap::InitHashTable(IN IMS_UINT32 nHashTableSize, IN IMS_BOOL bAlloc /*= IMS_TRUE*/)
{
    m_nHashTableSize = nHashTableSize;

    if (m_nHashTableSize == 0)
    {
        m_nHashTableSize = DEFAULT_SIZE;
    }

    if (bAlloc == IMS_TRUE)
    {
        m_ppHashTable = (Element**)new Element*[m_nHashTableSize];

        if (m_ppHashTable != IMS_NULL)
        {
            for (IMS_UINT32 i = 0; i < m_nHashTableSize; ++i)
            {
                m_ppHashTable[i] = IMS_NULL;
            }
        }
    }
}

// Add new (key, value) pair or modify value
PUBLIC
void ImsHashMap::SetAt(IN void* pvKey, IN void* pvNewValue)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    // add new pair
    if (pElement == IMS_NULL)
    {
        if (m_ppHashTable == IMS_NULL)
        {
            InitHashTable(m_nHashTableSize);
        }

        if (m_ppHashTable != NULL)
        {
            pElement = new Element(pvKey, pvNewValue);

            pElement->m_pNext = m_ppHashTable[nBucket];
            m_ppHashTable[nBucket] = pElement;

            m_nCount++;
        }
    }
    // modify value field
    else
    {
        pElement->m_pvValue = pvNewValue;
    }
}

// Get a value which has the same key
PUBLIC
void* ImsHashMap::GetValueAt(IN void* pvKey)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    if (pElement == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pElement->m_pvValue;
}

// Search key and return value
PUBLIC
IMS_BOOL ImsHashMap::Lookup(IN void* pvKey, OUT void*& pvValue)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    if (pElement == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pvValue = pElement->m_pvValue;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ImsHashMap::Lookup(IN void* pvKey, OUT void*& pvOrigKey, OUT void*& pvValue)
{
    IMS_UINT32 nBucket;
    Element* pElement = GetElementAt(pvKey, nBucket);

    if (pElement == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pvOrigKey = pElement->m_pvKey;
    pvValue = pElement->m_pvValue;

    return IMS_TRUE;
}

// Remove existing (key, ?) pair
PUBLIC
IMS_BOOL ImsHashMap::RemoveKey(IN void* pvKey)
{
    if (m_ppHashTable == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINTP nHashKey = GetHashKey(pvKey);
    IMS_UINT32 nBucket = static_cast<IMS_UINT32>(nHashKey % m_nHashTableSize);

    Element** ppPrevElement = &(m_ppHashTable[nBucket]);
    Element* pElement;

    for (pElement = (*ppPrevElement); pElement != IMS_NULL; pElement = pElement->m_pNext)
    {
        if (pElement->m_pvKey == pvKey)
        {
            (*ppPrevElement) = pElement->m_pNext;

            delete pElement;

            m_nCount--;

            return IMS_TRUE;
        }

        ppPrevElement = &(pElement->m_pNext);
    }

    return IMS_FALSE;
}

// Remove all the elements
PUBLIC
void ImsHashMap::RemoveAll()
{
    if (m_ppHashTable != IMS_NULL)
    {
        ImsIterator iterator;
        Element* pElement;
        void* pvKey;
        void* pvValue;

        iterator = GetStartPosition();
        while (iterator != IMS_NULL)
        {
            pElement = reinterpret_cast<Element*>(iterator);

            GetNext(iterator, pvKey, pvValue);

            delete pElement;
        }

        delete[] m_ppHashTable;
        m_ppHashTable = IMS_NULL;
    }

    m_nCount = 0;
}

// Get a start position from the hash table
PUBLIC
ImsIterator ImsHashMap::GetStartPosition()
{
    IMS_UINT32 nBucket;
    Element* pElement = IMS_NULL;

    if (m_nCount == 0)
    {
        return IMS_NULL;
    }

    for (nBucket = 0; nBucket < m_nHashTableSize; ++nBucket)
    {
        if ((pElement = m_ppHashTable[nBucket]) != IMS_NULL)
        {
            break;
        }
    }

    if (nBucket == m_nHashTableSize)
    {
        return IMS_NULL;
    }
    else
    {
        return reinterpret_cast<ImsIterator>(pElement);
    }
}

// Get a next position and return pair(key, value)
PUBLIC
void ImsHashMap::GetNext(IN_OUT ImsIterator& iterator, OUT void*& pvKey, OUT void*& pvValue)
{
    Element* pElement = reinterpret_cast<Element*>(iterator);

    if (pElement == IMS_NULL)
    {
        return;
    }

    Element* pNextElement;

    if ((pNextElement = pElement->m_pNext) == IMS_NULL)
    {
        // Goes to next bucket
        IMS_UINTP nHashKey = GetHashKey(pElement->m_pvKey);
        IMS_UINT32 nBucket = static_cast<IMS_UINT32>(nHashKey % m_nHashTableSize) + 1;

        for (; nBucket < m_nHashTableSize; ++nBucket)
        {
            if ((pNextElement = m_ppHashTable[nBucket]) != IMS_NULL)
            {
                break;
            }
        }
    }

    iterator = reinterpret_cast<ImsIterator>(pNextElement);

    pvKey = pElement->m_pvKey;
    pvValue = pElement->m_pvValue;
}

// Get a hash key
PROTECTED VIRTUAL IMS_UINTP ImsHashMap::GetHashKey(IN void* pvKey)
{
    IMS_UINTP nTemp = reinterpret_cast<IMS_UINTP>(pvKey);

    return static_cast<IMS_UINTP>(nTemp >> 2);
}

// Find an element using the key
PROTECTED VIRTUAL ImsHashMap::Element* ImsHashMap::GetElementAt(
        IN void* pvKey, OUT IMS_UINT32& nBucket)
{
    IMS_UINTP nHashKey = GetHashKey(pvKey);

    nBucket = static_cast<IMS_UINT32>(nHashKey % m_nHashTableSize);

    if (m_ppHashTable == IMS_NULL)
    {
        return IMS_NULL;
    }

    Element* pElement;

    // Find if the key exists

    for (pElement = m_ppHashTable[nBucket]; pElement != IMS_NULL; pElement = pElement->m_pNext)
    {
        if (pElement->m_pvKey == pvKey)
        {
            return pElement;
        }
    }

    return IMS_NULL;
}
