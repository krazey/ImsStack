/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "AsyncExecutor.h"
#include "PlatformContext.h"

#include "MockAsyncExecutor_IExecutor.h"
#include "MockAsyncExecutor_IListener.h"
#include "TestThreadService.h"

using ::testing::Eq;

namespace android
{

MATCHER_P2(IsMessageEqual, objExpectedMsg, piCallback, "")
{
    ImsMessage& objMsg = static_cast<ImsMessage&>(arg);
    return objMsg.GetName() == objExpectedMsg.GetName() &&
            objMsg.nWparam == objExpectedMsg.nWparam && objMsg.nLparam == objExpectedMsg.nLparam &&
            objMsg.IsSameCallback(piCallback);
}

class TestAsyncExecutor : public AsyncExecutor
{
public:
    inline explicit TestAsyncExecutor(IN IMS_BOOL bAutoDestroy) :
            AsyncExecutor(bAutoDestroy)
    {
    }
    inline TestAsyncExecutor(IN IListener* piListener, IN IMS_BOOL bAutoDestroy) :
            AsyncExecutor(piListener, bAutoDestroy)
    {
    }
    inline TestAsyncExecutor(
            IN IThread* piOwnerThread, IN IListener* piListener, IN IMS_BOOL bAutoDestroy) :
            AsyncExecutor(piOwnerThread, piListener, bAutoDestroy)
    {
    }
    ~TestAsyncExecutor() = default;

public:
    inline IMS_SINT32 GetExecuteMessage() const { return MSG_EXECUTE; }
    inline IMS_SINT32 GetDestroyMessage() const { return MSG_DESTROY; }
};

class AsyncExecutorTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pThreadService = new TestThreadService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
        m_pIExecutor = new MockAsyncExecutorIExecutor();
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete m_pThreadService;
    }

protected:
    TestThreadService* m_pThreadService;
    MockAsyncExecutorIExecutor* m_pIExecutor;
};

TEST_F(AsyncExecutorTest, Execute)
{
    TestAsyncExecutor objExecutor(IMS_FALSE);
    ImsMessage objExpectedMsg(objExecutor.GetExecuteMessage(), 0, 0);
    EXPECT_CALL(m_pThreadService->GetMockThread(),
            PostMessageI(IsMessageEqual(objExpectedMsg, &objExecutor)))
            .Times(1);

    objExecutor.Execute();
}

TEST_F(AsyncExecutorTest, ExecuteWithNullOwnerThread)
{
    TestAsyncExecutor objExecutor(IMS_NULL, IMS_NULL, IMS_FALSE);
    MockAsyncExecutorIExecutor objIExecutor;
    objExecutor.SetExecutor(&objIExecutor);
    EXPECT_CALL(objIExecutor, AsyncExecutor_OnExecute(Eq(&objExecutor), Eq(0), Eq(0))).Times(1);

    objExecutor.Execute();
}

TEST_F(AsyncExecutorTest, ExecuteWithListener)
{
    MockAsyncExecutorIListener objIListener;
    TestAsyncExecutor objExecutor(IMS_NULL, &objIListener, IMS_FALSE);
    MockAsyncExecutorIExecutor objIExecutor;
    objExecutor.SetExecutor(&objIExecutor);
    EXPECT_CALL(objIExecutor, AsyncExecutor_OnExecute(Eq(&objExecutor), Eq(0), Eq(0))).Times(1);
    EXPECT_CALL(objIListener, AsyncExecutor_OnExecuteCompleted(Eq(&objExecutor))).Times(1);

    objExecutor.Execute();
}

TEST_F(AsyncExecutorTest, Destroy)
{
    TestAsyncExecutor objExecutor(IMS_FALSE);
    ImsMessage objExpectedMsg(objExecutor.GetDestroyMessage(), 0, 0);
    EXPECT_CALL(m_pThreadService->GetMockThread(),
            PostMessageI(IsMessageEqual(objExpectedMsg, &objExecutor)))
            .Times(1);

    objExecutor.Destroy();
}

TEST_F(AsyncExecutorTest, DestroyWithNullOwnerThreadAndAutoDestroy)
{
    TestAsyncExecutor* pExecutor = new TestAsyncExecutor(IMS_NULL, IMS_NULL, IMS_TRUE);

    pExecutor->Destroy();
}

}  // namespace android
