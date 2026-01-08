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
#ifndef IMS_QUEUE_H_
#define IMS_QUEUE_H_

#include "ImsVector.h"

// The duplInheritedMember warning is suppressed because the identical member function name
// is an intentional design, which uses private inheritance and method wrapping.
// cppcheck-suppress-begin duplInheritedMember
template <class T>
class ImsQueue : private ImsVector<T>
{
public:
    inline ImsQueue() :
            ImsVector<T>()
    {
    }
    inline ImsQueue(IN const ImsQueue<T>& other) :
            ImsVector<T>(other)
    {
    }
    ~ImsQueue() override = default;

public:
    inline ImsQueue<T>& operator=(IN const ImsQueue<T>& other)
    {
        if (this != &other)
        {
            ImsVector<T>::operator=(other);
        }
        return (*this);
    }

public:
    // Empty the queue
    inline void Clear() { ImsVector<T>::Clear(); }
    // Checks if both queues are same.
    inline IMS_BOOL Equals(IN const ImsQueue<T>& other) const
    {
        return ImsVector<T>::Equals(other);
    }

    //
    // Queue stats
    //
    // Returns the number of elements in the queue
    inline IMS_UINT32 GetSize() const { return ImsVector<T>::GetSize(); }
    // Returns whether or not the queue is empty
    inline IMS_BOOL IsEmpty() const { return ImsVector<T>::IsEmpty(); }

    // Returns a reference to the last and most recently added element at the back of the queue
    inline T& GetBack()
    {
        IMS_ASSERT(!IsEmpty());
        return ImsVector<T>::GetAt(GetSize() - 1);
    }
    inline const T& GetBack() const
    {
        IMS_ASSERT(!IsEmpty());
        return ImsVector<T>::GetAt(GetSize() - 1);
    }

    // Returns a reference to the first element at the front of the queue
    inline T& GetFront()
    {
        IMS_ASSERT(!IsEmpty());
        return ImsVector<T>::GetAt(0);
    }
    inline const T& GetFront() const
    {
        IMS_ASSERT(!IsEmpty());
        return ImsVector<T>::GetAt(0);
    }

    // Removes an element from the front of the queue
    inline void Pop()
    {
        if (!IsEmpty())
        {
            ImsVector<T>::RemoveAt(0);
        }
    }

    // Adds an element to the back of the queue
    inline void Push(IN const T& element) { ImsVector<T>::Push(element); }
};
// cppcheck-suppress-end duplInheritedMember

// Overrides operators
PUBLIC
template <class T>
inline IMS_BOOL operator==(IN const ImsQueue<T>& objQueue1, IN const ImsQueue<T>& objQueue2)
{
    return objQueue1.Equals(objQueue2);
}

#endif
