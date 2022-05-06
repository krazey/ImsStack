/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100726  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IRetryTaskHelperListener.h"
#include "RetryTaskHelper.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
RetryTaskHelper::RetryTaskHelper(IN IMS_BOOL bTimerOnCmdCompleted /* = IMS_FALSE */) :
        nState(STATE_INACTIVE),
        bFlag_TimerOnCmdCompleted(bTimerOnCmdCompleted),
        pCmd(IMS_NULL),
        pCondition(IMS_NULL),
        pTimer(IMS_NULL),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RetryTaskHelper::~RetryTaskHelper()
{
    IMS_TRACE_D("Destructor :: RetryTaskHelper", 0, 0, 0);
}

/*
 Returns the state of current retry task.

Remarks

*/
PUBLIC
IMS_SINT32 RetryTaskHelper::GetState() const
{
    return nState;
}

/*
 Sets the retry command of this task and returns the previous retry command if present.

Remarks

*/
PUBLIC
RetryCmd* RetryTaskHelper::SetCommand(IN RetryCmd* pCmd)
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Retry Command can not be set in the ACTIVE state", 0, 0, 0);
        return IMS_NULL;
    }

    RetryCmd* pPrevCmd = this->pCmd;

    this->pCmd = pCmd;

    return pPrevCmd;
}

/*
 Sets the retry condition of this task and returns the previous retry condition if present.

Remarks

*/
PUBLIC
RetryCondition* RetryTaskHelper::SetCondition(IN RetryCondition* pCondition)
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Retry Condition can not be set in the ACTIVE state", 0, 0, 0);
        return IMS_NULL;
    }

    RetryCondition* pPrevCondition = this->pCondition;

    this->pCondition = pCondition;

    return pPrevCondition;
}

/*
 Sets the retry timer of this task and returns the previous retry timer if present.

Remarks

*/
PUBLIC
RetryTimer* RetryTaskHelper::SetTimer(IN RetryTimer* pTimer)
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Retry Timer can not be set in the ACTIVE state", 0, 0, 0);
        return IMS_NULL;
    }

    RetryTimer* pPrevTimer = this->pTimer;

    this->pTimer = pTimer;

    return pPrevTimer;
}

/*
 Sets the listener of this retry task.

Remarks

*/
PUBLIC
void RetryTaskHelper::SetListener(IN IRetryTaskHelperListener* piListener)
{
    this->piListener = piListener;
}

/*
 Starts the retry task.
Before calling this method, the application MUST set the command & condition at least.
If the application runs the retry timer, then it also sets the timer.

Remarks

*/
PUBLIC
IMS_BOOL RetryTaskHelper::Start(IN IMS_SINT32 nParam /* = START_COMMAND */)
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Retry Task is already in ACTIVE state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (pCmd == IMS_NULL)
    {
        IMS_TRACE_E(0, "Retry Command MUST be set before calling this method", 0, 0, 0);
        return IMS_FALSE;
    }

    pCmd->SetCmdListener(this);

    if (pTimer != IMS_NULL)
    {
        pTimer->SetListener(this);
    }

    switch (nParam)
    {
        case START_COMMAND_N_TIMER:
            if (pTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter (%d) - No retry timer", nParam, 0, 0);
                return IMS_FALSE;
            }

            if (pCmd->ExecuteCmd() != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }

            if (!pTimer->Start())
            {
                return IMS_FALSE;
            }
            break;

        case START_TIMER:
            if (pTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter (%d) - No retry timer", nParam, 0, 0);
                return IMS_FALSE;
            }

            if (!pTimer->Start())
            {
                return IMS_FALSE;
            }
            break;

        case START_TIMER_N_COMMAND:
            if (pTimer == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter (%d) - No retry timer", nParam, 0, 0);
                return IMS_FALSE;
            }

            if (!pTimer->Start())
            {
                return IMS_FALSE;
            }

            if (pCmd->ExecuteCmd() != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;

        case START_COMMAND:
        default:
            if (pCmd->ExecuteCmd() != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
    }

    nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/*
 Terminates the retry task.

Remarks

*/
PUBLIC
void RetryTaskHelper::Terminate()
{
    if (nState == STATE_INACTIVE)
    {
        return;
    }

    pCmd->SetCmdListener(IMS_NULL);

    if (pTimer != IMS_NULL)
    {
        pTimer->SetListener(IMS_NULL);
        pTimer->Terminate();
    }

    nState = STATE_INACTIVE;
}

/*
 Notify the result of the command execution to the command's executor.

Remarks

*/
PROTECTED VIRTUAL void RetryTaskHelper::RetryCmd_OnCompleted(
        IN RetryCmd* pCmd, IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    if (nState == STATE_INACTIVE)
    {
        return;
    }

    if (pCmd != this->pCmd)
    {
        return;
    }

    (void)nRetryAfter;

    if ((pCondition != IMS_NULL) && (pCondition->Verify(nResultCode)))
    {
        if (pTimer == IMS_NULL)
        {
            CallListener(RESULT_NOK_COMMAND_FAILED);
        }
        else
        {
            IMS_SINT32 nTimerState = pTimer->GetState();

            if (nTimerState == RetryTimer::STATE_INACTIVE)
            {
                if (!pTimer->Start())
                {
                    CallListener(RESULT_NOK_INTERNAL_OPERATION);
                }
            }
            else if (nTimerState == RetryTimer::STATE_PENDING)
            {
                if (!pTimer->Resume())
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

/*
 Notify the timer expiration of the interim retry timer.

Remarks

*/
PROTECTED VIRTUAL IMS_SINT32 RetryTaskHelper::RetryTimer_OnInterimExpired(IN RetryTimer* pTimer)
{
    (void)pTimer;

    if (nState == STATE_INACTIVE)
    {
        return RetryTimer::RESULT_STOP;
    }

    // TODO:: retry-after handling

    if (pCmd->ExecuteCmd() != IMS_SUCCESS)
    {
        CallListener(RESULT_NOK_INTERNAL_OPERATION);
        return RetryTimer::RESULT_STOP;
    }

    if (nState == STATE_INACTIVE)
    {
        return RetryTimer::RESULT_STOP;
    }

    if (bFlag_TimerOnCmdCompleted)
    {
        // Timer will be re-started after completing the command
        return RetryTimer::RESULT_PENDING;
    }

    return RetryTimer::RESULT_CONTINUE;
}

/*
 Notify the timer expiration of the final retry timer.

Remarks

*/
PROTECTED VIRTUAL void RetryTaskHelper::RetryTimer_OnFinalExpired(IN RetryTimer* pTimer)
{
    (void)pTimer;

    if (nState == STATE_INACTIVE)
    {
        return;
    }

    CallListener(RESULT_NOK_TIMER_EXPIRED);
}

/*
 Notify the result of the retry task.

Remarks

*/
PRIVATE
void RetryTaskHelper::CallListener(IN IMS_SINT32 nResultCode)
{
    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No retry task helper listener", 0, 0, 0);
        return;
    }

    piListener->RetryTaskHelper_OnCompleted(this, pCmd, nResultCode);
}
