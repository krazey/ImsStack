/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100128  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMS_VECTOR_H_
#define _IMS_VECTOR_H_

#include <vector>

#include "ImsNew.h"

//
// This is a wrapper class using c++ stl vector class.
//
template <class T>
class IMSVector
{
public:
    IMSVector();
    IMSVector(IN CONST IMSVector<T>& objRHS);
    virtual ~IMSVector();

public:
    IMSVector<T>& operator=(IN CONST IMSVector<T>& objRHS);

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
    inline const T& operator[](IN IMS_UINT32 nIndex) const;
    // Alternate name for operator[]
    inline const T& GetAt(IN IMS_UINT32 nIndex) const;
    // Stack-usage of the vector.
    // Returns the top of the stack (last element)
    inline const T& Top() const;

    // Same as operator[], but allows to access the vector backward (from the end)
    // with a negative index
    inline const T& GetAtMirror(IN IMS_SLONG nIndex) const;
    inline T GetValueAt(IN IMS_UINT32 nIndex) const { return mVector.at(nIndex); }

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
    inline IMS_SLONG InsertVectorAt(IN CONST IMSVector<T>& objVector, IN IMS_UINT32 nIndex);
    // Append another vector at the end of this one
    inline IMS_SLONG AppendVector(IN CONST IMSVector<T>& objVector);

    //
    // Add / Insert / Replace elements
    //

    // Pop the top of the stack (removes the last element). No-op if the stack's empty.
    inline void Pop();
    // Pushes an element initialized with its default constructor
    inline void Push();
    // Pushes an element on the top of the stack
    inline void Push(IN CONST T& element);

    // Same as push(), but returns the index that the element was added at (or an error)
    inline IMS_SLONG Add();
    // Same as push(), but returns the index that the element was added at (or an error)
    inline IMS_SLONG Add(IN CONST T& element);

    // Insert one or several elements initialized with their default constructor
    inline IMS_SLONG InsertAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1);
    // Insert one or several elements initialized from a prototype element
    inline IMS_SLONG InsertAt(IN CONST T& element, IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1);

    // Replaces an element with a new initialized with its default constructor
    inline IMS_SLONG ReplaceAt(IN IMS_UINT32 nIndex);
    // Replaces an element with a new prototype-element
    inline IMS_SLONG ReplaceAt(IN CONST T& element, IN IMS_UINT32 nIndex);

    //
    // Remove elements
    //

    // Remove several elements
    inline IMS_SLONG RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1);
    // Remove one element
    inline IMS_SLONG RemoveAt(IN IMS_UINT32 nIndex) { return RemoveElementsAt(nIndex); }

    inline void Shrink();

private:
    std::vector<T> mVector;
};

//-------------------------------------------------------------------------------------------------

PUBLIC
template <class T>
inline IMSVector<T>::IMSVector() :
        mVector(std::vector<T>())
{
}

PUBLIC
template <class T>
inline IMSVector<T>::IMSVector(IN CONST IMSVector<T>& objRHS) :
        mVector(objRHS.mVector)
{
}

PUBLIC
template <class T>
inline IMSVector<T>::~IMSVector()
{
    Clear();
}

PUBLIC
template <class T>
inline IMSVector<T>& IMSVector<T>::operator=(IN CONST IMSVector<T>& objRHS)
{
    if (this != &objRHS)
    {
        mVector = objRHS.mVector;
    }

    return (*this);
}

PUBLIC
template <class T>
inline const T* IMSVector<T>::GetArrayConst() const
{
    return mVector.data();
}

PUBLIC
template <class T>
inline T* IMSVector<T>::GetArray()
{
    return mVector.data();
}

PUBLIC
template <class T>
inline const T& IMSVector<T>::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < GetSize());
    return mVector.operator[](nIndex);
}

PUBLIC
template <class T>
inline const T& IMSVector<T>::GetAt(IN IMS_UINT32 nIndex) const
{
    return mVector.at(nIndex);
}

PUBLIC
template <class T>
inline const T& IMSVector<T>::Top() const
{
    if (mVector.empty())
    {
        return null;
    }

    return mVector.back();
}

PUBLIC
template <class T>
inline const T& IMSVector<T>::GetAtMirror(IN IMS_SLONG nIndex) const
{
    IMS_ASSERT(((nIndex > 0) ? nIndex : -nIndex) < GetSize());
    return GetAt((nIndex < 0) ? (GetSize() + nIndex) : nIndex);
}

PUBLIC
template <class T>
inline T& IMSVector<T>::GetAt(IN IMS_UINT32 nIndex)
{
    return mVector.at(nIndex);
}

PUBLIC
template <class T>
inline T& IMSVector<T>::Top()
{
    IMS_ASSERT(!mVector.empty());
    return mVector.back();
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::InsertVectorAt(
        IN CONST IMSVector<T>& objVector, IN IMS_UINT32 nIndex)
{
    const auto& it = mVector.begin();
    mVector.insert(it + nIndex, objVector.mVector.begin(), objVector.mVector.end());
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::AppendVector(IN CONST IMSVector<T>& objVector)
{
    return InsertVectorAt(objVector, GetSize());
}

PUBLIC
template <class T>
inline void IMSVector<T>::Pop()
{
    mVector.pop_back();
}

PUBLIC
template <class T>
inline void IMSVector<T>::Push()
{
    mVector.resize(GetSize() + 1);
}

PUBLIC
template <class T>
inline void IMSVector<T>::Push(IN CONST T& element)
{
    mVector.push_back(element);
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::Add()
{
    mVector.resize(GetSize() + 1);
    return GetSize() - 1;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::Add(IN CONST T& element)
{
    mVector.push_back(element);
    return GetSize() - 1;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::InsertAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ /* = 1 */)
{
    const auto& it = mVector.begin();
    mVector.insert(it + nIndex, nCount_, std::is_pointer<T>::value ? null : T());
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::InsertAt(
        IN CONST T& element, IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ /* = 1 */)
{
    const auto& it = mVector.begin();
    mVector.insert(it + nIndex, nCount_, element);
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::ReplaceAt(IN IMS_UINT32 nIndex)
{
    if (nIndex >= GetSize())
    {
        return (-1);
    }

    mVector.at(nIndex) = std::is_pointer<T>::value ? null : T();
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::ReplaceAt(IN CONST T& element, IN IMS_UINT32 nIndex)
{
    if (nIndex >= GetSize())
    {
        return (-1);
    }

    mVector.at(nIndex) = element;
    return nIndex;
}

PUBLIC
template <class T>
inline IMS_SLONG IMSVector<T>::RemoveElementsAt(
        IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ /* = 1 */)
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

PUBLIC
template <class T>
inline void IMSVector<T>::Shrink()
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

#endif  // _IMS_VECTOR_H_
