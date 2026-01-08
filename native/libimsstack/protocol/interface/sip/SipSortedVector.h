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
#ifndef __SIP_SORTED_VECTOR_H__
#define __SIP_SORTED_VECTOR_H__

#include <algorithm>
#include <iterator>
#include <vector>

#include "SipVector.h"

/**
 * @brief This is a wrapper class (sorted-vector) using c++ stl vector class.
 *        And, it replaces the element if the added element is already present in the vector.
 */
template <class T>
class SipSortedVector
{
public:
    SipSortedVector();
    SipSortedVector(IN const SipSortedVector<T>& other);
    virtual ~SipSortedVector();

public:
    SipSortedVector<T>& operator=(IN const SipSortedVector<T>& other);

public:
    // Empty the vector
    inline void Clear()
    {
        m_objVector.clear();
        Shrink();
    }

    //
    // Vector stats
    //
    // Returns the number of elements in the vector
    inline SIP_UINT32 GetSize() const { return static_cast<SIP_UINT32>(m_objVector.size()); }
    // Returns whether or not the vector is empty
    inline SIP_BOOL IsEmpty() const { return m_objVector.empty() ? SIP_TRUE : SIP_FALSE; }
    // Returns how many elements can be stored without reallocating the backing store
    inline SIP_UINT32 GetCapacity() const
    {
        return static_cast<SIP_UINT32>(m_objVector.capacity());
    }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline SIP_SLONG SetCapacity(IN SIP_UINT32 nNewCapacity)
    {
        m_objVector.reserve(nNewCapacity);
        return GetCapacity();
    }

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
    inline const T& operator[](IN SIP_UINT32 nIndex) const;
    // Alternate name for operator[]
    inline const T& GetAt(IN SIP_UINT32 nIndex) const;
    // Stack-usage of the vector.
    // Returns the top of the stack (last element)
    inline const T& Top() const;

    //
    // Modifying the array
    //
    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN SIP_UINT32 nIndex);
    // Grants right access to the top of the stack (last element)
    inline T& Top();
    // Checks if the same element is present
    inline SIP_BOOL Contains(IN const T& element) const;
    // Finds the index of an element
    inline SIP_SLONG GetIndexOf(IN const T& element) const;
    // Finds where this element should be inserted
    inline SIP_UINT32 GetOrderOf(IN const T& element) const;
    // Merges a vector into this one
    inline SIP_BOOL Merge(IN const SipVector<T>& other);
    inline SIP_BOOL Merge(IN const SipSortedVector<T>& other);

    //
    // Add an element in the right place (or replaces it if there is one)
    //
    inline SIP_BOOL Add(IN const T& element);
    //
    // Remove an element
    //
    inline SIP_BOOL Remove(IN const T& element);
    // Remove several elements
    inline SIP_BOOL RemoveElementsAt(IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount = 1);
    // Remove one element
    inline SIP_BOOL RemoveAt(IN SIP_UINT32 nIndex) { return RemoveElementsAt(nIndex); }

    inline void Shrink();

private:
    std::vector<T> m_objVector;
};

template <class T>
inline SipSortedVector<T>::SipSortedVector() :
        m_objVector(std::vector<T>())
{
}

template <class T>
inline SipSortedVector<T>::SipSortedVector(IN const SipSortedVector<T>& other) :
        m_objVector(other.m_objVector)
{
}

template <class T>
inline SipSortedVector<T>::~SipSortedVector()
{
    Clear();
}

template <class T>
inline SipSortedVector<T>& SipSortedVector<T>::operator=(IN const SipSortedVector<T>& other)
{
    if (this != &other)
    {
        m_objVector = other.m_objVector;
    }

    return (*this);
}

template <class T>
inline const T* SipSortedVector<T>::GetArrayConst() const
{
    return m_objVector.data();
}

template <class T>
inline T* SipSortedVector<T>::GetArray()
{
    return m_objVector.data();
}

