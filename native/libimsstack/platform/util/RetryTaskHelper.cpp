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
#include "IRetryTaskHelperListener.h"
#include "RetryTaskHelper.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
RetryTaskHelper::RetryTaskHelper(IN IMS_BOOL bTimerOnCmdCompleted /*= IMS_FALSE*/) :
        m_nState(STATE_INACTIVE),
        m_bTimerOnCmdCompleted(bTimerOnCmdCompleted),
        m_pCmd(IMS_NULL),
        m_pCondition(IMS_NULL),
        m_pTimer(IMS_NULL),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RetryTaskHelper::~RetryTaskHelper()
{
    IMS_TRACE_D("Destructor :: RetryTaskHelper", 0, 0, 0);
}

/**
 * @brief Sets the retry command of this task and returns the previous retry command if present.
 */
PUBLIC
RetryCmd* RetryTaskHelper::SetCommand(IN RetryCmd* pCmd)
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Retry Command can not be set in the ACTIVE state", 0, 0, 0);
        return IMS_NULL;
    }

    RetryCmd* pPrevCmd = m_pCmd;

    m_pCmd = pCmd;

    return pPrevCmd;
}

/**
 * @brief Sets the retry condition of this task and returns the previous retry condition if present.
 */
PUBLIC
RetryCondition* RetryTaskHelper::SetCondition(IN RetryCondition* pCondition)
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Retry Condition can not be set in the ACTIVE state", 0, 0, 0);
        return IMS_NULL;
    }

    RetryCondition* pPrevCondition = m_pCondition;

    m_pCondition = pCondition;

    return pPrevCondition;
}

/**
 * @brief Sets the retry timer of this task and returns the previous retry timer if present.
 */
PUBLIC
RetryTimer* RetryTaskHelper::SetTimer(IN RetryTimer* pTimer)
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Retry Timer can not be set in the ACTIVE state", 0, 0, 0);
        return IMS_NULL;
    }

    RetryTimer* pPrevTimer = m_pTimer;

    m_pTimer = pTimer;

    return pPrevTimer;
}

/**
 * @brief Starts the retry task.
 *
 * Before calling this method, the application MUST set the command & condition at least.
 * If the application runs the retry timer, then it also sets the timer.
 */
PUBLIC
IMS_BOOL RetryTaskHelper::Start(IN IMS_SINT32 nParam /*= START_COMMAND*/)
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Retry Task is already in ACTIVE state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_pCmd == IMS_NULL)
    {
        IMS_TRACE_E(0, "Retry Command MUST be set before calling this method", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pCmd->SetCmdListener(this);

    if (m_pTimer != IMS_NULL)
    {
        m_pTimer->SetListener(this);
    }

    switch (nParam)
    {
        case START_COMMAND_N_TIMER:
            if (m_pTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter (%d) - No retry timer", nParam, 0, 0);
                return IMS_FALSE;
            }

            if (m_pCmd->ExecuteCmd() != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }

            if (!m_pTimer->Start())
            {
                return IMS_FALSE;
            }
            break;
        case START_TIMER:
            if (m_pTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter (%d) - No retry timer", nParam, 0, 0);
                return IMS_FALSE;
            }

            if (!m_pTimer->Start())
            {
                return IMS_FALSE;
            }
            break;
        case START_TIMER_N_COMMAND:
            if (m_pTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter (%d) - No retry timer", nParam, 0, 0);
                return IMS_FALSE;
            }

            if (!m_pTimer->Start())
            {
                return IMS_FALSE;
            }

            if (m_pCmd->ExecuteCmd() != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
        case START_COMMAND:  // FALL-THROUGH
        default:
            if (m_pCmd->ExecuteCmd() != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
    }

    m_nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/**
 * @brief Terminates the retry task.
 */
PUBLIC
void RetryTaskHelper::Terminate()
{
    if (m_nState == STATE_INACTIVE)
    {
        return;
    }

    m_pCmd->SetCmdListener(IMS_NULL);

    if (m_pTimer != IMS_NULL)
    {
        m_pTimer->SetListener(IMS_NULL);
        m_pTimer->Terminate();
    }

    m_nState = STATE_INACTIVE;
}

/**
 * @brief Notify the result of the command execution to the command's executor.
 */
PROTECTED VIRTUAL void RetryTaskHelper::RetryCmd_OnCompleted(
        IN RetryCmd* pCmd, IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter /*= 0*/)
{
    if (m_nState == STATE_INACTIVE)
    {
        return;
    }

    if (pCmd != m_pCmd)
    {
        return;
    }

    (void)nRetryAfter;

    if ((m_pCondition != IMS_NULL) && (m_pCondition->Verify(nResultCode)))
    {
        if (m_pTimer == IMS_NULL)
        {
            CallListener(RESULT_NOK_COMMAND_FAILED);
        }
        else
        {
            IMS_SINT32 nTimerState = m_pTimer->GetState();

            if (nTimerState == RetryTimer::STATE_INACTIVE)
            {
                if (!m_pTimer->Start())
                {
                    CallListener(RESULT_NOK_INTERNAL_OPERATION);
                }
            }
            else if (nTimerState == RetryTimer::STATE_PENDING)
            {
                if (!m_pTimer->Resume())
                {
                    CallListener(RESULT_NOK_INTERNAL_OPERATION);
                }
            }
        }
    }
    else
    {
        Terminate();
        CallListener(RESULT_OK);
    }
}

/**
 * @brief Notify the timer expiration of the interim retry timer.
 */
PROTECTED VIRTUAL IMS_SINT32 RetryTaskHelper::RetryTimer_OnInterimExpired(IN RetryTimer* pTimer)
{
    (void)pTimer;

    if (m_nState == STATE_INACTIVE)
    {
        return RetryTimer::RESULT_STOP;
    }

    // TODO:: retry-after handling

    if (m_pCmd->ExecuteCmd() != IMS_SUCCESS)
    {
        CallListener(RESULT_NOK_INTERNAL_OPERATION);
        return RetryTimer::RESULT_STOP;
    }

    if (m_nState == STATE_INACTIVE)
    {
        return RetryTimer::RESULT_STOP;
    }

    if (m_bTimerOnCmdCompleted)
    {
        // Timer will be re-started after completing the command
        return RetryTimer::RESULT_PENDING;
    }

    return RetryTimer::RESULT_CONTINUE;
}

/**
 * @brief Notify the timer expiration of the final retry timer.
 */
PROTECTED VIRTUAL void RetryTaskHelper::RetryTimer_OnFinalExpired(IN RetryTimer* pTimer)
{
    (void)pTimer;

    if (m_nState == STATE_INACTIVE)
    {
        return;
    }

    CallListener(RESULT_NOK_TIMER_EXPIRED);
}

/**
 * @brief Notify the result of the retry task.
 */
PRIVATE
void RetryTaskHelper::CallListener(IN IMS_SINT32 nResultCode)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No retry task helper listener", 0, 0, 0);
        return;
    }

    m_piListener->RetryTaskHelper_OnCompleted(this, m_pCmd, nResultCode);
}
