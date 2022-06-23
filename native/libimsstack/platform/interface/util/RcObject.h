/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _RC_OBJECT_H_
#define _RC_OBJECT_H_

#include "IMSTypeDef.h"

#define RcObject RCObject

class RCObject
{
public:
    RCObject();
    RCObject(IN CONST RCObject& objRHS);
    virtual ~RCObject() = 0;

public:
    RCObject& operator=(IN CONST RCObject& objRHS);

public:
    void AddReference();
    void RemoveReference();
    void MarkUnshareable();
    IMS_BOOL IsShareable() const;
    IMS_BOOL IsShared() const;

private:
    IMS_SINT32 nRefCount;
    IMS_BOOL bShareable;
};

template <class T>
class RCPtr
{
public:
    RCPtr(T* pRealPtr = IMS_NULL);
    RCPtr(IN CONST RCPtr& objRhs);
    ~RCPtr();

public:
    RCPtr& operator=(IN CONST RCPtr& objRhs);

    T* operator->() const;
    T& operator*() const;

public:
    T* Get() const;
    IMS_BOOL IsNull() const;

private:
    void Init();

private:
    T* pPointee;
};

PUBLIC
template <class T>
inline RCPtr<T>::RCPtr(T* pRealPtr /* = IMS_NULL */) :
        pPointee(pRealPtr)
{
    Init();
}

PUBLIC
template <class T>
inline RCPtr<T>::RCPtr(IN CONST RCPtr<T>& objRhs) :
        pPointee(objRhs.pPointee)
{
    Init();
}

PUBLIC
template <class T>
inline RCPtr<T>::~RCPtr()
{
    if (pPointee != IMS_NULL)
    {
        pPointee->RemoveReference();
        pPointee = IMS_NULL;
    }
}

PUBLIC
template <class T>
inline RCPtr<T>& RCPtr<T>::operator=(IN CONST RCPtr<T>& objRhs)
{
    if (pPointee != objRhs.pPointee)
    {
        T* pOldPointee = pPointee;

        pPointee = objRhs.pPointee;
        Init();

        if (pOldPointee)
        {
            pOldPointee->RemoveReference();
        }
    }

    return (*this);
}

PUBLIC
template <class T>
inline T* RCPtr<T>::operator->() const
{
    return pPointee;
}

PUBLIC
template <class T>
inline T& RCPtr<T>::operator*() const
{
    return (*pPointee);
}

PUBLIC
template <class T>
inline T* RCPtr<T>::Get() const
{
    return pPointee;
}

PUBLIC
template <class T>
inline IMS_BOOL RCPtr<T>::IsNull() const
{
    return (pPointee == IMS_NULL);
}

PRIVATE
template <class T>
inline void RCPtr<T>::Init()
{
    if (pPointee == IMS_NULL)
    {
        return;
    }

    if (!pPointee->IsShareable())
    {
        pPointee = new T(*pPointee);
    }

    pPointee->AddReference();
}

#endif  // _RC_OBJECT_H_
