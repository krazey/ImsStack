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
#ifndef IMS_VECTOR_H_
#define IMS_VECTOR_H_

#include <algorithm>
#include <vector>

#include "ImsNew.h"

/**
 * @brief This is a wrapper class using c++ stl vector class.
 */
template <class T>
class ImsVector
{
public:
    ImsVector();
    ImsVector(IN const ImsVector<T>& other);
    virtual ~ImsVector();

public:
    ImsVector<T>& operator=(IN const ImsVector<T>& other);

public:
    // Empty the vector
    inline void Clear()
    {
        m_objVector.clear();
        Shrink();
    }
    // Checks if the specified element exists in this vector.
    inline IMS_BOOL Contains(IN const T& element) const;
    // Checks if both vectors are same.
    inline IMS_BOOL Equals(IN const ImsVector<T>& other) const;

    //
    // Vector stats
    //
    // Returns the number of elements in the vector
    inline IMS_UINT32 GetSize() const { return static_cast<IMS_UINT32>(m_objVector.size()); }
    // Returns whether or not the vector is empty
    inline IMS_BOOL IsEmpty() const { return m_objVector.empty() ? IMS_TRUE : IMS_FALSE; }
    // Returns how many elements can be stored without reallocating the backing store
    inline IMS_UINT32 GetCapacity() const { return m_objVector.capacity(); }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline IMS_UINT32 SetCapacity(IN IMS_UINT32 nNewCapacity)
    {
        m_objVector.reserve(nNewCapacity);
        return GetCapacity();
    }
    // Internal usage: returns std::vector object as reference
    inline const std::vector<T>& GetVector() const { return m_objVector; }

    //
    // C-style array access
    //
    // Read-only C-style access
    inline const T* GetArrayConst() const;
    // Read-write C-style access
    inline T* GetArray();

    //
    // Accessors
    //
    // Read-only access to an element at a given index
    inline const T& operator[](IN IMS_UINT32 nIndex) const;
    // Alternate name for operator[]
    inline const T& GetAt(IN IMS_UINT32 nIndex) const;
    // Stack-usage of the vector.
    // Returns the top of the stack (last element)
    inline const T& Top() const;
    // Returns the value as copy.
    inline T GetValueAt(IN IMS_UINT32 nIndex) const { return m_objVector.at(nIndex); }

    //
    // Modifying the array
    //
    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN IMS_UINT32 nIndex);
    // Grants right access to the top of the stack (last element)
    inline T& Top();

    //
    // Append / Insert another vector
    //
    // Insert another vector at a given index
    inline IMS_SLONG InsertVectorAt(IN const ImsVector<T>& other, IN IMS_UINT32 nIndex);
    // Append another vector at the end of this one
    inline IMS_SLONG AppendVector(IN const ImsVector<T>& other);

    //
    // Add / Insert / Replace elements
    //
    // Pop the top of the stack (removes the last element). No-op if the stack's empty.
    inline void Pop();
    // Pushes an element initialized with its default constructor
    inline void Push();
    // Pushes an element on the top of the stack
    inline void Push(IN const T& element);

    // Same as push(), but returns the index that the element was added at (or an error)
    inline IMS_SLONG Add();
    // Same as push(), but returns the index that the element was added at (or an error)
    inline IMS_SLONG Add(IN const T& element);

    // Insert one or several elements initialized with their default constructor
    inline IMS_SLONG InsertAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1);
    // Insert one or several elements initialized from a prototype element
    inline IMS_SLONG InsertAt(IN const T& element, IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1);

    // Replaces an element with a new initialized with its default constructor
    inline IMS_SLONG ReplaceAt(IN IMS_UINT32 nIndex);
    // Replaces an element with a new prototype-element
    inline IMS_SLONG ReplaceAt(IN const T& element, IN IMS_UINT32 nIndex);

    //
    // Remove elements
    //
    // Removes several elements
    inline IMS_SLONG RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1);
    // Removes one element
    inline IMS_SLONG RemoveAt(IN IMS_UINT32 nIndex) { return RemoveElementsAt(nIndex); }
    // Removes all elements that match the specified element.
    inline IMS_BOOL Remove(IN const T& element);

    inline void Shrink();

private:
    std::vector<T> m_objVector;
};

PUBLIC
template <class T>
inline ImsVector<T>::ImsVector() :
        m_objVector(std::vector<T>())
{
}

PUBLIC
template <class T>
inline ImsVector<T>::ImsVector(IN const ImsVector<T>& other) :
        m_objVector(other.m_objVector)
{
}

PUBLIC
template <class T>
inline ImsVector<T>::~ImsVector()
{
    Clear();
}

PUBLIC
template <class T>
inline ImsVector<T>& ImsVector<T>::operator=(IN const ImsVector<T>& other)
{
    if (this != &other)
    {
        m_objVector = other.m_objVector;
    }

    return (*this);
}

