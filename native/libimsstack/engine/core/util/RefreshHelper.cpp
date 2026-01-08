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
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "ISipClientConnection.h"
#include "Sip.h"
#include "SipError.h"
#include "SipMethod.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "util/IRefreshable.h"
#include "util/RefreshHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
RefreshHelper::RefreshHelper(IN IRefreshable* piRefreshable, IN IMS_BOOL bRepeatable) :
        m_piRefreshable(piRefreshable),
        m_nPolicy(POLICY_SPEC),
        m_nCriteriaInterval(CRITERIA_INTERVAL),
        m_nValueEorLt(50)  // half of expiration time
        ,
        m_nValueGt(MINIMUM_REMAIN_INTERVAL),
        m_bRepeatable(bRepeatable),
        m_nDuration(0),
        m_nRemainDuration(0),
        m_piTimer(IMS_NULL),
        m_piRefreshSc(IMS_NULL),
        m_piMessageMediator(IMS_NULL)
{
}

PUBLIC VIRTUAL RefreshHelper::~RefreshHelper()
{
    SetConnection(IMS_NULL);

    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
    }
}

PUBLIC
void RefreshHelper::AbortConnection()
{
    SetConnection(IMS_NULL);
}

PUBLIC
void RefreshHelper::SetPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
        IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt)
{
    m_nPolicy = nPolicy;
    m_nCriteriaInterval = nCriteriaInterval;
    m_nValueEorLt = nValueEorLt;
    m_nValueGt = nValueGt;
}

PROTECTED VIRTUAL IMS_RESULT RefreshHelper::SendRefreshRequest(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Overwrites the SIP client connection listener
    piScc->SetErrorListener(this);
    piScc->SetListener(this);

    // SIP_MESSAGE_MEDIATOR
    if (m_piMessageMediator != IMS_NULL)
    {
        m_piMessageMediator->MessageMediator_AdjustMessage(
                piScc->GetMessage(), IMessageMediator::MESSAGE_REFRESH);
    }

    if (piScc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Update the SIP client connection
    SetConnection(piScc);

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_SINT32 RefreshHelper::GetTimerInterval() const
{
    IMS_SINT32 nExpirationTime = GetDuration();
    IMS_SINT32 nTimerInterval;

    switch (m_nPolicy)
    {
        case POLICY_SPEC:
            if (nExpirationTime > m_nCriteriaInterval)
            {
                nTimerInterval = nExpirationTime - m_nValueGt;
            }
            else
            {
                nTimerInterval = static_cast<IMS_SINT32>((nExpirationTime * m_nValueEorLt) / 100);
            }
            break;
        case POLICY_REMAIN_TIME:
            if (nExpirationTime > m_nCriteriaInterval)
            {
                nTimerInterval = nExpirationTime - m_nValueGt;
            }
            else
            {
                nTimerInterval = nExpirationTime - m_nValueEorLt;
            }
            break;
        case POLICY_RATIO:
            if (nExpirationTime > m_nCriteriaInterval)
            {
                nTimerInterval = static_cast<IMS_SINT32>((nExpirationTime * m_nValueGt) / 100);
            }
            else
            {
                nTimerInterval = static_cast<IMS_SINT32>((nExpirationTime * m_nValueEorLt) / 100);
            }
            break;
        default:
            nTimerInterval = static_cast<IMS_SINT32>(nExpirationTime / 2);
            break;
    }

    return nTimerInterval;
}

PROTECTED
void RefreshHelper::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    if (m_piRefreshable == IMS_NULL)
    {
        return;
    }

    m_piRefreshable->Refreshable_RefreshCompleted(piScc, nCode);
}

PROTECTED
IMS_BOOL RefreshHelper::Refreshable_RefreshStarted()
{
    if (m_piRefreshable == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_piRefreshable->Refreshable_RefreshStarted();
}

PROTECTED
void RefreshHelper::Refreshable_RefreshTerminated()
{
    if (m_piRefreshable == IMS_NULL)
    {
        return;
    }

    m_piRefreshable->Refreshable_RefreshTerminated();
}

PROTECTED
IMS_BOOL RefreshHelper::ConsumeRemainedTime()
{
    if (m_nRemainDuration <= 0)
    {
        return IMS_TRUE;
    }

    StopRefresh();

    if (!SetTimer(m_nRemainDuration))
    {
        return IMS_FALSE;
    }

    m_nRemainDuration = 0;

    return IMS_TRUE;
}

PROTECTED
void RefreshHelper::SetConnection(IN ISipClientConnection* piScc)
{
    if (m_piRefreshSc != IMS_NULL)
    {
        m_piRefreshSc->Close();
    }

    m_piRefreshSc = piScc;
}

PROTECTED
IMS_BOOL RefreshHelper::StartRefresh()
{
    if (m_piTimer != IMS_NULL)
    {
        IMS_TRACE_D("The refresh timer already exists; It will be updated...", 0, 0, 0);

        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
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

    m_nRemainDuration = m_nDuration - nTimerDuration;

    return IMS_TRUE;
}

PROTECTED
void RefreshHelper::StopRefresh()
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("Refresh Timer (%p) - STOPPED ...", m_piTimer, 0, 0);

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}

PRIVATE VIRTUAL void RefreshHelper::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Refresh Timer - NOT ACTIVE", 0, 0, 0);
        return;
    }

    if (m_piTimer != piTimer)
    {
        IMS_TRACE_D("Refresh Timer - INVALID TIMER", 0, 0, 0);
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;

    if (m_nRemainDuration > 0)
    {
        if (m_bRepeatable)
        {
            // Re-start the refresh timer with the remained duration
            if (!SetTimer(m_nRemainDuration))
            {
                return;
            }

            m_nRemainDuration = 0;
        }

        RefreshStarted();
    }
    else
    {
        RefreshTerminated();
    }
}

PRIVATE VIRTUAL void RefreshHelper::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* /*piForkedScc = IMS_NULL*/)
{
    if (piScc == IMS_NULL)
    {
        return;
    }

    if (piScc->Receive() != IMS_SUCCESS)
    {
        return;
    }

    // Parse the message body if it is a multipart body
    if (!SipParsingHelper::CreateMessageBodyParts(piScc->GetMessage()))
    {
        IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

        Error_NotifyError(
                piScc, SipError::PARSING_ERROR, AString("Parsing Error :: message body part"));
        return;
    }

    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    RefreshCompleted(piScc);

    if ((nStatusCode >= SipStatusCode::SC_200) && (nStatusCode != SipStatusCode::SC_401) &&
            (nStatusCode != SipStatusCode::SC_407))
    {
        IMS_SINT32 nMethod = piScc->GetMethod().ToInt();

        SetConnection(IMS_NULL);

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
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    (void)strMessage;

    if (nCode == SipError::TRANSACTION_TIMER_EXPIRED)
    {
        RefreshCompleted(DYNAMIC_CAST(ISipClientConnection*, piSc), TRANSACTION_TIMEOUT);
    }
    else
    {
        RefreshCompleted(DYNAMIC_CAST(ISipClientConnection*, piSc), nCode);
    }

    SetConnection(IMS_NULL);
}

PRIVATE
IMS_BOOL RefreshHelper::SetTimer(IN IMS_SINT32 nTimerDuration)
{
    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a refresh timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piTimer->SetTimer(nTimerDuration * 1000L, this);

    IMS_TRACE_I("Refresh Timer (%p) :: START - Duration (%d)", m_piTimer, nTimerDuration, 0);

    return IMS_TRUE;
}
