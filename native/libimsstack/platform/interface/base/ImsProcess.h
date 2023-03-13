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
#ifndef IMS_PROCESS_H_
#define IMS_PROCESS_H_

#include "IImsActivityController.h"
#include "ImsAppThread.h"

class IMutex;
class ImsThreadMap;

typedef BaseThread* (*Thread_Entry)();
typedef BaseThread* (*Thread_EntryEx)(void*);
typedef ImsAppThread* (*AppThread_Entry)();
typedef ImsAppThread* (*AppThread_EntryEx)(void*);

class ImsProcess
{
private:
    ImsProcess();
    ~ImsProcess();

public:
    ImsProcess(IN const ImsProcess&) = delete;
    ImsProcess& operator=(IN const ImsProcess&) = delete;

public:
    static ImsProcess* GetInstance();

    const AString& GetFrameworkThreadName() const;
    IMS_BOOL Initialize();
    void Uninitialize();

    IMS_BOOL LoadThread(
            IN const AString& strThreadName, IN Thread_Entry pfnThreadEntry, IN IMS_SINT32 nSlotId);
    IMS_BOOL LoadThreadWithParam(IN const AString& strThreadName,
            IN Thread_EntryEx pfnThreadEntryEx, IN void* pvParam, IN IMS_SINT32 nSlotId);
    void UnloadThread(IN const AString& strThreadName);
    BaseThread* GetThread(IN const AString& strThreadName);

    IMS_BOOL LoadAppThread(IN const AString& strThreadName, IN AppThread_Entry pfnThreadEntry,
            IN IMS_SINT32 nSlotId);
    IMS_BOOL LoadAppThreadWithParam(IN const AString& strThreadName,
            IN AppThread_EntryEx pfnThreadEntryEx, IN void* pvParam, IN IMS_SINT32 nSlotId);
    void UnloadAppThread(IN const AString& strThreadName);
    ImsAppThread* GetApplicationThread(IN const AString& strThreadName);

    IImsActivityController* GetController(IN const AString& strControllerName);

private:
    IMS_BOOL AttachThread(IN const AString& strName, IN BaseThread* pThread);
    void DetachThread(IN const AString& strName);
    AString GetThreadName(IN const AString& strTargetName);

private:
    AString m_strFrameworkThreadName;
    IMutex* m_piLock;
    ImsList<ImsThreadMap*> m_objThreads;
};

#endif
