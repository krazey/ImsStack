/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090825  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "Sip.h"
#include "ISipClientConnection.h"
#include "SipParsingHelper.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "util/IRefreshable.h"
#include "util/RefreshHelper.h"

__IMS_TRACE_TAG_IMS__;

PUBLIC
RefreshHelper::RefreshHelper(IN IRefreshable* piRefreshable_, IN IMS_BOOL bRepeatable_) :
        piRefreshable(piRefreshable_),
        nPolicy(POLICY_SPEC),
        nCriteriaInterval(CRITERIA_INTERVAL),
        nValueEorLT(50)  // half of expiration time
        ,
        nValueGT(MINIMUM_REMAIN_INTERVAL),
        bRepeatable(bRepeatable_),
        nDuration(0),
        nRemainDuration(0),
        piTimer(IMS_NULL),
        piRefreshSC(IMS_NULL)
        // SIP_MESSAGE_MEDIATOR
        ,
        piMessageMediator(IMS_NULL)
{
}

PUBLIC VIRTUAL RefreshHelper::~RefreshHelper()
{
    //---------------------------------------------------------------------------------------------

    if (piRefreshSC != IMS_NULL)
    {
        piRefreshSC->Close();
    }

    if (piTimer != IMS_NULL)
    {
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        piTimer = IMS_NULL;
    }
}

PUBLIC
void RefreshHelper::AbortConnection()
{
    //---------------------------------------------------------------------------------------------

    if (piRefreshSC == IMS_NULL)
        return;

    piRefreshSC->Close();
    piRefreshSC = IMS_NULL;
}

PUBLIC
ISipClientConnection* RefreshHelper::GetConnection() const
{
    //---------------------------------------------------------------------------------------------

    return piRefreshSC;
}

PUBLIC
IMS_SINT32 RefreshHelper::GetDuration() const
{
    //---------------------------------------------------------------------------------------------

    return nDuration;
}

PUBLIC
IMS_BOOL RefreshHelper::IsRequestPending() const
{
    //---------------------------------------------------------------------------------------------

    return (piRefreshSC != IMS_NULL);
}

PUBLIC
IMS_BOOL RefreshHelper::IsTimerActive() const
{
    //---------------------------------------------------------------------------------------------

    return (piTimer != IMS_NULL);
}

// SIP_MESSAGE_MEDIATOR
PUBLIC
void RefreshHelper::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    piMessageMediator = piMediator;
}

PUBLIC
void RefreshHelper::SetPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
        IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    this->nPolicy = nPolicy;
    this->nCriteriaInterval = nCriteriaInterval;
    this->nValueEorLT = nValueEorLT;
    this->nValueGT = nValueGT;
}

PROTECTED VIRTUAL IMS_RESULT RefreshHelper::SendRefreshRequest(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Overwrites the SIP client connection listener
    piSCC->SetErrorListener(this);
    piSCC->SetListener(this);

    // SIP_MESSAGE_MEDIATOR
    if (piMessageMediator != IMS_NULL)
    {
        piMessageMediator->MessageMediator_AdjustMessage(
                piSCC->GetMessage(), IMessageMediator::MESSAGE_REFRESH);
    }

    if (piSCC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Update the SIP client connection
    piRefreshSC = piSCC;

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_SINT32 RefreshHelper::GetTimerInterval() const
{
    IMS_SINT32 nExpirationTime = GetDuration();
    IMS_SINT32 nTimerInterval;

    //---------------------------------------------------------------------------------------------

    switch (nPolicy)
    {
        case POLICY_SPEC:
            if (nExpirationTime > nCriteriaInterval)
                nTimerInterval = nExpirationTime - nValueGT;
            else
                nTimerInterval = static_cast<IMS_SINT32>((nExpirationTime * nValueEorLT) / 100);
            break;

        case POLICY_REMAIN_TIME:
            if (nExpirationTime > nCriteriaInterval)
                nTimerInterval = nExpirationTime - nValueGT;
            else
                nTimerInterval = nExpirationTime - nValueEorLT;
            break;

        case POLICY_RATIO:
            if (nExpirationTime > nCriteriaInterval)
                nTimerInterval = static_cast<IMS_SINT32>((nExpirationTime * nValueGT) / 100);
            else
                nTimerInterval = static_cast<IMS_SINT32>((nExpirationTime * nValueEorLT) / 100);
            break;

        default:
            nTimerInterval = static_cast<IMS_SINT32>(nExpirationTime / 2);
            break;
    }

    return nTimerInterval;
}

PROTECTED
void RefreshHelper::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if (piRefreshable == IMS_NULL)
        return;

    piRefreshable->Refreshable_RefreshCompleted(piSCC, nCode);
}

PROTECTED
IMS_BOOL RefreshHelper::Refreshable_RefreshStarted()
{
    //---------------------------------------------------------------------------------------------

    if (piRefreshable == IMS_NULL)
        return IMS_FALSE;

    return piRefreshable->Refreshable_RefreshStarted();
}

PROTECTED
void RefreshHelper::Refreshable_RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    if (piRefreshable == IMS_NULL)
        return;

    piRefreshable->Refreshable_RefreshTerminated();
}

PROTECTED
IMS_BOOL RefreshHelper::ConsumeRemainedTime()
{
    //---------------------------------------------------------------------------------------------

    if (nRemainDuration <= 0)
    {
        return IMS_TRUE;
    }

    StopRefresh();

    if (!SetTimer(nRemainDuration))
    {
        return IMS_FALSE;
    }

    nRemainDuration = 0;

    return IMS_TRUE;
}

