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
#ifndef INTERFACE_MUTEX_H_
#define INTERFACE_MUTEX_H_

#include "ImsTypeDef.h"

class IMutex
{
public:
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};

class LockGuard
{
public:
    inline explicit LockGuard(IN IMutex* piLock)
        : m_piLock(piLock)
    {
        if (m_piLock != IMS_NULL)
        {
            m_piLock->Lock();
        }
    }
    inline ~LockGuard()
    {
        if (m_piLock != IMS_NULL)
        {
            m_piLock->Unlock();
        }
    }

    LockGuard(IN const LockGuard&) = delete;
    LockGuard& operator=(IN const LockGuard&) = delete;

private:
    IMutex* m_piLock;
};

#endif
