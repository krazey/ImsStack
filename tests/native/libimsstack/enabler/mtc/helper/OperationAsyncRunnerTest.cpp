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

#include "BaseThread.h"
#include "EnablerUtils.h"
#include "ImsMessage.h"
#include "ImsProcess.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "helper/OperationAsyncRunner.h"
#include <gtest/gtest.h>

using ::testing::_;

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

class OperationAsyncRunnerTest : public ::testing::Test
{
public:
    inline OperationAsyncRunnerTest() :
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
    }
    inline ~OperationAsyncRunnerTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

protected:
    TestThreadService* pThreadService;

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

TEST_F(OperationAsyncRunnerTest, OperationIsNotRunSynchronously)
{
    EXPECT_CALL(pThreadService->GetMockThread(), PostMessageI(_));

    IMS_BOOL bUpdated = IMS_FALSE;
    OperationAsyncRunner* pRunner = new OperationAsyncRunner(SLOT_ID);
    pRunner->SetOperation(
            [&]()
            {
                bUpdated = IMS_TRUE;
            },
            []()
            {
            });

    EXPECT_FALSE(bUpdated);

    ImsMessage objMessage(0, 0, 0);
    pRunner->MessageCallback_OnMessage(objMessage);
    EXPECT_TRUE(bUpdated);

    delete pRunner;
}

TEST_F(OperationAsyncRunnerTest, DeletedByRemoveCallback)
{
    EXPECT_CALL(pThreadService->GetMockThread(), PostMessageI(_));

    IMS_BOOL bDeleted = IMS_FALSE;
    OperationAsyncRunner* pRunner = new OperationAsyncRunner(SLOT_ID);
    pRunner->SetOperation(
            []()
            {
            },
            [&]()
            {
                bDeleted = IMS_TRUE;
                delete pRunner;
            });

    EXPECT_FALSE(bDeleted);

    ImsMessage objMessage(0, 0, 0);
    pRunner->MessageCallback_OnMessage(objMessage);
    EXPECT_TRUE(bDeleted);
}

TEST_F(OperationAsyncRunnerTest, DestructorRemovesMessage)
{
    OperationAsyncRunner* pRunner = new OperationAsyncRunner(SLOT_ID);
    pRunner->SetOperation(
            []()
            {
            },
            []()
            {
            });

    EXPECT_CALL(pThreadService->GetMockThread(), RemoveMessages(pRunner, _));

    delete pRunner;
}

TEST_F(OperationAsyncRunnerTest, IsOperationStartedReturnsFalse)
{
    OperationAsyncRunner* pRunner = new OperationAsyncRunner(SLOT_ID);
    pRunner->SetOperation(
            []()
            {
            },
            []()
            {
            });
    EXPECT_FALSE(pRunner->IsOperationStarted());

    delete pRunner;
}

TEST_F(OperationAsyncRunnerTest, IsWaitingMessageReturnsFalseAfterOperationIsStarted)
{
    OperationAsyncRunner* pRunner = new OperationAsyncRunner(SLOT_ID);
    pRunner->SetOperation(
            []()
            {
            },
            []()
            {
            });
    ImsMessage objMessage(0, 0, 0);
    pRunner->MessageCallback_OnMessage(objMessage);

    delete pRunner;
}

}  // namespace android
