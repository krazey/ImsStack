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

#include "ImsProcess.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "TestMutexService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{

class ImsProcessTest : public ::testing::Test
{
public:
    MockIThread m_objMockThread;
    TestThreadService m_objThreadService;
    TestMutexService m_objMutexService;

    ImsProcess* m_pImsProcess;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, &m_objMutexService);

        m_pImsProcess = ImsProcess::GetInstance();
        ASSERT_TRUE(m_pImsProcess != nullptr);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_MUTEX, IMS_NULL);
    }
};

ImsAppThread* TestAppThread()
{
    return IMS_NULL;
}

ImsAppThread* TestAppThreadWithParam(void* /*pParam*/)
{
    return IMS_NULL;
}

ImsAppThread* CreateTestAppThread(void* /*pParam*/)
{
    return new ImsAppThread();
}

TEST_F(ImsProcessTest, Initialize)
{
    EXPECT_STREQ(m_pImsProcess->GetFrameworkThreadName().GetStr(), "Framework");

    EXPECT_CALL(m_objMockThread, SetRunnable(_)).Times(1);
    EXPECT_CALL(m_objMockThread, Activate()).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockThread, Deactivate()).Times(1);

    EXPECT_EQ(m_pImsProcess->Initialize(), IMS_TRUE);
    m_pImsProcess->Uninitialize();
}

TEST_F(ImsProcessTest, LoadAppThreadTest)
{
    EXPECT_CALL(m_objMockThread, SetRunnable(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockThread, Activate()).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockThread, Deactivate()).Times(AnyNumber());

    AString strName("TT00");
    EXPECT_EQ(m_pImsProcess->LoadAppThread("TT11", TestAppThread, IMS_SLOT_0), IMS_FALSE);
    EXPECT_EQ(m_pImsProcess->LoadAppThread(strName, TestAppThread, IMS_SLOT_0), IMS_FALSE);

    EXPECT_EQ(m_pImsProcess->LoadAppThreadWithParam(
                      "TT11", TestAppThreadWithParam, IMS_NULL, IMS_SLOT_0),
            IMS_FALSE);
    EXPECT_EQ(m_pImsProcess->LoadAppThreadWithParam(
                      strName, TestAppThreadWithParam, IMS_NULL, IMS_SLOT_0),
            IMS_FALSE);
    EXPECT_EQ(m_pImsProcess->LoadAppThreadWithParam(
                      strName, CreateTestAppThread, IMS_NULL, IMS_SLOT_0),
            IMS_TRUE);

    ImsAppThread* pAppThread = m_pImsProcess->GetApplicationThread(strName);
    ASSERT_TRUE(pAppThread != nullptr);
    AString strActivityName("unit_test");

    ImsApp* pActivity = new ImsApp(strActivityName);
    EXPECT_EQ(pAppThread->GetActivityManager()->Attach(pActivity), IMS_TRUE);

    strName = "TT00.unit_test";
    ASSERT_TRUE(m_pImsProcess->GetController(strName) == nullptr);

    delete pActivity;
    m_pImsProcess->UnloadThread(strName);
}

}  // namespace android
