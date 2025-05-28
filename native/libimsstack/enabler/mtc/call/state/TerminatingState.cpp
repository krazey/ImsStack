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
#include "ISession.h"
#include "ImsAosParameter.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcUiNotifier.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/state/TerminatingState.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ICallStateProxy.h"
#include "helper/IMtcAosConnector.h"
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

    ISession* piSession = GetISession();
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
        MaybeRequestRegisterStop();
    }
}

PRIVATE
void TerminatingState::NotifyCallSessionReleased()
{
    IMS_TRACE_D("NotifyCallSessionReleased", 0, 0, 0);
    m_objContext.GetCallStateProxy().NotifyCallSessionReleased(m_objContext.GetCallKey(),
            m_objContext.GetCallInfo().IsEmergency(), m_objContext.IsEstablished());
}

PRIVATE
void TerminatingState::MaybeRequestRegisterStop()
{
    IMS_TRACE_D("MaybeRequestRegisterStop", 0, 0, 0);

    if (!m_objContext.GetService().IsEmergency())
    {
        return;
    }

    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_CLEAR_REGISTRATION_ON_DOMAIN_RESELECTION_BOOL))
    {
        return;
    }

    if (!IsEmergencyDomainReselectionRequired(m_objContext.GetUiNotifier().GetStartFailedReason()))
    {
        return;
    }

    IMS_TRACE_I("Request REGISTER_STOP", 0, 0, 0);
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector != IMS_NULL)
    {
        pAosConnector->Control(ImsAosControl::REGISTER_STOP);
    }
}

PRIVATE GLOBAL IMS_BOOL TerminatingState::IsEmergencyDomainReselectionRequired(
        IN const CallReasonInfo& objReason)
{
    if (objReason.nCode == CODE_LOCAL_CALL_CS_RETRY_REQUIRED ||
            objReason.nCode == CODE_LOCAL_INTERNAL_ERROR ||
            objReason.nCode == CODE_LOCAL_NOT_REGISTERED ||
            objReason.nCode == CODE_SIP_ALTERNATE_EMERGENCY_CALL)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