PUBLIC
template <class T>
inline IMS_BOOL ImsVector<T>::Contains(IN const T& element) const
{
    auto it = std::find(m_objVector.begin(), m_objVector.end(), element);
    return it != m_objVector.end() ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
template <class T>
inline IMS_BOOL ImsVector<T>::Equals(IN const ImsVector<T>& other) const
{
    if (GetSize() != other.GetSize())
    {
        return IMS_FALSE;
    }

    return std::equal(m_objVector.begin(), m_objVector.end(), other.m_objVector.begin())
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC
template <class T>
inline const T* ImsVector<T>::GetArrayConst() const
{
    return m_objVector.data();
}

PUBLIC
template <class T>
inline T* ImsVector<T>::GetArray()
{
    return m_objVector.data();
}

PUBLIC
template <class T>
inline const T& ImsVector<T>::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < GetSize());
    return m_objVector.operator[](nIndex);
}

PUBLIC
template <class T>
inline const T& ImsVector<T>::GetAt(IN IMS_UINT32 nIndex) const
{
    return m_objVector.at(nIndex);
}

PUBLIC
template <class T>
inline const T& ImsVector<T>::Top() const
{
    return m_objVector.back();
}

PUBLIC
template <class T>
inline T& ImsVector<T>::GetAt(IN IMS_UINT32 nIndex)
{
    return m_objVector.at(nIndex);
}

PUBLIC
template <class T>
inline T& ImsVector<T>::Top()
{
    return m_objVector.back();
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::InsertVectorAt(IN const ImsVector<T>& other, IN IMS_UINT32 nIndex)
{
    const auto& it = (nIndex >= GetSize()) ? m_objVector.end() : (m_objVector.begin() + nIndex);
    m_objVector.insert(it, other.m_objVector.begin(), other.m_objVector.end());
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::AppendVector(IN const ImsVector<T>& other)
{
    return InsertVectorAt(other, GetSize());
}

PUBLIC
template <class T>
inline void ImsVector<T>::Pop()
{
    m_objVector.pop_back();
}

PUBLIC
template <class T>
inline void ImsVector<T>::Push()
{
    m_objVector.resize(GetSize() + 1);
}

PUBLIC
template <class T>
inline void ImsVector<T>::Push(IN const T& element)
{
    m_objVector.push_back(element);
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::Add()
{
    m_objVector.resize(GetSize() + 1);
    return GetSize() - 1;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::Add(IN const T& element)
{
    m_objVector.push_back(element);
    return GetSize() - 1;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::InsertAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount /*= 1*/)
{
    const auto& it = (nIndex >= GetSize()) ? m_objVector.end() : (m_objVector.begin() + nIndex);
    m_objVector.insert(it, nCount, std::is_pointer<T>::value ? null : T());
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::InsertAt(
        IN const T& element, IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount /*= 1*/)
{
    const auto& it = (nIndex >= GetSize()) ? m_objVector.end() : (m_objVector.begin() + nIndex);
    m_objVector.insert(it, nCount, element);
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::ReplaceAt(IN IMS_UINT32 nIndex)
{
    if (nIndex >= GetSize())
    {
        return (-1);
    }

    auto element = std::is_pointer<T>::value ? null : T();
    m_objVector.at(nIndex) = element;
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::ReplaceAt(IN const T& element, IN IMS_UINT32 nIndex)
{
    if (nIndex >= GetSize())
    {
        return (-1);
    }

    m_objVector.at(nIndex) = element;
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsVector<T>::RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount /*= 1*/)
{
    if (nIndex == 0 && nCount == GetSize())
    {
        m_objVector.clear();
        Shrink();
        return nIndex;
    }

    while (nCount > 0 && !m_objVector.empty())
    {
        m_objVector.erase(m_objVector.begin() + nIndex);
        nCount--;
    }

    Shrink();

    return nIndex;
}

PUBLIC
template <class T>
inline IMS_BOOL ImsVector<T>::Remove(IN const T& element)
{
    auto count = std::erase(m_objVector, element);
    return count > 0 ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
template <class T>
inline void ImsVector<T>::Shrink()
{
    if (m_objVector.capacity() > 0)
    {
        if (m_objVector.empty())
        {
            std::vector<T> objEmptyVector;
            m_objVector.swap(objEmptyVector);
        }
        else if (m_objVector.size() <= (m_objVector.capacity() / 2))
        {
            m_objVector.shrink_to_fit();
        }
    }
}

// Overrides operators
PUBLIC
template <class T>
inline IMS_BOOL operator==(IN const ImsVector<T>& objV1, IN const ImsVector<T>& objV2)
{
    return objV1.Equals(objV2);
}

#endif
