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
#include "IImsAosInfo.h"
#include "INetworkWatcher.h"
#include "ImsTypeDef.h"
#include "ServiceImsRadio.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/ISession.h"
#include "helper/IMtcAosConnector.h"
#include "precondition/IMtcPreconditionManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EpsFallbackTrigger::EpsFallbackTrigger(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_piTimerWatchdogWait(IMS_NULL),
        m_piTimerEpsFallbackWait(IMS_NULL),
        m_bWaitingEpsFallbackForNoResponse(IMS_FALSE),
        m_bWaitingEpsFallbackForNoTrigger(IMS_FALSE)
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

    if (m_bWaitingEpsFallbackForNoResponse)
    {
        m_objContext.GetService().GetAosConnector()->NotifyEpsfbCallState(
                IImsAosInfo::EPSFB_CALL_FAILED);
    }
}

PUBLIC GLOBAL IMS_BOOL EpsFallbackTrigger::IsRequired(
        IN const MtcConfigurationProxy& objConfigProxy)
{
    // without Watchdog timer, EPS fallback due to network no response isn't supported, either.
    return objConfigProxy.GetInt(Feature::EPS_FALLBACK_WATCHDOG_TIME) > 0;
}

PUBLIC
IMS_BOOL EpsFallbackTrigger::IsVoNr() const
{
    if (m_objContext.GetService().IsWlanIpCanType())
    {
        return IMS_FALSE;
    }

    return PhoneInfoService::GetPhoneInfoService()
                   ->GetNetworkWatcher(m_objContext.GetSlotId())
                   ->GetNetRadioTechType() == NW_REPORT_RADIO_NR;
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
    m_piTimerWatchdogWait->SetTimer(
            m_objContext.GetConfigurationProxy().GetInt(Feature::EPS_FALLBACK_WATCHDOG_TIME), this);
}

PUBLIC
void EpsFallbackTrigger::OnEpsFallbackCompleted()
{
    IMS_TRACE_D("OnEpsFallbackCompleted", 0, 0, 0);

    if (m_bWaitingEpsFallbackForNoResponse)
    {
        m_bWaitingEpsFallbackForNoResponse = IMS_FALSE;
        if (m_piTimerEpsFallbackWait)
        {
            m_piTimerEpsFallbackWait->KillTimer();
            TimerService::GetTimerService()->DestroyTimer(m_piTimerEpsFallbackWait);
            m_piTimerEpsFallbackWait = IMS_NULL;
        }
    }
    else if (m_bWaitingEpsFallbackForNoTrigger)
    {
        m_bWaitingEpsFallbackForNoTrigger = IMS_FALSE;
    }
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
        m_bWaitingEpsFallbackForNoResponse = IMS_FALSE;

        m_objContext.GetCall().Terminate(CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
        m_objContext.GetService().GetAosConnector()->NotifyEpsfbCallState(
                IImsAosInfo::EPSFB_CALL_FAILED);
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
    IMS_UINT32 eRadioReason;
    if (eReason == EpsFallbackReason::NO_NETWORK_RESPONSE)
    {
        eRadioReason = IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE;
        m_bWaitingEpsFallbackForNoResponse = IMS_TRUE;

        m_objContext.GetService().GetAosConnector()->NotifyEpsfbCallState(
                IImsAosInfo::EPSFB_CALL_START);

        m_piTimerEpsFallbackWait = TimerService::GetTimerService()->CreateTimer();
        m_piTimerEpsFallbackWait->SetTimer(EPS_FALLBACK_COMPLETE_INTERVAL, this);
    }
    else
    {
        eRadioReason = IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER;
        m_bWaitingEpsFallbackForNoTrigger = IMS_TRUE;
    }

    ImsRadioService::GetImsRadioService()
            ->GetImsRadio(m_objContext.GetSlotId())
            ->TriggerEpsFallback(eRadioReason);
}

PRIVATE
IMS_BOOL EpsFallbackTrigger::IsEpsFallbackTriggeredByNetwork() const
{
    return !IsVoNr() ||
            m_objContext.GetPreconditionManager().IsDedicatedBearerAllocated(
                    &m_objContext.GetSession()->GetISession(), MEDIATYPE_AUDIO);
}
