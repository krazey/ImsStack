/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef __SIP_MAP_H__
#define __SIP_MAP_H__

#include "SipDebug.h"
#include "SipSortedVector.h"

template <typename K, typename V>
class SipMapPair
{
public:
    inline SipMapPair() {}
    inline SipMapPair(const SipMapPair& other) :
            m_objPair(other.m_objPair)
    {
    }
    inline SipMapPair(const K& k, const V& v) :
            m_objPair(std::pair<K, V>(k, v))
    {
    }
    inline explicit SipMapPair(const K& k) :
            m_objPair(std::pair<K, V>())
    {
        m_objPair.first = k;
    }

public:
    inline const K& GetKey() const { return m_objPair.first; }
    inline K& GetKey() { return m_objPair.first; }

    inline const V& GetValue() const { return m_objPair.second; }
    inline V& GetValue() { return m_objPair.second; }
    inline void SetValue(const V& v) { m_objPair.second = v; }

private:
    std::pair<K, V> m_objPair;
};

template <typename K, typename V>
inline bool operator==(const SipMapPair<K, V>& objP1, const SipMapPair<K, V>& objP2)
{
    return objP1.GetKey() == objP2.GetKey();
}

template <typename K, typename V>
inline bool operator<(const SipMapPair<K, V>& objP1, const SipMapPair<K, V>& objP2)
{
    return objP1.GetKey() < objP2.GetKey();
}

/**
 * @brief This is a wrapper class using SipSortedVector which is based on c++ stl vector class.
 */
template <typename KEY, typename VALUE>
class SipMap
{
public:
    inline SipMap();
    inline SipMap(IN const SipMap<KEY, VALUE>& other);
    inline ~SipMap();

public:
    inline SipMap<KEY, VALUE>& operator=(IN const SipMap<KEY, VALUE>& other);

public:
    // Empty the map
    inline void Clear() { m_objVector.Clear(); }

    //
    // Map stats
    //

    // Returns the number of elements in the map
    inline SIP_UINT32 GetSize() const { return m_objVector.GetSize(); }
    // Returns whether or not the map is empty
    inline SIP_BOOL IsEmpty() const { return m_objVector.IsEmpty(); }
    // Returns how many elements can be stored without reallocating the backing store
    inline SIP_UINT32 GetCapacity() const { return m_objVector.GetCapacity(); }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline SIP_SLONG SetCapacity(IN SIP_UINT32 nNewCapacity)
    {
        return m_objVector.SetCapacity(nNewCapacity);
    }

    //
    // Accessors
    //

    // Finds an index from the given key
    inline SIP_SLONG GetIndexOfKey(IN const KEY& key) const;
    // Returns a key at the given index
    inline const KEY& GetKeyAt(IN SIP_UINT32 nIndex) const;

    // Read-only access to an element with the given key
    inline const VALUE& GetValue(IN const KEY& key) const;
    inline const VALUE& GetValueAt(IN SIP_UINT32 nIndex) const;

    //
    // Modifying the element
    //

    // Copy-on write support, grants write access to an element
    inline VALUE& GetValue(IN const KEY& key);
    inline VALUE& GetValueAt(IN SIP_UINT32 nIndex);

    //
    // Add an element in the right place (or replaces it if there is one)
    //
    inline SIP_BOOL Add(IN const KEY& key, IN const VALUE& value);
    inline SIP_BOOL SetValue(IN const KEY& key, IN const VALUE& value);
    inline SIP_BOOL SetValueAt(IN SIP_UINT32 nIndex, IN const VALUE& value);

    //
    // Remove an element
    //
    inline SIP_BOOL Remove(IN const KEY& key);

    // Remove several elements
    inline SIP_BOOL RemoveAt(IN SIP_UINT32 nIndex) { return RemoveElementsAt(nIndex); }
    inline SIP_BOOL RemoveElementsAt(IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount = 1);

private:
    SipSortedVector<SipMapPair<KEY, VALUE>> m_objVector;
};

