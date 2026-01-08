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
#ifndef IMS_MAP_H_
#define IMS_MAP_H_

#include "ImsSortedVector.h"

template <typename K, typename V>
class MapPair
{
public:
    inline MapPair() {}
    inline MapPair(const MapPair& other) :
            m_objPair(other.m_objPair)
    {
    }
    inline MapPair(const K& k, const V& v) :
            m_objPair(std::pair<K, V>(k, v))
    {
    }
    inline explicit MapPair(const K& k) :
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
inline bool operator==(const MapPair<K, V>& objP1, const MapPair<K, V>& objP2)
{
    return objP1.GetKey() == objP2.GetKey();
}

template <typename K, typename V>
inline bool operator<(const MapPair<K, V>& objP1, const MapPair<K, V>& objP2)
{
    return objP1.GetKey() < objP2.GetKey();
}

/**
 * @brief This is a wrapper class using ImsSortedVector which is based on c++ stl vector class.
 */
template <typename KEY, typename VALUE>
class ImsMap
{
public:
    inline ImsMap();
    inline ImsMap(IN const ImsMap<KEY, VALUE>& other);
    inline ~ImsMap();

public:
    inline ImsMap<KEY, VALUE>& operator=(IN const ImsMap<KEY, VALUE>& other);

public:
    // Empty the map
    inline void Clear() { m_objVector.Clear(); }

    //
    // Map stats
    //

    // Returns the number of elements in the map
    inline IMS_UINT32 GetSize() const { return m_objVector.GetSize(); }
    // Returns whether or not the map is empty
    inline IMS_BOOL IsEmpty() const { return m_objVector.IsEmpty(); }
    // Returns how many elements can be stored without reallocating the backing store
    inline IMS_UINT32 GetCapacity() const { return m_objVector.GetCapacity(); }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline IMS_SLONG SetCapacity(IN IMS_UINT32 nNewCapacity)
    {
        return m_objVector.SetCapacity(nNewCapacity);
    }

    //
    // Accessors
    //

    // Finds an index from the given key
    inline IMS_SLONG GetIndexOfKey(IN const KEY& key) const;
    // Returns a key at the given index
    inline const KEY& GetKeyAt(IN IMS_UINT32 nIndex) const;

    // Read-only access to an element with the given key
    inline const VALUE& GetValue(IN const KEY& key) const;
    inline const VALUE& GetValueAt(IN IMS_UINT32 nIndex) const;

    //
    // Modifying the element
    //

    // Copy-on write support, grants write access to an element
    inline VALUE& GetValue(IN const KEY& key);
    inline VALUE& GetValueAt(IN IMS_UINT32 nIndex);

    //
    // Add an element in the right place (or replaces it if there is one)
    //
    inline void Add(IN const KEY& key, IN const VALUE& value);
    inline void SetValue(IN const KEY& key, IN const VALUE& value);
    inline IMS_BOOL SetValueAt(IN IMS_UINT32 nIndex, IN const VALUE& value);

    //
    // Remove an element
    //
    inline void Remove(IN const KEY& key);

    // Remove several elements
    inline void RemoveAt(IN IMS_UINT32 nIndex) { RemoveElementsAt(nIndex); }
    inline void RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1);

private:
    ImsSortedVector<MapPair<KEY, VALUE>> m_objVector;
};

PUBLIC
template <typename KEY, typename VALUE>
inline ImsMap<KEY, VALUE>::ImsMap() :
        m_objVector(ImsSortedVector<MapPair<KEY, VALUE>>())
{
}

PUBLIC
template <typename KEY, typename VALUE>
inline ImsMap<KEY, VALUE>::ImsMap(IN const ImsMap<KEY, VALUE>& other) :
        m_objVector(other.m_objVector)
{
}

PUBLIC
template <typename KEY, typename VALUE>
inline ImsMap<KEY, VALUE>::~ImsMap()
{
    Clear();
}

PUBLIC
template <typename KEY, typename VALUE>
inline ImsMap<KEY, VALUE>& ImsMap<KEY, VALUE>::operator=(IN const ImsMap<KEY, VALUE>& other)
{
    if (this != &other)
    {
        m_objVector = other.m_objVector;
    }
    return (*this);
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_SLONG ImsMap<KEY, VALUE>::GetIndexOfKey(IN const KEY& key) const
{
    return m_objVector.GetIndexOf(MapPair<KEY, VALUE>(key));
}

PUBLIC
template <typename KEY, typename VALUE>
inline const KEY& ImsMap<KEY, VALUE>::GetKeyAt(IN IMS_UINT32 nIndex) const
{
    return m_objVector.GetAt(nIndex).GetKey();
}

PUBLIC
template <typename KEY, typename VALUE>
inline const VALUE& ImsMap<KEY, VALUE>::GetValue(IN const KEY& key) const
{
    IMS_SLONG nIndex = GetIndexOfKey(key);
    IMS_ASSERT(nIndex >= 0);
    return m_objVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline const VALUE& ImsMap<KEY, VALUE>::GetValueAt(IN IMS_UINT32 nIndex) const
{
    return m_objVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline VALUE& ImsMap<KEY, VALUE>::GetValue(IN const KEY& key)
{
    IMS_SLONG nIndex = GetIndexOfKey(key);
    IMS_ASSERT(nIndex >= 0);
    return m_objVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline VALUE& ImsMap<KEY, VALUE>::GetValueAt(IN IMS_UINT32 nIndex)
{
    return m_objVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline void ImsMap<KEY, VALUE>::Add(IN const KEY& key, IN const VALUE& value)
{
    m_objVector.Add(MapPair<KEY, VALUE>(key, value));
}

PUBLIC
template <typename KEY, typename VALUE>
inline void ImsMap<KEY, VALUE>::SetValue(IN const KEY& key, IN const VALUE& value)
{
    MapPair<KEY, VALUE> pair(key, value);
    IMS_SLONG nIndex = m_objVector.GetIndexOf(pair);

    if (nIndex >= 0)
    {
        m_objVector.GetAt(nIndex).SetValue(value);
        return;
    }

    m_objVector.Add(pair);
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_BOOL ImsMap<KEY, VALUE>::SetValueAt(IN IMS_UINT32 nIndex, IN const VALUE& value)
{
    if (nIndex < GetSize())
    {
        m_objVector.GetAt(nIndex).SetValue(value);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
template <typename KEY, typename VALUE>
inline void ImsMap<KEY, VALUE>::Remove(IN const KEY& key)
{
    m_objVector.Remove(MapPair<KEY, VALUE>(key));
}

PUBLIC
template <typename KEY, typename VALUE>
inline void ImsMap<KEY, VALUE>::RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount /*= 1*/)
{
    m_objVector.RemoveElementsAt(nIndex, nCount);
}

#endif
