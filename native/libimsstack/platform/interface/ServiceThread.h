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
#ifndef SERVICE_THREAD_H_
#define SERVICE_THREAD_H_

#include "INativeThreadMethods.h"
#include "IThread.h"
#include "PlatformService.h"

class IMutex;

class ThreadService : public PlatformService
{
public:
    ThreadService();
    ThreadService(IN const ThreadService&) = delete;
    ThreadService& operator=(IN const ThreadService&) = delete;

protected:
    virtual ~ThreadService();

public:
    virtual IThread* CreateThread(IN const AString& strName, IN IMS_SINT32 nSlotId);
    virtual void DestroyThread(IN IThread*& piThread);

    virtual IMS_BOOL Contains(IN const IThread* piThread) const;
    virtual IMS_BOOL ContainsLocked(IN const IThread* piThread) const;
    virtual IThread* GetCurrentThread() const;
    virtual IThread* GetThread(IN const AString& strName) const;
    virtual IThread* GetThreadLocked(IN const AString& strName) const;

    static ThreadService* GetThreadService();

    /**
     * It returns the slot-id based on the current thread.
     * If thread is not found, then it returns the input argument.
     * If thread is a Framework thread, then it always returns slot-0.
     */
    static IMS_SINT32 GetCurrentSlotId(IN IMS_SINT32 nDefaultSlotId = IMS_SLOT_ANY);

    /** Thread related method to interwork with the lower layer. */
    static INativeThreadMethods* GetNativeThreadMethods();
    static void SetNativeThreadMethods(IN INativeThreadMethods* piNativeThreadMethods);

private:
    void LockThreadPool() const;
    void UnlockThreadPool() const;

private:
    friend class MessageService;

    IMutex* m_piLock;
    // List of (IThread*)
    IMSList<IThread*> m_objThreads;

    static INativeThreadMethods* s_piNativeThreadMethods;
};

#endif