template <typename KEY, typename VALUE>
inline SipMap<KEY, VALUE>::SipMap() :
        m_objVector(SipSortedVector<SipMapPair<KEY, VALUE>>())
{
}

template <typename KEY, typename VALUE>
inline SipMap<KEY, VALUE>::SipMap(IN const SipMap<KEY, VALUE>& other) :
        m_objVector(other.m_objVector)
{
}

template <typename KEY, typename VALUE>
inline SipMap<KEY, VALUE>::~SipMap()
{
    Clear();
}

template <typename KEY, typename VALUE>
inline SipMap<KEY, VALUE>& SipMap<KEY, VALUE>::operator=(IN const SipMap<KEY, VALUE>& other)
{
    if (this != &other)
    {
        m_objVector = other.m_objVector;
    }
    return (*this);
}

template <typename KEY, typename VALUE>
inline SIP_SLONG SipMap<KEY, VALUE>::GetIndexOfKey(IN const KEY& key) const
{
    return m_objVector.GetIndexOf(SipMapPair<KEY, VALUE>(key));
}

template <typename KEY, typename VALUE>
inline const KEY& SipMap<KEY, VALUE>::GetKeyAt(IN SIP_UINT32 nIndex) const
{
    return m_objVector.GetAt(nIndex).GetKey();
}

template <typename KEY, typename VALUE>
inline const VALUE& SipMap<KEY, VALUE>::GetValue(IN const KEY& key) const
{
    SIP_SLONG nIndex = GetIndexOfKey(key);
    SIP_ASSERT(nIndex >= 0);
    return m_objVector.GetAt(nIndex).GetValue();
}

template <typename KEY, typename VALUE>
inline const VALUE& SipMap<KEY, VALUE>::GetValueAt(IN SIP_UINT32 nIndex) const
{
    return m_objVector.GetAt(nIndex).GetValue();
}

template <typename KEY, typename VALUE>
inline VALUE& SipMap<KEY, VALUE>::GetValue(IN const KEY& key)
{
    SIP_SLONG nIndex = GetIndexOfKey(key);
    SIP_ASSERT(nIndex >= 0);
    return m_objVector.GetAt(nIndex).GetValue();
}

template <typename KEY, typename VALUE>
inline VALUE& SipMap<KEY, VALUE>::GetValueAt(IN SIP_UINT32 nIndex)
{
    return m_objVector.GetAt(nIndex).GetValue();
}

template <typename KEY, typename VALUE>
inline SIP_BOOL SipMap<KEY, VALUE>::Add(IN const KEY& key, IN const VALUE& value)
{
    return m_objVector.Add(SipMapPair<KEY, VALUE>(key, value));
}

template <typename KEY, typename VALUE>
inline SIP_BOOL SipMap<KEY, VALUE>::SetValue(IN const KEY& key, IN const VALUE& value)
{
    SipMapPair<KEY, VALUE> pair(key, value);
    SIP_SLONG nIndex = m_objVector.GetIndexOf(pair);

    if (nIndex >= 0)
    {
        m_objVector.GetAt(nIndex).SetValue(value);
        return SIP_TRUE;
    }

    return m_objVector.Add(pair);
}

template <typename KEY, typename VALUE>
inline SIP_BOOL SipMap<KEY, VALUE>::SetValueAt(IN SIP_UINT32 nIndex, IN const VALUE& value)
{
    if (nIndex < GetSize())
    {
        m_objVector.GetAt(nIndex).SetValue(value);
        return SIP_TRUE;
    }

    return SIP_FALSE;
}

template <typename KEY, typename VALUE>
inline SIP_BOOL SipMap<KEY, VALUE>::Remove(IN const KEY& key)
{
    return m_objVector.Remove(SipMapPair<KEY, VALUE>(key));
}

template <typename KEY, typename VALUE>
inline SIP_BOOL SipMap<KEY, VALUE>::RemoveElementsAt(
        IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount /*= 1*/)
{
    return m_objVector.RemoveElementsAt(nIndex, nCount);
}

#endif
