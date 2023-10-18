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
#ifndef IMS_HASH_MAP_H_
#define IMS_HASH_MAP_H_

#include "ImsTypeDef.h"

struct __ImsIterator
{
    IMS_UINT32 nDummy;
};

typedef struct __ImsIterator* ImsIterator;

/**
 * @brief This class implements a container class using hash table internally.
 */
class ImsHashMap
{
protected:
    class Element
    {
    public:
        Element(IN void* pvKey, IN void* pvValue) :
                m_pvKey(pvKey),
                m_pvValue(pvValue),
                m_pNext(IMS_NULL)
        {
        }
        ~Element() {}

    public:
        void* m_pvKey;
        void* m_pvValue;
        Element* m_pNext;
    };

public:
    explicit ImsHashMap(IN IMS_UINT32 nHashTableSize = DEFAULT_SIZE);
    virtual ~ImsHashMap();

public:
    void InitHashTable(IN IMS_UINT32 nHashTableSize, IN IMS_BOOL bAlloc = IMS_TRUE);
    inline IMS_UINT32 GetHashTableSize() const { return m_nHashTableSize; }

    // Add new (key, value) pair or modify value
    void SetAt(IN void* pvKey, IN void* pvNewValue);
    void* GetValueAt(IN void* pvKey);

    // Search key and return value
    IMS_BOOL Lookup(IN void* pvKey, OUT void*& pvValue);
    IMS_BOOL Lookup(IN void* pvKey, OUT void*& pvOrigKey, OUT void*& pvValue);

    // Remove existing (key, ?) pair
    IMS_BOOL RemoveKey(IN void* pvKey);
    void RemoveAll();

    inline IMS_BOOL IsEmpty() const { return (m_nCount == 0); }
    inline IMS_UINT32 GetSize() const { return m_nCount; }

    ImsIterator GetStartPosition();
    void GetNext(IN_OUT ImsIterator& iterator, OUT void*& pvKey, OUT void*& pvValue);

protected:
    IMS_UINTP GetHashKey(IN void* pvKey);
    Element* GetElementAt(IN void* pvKey, OUT IMS_UINT32& nBucket);

protected:
    IMS_UINT32 m_nHashTableSize;
    Element** m_ppHashTable;

private:
    enum
    {
        DEFAULT_SIZE = 5
    };

    IMS_UINT32 m_nCount;
};

#endif
