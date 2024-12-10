/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "BaseThread.h"
#include "EnablerUtils.h"
#include "ImsMessage.h"
#include "ImsProcess.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "helper/OperationAsyncRunner.h"
#include "helper/OperationAsyncRunnerManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Mock;

namespace android
{
LOCAL const IMS_SINT32 SLOT_ID = 0;

class TestBaseThread : public BaseThread
{
public:
    inline TestBaseThread() :
            BaseThread()
    {
    }
    inline ~TestBaseThread() {}
};

class OperationAsyncRunnerManagerTest : public ::testing::Test
{
public:
    inline OperationAsyncRunnerManagerTest() :
            pThreadService(new TestThreadService()),
            objManager(SLOT_ID)
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
    }
    inline ~OperationAsyncRunnerManagerTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

protected:
    TestThreadService* pThreadService;
    OperationAsyncRunnerManager objManager;

    virtual void SetUp() override
    {
        auto fnEntry = []() -> BaseThread*
        {
            return new TestBaseThread();
        };

        ImsProcess::GetInstance()->LoadThread(
                EnablerUtils::GetEnablerThreadName(SLOT_ID), fnEntry, SLOT_ID);
    }

    virtual void TearDown() override
    {
        ImsProcess::GetInstance()->UnloadThread(EnablerUtils::GetEnablerThreadName(SLOT_ID));
    }
};

TEST_F(OperationAsyncRunnerManagerTest, RunDoesNothingIfOwnerIsNull)
{
    EXPECT_CALL(pThreadService->GetMockThread(), PostMessageI(_)).Times(0);
    objManager.Run(IMS_NULL,
            []()
            {
            });
}

TEST_F(OperationAsyncRunnerManagerTest, RunDoesNothingIfOperationIsNull)
{
    EXPECT_CALL(pThreadService->GetMockThread(), PostMessageI(_)).Times(0);
    objManager.Run(this, IMS_NULL);
}

TEST_F(OperationAsyncRunnerManagerTest, RunCreatesOperationAsyncRunner)
{
    EXPECT_CALL(pThreadService->GetMockThread(), PostMessageI(_));
    objManager.Run(this,
            []()
            {
            });
}

TEST_F(OperationAsyncRunnerManagerTest, ReleaseCancelsOperation)
{
    OperationAsyncRunnerManager* pManager = new OperationAsyncRunnerManager(SLOT_ID);
    pManager->Run(this,
            []()
            {
            });
    pManager->Run(this,
            []()
            {
            });

    EXPECT_CALL(pThreadService->GetMockThread(), RemoveMessages(_, _)).Times(2);

    pManager->Release(this);

    Mock::VerifyAndClearExpectations(pThreadService);
    delete pManager;
}

TEST_F(OperationAsyncRunnerManagerTest, ReleaseDoesNotCancelOperationIfNotWaitingMessage)
{
    // Not applicable.
}

TEST_F(OperationAsyncRunnerManagerTest, DestructorCancelsOperations)
{
    OperationAsyncRunnerManager* pManager = new OperationAsyncRunnerManager(SLOT_ID);
    pManager->Run(reinterpret_cast<void*>(0x01),
            []()
            {
            });
    pManager->Run(reinterpret_cast<void*>(0x02),
            []()
            {
            });

    EXPECT_CALL(pThreadService->GetMockThread(), RemoveMessages(_, _)).Times(2);

    delete pManager;
}

}  // namespace android
