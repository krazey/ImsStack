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
#include "ImsAppThread.h"
#include "ImsMessageDef.h"
#include "ImsProcess.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace android
{

class ImsAppThreadTest : public ::testing::Test
{
public:
    MockIThread m_objMockThread;
    TestThreadService m_objThreadService;

    ImsAppThread* m_pImsAppThread;
    IRunnable* m_piRunnable;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);

        m_pImsAppThread = new ImsAppThread();
        ASSERT_TRUE(m_pImsAppThread != nullptr);

        m_piRunnable = static_cast<IRunnable*>(m_pImsAppThread);
    }

    virtual void TearDown() override
    {
        if (m_pImsAppThread != IMS_NULL)
        {
            delete m_pImsAppThread;
            m_pImsAppThread = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }
};

ImsApp* CreateTestApp(IN const AString& strName)
{
    return new ImsApp(strName);
}

ImsAppThread* CreateTestImsAppThread()
{
    return new ImsAppThread();
}

TEST_F(ImsAppThreadTest, AppControlTest)
{
    ImsProcess* pProcess = ImsProcess::GetInstance();
    pProcess->LoadAppThread("TT00", CreateTestImsAppThread, IMS_SLOT_0);

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _))
            .Times(4)
            .WillRepeatedly(Invoke(
                    [&](IMS_UINT32 nMsg, IMS_UINTP nWparam, IN IMS_UINTP nLparam)
                    {
                        ImsMessage objMsg(nMsg, nWparam, nLparam);
                        EXPECT_EQ(m_piRunnable->Runnable_Run(objMsg), IMS_TRUE);
                        return IMS_TRUE;
                    }));

    AString strName("unit_test");

    m_pImsAppThread->AddApp(CreateTestApp, strName);

    EXPECT_CALL(m_objMockThread, SetRunnable(_)).Times(1);
    EXPECT_CALL(m_objMockThread, Activate()).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(m_pImsAppThread->Start(strName, IMS_SLOT_0), IMS_TRUE);

    m_pImsAppThread->AddApp(CreateTestApp, strName);
    AString strName2 = "unit_test2";
    strName = "TT00.unit_test";
    m_pImsAppThread->AddApp(CreateTestApp, strName2);
    m_pImsAppThread->RemoveApp(strName);
    strName2 = "TT00.unit_test2";
    m_pImsAppThread->RemoveAndDestroyApp(strName2);

    EXPECT_CALL(m_objMockThread, Deactivate()).Times(2);
    m_pImsAppThread->Terminate();

    pProcess->UnloadThread("TT00");
}

TEST_F(ImsAppThreadTest, AppThreadTest)
{
    ImsMessage objMsg(IMS_MSG_START, 0, 0);
    EXPECT_EQ(m_piRunnable->Runnable_Run(objMsg), IMS_TRUE);

    ImsMessage objTerminateMsg(IMS_MSG_TERMINATE, 0, 0);
    EXPECT_EQ(m_piRunnable->Runnable_Run(objTerminateMsg), IMS_TRUE);
}

}  // namespace android
