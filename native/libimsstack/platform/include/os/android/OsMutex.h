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
#ifndef OS_MUTEX_H_
#define OS_MUTEX_H_

#include "ImsMutex.h"

class OsMutexPrivate;

class OsMutex : public ImsMutex
{
public:
    explicit OsMutex(IN IMS_SINT32 nType = ATTRIBUTE_RECURSIVE);
    ~OsMutex() override;

    OsMutex(IN const OsMutex&) = delete;
    OsMutex& operator=(IN const OsMutex&) = delete;

public:
    void Lock() override;
    void Unlock() override;

    IMS_SINT32 TryLock();
    IMS_PVOID GetMutexObj();

private:
    IMS_SINT32 GetMutexType();
    IMS_SINT32 SetMutexType(IN IMS_SINT32 nType);

public:
    enum
    {
        ATTRIBUTE_NORMAL = 0,
        ATTRIBUTE_RECURSIVE,
        ATTRIBUTE_ERRORCHECK,
        ATTRIBUTE_MAX
    };

private:
    OsMutexPrivate* m_pMutexP;
};

#endif
