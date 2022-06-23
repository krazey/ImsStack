/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100128  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMS_SORTED_VECTOR_H_
#define _IMS_SORTED_VECTOR_H_

#include <vector>

#include "ImsNew.h"
#include "ImsVector.h"

//
// This is a wrapper class (sorted-vector) using c++ stl vector class.
// And, it replaces the element if the added element is already present in the vector.
//
template <class T>
class IMSSortedVector
{
public:
    IMSSortedVector();
    IMSSortedVector(IN CONST IMSSortedVector<T>& objRHS);
    virtual ~IMSSortedVector();

public:
    IMSSortedVector<T>& operator=(IN CONST IMSSortedVector<T>& objRHS);

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
    inline IMS_UINT32 GetSize() const { return static_cast<IMS_UINT32>(mVector.size()); }
    // Returns whether or not the vector is empty
    inline IMS_BOOL IsEmpty() const { return mVector.empty() ? IMS_TRUE : IMS_FALSE; }
    // Returns how many elements can be stored without reallocating the backing store
    inline IMS_UINT32 GetCapacity() const { return static_cast<IMS_UINT32>(mVector.capacity()); }
    // Sets the capacity. The capacity can never be reduced less than GetSize().
    inline IMS_SLONG SetCapacity(IN IMS_UINT32 nNewCapacity)
    {
        return mVector.reserve(nNewCapacity);
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

    // Same as operator[], but allows to access the vector backward (from the end)
    // with a negative index
    inline const T& GetAtMirror(IN IMS_SLONG nIndex) const;

    //
    // Modifying the array
    //

    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN IMS_UINT32 nIndex);
    // Grants right access to the top of the stack (last element)
    inline T& Top();

    // Checks if the same element is present
    inline IMS_BOOL Contains(IN CONST T& element) const;
    // Finds the index of an element
    inline IMS_SLONG GetIndexOf(IN CONST T& element) const;
    // Finds where this element should be inserted
    inline IMS_UINT32 GetOrderOf(IN CONST T& element) const;

    // Merges a vector into this one
    inline IMS_BOOL Merge(IN CONST IMSVector<T>& objVector);
    inline IMS_BOOL Merge(IN CONST IMSSortedVector<T>& objVector);

    //
    // Add an element in the right place (or replaces it if there is one)
    //
    inline IMS_BOOL Add(IN CONST T& element);

    //
    // Remove an element
    //
    inline IMS_BOOL Remove(IN CONST T& element);

    // Remove several elements
    inline IMS_BOOL RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1);
    // Remove one element
    inline IMS_BOOL RemoveAt(IN IMS_UINT32 nIndex) { return RemoveElementsAt(nIndex); }

    inline void Shrink();

private:
    std::vector<T> mVector;
};

//-------------------------------------------------------------------------------------------------

PUBLIC
template <class T>
inline IMSSortedVector<T>::IMSSortedVector() :
        mVector(std::vector<T>())
{
}

PUBLIC
template <class T>
inline IMSSortedVector<T>::IMSSortedVector(IN CONST IMSSortedVector<T>& objRHS) :
        mVector(objRHS.mVector)
{
}

PUBLIC
template <class T>
inline IMSSortedVector<T>::~IMSSortedVector()
{
    Clear();
}

PUBLIC
template <class T>
inline IMSSortedVector<T>& IMSSortedVector<T>::operator=(IN CONST IMSSortedVector<T>& objRHS)
{
    if (this != &objRHS)
    {
        mVector = objRHS.mVector;
    }

    return (*this);
}

PUBLIC
template <class T>
inline const T* IMSSortedVector<T>::GetArrayConst() const
{
    return mVector.data();
}

PUBLIC
template <class T>
inline T* IMSSortedVector<T>::GetArray()
{
    return mVector.data();
}

PUBLIC
template <class T>
inline const T& IMSSortedVector<T>::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < GetSize());
    return mVector.operator[](nIndex);
}

PUBLIC
template <class T>
inline const T& IMSSortedVector<T>::GetAt(IN IMS_UINT32 nIndex) const
{
    return mVector.at(nIndex);
}

PUBLIC
template <class T>
inline const T& IMSSortedVector<T>::Top() const
{
    IMS_ASSERT(!mVector.empty());
    return mVector.back();
}

