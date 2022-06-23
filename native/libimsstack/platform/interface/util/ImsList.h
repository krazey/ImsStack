/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100128  hwangoo.park@             From LibList & Android
    </table>

    Description

*/

#ifndef _IMS_LIST_H_
#define _IMS_LIST_H_

#include "ImsVector.h"

//
// The main templated list class ensuring type safety while making use of IMSVectorImpl.
// This is the class users want to use it.

template <class T>
class IMSList : private IMSVector<T>
{
public:
    inline IMSList() :
            IMSVector<T>()
    {
    }
    inline IMSList(IN CONST IMSList<T>& objRHS) :
            IMSVector<T>(objRHS)
    {
    }
    inline virtual ~IMSList() {}

public:
    inline IMSList<T>& operator=(IN CONST IMSList<T>& objRHS)
    {
        IMSVector<T>::operator=(objRHS);
        return (*this);
    }

public:
    // Empty the list
    inline void Clear() { IMSVector<T>::Clear(); }

    //
    // List stats
    //

    // Returns the number of elements in the list
    inline IMS_UINT32 GetSize() const { return IMSVector<T>::GetSize(); }
    // Returns whether or not the list is empty
    inline IMS_BOOL IsEmpty() const { return IMSVector<T>::IsEmpty(); }

    //
    // Accessors
    //

    // Gets an element at the given index with read-only property
    inline const T& GetAt(IN IMS_UINT32 nIndex) const { return IMSVector<T>::GetAt(nIndex); }
    inline T GetValueAt(IN IMS_UINT32 nIndex) const { return IMSVector<T>::GetValueAt(nIndex); }

    //
    // Modifying the array
    //

    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN IMS_UINT32 nIndex) { return IMSVector<T>::GetAt(nIndex); }

    //
    // Append / Insert another list
    //

    // Insert another list at a given index
    inline IMS_BOOL InsertListAt(IN CONST IMSList<T>& objList, IN IMS_UINT32 nIndex)
    {
        return (IMSVector<T>::InsertVectorAt(objList, nIndex) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    // Append another list at the end of this one
    inline IMS_BOOL AppendList(IN CONST IMSList<T>& objList)
    {
        return (IMSVector<T>::AppendVector(objList) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    //
    // Add / Insert / Replace elements
    //

    // Append / Prepend the given element in the last / in the first position
    inline IMS_BOOL Append(IN CONST T& element)
    {
        return (IMSVector<T>::Add(element) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    inline IMS_BOOL Prepend(IN CONST T& element)
    {
        return (IMSVector<T>::InsertAt(element, 0) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    // Insert one or several elements initialized from a prototype element
    inline IMS_BOOL InsertAt(IN CONST T& element, IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1)
    {
        return (IMSVector<T>::InsertAt(element, nIndex, nCount_) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    // Replaces an element with a new prototype-element
    inline IMS_BOOL SetAt(IN CONST T& element, IN IMS_UINT32 nIndex)
    {
        return (IMSVector<T>::ReplaceAt(element, nIndex) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    //
    // Remove elements
    //

    // Remove several elements
    inline IMS_BOOL RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount_ = 1)
    {
        return (IMSVector<T>::RemoveElementsAt(nIndex, nCount_) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    // Remove one element
    inline IMS_BOOL RemoveAt(IN IMS_UINT32 nIndex)
    {
        return (IMSVector<T>::RemoveAt(nIndex) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
};

#endif  // _IMS_LIST_H_
