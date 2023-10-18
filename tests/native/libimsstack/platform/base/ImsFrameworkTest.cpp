/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>

#include "ImsFramework.h"
#include "ImsMessageDef.h"
#include "MockIFrameworkThreadListener.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "TestMutexService.h"

namespace android
{

class TestImsFramework : public ImsFramework
{
public:
    inline TestImsFramework() :
            ImsFramework()
    {
    }
    inline ~TestImsFramework() {}

    IMS_BOOL TestInitialize() { return Initialize(); }
    void TestUninitialize() { Uninitialize(); }

    IMS_BOOL TestOnStart(IN ImsMessage& objMsg) { return OnStart(objMsg); }
    IMS_BOOL TestOnTerminate(IN ImsMessage& objMsg) { return OnTerminate(objMsg); }
    IMS_BOOL TestOnMessage(IN ImsMessage& objMsg) { return OnMessage(objMsg); }
};

class ImsFrameworkTest : public ::testing::Test
{
public:
    MockIThread m_objMockThread;
    MockIFrameworkThreadListener m_objMockFrameworkListener;
    TestThreadService m_objThreadService;
    TestMutexService m_objMutexService;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, &m_objMutexService);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_MUTEX, IMS_NULL);
    }
};

TEST_F(ImsFrameworkTest, AddListener)
{
    ImsFramework* pFramework = new ImsFramework();
    ASSERT_TRUE(pFramework != nullptr);

    pFramework->AddListener(IMS_NULL);

    EXPECT_CALL(m_objMutexService.GetMockMutex(), Lock()).Times(3);
    EXPECT_CALL(m_objMutexService.GetMockMutex(), Unlock()).Times(3);

    pFramework->AddListener(&m_objMockFrameworkListener);
    pFramework->AddListener(&m_objMockFrameworkListener);

    pFramework->RemoveListener(IMS_NULL);
    pFramework->RemoveListener(&m_objMockFrameworkListener);

    delete pFramework;
}

TEST_F(ImsFrameworkTest, Initialize)
{
    TestImsFramework* pTestFramework = new TestImsFramework();

    EXPECT_CALL(m_objMutexService.GetMockMutex(), Lock()).Times(4);
    EXPECT_CALL(m_objMutexService.GetMockMutex(), Unlock()).Times(4);

    EXPECT_CALL(m_objMockFrameworkListener, FrameworkThread_OnStarted()).Times(1);
    EXPECT_CALL(m_objMockFrameworkListener, FrameworkThread_OnTerminated()).Times(1);

    pTestFramework->AddListener(&m_objMockFrameworkListener);

    EXPECT_EQ(pTestFramework->TestInitialize(), IMS_TRUE);
    ImsMessage objMsg(IMS_MSG_START, 0, 0);
    EXPECT_EQ(pTestFramework->TestOnStart(objMsg), IMS_TRUE);

    ImsMessage objTerminateMsg(IMS_MSG_TERMINATE, 0, 0);
    EXPECT_EQ(pTestFramework->TestOnTerminate(objTerminateMsg), IMS_TRUE);

    ImsMessage objAppMsg(IMS_MSG_APP_CONTROL, 0, 0);
    EXPECT_EQ(pTestFramework->TestOnMessage(objAppMsg), IMS_TRUE);

    pTestFramework->RemoveListener(&m_objMockFrameworkListener);
    pTestFramework->TestUninitialize();

    delete pTestFramework;
}

}  // namespace android
