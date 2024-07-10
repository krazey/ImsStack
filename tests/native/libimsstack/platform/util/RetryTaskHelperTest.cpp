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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "RetryCmd.h"
#include "RetryTaskHelper.h"

#include "MockITimer.h"
#include "MockIRetryTaskHelperListener.h"
#include "PlatformContext.h"
#include "TestTimerService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class TestRetryCmd : public RetryCmd
{
public:
    explicit inline TestRetryCmd(IN IMS_UINT32 nCmdId = 0) :
            RetryCmd(nCmdId),
            m_nExecutionResult(IMS_SUCCESS),
            m_nExecutedCmd(nCmdId),
            m_bSetTimerExpectCallAfterExecute(IMS_FALSE),
            m_bSetTimerExpectCallBeforeExecute(IMS_FALSE),
            m_pMockTimer(IMS_NULL)
    {
    }

    inline ~TestRetryCmd() override {}

public:
    IMS_RESULT ExecuteCmd() override
    {
        if (m_bSetTimerExpectCallAfterExecute == IMS_TRUE)
        {
            EXPECT_CALL(*m_pMockTimer, SetTimer(_, _)).Times(1);
            m_bSetTimerExpectCallAfterExecute = IMS_FALSE;
        }

        if (m_bSetTimerExpectCallBeforeExecute == IMS_TRUE)
        {
            EXPECT_CALL(*m_pMockTimer, SetTimer(_, _)).Times(0);
            m_bSetTimerExpectCallBeforeExecute = IMS_FALSE;
        }

        return m_nExecutionResult;
    }

    void SetExecutedResult(IMS_RESULT nExecutionResult) { m_nExecutionResult = nExecutionResult; }

    void SetExecutedCommand(IMS_UINT32 nExecutedCmd) { m_nExecutedCmd = nExecutedCmd; }

    void SetTimerExpectCallAfterExecution(MockITimer* pMockTimer)
    {
        m_pMockTimer = pMockTimer;
        m_bSetTimerExpectCallAfterExecute = IMS_TRUE;
    }

    void SetTimerExpectCallBeforeExecution(MockITimer* pMockTimer)
    {
        m_pMockTimer = pMockTimer;
        m_bSetTimerExpectCallBeforeExecute = IMS_TRUE;
    }

    void CommandCompleted()
    {
        // 2nd arg 'nRetryAfter' is unused in RetryTaskHelper
        OnCmdCompleted(m_nExecutedCmd, 0);
    }

private:
    IMS_RESULT m_nExecutionResult;
    IMS_UINT32 m_nExecutedCmd;
    IMS_BOOL m_bSetTimerExpectCallAfterExecute;
    IMS_BOOL m_bSetTimerExpectCallBeforeExecute;
    MockITimer* m_pMockTimer;
};

class RetryTaskHelperTest : public ::testing::Test
{
public:
    inline RetryTaskHelperTest() :
            m_pOldTimerService(IMS_NULL),
            m_pTimerService(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pTimerService = new TestTimerService();
        m_pOldTimerService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, m_pTimerService);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, m_pOldTimerService);
        delete m_pTimerService;
    }

protected:
    MockITimer m_objTimer;
    PlatformService* m_pOldTimerService;
    TestTimerService* m_pTimerService;
};

TEST_F(RetryTaskHelperTest, SetCommand)
{
    RetryTaskHelper objRetryTaskHelper;
    TestRetryCmd objTestRetryCmd;

    // returned value previous command by default null
    EXPECT_TRUE(objRetryTaskHelper.SetCommand(&objTestRetryCmd) == nullptr);

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // Cannot change the command during active state
    EXPECT_TRUE(objRetryTaskHelper.SetCommand(&objTestRetryCmd) == nullptr);

    objRetryTaskHelper.Terminate();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // previous command should be same as last successful set command
    EXPECT_TRUE(objRetryTaskHelper.SetCommand(nullptr) == &objTestRetryCmd);
}

TEST_F(RetryTaskHelperTest, SetCondition)
{
    RetryTaskHelper objRetryTaskHelper;
    RetryCondition objRetryCondition;

    // returned value previous condition by default null
    EXPECT_TRUE(objRetryTaskHelper.SetCondition(&objRetryCondition) == nullptr);

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    TestRetryCmd objTestRetryCmd;
    objRetryTaskHelper.SetCommand(&objTestRetryCmd);

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // Cannot change the condition during active state
    EXPECT_TRUE(objRetryTaskHelper.SetCondition(&objRetryCondition) == nullptr);

    objRetryTaskHelper.Terminate();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // previous condition should be same as last successful set condition
    EXPECT_TRUE(objRetryTaskHelper.SetCondition(nullptr) == &objRetryCondition);
}

