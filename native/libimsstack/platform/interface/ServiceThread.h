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

#include "IThread.h"

class IMutex;

class ThreadService
{
private:
    ThreadService();
    ~ThreadService();

public:
    ThreadService(IN const ThreadService&) = delete;
    ThreadService& operator=(IN const ThreadService&) = delete;

public:
    inline IThread* Create(IN const AString& strName)
            __IMS_DEPRECATED__("Use Create(AString,IMS_SINT32) instead")
    {
        return Create(strName, IMS_SLOT_0);
    }
    IThread* Create(IN const AString& strName, IN IMS_SINT32 nSlotId);
    // Creates a thread which needs to communicate with the external module (IN & OUT)
    IThread* CreateEx(IN const AString& strName, IN IMS_SINT32 nSlotId);
    void Destroy(IN IThread*& piThread);

    IMS_BOOL Contains(IN const IThread* piThread) const;
    IMS_BOOL ContainsLocked(IN const IThread* piThread) const;
    IThread* GetCurrentThread() const;
    IThread* GetThread(IN const AString& strName) const;
    IThread* GetThreadLocked(IN const AString& strName) const;

    static ThreadService* GetThreadService();

    /**
     * It returns the slot-id based on the current thread.
     * If thread is not found, then it returns the input argument.
     * If thread is a Framework thread, then it always returns slot-0.
     */
    static IMS_SINT32 GetCurrentSlotId(IN IMS_SINT32 nDefaultSlotId = IMS_SLOT_ANY);

private:
    void LockThreadPool() const;
    void UnlockThreadPool() const;

private:
    friend class MessageService;

    IMutex* m_piLock;
    // List of (IThread*)
    IMSList<IThread*> m_objThreads;
};

#endif
