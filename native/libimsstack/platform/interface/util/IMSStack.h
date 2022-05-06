/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20060822  eastsky@                  Initial Creation
    20070529  yhrhee@                   IMSFnLibStack.c -> convert to CPP.
    20090302  hsyun@                    Porting
    20100302  hwangoo.park@             Change the method from IMSVector class
    </table>

Description
Stack Library
*/

#ifndef _IMS_STACK_H_
#define _IMS_STACK_H_

#include "IMSVector.h"

template <class T>
class IMSStack : private IMSVector<T>
{
public:
    inline IMSStack() :
            IMSVector<T>()
    {
    }
    inline IMSStack(IN CONST IMSStack<T>& objRHS) :
            IMSVector<T>(objRHS)
    {
    }
    inline virtual ~IMSStack() {}

public:
    inline IMSStack<T>& operator=(IN CONST IMSStack<T>& objRHS)
    {
        IMSVector<T>::operator=(objRHS);
        return (*this);
    }

public:
    // Empty the stack
    inline void Clear() { IMSVector<T>::Clear(); }

    //
    // Stack stats
    //

    // Returns the number of elements in the stack
    inline IMS_UINT32 GetSize() const { return IMSVector<T>::GetSize(); }
    // Returns whether or not the stack is empty
    inline IMS_BOOL IsEmpty() const { return IMSVector<T>::IsEmpty(); }

    // Removes an element from the top of the stack
    inline void Pop() { IMSVector<T>::Pop(); }

    // Adds an element to the top of the stack
    inline void Push(IN CONST T& element) { IMSVector<T>::Push(element); }

    // Returns a reference to an element at the top of the stack
    inline T& Top() { return IMSVector<T>::Top(); }
    inline const T& Top() const { return IMSVector<T>::Top(); }
};

#endif  // _IMS_STACK_H_