TEST_F(RetryTaskHelperTest, SetTimer)
{
    RetryTaskHelper objRetryTaskHelper;
    RetryTimer objRetryTimer;

    // returned value previous timer by default null
    EXPECT_TRUE(objRetryTaskHelper.SetTimer(&objRetryTimer) == nullptr);

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    TestRetryCmd objTestRetryCmd;
    objRetryTaskHelper.SetCommand(&objTestRetryCmd);

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // Cannot change the timer during active state
    EXPECT_TRUE(objRetryTaskHelper.SetTimer(&objRetryTimer) == nullptr);

    objRetryTaskHelper.Terminate();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // previous timer should be same as last successful set timer
    EXPECT_TRUE(objRetryTaskHelper.SetTimer(nullptr) == &objRetryTimer);
}

TEST_F(RetryTaskHelperTest, Start)
{
    RetryTaskHelper objRetryTaskHelper;

    // Retry command not added - fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start());

    // state should be inactive
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    TestRetryCmd objTestRetryCmd;

    // Set retry command
    objRetryTaskHelper.SetCommand(&objTestRetryCmd);

    // 1. start - START_COMMAND (default)

    // set retry command execution fail
    objTestRetryCmd.SetExecutedResult(IMS_FAILURE);

    // retry command execution fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start());
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // set retry command execution success
    objTestRetryCmd.SetExecutedResult(IMS_SUCCESS);

    // retry command execution success
    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // Terminate the retry task
    objRetryTaskHelper.Terminate();
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // 2. start - START_TIMER

    // retry timer not set - fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();
    EXPECT_CALL(objMockTimer, KillTimer()).Times(AnyNumber());

    RetryTimer objRetryTimer;
    objRetryTimer.AddValue(1000);

    // set retry timer
    EXPECT_TRUE(objRetryTaskHelper.SetTimer(&objRetryTimer) == nullptr);

    // start timer success
    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // Terminate the retry task
    objRetryTaskHelper.Terminate();
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // 3. start - START_COMMAND_N_TIMER

    // set retry timer to null, retry command already present
    objRetryTaskHelper.SetTimer(nullptr);

    // retry timer not set - fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start(RetryTaskHelper::START_COMMAND_N_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // set retry timer
    objRetryTaskHelper.SetTimer(&objRetryTimer);

    // set retry command execution fail
    objTestRetryCmd.SetExecutedResult(IMS_FAILURE);

    // retry command execution fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start(RetryTaskHelper::START_COMMAND_N_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // set retry command execution success
    objTestRetryCmd.SetExecutedResult(IMS_SUCCESS);

    // SetTimer should be called after retry command execution
    objTestRetryCmd.SetTimerExpectCallAfterExecution(&objMockTimer);

    // retry command execution success
    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start(RetryTaskHelper::START_COMMAND_N_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // Terminate the retry task
    objRetryTaskHelper.Terminate();
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // 4. start - START_TIMER_N_COMMAND

    // set retry timer to null, retry command already present
    objRetryTaskHelper.SetTimer(nullptr);

    // retry timer not set - fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER_N_COMMAND));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // set retry timer
    objRetryTaskHelper.SetTimer(&objRetryTimer);

    // set retry command execution fail
    objTestRetryCmd.SetExecutedResult(IMS_FAILURE);

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(1);

    // retry command execution fail
    EXPECT_EQ(IMS_FALSE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER_N_COMMAND));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    objRetryTimer.Terminate();

    // set retry command execution success
    objTestRetryCmd.SetExecutedResult(IMS_SUCCESS);

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(1);

    // SetTimer should be called before retry command execution
    objTestRetryCmd.SetTimerExpectCallBeforeExecution(&objMockTimer);

    // retry command execution success
    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER_N_COMMAND));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    // If already active, start should return immediately
    objTestRetryCmd.SetExecutedResult(IMS_FAILURE);
    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(0);
    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER_N_COMMAND));

    // Terminate the retry task
    objRetryTaskHelper.Terminate();
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // Already terminated, should return immediately
    objRetryTaskHelper.Terminate();
}

TEST_F(RetryTaskHelperTest, RetryCmd_OnCompleted)
{
    RetryTaskHelper objRetryTaskHelper;
    TestRetryCmd objTestRetryCmd;

    objRetryTaskHelper.SetCommand(&objTestRetryCmd);

    MockIRetryTaskHelperListener objMockIRetryTaskHelperListener;

    objRetryTaskHelper.SetListener(&objMockIRetryTaskHelperListener);

    // 1. No Retry Condition added, result is ok and terminates the retry task - success
    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    EXPECT_CALL(objMockIRetryTaskHelperListener,
            RetryTaskHelper_OnCompleted(_, _, RetryTaskHelper::RESULT_OK))
            .Times(1);

    objTestRetryCmd.CommandCompleted();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // 2. Added condition and but no retry codes included, result is ok and terminates the retry
    // task - success
    RetryCondition objRetryCondition;
    objRetryTaskHelper.SetCondition(&objRetryCondition);

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    EXPECT_CALL(objMockIRetryTaskHelperListener,
            RetryTaskHelper_OnCompleted(_, _, RetryTaskHelper::RESULT_OK))
            .Times(1);

    objTestRetryCmd.CommandCompleted();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // 3. Added condition and with retry codes included but wrong code executed, result is ok and
    // terminates the retry task
    objRetryCondition.Add(100);       // Retry single code
    objRetryCondition.Add(101, 199);  // Retry range code

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    EXPECT_CALL(objMockIRetryTaskHelperListener,
            RetryTaskHelper_OnCompleted(_, _, RetryTaskHelper::RESULT_OK))
            .Times(1);

    objTestRetryCmd.SetExecutedCommand(220);
    objTestRetryCmd.CommandCompleted();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_INACTIVE);

    // 4. Add timer, starts retry timer and task state is still active
    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();
    EXPECT_CALL(objMockTimer, KillTimer()).Times(AnyNumber());

    RetryTimer objRetryTimer;
    objRetryTimer.AddValue(1000);
    objRetryTaskHelper.SetTimer(&objRetryTimer);

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(1);

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start());

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    EXPECT_CALL(objMockIRetryTaskHelperListener, RetryTaskHelper_OnCompleted(_, _, _)).Times(0);

    objTestRetryCmd.SetExecutedCommand(120);
    objTestRetryCmd.CommandCompleted();

    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);
}

