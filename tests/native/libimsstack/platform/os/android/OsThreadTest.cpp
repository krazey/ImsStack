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
#include <gtest/gtest.h>

#include "ImsMessageDef.h"
#include "MockIMessageCallback.h"
#include "MockIRunnable.h"
#include "OsThread.h"

namespace android
{
static const IMS_ULONG THREAD_ID = 1L;
static const IMS_CHAR THREAD_NAME[] = "TestOsThread";

MATCHER_P(IsSameMessageByName, name, "")
{
    return arg.GetName() == name;
}

class TestOsThread : public OsThread
{
public:
    inline TestOsThread() :
            OsThread(),
            m_nThreadId(0),
            m_bPthreadEnabled(IMS_FALSE),
            m_bJoinThread(IMS_FALSE)
    {
    }
    ~TestOsThread() override = default;

    inline IMS_BOOL IsJoinThreadCalled() { return m_bJoinThread; }
    inline void SetThreadId(IMS_ULONG nThreadId) { m_nThreadId = nThreadId; }
    inline void SetPthreadEnabled(IMS_BOOL bEnabled) { m_bPthreadEnabled = bEnabled; }
    inline IMS_BOOL IsSystemMessageEx(IMS_SINT32 nMsg) { return IsSystemMessage(nMsg); }

protected:
    inline pthread_t CreateThread() override
    {
        if (m_bPthreadEnabled)
        {
            return OsThread::CreateThread();
        }
        return m_nThreadId;
    }
    inline void JoinThread() override
    {
        m_bJoinThread = IMS_TRUE;

        if (m_bPthreadEnabled)
        {
            OsThread::JoinThread();
        }
    }
    inline IMS_BOOL SendSignal() override
    {
        if (m_bPthreadEnabled)
        {
            return OsThread::SendSignal();
        }

        return IMS_TRUE;
    }
    inline IMS_SINT32 WaitForSignal(IN IMS_SINT32 nMsgCount) override
    {
        if (m_bPthreadEnabled)
        {
            return OsThread::WaitForSignal(nMsgCount);
        }

        return 0;
    }

private:
    pthread_t m_nThreadId;
    IMS_BOOL m_bPthreadEnabled;
    IMS_BOOL m_bJoinThread;
};

class OsThreadTest : public ::testing::Test
{
public:
    inline OsThreadTest() :
            m_pRunnable(IMS_NULL),
            m_pThread(IMS_NULL)
    {
    }

    inline ~OsThreadTest()
    {
        if (m_pThread != IMS_NULL)
        {
            delete m_pThread;
        }

        if (m_pRunnable != IMS_NULL)
        {
            delete m_pRunnable;
        }
    }

protected:
    virtual void SetUp() override
    {
        m_pThread = new TestOsThread();
        m_pRunnable = new MockIRunnable();
    }

    virtual void TearDown() override
    {
        if (m_pThread != IMS_NULL)
        {
            delete m_pThread;
            m_pThread = IMS_NULL;
        }

        if (m_pRunnable != IMS_NULL)
        {
            delete m_pRunnable;
            m_pRunnable = IMS_NULL;
        }
    }

protected:
    MockIRunnable* m_pRunnable;
    TestOsThread* m_pThread;
};

TEST_F(OsThreadTest, Constructor)
{
    EXPECT_EQ(AString::ConstNull(), m_pThread->GetName());
    EXPECT_FALSE(m_pThread->IsRunning());
    EXPECT_EQ(0, m_pThread->GetThreadId());
}

TEST_F(OsThreadTest, Create)
{
    IMS_BOOL bCreated = m_pThread->Create(THREAD_NAME);

    EXPECT_TRUE(bCreated);
    EXPECT_EQ(AString(THREAD_NAME), m_pThread->GetName());
}

TEST_F(OsThreadTest, Equals)
{
    m_pThread->Create(THREAD_NAME);

    TestOsThread objThread1;
    objThread1.Create(THREAD_NAME);

    EXPECT_TRUE(m_pThread->Equals(&objThread1));

    TestOsThread objThread2;
    objThread2.Create("other-thread");

    EXPECT_FALSE(m_pThread->Equals(&objThread2));
}

TEST_F(OsThreadTest, Activate)
{
    m_pThread->SetThreadId(THREAD_ID);
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());
    EXPECT_CALL(*m_pRunnable, Runnable_Run(IsSameMessageByName(IMS_MSG_START)));
    EXPECT_CALL(*m_pRunnable, Runnable_Run(IsSameMessageByName(IMS_MSG_TERMINATE)));

    m_pThread->SetRunnable(m_pRunnable);
    m_pThread->PostMessage(IMS_MSG_TERMINATE);  // To exit the thread loop
    m_pThread->Run();
}

