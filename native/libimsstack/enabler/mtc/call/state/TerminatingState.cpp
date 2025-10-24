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

#include "CarrierConfig.h"
#include "INetworkWatcher.h"
#include "ISession.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcUiNotifier.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/state/TerminatingState.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ICallStateProxy.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TerminatingState::TerminatingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::TERMINATING, objContext),
        m_bSessionReleasedNotified(IMS_FALSE)
{
}

PUBLIC VIRTUAL TerminatingState::~TerminatingState()
{
    if (!m_bSessionReleasedNotified)
    {
        NotifyCallSessionReleased();
    }
}

PUBLIC VIRTUAL void TerminatingState::OnEnter()
{
    m_objContext.GetMediaManager().DestroyMediaSession();

    const ISession* piSession = GetISession();
    if (piSession && piSession->GetState() == ISession::STATE_TERMINATED)
    {
        HandleCallSessionReleased();
    }

    if (m_objContext.GetCallInfo().IsEmergency())
    {
        IMS_TRACE_I("Wait emergency call session released.", 0, 0, 0);
        m_objContext.GetTimer().Start(TIMER_E911_WAIT_SESSION_RELEASED,
                m_objContext.GetConfigurationProxy().GetInt(
                        ConfigIms::KEY_SIP_TIMER_T1_MILLIS_INT));
    }

    if (!piSession)
    {
        m_objContext.GetRadioChecker().OnTerminatedBeforeCreatingSession(m_objContext.GetCallKey());
    }
}

PUBLIC VIRTUAL CallStateName TerminatingState::SessionStartFailed(
        IN [[maybe_unused]] ISession* piSession)
{
    HandleCallSessionReleased();

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName TerminatingState::SessionTerminated(
        IN [[maybe_unused]] ISession* piSession)
{
    HandleCallSessionReleased();

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName TerminatingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    if (nType == TIMER_E911_WAIT_SESSION_RELEASED)
    {
        HandleCallSessionReleased();
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName TerminatingState::OnRatChanged(
        IN [[maybe_unused]] IMS_SINT32 eOldRatType, IN [[maybe_unused]] IMS_SINT32 eRatType)
{
    return GetStateName();
}

PRIVATE
void TerminatingState::HandleCallSessionReleased()
{
    IMS_TRACE_D("HandleCallSessionReleased - session released notified [%d]",
            m_bSessionReleasedNotified, 0, 0);

    if (m_bSessionReleasedNotified)
    {
        return;
    }
    m_bSessionReleasedNotified = IMS_TRUE;
    NotifyCallSessionReleased();

    if (m_objContext.GetCallInfo().IsEmergency())
    {
        m_objContext.GetUiNotifier().OnCallSessionReleased();
    }
}

PRIVATE
void TerminatingState::NotifyCallSessionReleased()
{
    IMS_TRACE_D("NotifyCallSessionReleased", 0, 0, 0);
    m_objContext.GetCallStateProxy().NotifyCallSessionReleased(m_objContext.GetCallKey(),
            m_objContext.GetCallInfo().IsEmergency(), m_objContext.IsEstablished());
}
