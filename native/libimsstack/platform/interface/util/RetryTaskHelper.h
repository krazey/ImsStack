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
#ifndef RETRY_TASK_HELPER_H_
#define RETRY_TASK_HELPER_H_

#include "IRetryCmdListener.h"
#include "IRetryTimerListener.h"
#include "RetryCmd.h"
#include "RetryCondition.h"
#include "RetryTimer.h"

class IRetryTaskHelperListener;

class RetryTaskHelper : public IRetryCmdListener, public IRetryTimerListener
{
public:
    explicit RetryTaskHelper(IN IMS_BOOL bTimerOnCmdCompleted = IMS_FALSE);
    virtual ~RetryTaskHelper();

    RetryTaskHelper(IN const RetryTaskHelper&) = delete;
    RetryTaskHelper& operator=(IN const RetryTaskHelper&) = delete;

public:
    /**
     * @brief Returns the state of current retry task.
     */
    inline IMS_SINT32 GetState() const { return m_nState; }
    RetryCmd* SetCommand(IN RetryCmd* pCmd);
    RetryCondition* SetCondition(IN RetryCondition* pCondition);
    RetryTimer* SetTimer(IN RetryTimer* pTimer);

    /**
     * @brief Sets the listener of this retry task.
     */
    inline void SetListener(IN IRetryTaskHelperListener* piListener) { m_piListener = piListener; }
    IMS_BOOL Start(IN IMS_SINT32 nParam = START_COMMAND);
    void Terminate();

protected:
    // IRetryCmdListener class
    void RetryCmd_OnCompleted(
            IN RetryCmd* pCmd, IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter = 0) override;

    // IRetryTimerListener class
    IMS_SINT32 RetryTimer_OnInterimExpired(IN RetryTimer* pTimer) override;
    void RetryTimer_OnFinalExpired(IN RetryTimer* pTimer) override;

private:
    void CallListener(IN IMS_SINT32 nResultCode);

public:
    /// State of retry task
    enum
    {
        STATE_INACTIVE = 0,
        STATE_ACTIVE
    };

    /// Codes of result
    enum
    {
        RESULT_OK = 0,
        RESULT_NOK_TIMER_EXPIRED = (-1),
        RESULT_NOK_INTERNAL_OPERATION = (-2),
        RESULT_NOK_COMMAND_FAILED = (-3)
    };

    /// Start parameter of retry task
    enum
    {
        START_COMMAND = 0,
        START_COMMAND_N_TIMER,
        START_TIMER,
        START_TIMER_N_COMMAND,
    };

private:
    IMS_SINT32 m_nState;
    // If it is TRUE, the timer will be started after completing the command
    // If it is FALSE, the timer will be started after executing the command
    IMS_BOOL m_bTimerOnCmdCompleted;
    RetryCmd* m_pCmd;
    RetryCondition* m_pCondition;
    RetryTimer* m_pTimer;
    IRetryTaskHelperListener* m_piListener;
};

#endif
