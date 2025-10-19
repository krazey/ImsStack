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

#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "IImsAosInfo.h"
#include "INetworkWatcher.h"
#include "ISession.h"
#include "ImsTypeDef.h"
#include "ServiceImsRadio.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "precondition/IMtcPreconditionManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EpsFallbackTrigger::EpsFallbackTrigger(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_piTimerWatchdogWait(IMS_NULL),
        m_piTimerEpsFallbackWait(IMS_NULL),
        m_eTriggerReason(EpsFallbackReason::NONE)
{
    IMS_TRACE_D("+EpsFallbackTrigger[%d]", m_objContext.GetCallKey(), 0, 0);
}

PUBLIC VIRTUAL EpsFallbackTrigger::~EpsFallbackTrigger()
{
    IMS_TRACE_D("~EpsFallbackTrigger[%d]", m_objContext.GetCallKey(), 0, 0);
    if (m_piTimerWatchdogWait)
    {
        m_piTimerWatchdogWait->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimerWatchdogWait);
    }

    if (m_piTimerEpsFallbackWait)
    {
        m_piTimerEpsFallbackWait->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimerEpsFallbackWait);
    }

    if (m_eTriggerReason == EpsFallbackReason::NO_NETWORK_RESPONSE_REQUIRING_REG)
    {
        m_objContext.GetService().GetAosConnector()->NotifyEpsfbCallState(
                IImsAosInfo::EPSFB_CALL_FAILED);
    }
}

PUBLIC GLOBAL IMS_BOOL EpsFallbackTrigger::ShouldTriggerByReasonInfo(
        IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason)
{
    if (objContext.GetCallInfo().ePeerType != PeerType::MO || !objContext.GetService().IsNr() ||
            !IsEpsFbAvailable(objContext))
    {
        return IMS_FALSE;
    }

    if (objReason.nCode == CODE_ACCESS_CLASS_BLOCKED &&
            objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
    {
        return IMS_TRUE;
    }
    else if (objReason.nCode == CODE_INTERNAL_RRC_REJECT &&
            objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_RRC_REJECT_WAIT_TIME_BOOL))
    {
        const IMS_UINT32 nRrcRejectWaitTimeMillis = objReason.nExtraCode;
        const IMS_UINT32 nMoCallRequestTimeoutMillis = objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT);
        IMS_TRACE_D("RRC reject wait time: %d, MO call request timeout duration: %d",
                nRrcRejectWaitTimeMillis, nMoCallRequestTimeoutMillis, 0);

        return nRrcRejectWaitTimeMillis >= nMoCallRequestTimeoutMillis;
    }
    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL EpsFallbackTrigger::ShouldTriggerByWatchdogTimer(
        IN IMtcCallContext& objContext)
{
    // Start only if Teps_fb_watchdog > 0 by Verizon's requirement
    return objContext.GetService().IsNr() && IsEpsFbAvailable(objContext) &&
            objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT) > 0;
}

PUBLIC GLOBAL IMS_BOOL EpsFallbackTrigger::ShouldTriggerByMoRequestTimeout(
        IN IMtcCallContext& objContext)
{
    return objContext.GetService().IsNr() && IsEpsFbAvailable(objContext) &&
            objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT) >=
            0;
}

PUBLIC GLOBAL IMS_BOOL EpsFallbackTrigger::IsEpsFbAvailable(IN IMtcCallContext& objContext)
{
    const ImsList<IMtcCall*>& lstCalls = objContext.GetOtherCalls();
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        const IMtcCall* pCall = lstCalls.GetAt(nIndex);
        if (pCall->GetState() == IMtcCall::State::ESTABLISHED ||
                pCall->GetState() == IMtcCall::State::UPDATING)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void EpsFallbackTrigger::StartWatchdog()
{
    IMS_TRACE_D("StartWatchdog", 0, 0, 0);
    if (m_piTimerWatchdogWait)
    {
        // in the forking case, the timer is checked by the last received session
        m_piTimerWatchdogWait->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimerWatchdogWait);
    }

    m_piTimerWatchdogWait = TimerService::GetTimerService()->CreateTimer();
    m_piTimerWatchdogWait->SetTimer(m_objContext.GetConfigurationProxy().GetInt(
                                            ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT),
            this);
}

