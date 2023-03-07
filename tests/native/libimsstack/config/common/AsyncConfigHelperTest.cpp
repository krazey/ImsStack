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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>

#include "AsyncConfigHelper.h"
#include "ImsAppThread.h"
#include "ImsProcess.h"
#include "MockIAsyncConfig.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "IRunnable.h"
#include "TestThreadService.h"

using ::testing::_;
using ::testing::An;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;

namespace android
{
static const AString THREAD_NAME("TT00");

ImsAppThread* CreateTestImsApplicationThread()
{
    return new ImsAppThread();
}

class AsyncConfigHelperTest : public ::testing::Test
{
public:
    inline AsyncConfigHelperTest() :
            m_piRunnableThread(IMS_NULL),
            m_pThreadService(IMS_NULL),
            m_pOldThreadService(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pThreadService = new TestThreadService();
        m_pOldThreadService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
        m_pThreadService->SetThread(&m_objMockThread);

        m_piRunnableThread = IMS_NULL;

        EXPECT_CALL(m_objMockThread, SetRunnable(_)).Times(1);

        ON_CALL(m_objMockThread, SetRunnable)
                .WillByDefault(Invoke(
                        [&](IRunnable* piRunnable)
                        {
                            m_piRunnableThread = piRunnable;
                        }));

        EXPECT_CALL(m_objMockThread, PostMessageI(_)).Times(AnyNumber());

        ON_CALL(m_objMockThread, PostMessageI(An<ImsMessage&>()))
                .WillByDefault(Invoke(
                        [&](ImsMessage& objMsg)
                        {
                            return (m_piRunnableThread != IMS_NULL)
                                    ? m_piRunnableThread->Runnable_Run(objMsg)
                                    : IMS_FALSE;
                        }));

        EXPECT_CALL(m_objMockThread, Activate()).Times(1).WillOnce(Return(IMS_TRUE));
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pOldThreadService);
        delete m_pThreadService;

        m_piRunnableThread = IMS_NULL;
    }

public:
    IRunnable* m_piRunnableThread;
    MockIAsyncConfig m_objMockIAsyncConfig;
    MockIThread m_objMockThread;
    TestThreadService* m_pThreadService;
    PlatformService* m_pOldThreadService;
};

TEST_F(AsyncConfigHelperTest, SendToAndOnMessage)
{
    AsyncConfigHelper objAsyncConfigHelper;

    // 1. AsyncConfig not registered, fail
    EXPECT_EQ(objAsyncConfigHelper.SendTo(&m_objMockIAsyncConfig, 0, 0, 0), IMS_FALSE);

    objAsyncConfigHelper.Register(&m_objMockIAsyncConfig);
    // Already registered, should not register again.
    objAsyncConfigHelper.Register(&m_objMockIAsyncConfig);

    // 2. App thread not loaded, fail
    EXPECT_EQ(objAsyncConfigHelper.SendTo(&m_objMockIAsyncConfig, 0, 0, 0), IMS_FALSE);

    objAsyncConfigHelper.Unregister(&m_objMockIAsyncConfig);

    ImsProcess::GetInstance()->LoadAppThread(
            THREAD_NAME, CreateTestImsApplicationThread, IMS_SLOT_0);

    AsyncConfigHelper objAsyncConfigHelper1;

    objAsyncConfigHelper1.Register(&m_objMockIAsyncConfig);

    EXPECT_CALL(m_objMockIAsyncConfig, HandleMessage(_, _, _)).Times(1);

    // 3. App thread loaded and OnMessage returns success
    EXPECT_EQ(objAsyncConfigHelper1.SendTo(&m_objMockIAsyncConfig, 0, 0, 0), IMS_TRUE);

    // 4. AsyncConfig null and OnMessage returns fail
    EXPECT_EQ(objAsyncConfigHelper1.SendTo(IMS_NULL, 0, 0, 0), IMS_FALSE);

    objAsyncConfigHelper1.Unregister(&m_objMockIAsyncConfig);

    ImsProcess::GetInstance()->UnloadThread(THREAD_NAME);
}

}  // namespace android