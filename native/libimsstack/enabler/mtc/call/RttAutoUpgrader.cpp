/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "IMtcCallController.h"
#include "IMtcImsEventReceiver.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/RttAutoUpgrader.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ICallStateProxy.h"
#include "helper/IPassiveTimerHolder.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
RttAutoUpgrader::RttAutoUpgrader(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_bRttEmergencyCallEstablished(IMS_TRUE),
        m_nIncomingVoiceCallKey(IMtcCall::CALL_KEY_INVALID)
{
    IMS_TRACE_D("+RttAutoUpgrader", 0, 0, 0);
    m_objContext.GetCallStateProxy().AddListener(this);
}

PUBLIC VIRTUAL RttAutoUpgrader::~RttAutoUpgrader()
{
    IMS_TRACE_D("~RttAutoUpgrader", 0, 0, 0);
    m_objContext.GetPassiveTimerHolder().RemoveListener(
            IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD, this);
    m_objContext.GetCallStateProxy().RemoveListener(this);
}

PUBLIC GLOBAL IMS_BOOL RttAutoUpgrader::IsRequired(IN const MtcConfigurationProxy& objConfigProxy,
        IN const CallInfo& objCallInfo, IN const IMtcSession* pMtcSession)
{
    return objConfigProxy.GetInt(ConfigEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT) > 0 &&
            objCallInfo.IsEmergency() && pMtcSession->GetCallType() >= CallType::RTT;
}

PUBLIC VIRTUAL void RttAutoUpgrader::OnCallStateChanged(IN CallKey nCallKey,
        IN IMtcCall::State eState, IN Type eType, IN IMS_BOOL bEmergency,
        IN IMS_SINT32 /* nReason */)
{
    IMS_TRACE_D(
            "OnCallStateChanged() - state[%d], type[%d], emergency[%d]", eState, eType, bEmergency);

    if (bEmergency)
    {
        switch (eState)
        {
            case IMtcCall::State::ESTABLISHED:
                if (eType >= CallType::RTT)
                {
                    StopRttGuardTimer();
                    m_bRttEmergencyCallEstablished = IMS_TRUE;
                }
                break;

            case IMtcCall::State::TERMINATING:
                if (m_bRttEmergencyCallEstablished)
                {
                    StartRttGuardTimer();
                }
                m_bRttEmergencyCallEstablished = IMS_FALSE;
                break;

            default:
                break;
        }
    }

    if (!bEmergency && eType == CallType::VOIP)
    {
        switch (eState)
        {
            case IMtcCall::State::INCOMING:
                DetermineIfRttUpgradeIsNeeded(nCallKey);
                break;

            case IMtcCall::State::ESTABLISHED:
                UpgradeToRttIfNeeded(nCallKey);
                break;

            default:
                break;
        }
    }
}

PUBLIC VIRTUAL void RttAutoUpgrader::OnPassiveTimerExpired(IN IPassiveTimerHolder::Type /* eType */)
{
    m_objContext.DestroyRttAutoUpgrader();
}

PRIVATE void RttAutoUpgrader::StartRttGuardTimer()
{
    IMS_SINT32 nRttGuardTime = m_objContext.GetConfigurationProxy().GetInt(
            ConfigEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT);

    IMS_TRACE_D("StartRttGuardTimer GuardTime[%d]", nRttGuardTime, 0, 0);

    m_objContext.GetPassiveTimerHolder().AddTimer(
            IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD, nRttGuardTime, IMS_TRUE, IMS_TRUE);
    m_objContext.GetPassiveTimerHolder().AddListener(
            IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD, this);
}

PRIVATE void RttAutoUpgrader::StopRttGuardTimer()
{
    IMS_TRACE_D("StopRttGuardTimer", 0, 0, 0);

    m_objContext.GetPassiveTimerHolder().RemoveListener(
            IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD, this);
    m_objContext.GetPassiveTimerHolder().RemoveTimer(
            IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD);
}

PRIVATE void RttAutoUpgrader::DetermineIfRttUpgradeIsNeeded(IN CallKey nCallKey)
{
    IMS_BOOL bIsRttGuardTimerActive = m_objContext.GetPassiveTimerHolder().IsActive(
            IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD);
    IMS_SINT32 nRttSetting = m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_RTT_SETTING);

    IMS_TRACE_D("DetermineIfRttUpgradeIsNeeded CallKey[%d], rtt guard timer active[%d], rtt "
                "setting[%d]",
            nCallKey, bIsRttGuardTimerActive, nRttSetting);
    if (bIsRttGuardTimerActive &&
            (nRttSetting == IMS_RTT_ALWAYS_VISIBLE || nRttSetting == IMS_RTT_VISIBLE_DURING_CALL))
    {
        m_nIncomingVoiceCallKey = nCallKey;
    }
}

PRIVATE void RttAutoUpgrader::UpgradeToRttIfNeeded(IN CallKey nCallKey)
{
    IMS_TRACE_D("UpgradeToRttIfNeeded CallKey[%d]", nCallKey, 0, 0);

    if (m_nIncomingVoiceCallKey == nCallKey)
    {
        m_nIncomingVoiceCallKey = IMtcCall::CALL_KEY_INVALID;

        IMtcCallContext& objCallContext =
                m_objContext.GetCallManager().GetCallByCallKey(nCallKey)->GetCallContext();
        MediaInfo objNewMediaInfo = objCallContext.GetMediaManager().GetMediaInfo(
                &objCallContext.GetSession()->GetISession());

        objNewMediaInfo.eTextDirection = DIRECTION_SEND_RECEIVE;
        objNewMediaInfo.eGttMode = GTT_MODE_FULL;

        m_objContext.GetCallController().Update(nCallKey, CallType::RTT, objNewMediaInfo);
    }
}
