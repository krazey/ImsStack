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
#include "ImsProcess.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"

namespace android
{

class ImsAppTest : public ::testing::Test
{
public:
    MockIThread m_objMockThread;
    TestThreadService m_objThreadService;
    ImsApp* m_pImsApp;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);

        m_pImsApp = new ImsApp("unit_test");
        ASSERT_TRUE(m_pImsApp != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pImsApp != IMS_NULL)
        {
            delete m_pImsApp;
            m_pImsApp = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }
};

ImsAppThread* CreateTestAppThread()
{
    return new ImsAppThread();
}

TEST_F(ImsAppTest, AttachAndDetach)
{
    ImsProcess* pProcess = ImsProcess::GetInstance();
    pProcess->LoadAppThread("TT00", CreateTestAppThread, IMS_SLOT_0);

    AString strName("unit_test");
    EXPECT_EQ(m_pImsApp->AttachService(IMS_NULL), IMS_FALSE);
    ASSERT_TRUE(m_pImsApp->GetService(IMS_NULL) == nullptr);

    ImsService* pService = new ImsService(strName);
    EXPECT_EQ(m_pImsApp->AttachService(pService), IMS_TRUE);

    strName = "TT00.unit_test";
    EXPECT_EQ(m_pImsApp->GetService(strName), pService);
    ImsList<ImsService*> objServiceList = m_pImsApp->GetServices();
    EXPECT_EQ(objServiceList.GetSize(), 1);
    EXPECT_EQ(objServiceList.GetValueAt(0), pService);

    m_pImsApp->DetachService(pService);
    delete pService;
    ASSERT_TRUE(m_pImsApp->GetService(strName) == nullptr);

    pService = new ImsService(strName);
    EXPECT_EQ(m_pImsApp->AttachService(pService), IMS_TRUE);

    pProcess->UnloadThread("TT00");
}

}  // namespace android
