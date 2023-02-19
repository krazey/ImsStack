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

#include "ImsApp.h"
#include "ImsActivityManager.h"
#include "ImsMessageDef.h"
#include "ImsProcess.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"

namespace android
{

class ImsActivityManagerTest : public ::testing::Test
{
public:
    MockIThread m_objMockThread;
    TestThreadService m_objThreadService;

    ImsActivityManager* m_pActivityManager;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);

        m_pActivityManager = new ImsActivityManager();
        ASSERT_TRUE(m_pActivityManager != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pActivityManager != IMS_NULL)
        {
            delete m_pActivityManager;
            m_pActivityManager = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }
};

ImsAppThread* CreateAppThread()
{
    return new ImsAppThread();
}

TEST_F(ImsActivityManagerTest, AttachAndDetach)
{
    ImsProcess* pProcess = ImsProcess::GetInstance();
    pProcess->LoadAppThread("TT00", CreateAppThread, IMS_SLOT_0);

    AString strActivityName("unit_test");
    EXPECT_EQ(m_pActivityManager->Attach(IMS_NULL), IMS_FALSE);
    ASSERT_TRUE(m_pActivityManager->Get(strActivityName) == nullptr);

    ImsApp* pActivity = new ImsApp(strActivityName);
    EXPECT_EQ(m_pActivityManager->Attach(pActivity), IMS_TRUE);

    strActivityName = "TT00.unit_test";
    EXPECT_EQ(m_pActivityManager->Get(strActivityName), pActivity);
    ASSERT_TRUE(m_pActivityManager->GetController(strActivityName) == nullptr);

    m_pActivityManager->Detach(pActivity);
    ASSERT_TRUE(m_pActivityManager->Get(strActivityName) == nullptr);
    ASSERT_TRUE(m_pActivityManager->GetController(strActivityName) == nullptr);

    pProcess->UnloadThread("TT00");
}

TEST_F(ImsActivityManagerTest, GenerateName)
{
    AString strActivityName("unit_test");
    AString strOut = "ImsTestThread.unit_test";

    EXPECT_EQ(m_pActivityManager->GenerateName("ImsTestThread", strActivityName), strOut);

    strOut = "ImsTestThread.ATVT0_0";
    EXPECT_STREQ(
            m_pActivityManager->GenerateName("ImsTestThread", IMS_NULL).GetStr(), strOut.GetStr());
}

TEST_F(ImsActivityManagerTest, HandleMessage)
{
    AString strActivityName("unit_test");

    ImsApp* pActivity = new ImsApp(strActivityName);
    EXPECT_EQ(m_pActivityManager->Attach(pActivity), IMS_TRUE);

    ImsMessage objMsg(IMS_MSG_TIMER, 0, 0, "ImsTestThread.unit_test");
    EXPECT_EQ(m_pActivityManager->HandleMessage(objMsg), IMS_FALSE);

    ImsMessage objMessage(IMS_MSG_TIMER, 0, 0, "ImsTestThread.ims_test");
    EXPECT_EQ(m_pActivityManager->HandleMessage(objMessage), IMS_FALSE);

    m_pActivityManager->Detach(pActivity);
}

}  // namespace android
