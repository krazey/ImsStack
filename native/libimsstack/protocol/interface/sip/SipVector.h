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

#ifndef __SIP_VECTOR_H__
#define __SIP_VECTOR_H__

#include <type_traits>
#include <vector>

#define IN

typedef signed long SIP_SLONG;

//
// This is a wrapper class using c++ stl vector class.
//
template <class T>
class SipVector
{
public:
    SipVector();
    SipVector(IN const SipVector<T>& objRHS);
    virtual ~SipVector();

public:
    SipVector<T>& operator=(IN const SipVector<T>& objRHS);

public:
    // Empty the vector
    inline void Clear()
    {
        mVector.clear();
        Shrink();
    }

    //
    // Vector stats
    //

    // Returns the number of elements in the vector
    inline SIP_UINT32 GetSize() const { return static_cast<SIP_UINT32>(mVector.size()); }
    // Returns whether or not the vector is empty
    inline SIP_BOOL IsEmpty() const { return mVector.empty() ? SIP_TRUE : SIP_FALSE; }
    // Returns how many elements can be stored without reallocating the backing store
    inline SIP_UINT32 GetCapacity() const { return static_cast<SIP_UINT32>(mVector.capacity()); }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline SIP_SLONG SetCapacity(IN SIP_UINT32 nNewCapacity)
    {
        mVector.reserve(nNewCapacity);
        return GetCapacity();
    }
    // Internal usage: returns std::vector object as reference
    inline const std::vector<T>& GetVector() const { return mVector; }

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

    // Same as operator[], but allows to access the vector backward (from the end)
    // with a negative index
    inline const T& GetAtMirror(IN SIP_SLONG nIndex) const;
    inline T GetValueAt(IN SIP_UINT32 nIndex) const { return mVector.at(nIndex); }

    //
    // Modifying the array
    //

    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN SIP_UINT32 nIndex);
    // Grants right access to the top of the stack (last element)
    inline T& Top();

    //
    // Append / Insert another vector
    //

    // Insert another vector at a given index
    inline SIP_SLONG InsertVectorAt(IN const SipVector<T>& objVector, IN SIP_UINT32 nIndex);
    // Append another vector at the end of this one
    inline SIP_SLONG AppendVector(IN const SipVector<T>& objVector);

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
    inline SIP_SLONG Add();
    // Same as push(), but returns the index that the element was added at (or an error)
    inline SIP_SLONG Add(IN const T& element);

    // Insert one or several elements initialized with their default constructor
    inline SIP_SLONG InsertAt(IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount_ = 1);
    // Insert one or several elements initialized from a prototype element
    inline SIP_SLONG InsertAt(IN const T& element, IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount_ = 1);

    // Replaces an element with a new initialized with its default constructor
    inline SIP_SLONG ReplaceAt(IN SIP_UINT32 nIndex);
    // Replaces an element with a new prototype-element
    inline SIP_SLONG ReplaceAt(IN const T& element, IN SIP_UINT32 nIndex);

    //
    // Remove elements
    //

    // Remove several elements
    inline SIP_SLONG RemoveElementsAt(IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount_ = 1);
    // Remove one element
    inline SIP_SLONG RemoveAt(IN SIP_UINT32 nIndex) { return RemoveElementsAt(nIndex); }

    inline void Shrink();

private:
    std::vector<T> mVector;
};

//-------------------------------------------------------------------------------------------------

template <class T>
inline SipVector<T>::SipVector() :
        mVector(std::vector<T>())
{
}

template <class T>
inline SipVector<T>::SipVector(IN const SipVector<T>& objRHS) :
        mVector(objRHS.mVector)
{
}

template <class T>
inline SipVector<T>::~SipVector()
{
    Clear();
}

template <class T>
inline SipVector<T>& SipVector<T>::operator=(IN const SipVector<T>& objRHS)
{
    if (this != &objRHS)
    {
        mVector = objRHS.mVector;
    }

    return (*this);
}

template <class T>
inline const T* SipVector<T>::GetArrayConst() const
{
    return mVector.data();
}

template <class T>
inline T* SipVector<T>::GetArray()
{
    return mVector.data();
}

