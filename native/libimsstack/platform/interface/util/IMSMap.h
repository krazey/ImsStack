/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    2010303  hwangoo.park@              Created
    </table>

    Description

*/

#ifndef _IMS_MAP_H_
#define _IMS_MAP_H_

#include "IMSSortedVector.h"

template <typename K, typename V>
class MapPair
{
public:
    inline MapPair() {}
    inline MapPair(const MapPair& objRHS) :
            mPair(objRHS.mPair)
    {
    }
    inline MapPair(const K& k, const V& v) :
            mPair(std::pair<K, V>(k, v))
    {
    }
    inline MapPair(const K& k) :
            mPair(std::pair<K, V>())
    {
        mPair.first = k;
    }

public:
    inline const K& GetKey() const { return mPair.first; }
    inline K& GetKey() { return mPair.first; }

    inline const V& GetValue() const { return mPair.second; }
    inline V& GetValue() { return mPair.second; }
    inline void SetValue(const V& v) { mPair.second = v; }

private:
    std::pair<K, V> mPair;
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

//
// This is a wrapper class using IMSSortedVector which is based on c++ stl vector class.
//
template <typename KEY, typename VALUE>
class IMSMap
{
public:
    inline IMSMap();
    inline IMSMap(IN CONST IMSMap<KEY, VALUE>& objRHS);
    inline ~IMSMap();

public:
    inline IMSMap<KEY, VALUE>& operator=(IN CONST IMSMap<KEY, VALUE>& objRHS);

public:
    // Empty the map
    inline void Clear() { mVector.Clear(); }

    //
    // Map stats
    //

    // Returns the number of elements in the map
    inline IMS_UINT32 GetSize() const { return mVector.GetSize(); }
    // Returns whether or not the map is empty
    inline IMS_BOOL IsEmpty() const { return mVector.IsEmpty(); }
    // Returns how many elements can be stored without reallocating the backing store
    inline IMS_UINT32 GetCapacity() const { return mVector.GetCapacity(); }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline IMS_SLONG SetCapacity(IN IMS_UINT32 nNewCapacity)
    {
        return mVector.SetCapacity(nNewCapacity);
    }

    //
    // Accessors
    //

    // Finds an index from the given key
    inline IMS_SLONG GetIndexOfKey(IN CONST KEY& key) const;
    // Returns a key at the given index
    inline const KEY& GetKeyAt(IN IMS_UINT32 nIndex) const;

    // Read-only access to an element with the given key
    inline const VALUE& GetValue(IN CONST KEY& key) const;
    inline const VALUE& GetValueAt(IN IMS_UINT32 nIndex) const;

    //
    // Modifying the element
    //

    // Copy-on write support, grants write access to an element
    inline VALUE& GetValue(IN CONST KEY& key);
    inline VALUE& GetValueAt(IN IMS_UINT32 nIndex);

    //
    // Add an element in the right place (or replaces it if there is one)
    //
    inline IMS_BOOL Add(IN CONST KEY& key, IN CONST VALUE& value);
    inline IMS_BOOL SetValue(IN CONST KEY& key, IN CONST VALUE& value);
    inline IMS_BOOL SetValueAt(IN IMS_UINT32 nIndex, IN CONST VALUE& value);

    //
    // Remove an element
    //
    inline IMS_BOOL Remove(IN CONST KEY& key);

    // Remove several elements
    inline IMS_BOOL RemoveAt(IN IMS_UINT32 nIndex) { return RemoveElementsAt(nIndex); }
    inline IMS_BOOL RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1);

private:
    IMSSortedVector<MapPair<KEY, VALUE>> mVector;
};

//-------------------------------------------------------------------------------------------------

PUBLIC
template <typename KEY, typename VALUE>
inline IMSMap<KEY, VALUE>::IMSMap() :
        mVector(IMSSortedVector<MapPair<KEY, VALUE>>())
{
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMSMap<KEY, VALUE>::IMSMap(IN CONST IMSMap<KEY, VALUE>& objRHS) :
        mVector(objRHS.mVector)
{
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMSMap<KEY, VALUE>::~IMSMap()
{
    Clear();
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMSMap<KEY, VALUE>& IMSMap<KEY, VALUE>::operator=(IN CONST IMSMap<KEY, VALUE>& objRHS)
{
    if (this != &objRHS)
    {
        mVector = objRHS.mVector;
    }
    return (*this);
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_SLONG IMSMap<KEY, VALUE>::GetIndexOfKey(IN CONST KEY& key) const
{
    return mVector.GetIndexOf(MapPair<KEY, VALUE>(key));
}

PUBLIC
template <typename KEY, typename VALUE>
inline const KEY& IMSMap<KEY, VALUE>::GetKeyAt(IN IMS_UINT32 nIndex) const
{
    return mVector.GetAt(nIndex).GetKey();
}

PUBLIC
template <typename KEY, typename VALUE>
inline const VALUE& IMSMap<KEY, VALUE>::GetValue(IN CONST KEY& key) const
{
    IMS_SLONG nIndex = GetIndexOfKey(key);
    IMS_ASSERT(nIndex >= 0);
    return mVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline const VALUE& IMSMap<KEY, VALUE>::GetValueAt(IN IMS_UINT32 nIndex) const
{
    return mVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline VALUE& IMSMap<KEY, VALUE>::GetValue(IN CONST KEY& key)
{
    IMS_SLONG nIndex = GetIndexOfKey(key);
    IMS_ASSERT(nIndex >= 0);
    return mVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline VALUE& IMSMap<KEY, VALUE>::GetValueAt(IN IMS_UINT32 nIndex)
{
    return mVector.GetAt(nIndex).GetValue();
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_BOOL IMSMap<KEY, VALUE>::Add(IN CONST KEY& key, IN CONST VALUE& value)
{
    return mVector.Add(MapPair<KEY, VALUE>(key, value));
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_BOOL IMSMap<KEY, VALUE>::SetValue(IN CONST KEY& key, IN CONST VALUE& value)
{
    MapPair<KEY, VALUE> pair(key, value);
    IMS_SLONG nIndex = mVector.GetIndexOf(pair);

    if (nIndex >= 0)
    {
        mVector.GetAt(nIndex).SetValue(value);
        return IMS_TRUE;
    }

    return mVector.Add(pair);
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_BOOL IMSMap<KEY, VALUE>::SetValueAt(IN IMS_UINT32 nIndex, IN CONST VALUE& value)
{
    if (nIndex < GetSize())
    {
        mVector.GetAt(nIndex).SetValue(value);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_BOOL IMSMap<KEY, VALUE>::Remove(IN CONST KEY& key)
{
    return mVector.Remove(MapPair<KEY, VALUE>(key));
}

PUBLIC
template <typename KEY, typename VALUE>
inline IMS_BOOL IMSMap<KEY, VALUE>::RemoveElementsAt(
        IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ /*= 1*/)
{
    return mVector.RemoveElementsAt(nIndex, nCount_);
}

#endif  // _IMS_MAP_H_