template <class T>
inline const T& SipSortedVector<T>::operator[](IN SIP_UINT32 nIndex) const
{
    SIP_ASSERT(nIndex < GetSize());
    return m_objVector.operator[](nIndex);
}

template <class T>
inline const T& SipSortedVector<T>::GetAt(IN SIP_UINT32 nIndex) const
{
    return m_objVector.at(nIndex);
}

template <class T>
inline const T& SipSortedVector<T>::Top() const
{
    return m_objVector.back();
}

template <class T>
inline T& SipSortedVector<T>::GetAt(IN SIP_UINT32 nIndex)
{
    return m_objVector.at(nIndex);
}

template <class T>
inline T& SipSortedVector<T>::Top()
{
    return m_objVector.back();
}

template <class T>
inline SIP_BOOL SipSortedVector<T>::Contains(IN const T& element) const
{
    return (GetIndexOf(element) >= 0) ? SIP_TRUE : SIP_FALSE;
}

template <class T>
inline SIP_SLONG SipSortedVector<T>::GetIndexOf(IN const T& element) const
{
    const auto& it = std::find(m_objVector.begin(), m_objVector.end(), element);

    if (it == m_objVector.end())
    {
        return -1;
    }

    return std::distance(m_objVector.begin(), it);
}

template <class T>
inline SIP_UINT32 SipSortedVector<T>::GetOrderOf(IN const T& element) const
{
    if (m_objVector.empty())
    {
        return 0;
    }

    const auto& it = std::upper_bound(m_objVector.begin(), m_objVector.end(), element);
    return std::distance(m_objVector.begin(), it);
}

template <class T>
inline SIP_BOOL SipSortedVector<T>::Merge(IN const SipVector<T>& other)
{
    for (SIP_UINT32 i = 0; i < other.GetSize(); i++)
    {
        Add(other.GetAt(i));
    }
}

template <class T>
inline SIP_BOOL SipSortedVector<T>::Merge(IN const SipSortedVector<T>& other)
{
    if (other.IsEmpty())
    {
        return SIP_TRUE;
    }

    if (IsEmpty())
    {
        m_objVector = other.m_objVector;
        return SIP_TRUE;
    }

    if (other.GetAt(other.GetSize() - 1) < GetAt(0))
    {
        m_objVector.insert(m_objVector.begin(), other.m_objVector.begin(), other.m_objVector.end());
    }
    else if (GetAt(GetSize() - 1) < other.GetAt(0))
    {
        m_objVector.insert(m_objVector.end(), other.m_objVector.begin(), other.m_objVector.end());
    }
    else
    {
        for (SIP_UINT32 i = 0; i < other.GetSize(); i++)
        {
            Add(other.GetAt(i));
        }
    }

    return SIP_TRUE;
}

template <class T>
inline SIP_BOOL SipSortedVector<T>::Add(IN const T& element)
{
    SIP_SLONG nIndex = GetIndexOf(element);

    if (nIndex < 0)
    {
        nIndex = GetOrderOf(element);
        m_objVector.insert(m_objVector.begin() + nIndex, element);
    }
    else
    {
        m_objVector.at(nIndex) = element;
    }

    return SIP_TRUE;
}

template <class T>
inline SIP_BOOL SipSortedVector<T>::Remove(IN const T& element)
{
    SIP_SLONG nIndex = GetIndexOf(element);

    if (nIndex < 0)
    {
        return SIP_FALSE;
    }

    return RemoveElementsAt(nIndex);
}

template <class T>
inline SIP_BOOL SipSortedVector<T>::RemoveElementsAt(
        IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount /*= 1*/)
{
    if (nIndex == 0 && nCount == GetSize())
    {
        m_objVector.clear();
        Shrink();
        return SIP_TRUE;
    }

    while (nCount > 0 && !m_objVector.empty())
    {
        m_objVector.erase(m_objVector.begin() + nIndex);
        nCount--;
    }

    Shrink();

    return SIP_TRUE;
}

template <class T>
inline void SipSortedVector<T>::Shrink()
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

#endif
