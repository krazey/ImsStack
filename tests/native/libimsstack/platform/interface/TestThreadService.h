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
#ifndef TEST_THREAD_SERVICE_H_
#define TEST_THREAD_SERVICE_H_

#include "MockIThread.h"
#include "ServiceThread.h"

class TestThreadService : public ThreadService
{
public:
    inline IThread* CreateThread(IN const AString& /*strName*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objThread;
    }
    inline void DestroyThread(IN IThread*& /*piThread*/) override {}

    inline IMS_BOOL Contains(IN const IThread* piThread) const override
    {
        return &m_objThread == piThread;
    }
    inline IMS_BOOL ContainsLocked(IN const IThread* piThread) const override
    {
        return &m_objThread == piThread;
    }
    inline IThread* GetCurrentThread() const override
    {
        return const_cast<MockIThread*>(&m_objThread);
    }
    inline IThread* GetThread(IN const AString& /*strName*/) const override
    {
        return const_cast<MockIThread*>(&m_objThread);
    }
    inline IThread* GetThreadLocked(IN const AString& /*strName*/) const override
    {
        return const_cast<MockIThread*>(&m_objThread);
    }

    inline MockIThread& GetMockThread() { return m_objThread; }

private:
    MockIThread m_objThread;
};

#endif
