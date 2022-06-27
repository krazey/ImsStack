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
#ifndef IMS_STACK_H_
#define IMS_STACK_H_

#include "ImsVector.h"

template <class T>
class ImsStack : private ImsVector<T>
{
public:
    inline ImsStack() :
            ImsVector<T>()
    {
    }
    inline ImsStack(IN const ImsStack<T>& other) :
            ImsVector<T>(other)
    {
    }
    inline virtual ~ImsStack() {}

public:
    inline ImsStack<T>& operator=(IN const ImsStack<T>& other)
    {
        ImsVector<T>::operator=(other);
        return (*this);
    }

public:
    // Empty the stack
    inline void Clear() { ImsVector<T>::Clear(); }

    //
    // Stack stats
    //

    // Returns the number of elements in the stack
    inline IMS_UINT32 GetSize() const { return ImsVector<T>::GetSize(); }
    // Returns whether or not the stack is empty
    inline IMS_BOOL IsEmpty() const { return ImsVector<T>::IsEmpty(); }

    // Removes an element from the top of the stack
    inline void Pop() { ImsVector<T>::Pop(); }

    // Adds an element to the top of the stack
    inline void Push(IN const T& element) { ImsVector<T>::Push(element); }

    // Returns a reference to an element at the top of the stack
    inline T& Top() { return ImsVector<T>::Top(); }
    inline const T& Top() const { return ImsVector<T>::Top(); }
};

#endif