PUBLIC
template <class T>
inline const T& IMSSortedVector<T>::GetAtMirror(IN IMS_SLONG nIndex) const
{
    IMS_ASSERT(((nIndex > 0) ? nIndex : -nIndex) < GetSize());
    return GetAt((nIndex < 0) ? (GetSize() + nIndex) : nIndex);
}

PUBLIC
template <class T>
inline T& IMSSortedVector<T>::GetAt(IN IMS_UINT32 nIndex)
{
    return mVector.at(nIndex);
}

PUBLIC
template <class T>
inline T& IMSSortedVector<T>::Top()
{
    IMS_ASSERT(!mVector.empty());
    return mVector.back();
}

PUBLIC
template <class T>
inline IMS_BOOL IMSSortedVector<T>::Contains(IN CONST T& element) const
{
    return (GetIndexOf(element) >= 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSSortedVector<T>::GetIndexOf(IN CONST T& element) const
{
    const auto& it = std::find(mVector.begin(), mVector.end(), element);

    if (it == mVector.end())
    {
        return -1;
    }

    return std::distance(mVector.begin(), it);
}

PUBLIC
template <class T>
inline IMS_UINT32 IMSSortedVector<T>::GetOrderOf(IN CONST T& element) const
{
    if (mVector.empty())
    {
        return 0;
    }

    const auto& it = std::upper_bound(mVector.begin(), mVector.end(), element);
    return std::distance(mVector.begin(), it);
}

PUBLIC
template <class T>
inline IMS_BOOL IMSSortedVector<T>::Merge(IN CONST IMSVector<T>& objVector)
{
    for (IMS_UINT32 i = 0; i < objVector.GetSize(); i++)
    {
        Add(objVector.GetAt(i));
    }
}

PUBLIC
template <class T>
inline IMS_BOOL IMSSortedVector<T>::Merge(IN CONST IMSSortedVector<T>& objVector)
{
    if (objVector.IsEmpty())
    {
        return IMS_TRUE;
    }

    if (IsEmpty())
    {
        mVector = objVector.mVector;
        return IMS_TRUE;
    }

    if (objVector.GetAt(objVector.GetSize() - 1) < GetAt(0))
    {
        mVector.insert(mVector.begin(), objVector.mVector.begin(), objVector.mVector.end());
    }
    else if (GetAt(GetSize() - 1) < objVector.GetAt(0))
    {
        mVector.insert(mVector.end(), objVector.mVector.begin(), objVector.mVector.end());
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objVector.GetSize(); i++)
        {
            Add(objVector.GetAt(i));
        }
    }

    return IMS_TRUE;
}

PUBLIC
template <class T>
inline IMS_BOOL IMSSortedVector<T>::Add(IN CONST T& element)
{
    IMS_SLONG nIndex = GetIndexOf(element);

    if (nIndex < 0)
    {
        nIndex = GetOrderOf(element);
        mVector.insert(mVector.begin() + nIndex, element);
    }
    else
    {
        mVector.at(nIndex) = element;
    }

    return IMS_TRUE;
}

PUBLIC
template <class T>
inline IMS_BOOL IMSSortedVector<T>::Remove(IN CONST T& element)
{
    IMS_SLONG nIndex = GetIndexOf(element);

    if (nIndex < 0)
    {
        return IMS_FALSE;
    }

    return RemoveElementsAt(nIndex);
}

PUBLIC
template <class T>
inline IMS_BOOL IMSSortedVector<T>::RemoveElementsAt(
        IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ /*= 1*/)
{
    if (nIndex == 0 && nCount_ == GetSize())
    {
        mVector.clear();
        Shrink();
        return IMS_TRUE;
    }

    while (nCount_ > 0 && !mVector.empty())
    {
        mVector.erase(mVector.begin() + nIndex);
        nCount_--;
    }

    Shrink();

    return IMS_TRUE;
}

PUBLIC
template <class T>
inline void IMSSortedVector<T>::Shrink()
{
    if (mVector.empty())
    {
        mVector.shrink_to_fit();
    }
    else if (mVector.size() <= (mVector.capacity() / 2))
    {
        mVector.shrink_to_fit();
    }
}

#endif  // _IMS_SORTED_VECTOR_H_