TEST_F(OsThreadTest, ActivateWhenThreadIsNotCreated)
{
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_FALSE(bActivated);
    EXPECT_FALSE(m_pThread->IsRunning());
    EXPECT_EQ(0, m_pThread->GetThreadId());
}

TEST_F(OsThreadTest, ActivateWhenAlreadyActivated)
{
    m_pThread->SetThreadId(THREAD_ID);
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());

    m_pThread->SetThreadId(0);
    bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());
}

TEST_F(OsThreadTest, Deactivate)
{
    m_pThread->SetThreadId(THREAD_ID);
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());

    m_pThread->Deactivate();

    EXPECT_TRUE(m_pThread->IsJoinThreadCalled());
    EXPECT_CALL(*m_pRunnable, Runnable_Run(IsSameMessageByName(IMS_MSG_START)));
    EXPECT_CALL(*m_pRunnable, Runnable_Run(IsSameMessageByName(IMS_MSG_TERMINATE)));

    m_pThread->SetRunnable(m_pRunnable);
    m_pThread->Run();
}

TEST_F(OsThreadTest, DeactivateWhenNotActivated)
{
    m_pThread->Deactivate();

    EXPECT_FALSE(m_pThread->IsJoinThreadCalled());
}

TEST_F(OsThreadTest, DeactivateWhenNoRunnable)
{
    m_pThread->SetThreadId(THREAD_ID);
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());

    m_pThread->Deactivate();

    EXPECT_TRUE(m_pThread->IsJoinThreadCalled());
    EXPECT_CALL(*m_pRunnable, Runnable_Run(IsSameMessageByName(IMS_MSG_START))).Times(0);
    EXPECT_CALL(*m_pRunnable, Runnable_Run(IsSameMessageByName(IMS_MSG_TERMINATE))).Times(0);

    m_pThread->Run();
}

TEST_F(OsThreadTest, PostMessageI)
{
    EXPECT_FALSE(m_pThread->PostMessageI(IMS_MSG_TERMINATE, 0, 0));

    m_pThread->SetThreadId(THREAD_ID);
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());

    EXPECT_TRUE(m_pThread->PostMessageI(IMS_MSG_TERMINATE, 0, 0));

    m_pThread->Run();  // Thread should be exited after calling Run().
}

TEST_F(OsThreadTest, RemoveMessages)
{
    m_pThread->SetThreadId(THREAD_ID);
    IMS_BOOL bActivated = m_pThread->Activate();

    EXPECT_TRUE(bActivated);
    EXPECT_TRUE(m_pThread->IsRunning());
    EXPECT_EQ(THREAD_ID, m_pThread->GetThreadId());

    MockIMessageCallback objCallback1;
    MockIMessageCallback objCallback2;
    ImsMessage objMsg1(101, 0, 0, &objCallback1);
    ImsMessage objMsg2(102, 0, 0, &objCallback1);
    ImsMessage objMsg3(103, 0, 0);
    ImsMessage objMsg4(104, 0, 0, &objCallback2);
    m_pThread->PostMessageI(objMsg1);
    m_pThread->PostMessageI(objMsg2);
    m_pThread->PostMessageI(objMsg3);
    m_pThread->PostMessageI(objMsg4);

    ImsList<ImsMessage> objImsMsgs;
    IMS_SINT32 nRemovedMsgCount = m_pThread->RemoveMessages(&objCallback1, &objImsMsgs);

    EXPECT_EQ(2, nRemovedMsgCount);
    EXPECT_EQ(2, objImsMsgs.GetSize());
    EXPECT_EQ(101, objImsMsgs.GetAt(0).GetName());
    EXPECT_EQ(102, objImsMsgs.GetAt(1).GetName());

    objImsMsgs.Clear();
    nRemovedMsgCount = m_pThread->RemoveMessages(&objCallback2, &objImsMsgs);

    EXPECT_EQ(1, nRemovedMsgCount);
    EXPECT_EQ(1, objImsMsgs.GetSize());
    EXPECT_EQ(104, objImsMsgs.GetAt(0).GetName());
}

TEST_F(OsThreadTest, IsSystemMessage)
{
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_NETWORK));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_SOCKET));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_BATTERY));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_NETWORK_STATUS));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_TIMER));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_CONFIGURATION));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_WIFI_STATUS));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_ISIM));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_USIM));
    EXPECT_TRUE(m_pThread->IsSystemMessageEx(IMS_MSG_RADIO));

    EXPECT_FALSE(m_pThread->IsSystemMessageEx(IMS_MSG_SYSTEM_BASE));
    EXPECT_FALSE(m_pThread->IsSystemMessageEx(IMS_MSG_SYSTEM_MAX));
}

}  // namespace android