PUBLIC
void EpsFallbackTrigger::OnEpsFallbackCompleted()
{
    IMS_TRACE_D("OnEpsFallbackCompleted", 0, 0, 0);

    if (m_piTimerEpsFallbackWait)
    {
        m_piTimerEpsFallbackWait->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimerEpsFallbackWait);
        m_piTimerEpsFallbackWait = IMS_NULL;
    }

    m_eTriggerReason = EpsFallbackReason::NONE;
}

PUBLIC VIRTUAL void EpsFallbackTrigger::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piTimerWatchdogWait == piTimer)
    {
        IMS_TRACE_D("Timer_TimerExpired - watchdog wait", 0, 0, 0);
        m_piTimerWatchdogWait = IMS_NULL;
        if (IsEpsFallbackTriggeredByNetwork() == IMS_FALSE)
        {
            TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_TRIGGER);
        }
    }
    else if (m_piTimerEpsFallbackWait == piTimer)
    {
        IMS_TRACE_D("Timer_TimerExpired - epsfallback wait", 0, 0, 0);
        m_piTimerEpsFallbackWait = IMS_NULL;

        m_objContext.GetCall().Terminate(CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
        if (m_eTriggerReason == EpsFallbackReason::NO_NETWORK_RESPONSE_REQUIRING_REG)
        {
            m_objContext.GetService().GetAosConnector()->NotifyEpsfbCallState(
                    IImsAosInfo::EPSFB_CALL_FAILED);
        }

        m_eTriggerReason = EpsFallbackReason::NONE;
    }
    else
    {
        return;
    }

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
}

PUBLIC
void EpsFallbackTrigger::TriggerEpsFallback(IN EpsFallbackReason eReason)
{
    IMS_TRACE_D("TriggerEpsFallback Reason[%d]", eReason, 0, 0);
    if (m_eTriggerReason != EpsFallbackReason::NONE)
    {
        IMS_TRACE_D("TriggerEpsFallback : Already triggered", 0, 0, 0);
        return;
    }

    m_eTriggerReason = eReason;
    IMS_UINT32 eRadioReason;
    if (m_eTriggerReason == EpsFallbackReason::NO_NETWORK_RESPONSE)
    {
        eRadioReason = IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE;
    }
    else if (m_eTriggerReason == EpsFallbackReason::NO_NETWORK_RESPONSE_REQUIRING_REG ||
            m_eTriggerReason == EpsFallbackReason::RADIO_CHECK_BLOCK)
    {
        eRadioReason = IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE;
        m_objContext.GetService().GetAosConnector()->NotifyEpsfbCallState(
                IImsAosInfo::EPSFB_CALL_START);
    }
    else
    {
        eRadioReason = IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER;
    }

    if (m_eTriggerReason != EpsFallbackReason::NO_NETWORK_TRIGGER)
    {
        m_piTimerEpsFallbackWait = TimerService::GetTimerService()->CreateTimer();
        m_piTimerEpsFallbackWait->SetTimer(EPS_FALLBACK_COMPLETE_TIMEOUT, this);
    }

    ImsRadioService::GetImsRadioService()
            ->GetImsRadio(m_objContext.GetSlotId())
            ->TriggerEpsFallback(eRadioReason);
}

PRIVATE
IMS_BOOL EpsFallbackTrigger::IsEpsFallbackTriggeredByNetwork() const
{
    return !m_objContext.GetService().IsNr() ||
            m_objContext.GetPreconditionManager().IsDedicatedBearerAllocated(
                    &m_objContext.GetSession()->GetISession(), MEDIATYPE_AUDIO);
}