template <class T>
inline const T& SipVector<T>::operator[](IN SIP_UINT32 nIndex) const
{
    return mVector.operator[](nIndex);
}

template <class T>
inline const T& SipVector<T>::GetAt(IN SIP_UINT32 nIndex) const
{
    return mVector.at(nIndex);
}

template <class T>
inline const T& SipVector<T>::Top() const
{
    if (mVector.empty())
    {
        return nullptr;
    }

    return mVector.back();
}

template <class T>
inline const T& SipVector<T>::GetAtMirror(IN SIP_SLONG nIndex) const
{
    return GetAt((nIndex < 0) ? (GetSize() + nIndex) : nIndex);
}

template <class T>
inline T& SipVector<T>::GetAt(IN SIP_UINT32 nIndex)
{
    return mVector.at(nIndex);
}

template <class T>
inline T& SipVector<T>::Top()
{
    return mVector.back();
}

template <class T>
inline SIP_SLONG SipVector<T>::InsertVectorAt(
        IN const SipVector<T>& objVector, IN SIP_UINT32 nIndex)
{
    const auto& it = mVector.begin();
    mVector.insert(it + nIndex, objVector.mVector.begin(), objVector.mVector.end());
    return nIndex;
}

template <class T>
inline SIP_SLONG SipVector<T>::AppendVector(IN const SipVector<T>& objVector)
{
    return InsertVectorAt(objVector, GetSize());
}

template <class T>
inline void SipVector<T>::Pop()
{
    mVector.pop_back();
}

template <class T>
inline void SipVector<T>::Push()
{
    mVector.resize(GetSize() + 1);
}

template <class T>
inline void SipVector<T>::Push(IN const T& element)
{
    mVector.push_back(element);
}

template <class T>
inline SIP_SLONG SipVector<T>::Add()
{
    mVector.resize(GetSize() + 1);
    return GetSize() - 1;
}

template <class T>
inline SIP_SLONG SipVector<T>::Add(IN const T& element)
{
    mVector.push_back(element);
    return GetSize() - 1;
}

template <class T>
inline SIP_SLONG SipVector<T>::InsertAt(IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount_ /* = 1 */)
{
    const auto& it = mVector.begin();
    mVector.insert(it + nIndex, nCount_, std::is_pointer<T>::value ? nullptr : T());
    return nIndex;
}

template <class T>
inline SIP_SLONG SipVector<T>::InsertAt(
        IN const T& element, IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount_ /* = 1 */)
{
    const auto& it = mVector.begin();
    mVector.insert(it + nIndex, nCount_, element);
    return nIndex;
}

template <class T>
inline SIP_SLONG SipVector<T>::ReplaceAt(IN SIP_UINT32 nIndex)
{
    if (nIndex >= GetSize())
    {
        return (-1);
    }

    auto element = std::is_pointer<T>::value ? nullptr : T();
    mVector.at(nIndex) = element;
    return nIndex;
}

template <class T>
inline SIP_SLONG SipVector<T>::ReplaceAt(IN const T& element, IN SIP_UINT32 nIndex)
{
    if (nIndex >= GetSize())
    {
        return (-1);
    }

    mVector.at(nIndex) = element;
    return nIndex;
}

template <class T>
inline SIP_SLONG SipVector<T>::RemoveElementsAt(
        IN SIP_UINT32 nIndex, IN SIP_UINT32 nCount_ /* = 1 */)
{
    if (nIndex == 0 && nCount_ == GetSize())
    {
        mVector.clear();
        Shrink();
        return nIndex;
    }

    while (nCount_ > 0 && !mVector.empty())
    {
        mVector.erase(mVector.begin() + nIndex);
        nCount_--;
    }

    Shrink();

    return nIndex;
}

template <class T>
inline void SipVector<T>::Shrink()
{
    if (mVector.capacity() > 0)
    {
        if (mVector.empty())
        {
            std::vector<T> objEmptyVector;
            mVector.swap(objEmptyVector);
        }
        else if (mVector.size() <= (mVector.capacity() / 2))
        {
            mVector.shrink_to_fit();
        }
    }
}

#endif  //__SIP_VECTOR_H__
