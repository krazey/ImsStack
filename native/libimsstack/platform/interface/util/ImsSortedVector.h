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
#ifndef IMS_SORTED_VECTOR_H_
#define IMS_SORTED_VECTOR_H_

#include <vector>

#include "ImsNew.h"
#include "ImsVector.h"

/**
 * @brief This is a wrapper class (sorted-vector) using c++ stl vector class.
 *        And, it replaces the element if the added element is already present in the vector.
 */
template <class T>
class ImsSortedVector
{
public:
    ImsSortedVector();
    ImsSortedVector(IN const ImsSortedVector<T>& other);
    virtual ~ImsSortedVector();

public:
    ImsSortedVector<T>& operator=(IN const ImsSortedVector<T>& other);

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
    inline IMS_UINT32 GetSize() const { return static_cast<IMS_UINT32>(m_objVector.size()); }
    // Returns whether or not the vector is empty
    inline IMS_BOOL IsEmpty() const { return m_objVector.empty() ? IMS_TRUE : IMS_FALSE; }
    // Returns how many elements can be stored without reallocating the backing store
    inline IMS_UINT32 GetCapacity() const
    {
        return static_cast<IMS_UINT32>(m_objVector.capacity());
    }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline IMS_SLONG SetCapacity(IN IMS_UINT32 nNewCapacity)
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
    inline const T& operator[](IN IMS_UINT32 nIndex) const;
    // Alternate name for operator[]
    inline const T& GetAt(IN IMS_UINT32 nIndex) const;
    // Stack-usage of the vector.
    // Returns the top of the stack (last element)
    inline const T& Top() const;

    //
    // Modifying the array
    //
    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN IMS_UINT32 nIndex);
    // Grants right access to the top of the stack (last element)
    inline T& Top();
    // Checks if the same element is present
    inline IMS_BOOL Contains(IN const T& element) const;
    // Finds the index of an element
    inline IMS_SLONG GetIndexOf(IN const T& element) const;
    // Finds where this element should be inserted
    inline IMS_UINT32 GetOrderOf(IN const T& element) const;
    // Merges a vector into this one
    inline IMS_BOOL Merge(IN const ImsVector<T>& other);
    inline IMS_BOOL Merge(IN const ImsSortedVector<T>& other);

    //
    // Add an element in the right place (or replaces it if there is one)
    //
    inline void Add(IN const T& element);
    //
    // Remove an element
    //
    inline void Remove(IN const T& element);
    // Remove several elements
    inline void RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1);
    // Remove one element
    inline void RemoveAt(IN IMS_UINT32 nIndex) { RemoveElementsAt(nIndex); }

    inline void Shrink();

private:
    std::vector<T> m_objVector;
};

PUBLIC
template <class T>
inline ImsSortedVector<T>::ImsSortedVector() :
        m_objVector(std::vector<T>())
{
}

PUBLIC
template <class T>
inline ImsSortedVector<T>::ImsSortedVector(IN const ImsSortedVector<T>& other) :
        m_objVector(other.m_objVector)
{
}

PUBLIC
template <class T>
inline ImsSortedVector<T>::~ImsSortedVector()
{
    Clear();
}

PUBLIC
template <class T>
inline ImsSortedVector<T>& ImsSortedVector<T>::operator=(IN const ImsSortedVector<T>& other)
{
    if (this != &other)
    {
        m_objVector = other.m_objVector;
    }

    return (*this);
}

PUBLIC
template <class T>
inline const T* ImsSortedVector<T>::GetArrayConst() const
{
    return m_objVector.data();
}

PUBLIC
template <class T>
inline T* ImsSortedVector<T>::GetArray()
{
    return m_objVector.data();
}

PUBLIC
template <class T>
inline const T& ImsSortedVector<T>::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < GetSize());
    return m_objVector.operator[](nIndex);
}

PUBLIC
template <class T>
inline const T& ImsSortedVector<T>::GetAt(IN IMS_UINT32 nIndex) const
{
    return m_objVector.at(nIndex);
}

PUBLIC
template <class T>
inline const T& ImsSortedVector<T>::Top() const
{
    return m_objVector.back();
}

PUBLIC
template <class T>
inline T& ImsSortedVector<T>::GetAt(IN IMS_UINT32 nIndex)
{
    return m_objVector.at(nIndex);
}

PUBLIC
template <class T>
inline T& ImsSortedVector<T>::Top()
{
    return m_objVector.back();
}

PUBLIC
template <class T>
inline IMS_BOOL ImsSortedVector<T>::Contains(IN const T& element) const
{
    return (GetIndexOf(element) >= 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
template <class T>
inline IMS_SLONG ImsSortedVector<T>::GetIndexOf(IN const T& element) const
{
    const auto& it = std::find(m_objVector.begin(), m_objVector.end(), element);

    if (it == m_objVector.end())
    {
        return -1;
    }

    return std::distance(m_objVector.begin(), it);
}

PUBLIC
template <class T>
inline IMS_UINT32 ImsSortedVector<T>::GetOrderOf(IN const T& element) const
{
    if (m_objVector.empty())
    {
        return 0;
    }

    const auto& it = std::upper_bound(m_objVector.begin(), m_objVector.end(), element);
    return std::distance(m_objVector.begin(), it);
}

PUBLIC
template <class T>
inline IMS_BOOL ImsSortedVector<T>::Merge(IN const ImsVector<T>& other)
{
    for (IMS_UINT32 i = 0; i < other.GetSize(); i++)
    {
        Add(other.GetAt(i));
    }
}

PUBLIC
template <class T>
inline IMS_BOOL ImsSortedVector<T>::Merge(IN const ImsSortedVector<T>& other)
{
    if (other.IsEmpty())
    {
        return IMS_TRUE;
    }

    if (IsEmpty())
    {
        m_objVector = other.m_objVector;
        return IMS_TRUE;
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
        for (IMS_UINT32 i = 0; i < other.GetSize(); i++)
        {
            Add(other.GetAt(i));
        }
    }

    return IMS_TRUE;
}

PUBLIC
template <class T>
inline void ImsSortedVector<T>::Add(IN const T& element)
{
    IMS_SLONG nIndex = GetIndexOf(element);

    if (nIndex < 0)
    {
        nIndex = GetOrderOf(element);
        m_objVector.insert(m_objVector.begin() + nIndex, element);
    }
    else
    {
        m_objVector.at(nIndex) = element;
    }
}

PUBLIC
template <class T>
inline void ImsSortedVector<T>::Remove(IN const T& element)
{
    IMS_SLONG nIndex = GetIndexOf(element);

    if (nIndex < 0)
    {
        return;
    }

    RemoveElementsAt(nIndex);
}

PUBLIC
template <class T>
inline void ImsSortedVector<T>::RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount /*= 1*/)
{
    if (nIndex == 0 && nCount == GetSize())
    {
        m_objVector.clear();
        Shrink();
        return;
    }

    while (nCount > 0 && !m_objVector.empty())
    {
        m_objVector.erase(m_objVector.begin() + nIndex);
        nCount--;
    }

    Shrink();
}

PUBLIC
template <class T>
inline void ImsSortedVector<T>::Shrink()
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