PROTECTED
IMS_SINT32 RefreshHelper::GetPolicy() const
{
    //---------------------------------------------------------------------------------------------

    return nPolicy;
}

PROTECTED
void RefreshHelper::SetDuration(IN IMS_SINT32 nDuration)
{
    //---------------------------------------------------------------------------------------------

    this->nDuration = nDuration;
}

PROTECTED
IMS_BOOL RefreshHelper::StartRefresh()
{
    //---------------------------------------------------------------------------------------------

    if (piTimer != IMS_NULL)
    {
        IMS_TRACE_D("The refresh timer already exists; It will be updated...", 0, 0, 0);

        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        piTimer = IMS_NULL;
    }

    if (GetPolicy() == POLICY_NO_REFRESH)
    {
        IMS_TRACE_D("Refresh operation is not supported by the engine...", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nTimerDuration = GetTimerInterval();

    if (nTimerDuration <= 0)
    {
        IMS_TRACE_E(0, "Timer duration is ZERO; STOPPED ...", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SetTimer(nTimerDuration))
    {
        return IMS_FALSE;
    }

    nRemainDuration = nDuration - nTimerDuration;

    return IMS_TRUE;
}

PROTECTED
void RefreshHelper::StopRefresh()
{
    //---------------------------------------------------------------------------------------------

    if (piTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("Refresh Timer (%p) - STOPPED ...", piTimer, 0, 0);

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
    piTimer = IMS_NULL;
}

PRIVATE VIRTUAL void RefreshHelper::Timer_TimerExpired(IN ITimer* piTimer)
{
    //---------------------------------------------------------------------------------------------

    if (this->piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Refresh Timer - NOT ACTIVE", 0, 0, 0);
        return;
    }

    if (this->piTimer != piTimer)
    {
        IMS_TRACE_D("Refresh Timer - INVALID TIMER", 0, 0, 0);
        return;
    }

    this->piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(this->piTimer);
    this->piTimer = IMS_NULL;

    if (nRemainDuration > 0)
    {
        if (bRepeatable)
        {
            // Re-start the refresh timer with the remained duration
            if (!SetTimer(nRemainDuration))
            {
                return;
            }

            nRemainDuration = 0;
        }

        RefreshStarted();
    }
    else
    {
        RefreshTerminated();
    }
}

PRIVATE VIRTUAL void RefreshHelper::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piSCC, IN ISipClientConnection* /* piForkedSCC = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
        return;

    if (piSCC->Receive() != IMS_SUCCESS)
        return;

    // Parse the message body if it is a multipart body
    if (!SipParsingHelper::CreateMessageBodyParts(piSCC->GetMessage()))
    {
        IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

        Error_NotifyError(
                piSCC, SipError::PARSING_ERROR, AString("Parsing Error :: message body part"));
        return;
    }

    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

    RefreshCompleted(piSCC);

    if ((nStatusCode >= SipStatusCode::SC_200) && (nStatusCode != SipStatusCode::SC_401) &&
            (nStatusCode != SipStatusCode::SC_407))
    {
        IMS_SINT32 nMethod = piSCC->GetMethod().ToInt();

        if (piRefreshSC != IMS_NULL)
        {
            piSCC->Close();
            piRefreshSC = IMS_NULL;
        }

        // Re-submit the session refresh request with the new session interval (Session-Expires)
        if ((nMethod == SipMethod::INVITE) && (nStatusCode == SipStatusCode::SC_422))
        {
            RefreshStarted();
        }

        if (!IsSessionTimerUpdateRequiredByReInvite())
        {
            if ((nMethod == SipMethod::UPDATE) && (nStatusCode == SipStatusCode::SC_500))
            {
                // Out of sequence (race condition: re-INVITE & UPDATE)
                // : re-send the session refresh request (UPDATE)
                RefreshStarted();
            }
        }
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // Do something when responding to the challenge is failed
    }
}

PRIVATE VIRTUAL void RefreshHelper::Error_NotifyError(
        IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    (void)strMessage;

    if (nCode == SipError::TRANSACTION_TIMER_EXPIRED)
    {
        RefreshCompleted(DYNAMIC_CAST(ISipClientConnection*, piSC), TRANSACTION_TIMEOUT);
    }
    else
    {
        RefreshCompleted(DYNAMIC_CAST(ISipClientConnection*, piSC), nCode);
    }

    if (piRefreshSC != IMS_NULL)
    {
        piSC->Close();
        piRefreshSC = IMS_NULL;
    }
}

PRIVATE
IMS_BOOL RefreshHelper::SetTimer(IN IMS_SINT32 nTimerDuration)
{
    //---------------------------------------------------------------------------------------------

    piTimer = TimerService::GetTimerService()->CreateTimer();

    if (piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a refresh timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    piTimer->SetTimer(nTimerDuration * 1000, this);
    /*
    {
        piTimer->Close();
        piTimer = IMS_NULL;

        IMS_ERROR0("Starting a refresh timer failed");
        return IMS_FALSE;
    }*/

    IMS_TRACE_I("Refresh Timer (%p) :: START - Duration (%d)", piTimer, nTimerDuration, 0);

    return IMS_TRUE;
}
