/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "IRetryTimerListener.h"
#include "RetryTimer.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
RetryTimer::RetryTimer(IN IMS_BOOL bRepeatable_ /* = IMS_FALSE */) :
        nState(STATE_INACTIVE),
        piTimer(IMS_NULL),
        nTracker(0),
        bFlag_Repeatable(bRepeatable_),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RetryTimer::~RetryTimer()
{
    if (piTimer != IMS_NULL)
    {
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
    }

    IMS_TRACE_D("Destructor :: RetryTimer", 0, 0, 0);
}

/*
 Adds a time interval in milli-seconds

Remarks

*/
PUBLIC
IMS_BOOL RetryTimer::AddValue(IN IMS_UINT32 nInterval)
{
    if (nState == STATE_ACTIVE)
    {
        return IMS_FALSE;
    }

    return objIntervals.Append(nInterval);
}

/*
 Adds the set of time intervals in milli-seconds

Remarks

*/
PUBLIC
IMS_BOOL RetryTimer::AddValues(IN IMSList<IMS_UINT32>& objIntervals)
{
    if (nState == STATE_ACTIVE)
    {
        return IMS_FALSE;
    }

    return this->objIntervals.AppendList(objIntervals);
}

/*
 Returns the next time interval in milli-seconds

Remarks

*/
PUBLIC
IMS_UINT32 RetryTimer::GetNextInterval() const
{
    if (objIntervals.IsEmpty())
    {
        return 0;
    }

    if (nTracker >= objIntervals.GetSize())
    {
        return 0;
    }

    return objIntervals.GetAt(nTracker);
}

/*
 Returns the state of retry timer.

Remarks

*/
PUBLIC
IMS_SINT32 RetryTimer::GetState() const
{
    return nState;
}

/*
 Resumes the retry timer.
It can be invoked when receiving the result of the command.

Remarks

*/
PUBLIC
IMS_BOOL RetryTimer::Resume()
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Retry Timer is already in ACTIVE state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (nState != STATE_PENDING)
    {
        IMS_TRACE_E(0, "Retry Timer can't be resumed not in PENDING state (%d)", nState, 0, 0);
        return IMS_FALSE;
    }

    if (objIntervals.IsEmpty())
    {
        IMS_TRACE_E(0, "Retry Timer has no intervals", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!StartTimer())
    {
        IMS_TRACE_E(0, "Starting the retry timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/*
 Sets a listener when the timer expired

Remarks

*/
PUBLIC
void RetryTimer::SetListener(IN IRetryTimerListener* piListener)
{
    this->piListener = piListener;
}

/*
 Starts the retry timer rule

Remarks

*/
PUBLIC
IMS_BOOL RetryTimer::Start()
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Retry Timer is already in ACTIVE state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (objIntervals.IsEmpty())
    {
        IMS_TRACE_E(0, "Retry Timer has no intervals", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!StartTimer())
    {
        IMS_TRACE_E(0, "Starting the retry timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/*
 Terminates the retry timer

Remarks

*/
PUBLIC
void RetryTimer::Terminate()
{
    if (nState == STATE_INACTIVE)
    {
        return;
    }

    if (piTimer != IMS_NULL)
    {
        IMS_TRACE_D("Retry Timer (%p) is terminated (%02d-th)", piTimer, nTracker, 0);

        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
    }

    nTracker = 0;
    nState = STATE_INACTIVE;
}

/*
 It is invoked when the timer expired

Remarks

*/
PRIVATE VIRTUAL void RetryTimer::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (this->piTimer == IMS_NULL)
    {
        return;
    }

    if (!this->piTimer->Equals(piTimer))
    {
        return;
    }

    this->piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(this->piTimer);

    if (nTracker >= objIntervals.GetSize())
    {
        if (bFlag_Repeatable)
        {
            nTracker = objIntervals.GetSize() - 1;
        }
        else
        {
            nTracker = 0;
            nState = STATE_INACTIVE;

            if (piListener != IMS_NULL)
            {
                piListener->RetryTimer_OnFinalExpired(this);
            }

            return;
        }
    }

    // According to the result, re-start the timer
    if (piListener != IMS_NULL)
    {
        IMS_SINT32 nResult = piListener->RetryTimer_OnInterimExpired(this);

        if (nResult == RESULT_CONTINUE)
        {
            StartTimer();
        }
        else if (nResult == RESULT_PENDING)
        {
            // Do not start a timer & preserve the current state
            IMS_TRACE_I("Retry Timer is pending (%02d-th)", nTracker, 0, 0);

            nState = STATE_PENDING;
        }
        else
        {
            IMS_TRACE_I("Retry Timer is stopped (%02d-th)", nTracker, 0, 0);

            // Do not start a timer & clear the state
            nTracker = 0;
            nState = STATE_INACTIVE;
        }
    }
    else
    {
        StartTimer();
    }
}

/*
 It is invoked when the timer expired

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RetryTimer::StartTimer()
{
    piTimer = TimerService::GetTimerService()->CreateTimer();

    if (piTimer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const IMS_UINT32& nInterval = objIntervals.GetAt(nTracker);

    piTimer->SetTimer(nInterval, this);

    ++nTracker;

    IMS_TRACE_I("Retry Timer (%p, %d) is started (%02d-th)", piTimer, nInterval, nTracker);

    return IMS_TRUE;
}
