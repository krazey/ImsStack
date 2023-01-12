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
#ifndef IMS_LIST_H_
#define IMS_LIST_H_

#include "ImsVector.h"

#define IMSList ImsList

template <class T>
class ImsList : private ImsVector<T>
{
public:
    inline ImsList() :
            ImsVector<T>()
    {
    }
    inline ImsList(IN const ImsList<T>& other) :
            ImsVector<T>(other)
    {
    }
    inline virtual ~ImsList() {}

public:
    inline ImsList<T>& operator=(IN const ImsList<T>& other)
    {
        if (this != &other)
        {
            ImsVector<T>::operator=(other);
        }
        return (*this);
    }

public:
    // Empty the list
    inline void Clear() { ImsVector<T>::Clear(); }
    // Checks if both lists are same.
    inline IMS_BOOL Equals(IN const ImsList<T>& other) const { return ImsVector<T>::Equals(other); }

    //
    // List stats
    //
    // Returns the number of elements in the list
    inline IMS_UINT32 GetSize() const { return ImsVector<T>::GetSize(); }
    // Returns whether or not the list is empty
    inline IMS_BOOL IsEmpty() const { return ImsVector<T>::IsEmpty(); }

    //
    // Accessors
    //
    // Gets an element at the given index with read-only property
    inline const T& GetAt(IN IMS_UINT32 nIndex) const { return ImsVector<T>::GetAt(nIndex); }
    inline T GetValueAt(IN IMS_UINT32 nIndex) const { return ImsVector<T>::GetValueAt(nIndex); }

    //
    // Modifying the array
    //
    // Copy-on write support, grants write access to an element
    inline T& GetAt(IN IMS_UINT32 nIndex) { return ImsVector<T>::GetAt(nIndex); }

    //
    // Append / Insert another list
    //
    // Insert another list at a given index
    inline IMS_BOOL InsertListAt(IN const ImsList<T>& objList, IN IMS_UINT32 nIndex)
    {
        return (ImsVector<T>::InsertVectorAt(objList, nIndex) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    // Append another list at the end of this one
    inline IMS_BOOL AppendList(IN const ImsList<T>& objList)
    {
        return (ImsVector<T>::AppendVector(objList) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    //
    // Add / Insert / Replace elements
    //
    // Append / Prepend the given element in the last / in the first position
    inline IMS_BOOL Append(IN const T& element)
    {
        return (ImsVector<T>::Add(element) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    inline IMS_BOOL Prepend(IN const T& element)
    {
        return (ImsVector<T>::InsertAt(element, 0) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    // Insert one or several elements initialized from a prototype element
    inline IMS_BOOL InsertAt(IN const T& element, IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1)
    {
        return (ImsVector<T>::InsertAt(element, nIndex, nCount) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    // Replaces an element with a new prototype-element
    inline IMS_BOOL SetAt(IN const T& element, IN IMS_UINT32 nIndex)
    {
        return (ImsVector<T>::ReplaceAt(element, nIndex) >= 0) ? IMS_TRUE : IMS_FALSE;
    }

    //
    // Remove elements
    //
    // Remove several elements
    inline IMS_BOOL RemoveElementsAt(IN IMS_UINT32 nIndex, IN IMS_UINT32 nCount = 1)
    {
        return (ImsVector<T>::RemoveElementsAt(nIndex, nCount) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
    // Remove one element
    inline IMS_BOOL RemoveAt(IN IMS_UINT32 nIndex)
    {
        return (ImsVector<T>::RemoveAt(nIndex) >= 0) ? IMS_TRUE : IMS_FALSE;
    }
};

// Overrides operators
PUBLIC
template <class T>
inline IMS_BOOL operator==(IN const ImsList<T>& objList1, IN const ImsList<T>& objList2)
{
    return objList1.Equals(objList2);
}

#endif
