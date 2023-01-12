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
#ifndef RC_OBJECT_H_
#define RC_OBJECT_H_

#include "ImsTypeDef.h"

class RcObject
{
public:
    RcObject();
    RcObject(IN const RcObject& other);
    virtual ~RcObject() = 0;

public:
    RcObject& operator=(IN const RcObject& other);

public:
    void AddReference();
    void RemoveReference();
    void MarkUnshareable();
    IMS_BOOL IsShareable() const;
    IMS_BOOL IsShared() const;

private:
    IMS_SINT32 m_nRefCount;
    IMS_BOOL m_bShareable;
};

template <class T>
class RcPtr
{
public:
    // cppcheck-suppress noExplicitConstructor
    RcPtr(T* pRealPtr = IMS_NULL);
    RcPtr(IN const RcPtr& other);
    ~RcPtr();

public:
    RcPtr& operator=(IN const RcPtr& other);

    T* operator->() const;
    T& operator*() const;

public:
    T* Get() const;
    IMS_BOOL IsNull() const;

private:
    void Init();

private:
    T* m_pPointee;
};

PUBLIC
template <class T>
inline RcPtr<T>::RcPtr(T* pRealPtr /*= IMS_NULL*/) :
        m_pPointee(pRealPtr)
{
    Init();
}

PUBLIC
template <class T>
inline RcPtr<T>::RcPtr(IN const RcPtr<T>& other) :
        m_pPointee(other.m_pPointee)
{
    Init();
}

PUBLIC
template <class T>
inline RcPtr<T>::~RcPtr()
{
    if (m_pPointee != IMS_NULL)
    {
        m_pPointee->RemoveReference();
        m_pPointee = IMS_NULL;
    }
}

PUBLIC
template <class T>
inline RcPtr<T>& RcPtr<T>::operator=(IN const RcPtr<T>& other)
{
    if (this != &other)
    {
        if (m_pPointee != other.m_pPointee)
        {
            T* pOldPointee = m_pPointee;

            m_pPointee = other.m_pPointee;

            Init();

            if (pOldPointee)
            {
                pOldPointee->RemoveReference();
            }
        }
    }

    return (*this);
}

PUBLIC
template <class T>
inline T* RcPtr<T>::operator->() const
{
    return m_pPointee;
}

PUBLIC
template <class T>
inline T& RcPtr<T>::operator*() const
{
    return (*m_pPointee);
}

PUBLIC
template <class T>
inline T* RcPtr<T>::Get() const
{
    return m_pPointee;
}

PUBLIC
template <class T>
inline IMS_BOOL RcPtr<T>::IsNull() const
{
    return (m_pPointee == IMS_NULL);
}

PRIVATE
template <class T>
inline void RcPtr<T>::Init()
{
    if (m_pPointee == IMS_NULL)
    {
        return;
    }

    if (!m_pPointee->IsShareable())
    {
        m_pPointee = new T(*m_pPointee);
    }

    m_pPointee->AddReference();
}

#endif