TEST_F(RetryTaskHelperTest, RetryTimer_OnInterimExpired)
{
    RetryTaskHelper objRetryTaskHelper;
    TestRetryCmd objTestRetryCmd;

    objRetryTaskHelper.SetCommand(&objTestRetryCmd);

    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();

    RetryTimer objRetryTimer;
    objRetryTimer.AddValue(1000);
    objRetryTimer.AddValue(2000);
    objRetryTimer.AddValue(3000);
    objRetryTaskHelper.SetTimer(&objRetryTimer);

    ITimerListener* pTimerListener = IMS_NULL;

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(2);

    ON_CALL(objMockTimer, SetTimer)
            .WillByDefault(Invoke(
                    [&](Unused, ITimerListener* pListener)
                    {
                        pTimerListener = pListener;
                        return 0;
                    }));

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    ASSERT_TRUE(pTimerListener != nullptr);

    // passing different timer object - ignore, no action
    EXPECT_CALL(objMockTimer, Equals(_)).Times(1).WillOnce(Return(IMS_FALSE));
    MockITimer objDiffMockTimer;
    pTimerListener->Timer_TimerExpired(&objDiffMockTimer);

    // passing proper timer with only one interval added,
    // current retry timer is active - starts timer again after command execution.
    EXPECT_CALL(objMockTimer, KillTimer()).Times(AnyNumber());
    EXPECT_CALL(objMockTimer, Equals(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));

    pTimerListener->Timer_TimerExpired(&objMockTimer);

    // If command execution fails during retry,
    // notify to retry task helper listener result as NOK and make retry timer inactive
    MockIRetryTaskHelperListener objMockIRetryTaskHelperListener;
    objRetryTaskHelper.SetListener(&objMockIRetryTaskHelperListener);
    objTestRetryCmd.SetExecutedResult(IMS_FAILURE);

    EXPECT_CALL(objMockIRetryTaskHelperListener,
            RetryTaskHelper_OnCompleted(_, _, RetryTaskHelper::RESULT_NOK_INTERNAL_OPERATION))
            .Times(1);

    pTimerListener->Timer_TimerExpired(&objMockTimer);

    EXPECT_EQ(objRetryTimer.GetState(), RetryTimer::STATE_INACTIVE);
}

TEST_F(RetryTaskHelperTest, RetryTimer_OnFinalExpired)
{
    RetryTaskHelper objRetryTaskHelper;
    TestRetryCmd objTestRetryCmd;

    objRetryTaskHelper.SetCommand(&objTestRetryCmd);

    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();

    RetryTimer objRetryTimer;
    objRetryTimer.AddValue(1000);

    objRetryTaskHelper.SetTimer(&objRetryTimer);

    ITimerListener* pTimerListener = IMS_NULL;

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(1);

    ON_CALL(objMockTimer, SetTimer)
            .WillByDefault(Invoke(
                    [&](Unused, ITimerListener* pListener)
                    {
                        pTimerListener = pListener;
                        return 0;
                    }));

    EXPECT_EQ(IMS_TRUE, objRetryTaskHelper.Start(RetryTaskHelper::START_TIMER));
    EXPECT_TRUE(objRetryTaskHelper.GetState() == RetryTaskHelper::STATE_ACTIVE);

    ASSERT_TRUE(pTimerListener != nullptr);

    EXPECT_CALL(objMockTimer, KillTimer()).Times(AnyNumber());
    EXPECT_CALL(objMockTimer, Equals(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));

    MockIRetryTaskHelperListener objMockIRetryTaskHelperListener;
    objRetryTaskHelper.SetListener(&objMockIRetryTaskHelperListener);

    EXPECT_CALL(objMockIRetryTaskHelperListener,
            RetryTaskHelper_OnCompleted(_, _, RetryTaskHelper::RESULT_NOK_TIMER_EXPIRED))
            .Times(1);

    // notify to retry task helper listener result as NOK timer expired and make retry timer
    // inactive
    pTimerListener->Timer_TimerExpired(&objMockTimer);

    EXPECT_EQ(objRetryTimer.GetState(), RetryTimer::STATE_INACTIVE);
}

}  // namespace android