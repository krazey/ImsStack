/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20070306  yhrhee@                   Initial Creation
    20070416  yhrhee@                   Re-design to Template class
    20090302  hsyun@                    Porting
    20100302  hwangoo.park@             Change the method from IMSVector class
    </table>

Description
Queue Library

*/

#ifndef _IMS_QUEUE_H_
#define _IMS_QUEUE_H_

#include "IMSVector.h"

template <class T>
class IMSQueue : private IMSVector<T>
{
public:
    inline IMSQueue() :
            IMSVector<T>()
    {
    }
    inline IMSQueue(IN CONST IMSQueue<T>& objRHS) :
            IMSVector<T>(objRHS)
    {
    }
    inline virtual ~IMSQueue() {}

public:
    inline IMSQueue<T>& operator=(IN CONST IMSQueue<T>& objRHS)
    {
        IMSVector<T>::operator=(objRHS);
        return (*this);
    }

public:
    // Empty the queue
    inline void Clear() { IMSVector<T>::Clear(); }

    //
    // Queue stats
    //

    // Returns the number of elements in the queue
    inline IMS_UINT32 GetSize() const { return IMSVector<T>::GetSize(); }
    // Returns whether or not the queue is empty
    inline IMS_BOOL IsEmpty() const { return IMSVector<T>::IsEmpty(); }

    // Returns a reference to the last and most recently added element at the back of the queue
    inline T& GetBack()
    {
        IMS_ASSERT(!IsEmpty());
        return IMSVector<T>::GetAt(GetSize());
    }
    inline const T& GetBack() const
    {
        IMS_ASSERT(!IsEmpty());
        return IMSVector<T>::GetAt(GetSize());
    }

    // Returns a reference to the first element at the front of the queue
    inline T& GetFront()
    {
        IMS_ASSERT(!IsEmpty());
        return IMSVector<T>::GetAt(0);
    }
    inline const T& GetFront() const
    {
        IMS_ASSERT(!IsEmpty());
        return IMSVector<T>::GetAt(0);
    }

    // Removes an element from the front of the queue
    inline void Pop()
    {
        if (!IsEmpty())
            IMSVector<T>::RemoveAt(0);
    }

    // Adds an element to the back of the queue
    inline void Push(IN CONST T& element) { IMSVector<T>::Push(element); }

    // TODO:: Temporary; DO NOT USE THIS METHOD!!!IT WILL BE REMOVED LATER.
    inline void Prepend(IN CONST T& element) { IMSVector<T>::InsertAt(element, 0); }
};

#endif  // _IMS_QUEUE_H_
