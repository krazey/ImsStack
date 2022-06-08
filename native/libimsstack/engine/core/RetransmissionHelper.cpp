/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090722  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "IMSLib.h"
#include "SipConfigProxy.h"
#include "base/IMS.h"
#include "IRetransmissionHelperListener.h"
#include "Service.h"
#include "RetransmissionHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
RetransmissionHelper::RetransmissionHelper(
        IN Service* pService_, IN IMS_BOOL bIntervalCap /* = IMS_TRUE */) :
        nDuration(TIMER_T1),
        nCumulativeDuration(TIMER_T1),
        nMaxDuration(TIMER_MAX),
        nIntervalCap(TIMER_MAX),
        piTimer(IMS_NULL),
        piListener(IMS_NULL)
{
    const ISipConfigV* piSipConfigV = pService_->GetISipConfigV();

    nDuration = SipConfigProxy::GetTimerValueT1(
            pService_->GetSlotId(), pService_->GetSipProfile(), piSipConfigV);

    nCumulativeDuration = nDuration;
    nMaxDuration = nDuration * 64;

    if (bIntervalCap)
    {
        // Interval cap will be read from the timer T2...
        nIntervalCap = SipConfigProxy::GetTimerValueT2(
                pService_->GetSlotId(), pService_->GetSipProfile(), piSipConfigV);
    }
    else
    {
        nIntervalCap = nMaxDuration;
    }
}

PUBLIC VIRTUAL RetransmissionHelper::~RetransmissionHelper()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: RetransmissionHelper", 0, 0, 0);

    if (piTimer != IMS_NULL)
    {
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        piTimer = IMS_NULL;
    }
}

PUBLIC
void RetransmissionHelper::SetListener(IN IRetransmissionHelperListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PUBLIC
void RetransmissionHelper::SetMaxDuration(IN IMS_SINT32 nValue)
{
    //---------------------------------------------------------------------------------------------

    if (piTimer != IMS_NULL)
        return;

    nMaxDuration = nValue;
}

PUBLIC
IMS_RESULT RetransmissionHelper::Start()
{
    //---------------------------------------------------------------------------------------------

    if (piTimer != IMS_NULL)
    {
        IMS_TRACE_D("Retransmission timer is already running ...", 0, 0, 0);
        return IMS_SUCCESS;
    }

    piTimer = TimerService::GetTimerService()->CreateTimer();

    if (piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a retransmission timer failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piTimer->SetTimer(nDuration, this);
    /*
    {
        piTimer->Close();
        piTimer = IMS_NULL;

        IMS_ERROR0("Starting a retransmission timer failed");
        return IMS_FAILURE;
    }*/

    IMS_TRACE_I("Retransmission timer (%p) is started .....", piTimer, 0, 0);

    return IMS_SUCCESS;
}

PUBLIC
void RetransmissionHelper::Stop()
{
    //---------------------------------------------------------------------------------------------

    if (piTimer == IMS_NULL)
        return;

    IMS_TRACE_I("Retransmission timer (%p) is stopped .....", piTimer, 0, 0);

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
    piTimer = IMS_NULL;
}

PROTECTED VIRTUAL void RetransmissionHelper::Timer_TimerExpired(IN ITimer* piTimer)
{
    //---------------------------------------------------------------------------------------------

    if (this->piTimer == IMS_NULL)
    {
        return;
    }

    if (this->piTimer != piTimer)
    {
        return;
    }

    if (nCumulativeDuration == nMaxDuration)
    {
        if (piListener != IMS_NULL)
        {
            if (piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_TIMER_EXPIRED) !=
                    IMS_SUCCESS)
            {
                return;
            }
        }

        this->piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(this->piTimer);
        this->piTimer = IMS_NULL;
        return;
    }

    if (piListener != IMS_NULL)
    {
        if (piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_RETRANSMIT) != IMS_SUCCESS)
        {
            return;
        }
    }

    IMS_SINT32 nTempDuration = nDuration << 1;
    IMS_SINT32 nNextDuration = IMS_MIN(nTempDuration, nIntervalCap);

    nTempDuration = nNextDuration + nCumulativeDuration;

    if (nTempDuration <= nMaxDuration)
    {
        nDuration = nNextDuration;
        nCumulativeDuration = nTempDuration;
    }
    else
    {
        nDuration = nMaxDuration - nCumulativeDuration;
        nCumulativeDuration = nMaxDuration;
    }

    this->piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(this->piTimer);

    this->piTimer = TimerService::GetTimerService()->CreateTimer();

    if (this->piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a retransmission timer falied", 0, 0, 0);

        if (piListener != IMS_NULL)
        {
            piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_INTERNAL_ERROR);
        }

        return;
    }

    if (!this->piTimer->SetTimer(nDuration, this))
    {
        IMS_TRACE_E(0, "Starting a retransmission timer falied", 0, 0, 0);

        if (piListener != IMS_NULL)
        {
            piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_INTERNAL_ERROR);
        }

        return;
    }
}
