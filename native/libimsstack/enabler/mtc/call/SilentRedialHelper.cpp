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
#include "IMtcCallController.h"
#include "ImsTypeDef.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/ParticipantInfo.h"
#include "call/SilentRedialHelper.h"
#include "call/state/MtcCallState.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ICallStateProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SilentRedialHelper::SilentRedialHelper(
        IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason) :
        m_objContext(objContext),
        m_nCallKey(objContext.GetCallKey()),
        m_nType(objReason.nExtraCode),
        m_nInterval(INTERVAL_BY_TYPE),
        m_nMaxCount(0),
        m_nCount(0),
        m_strExtra(objReason.strExtraMessage),
        m_piTimer(IMS_NULL)
{
    IMS_TRACE_D("+SilentRedialHelper[%d] type[%d]", m_nCallKey, m_nType, 0);
    m_objContext.GetCallStateProxy().AddListener(this);
    SetRedialDetail();
}

PUBLIC VIRTUAL SilentRedialHelper::~SilentRedialHelper()
{
    IMS_TRACE_D("~SilentRedialHelper[%d]", m_nCallKey, 0, 0);
    if (m_piTimer)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    }
    m_objContext.GetCallStateProxy().RemoveListener(this);
}

PUBLIC VIRTUAL IMS_RESULT SilentRedialHelper::Redial(
        IN IMS_SINT32 nIntervalInMillis /* = INTERVAL_BY_TYPE*/)
{
    if (nIntervalInMillis != INTERVAL_BY_TYPE)
    {
        // in case of EXTRA_CODE_REDIAL_BY_RETRY_AFTER, nIntervalInMillis must be specified.
        m_nInterval = nIntervalInMillis;
    }

    if (IsRedialAvailable() == IMS_FALSE)
    {
        IMS_TRACE_D("Redial stop[%d]", m_nCount, 0, 0);
        return IMS_FAILURE;
    }

    ReleaseCallResources();
    m_nCount += 1;

    IMS_TRACE_D("Redial count[%d]", m_nCount, 0, 0);

    m_piTimer = TimerService::GetTimerService()->CreateTimer();
    m_piTimer->SetTimer(m_nInterval, this);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL void SilentRedialHelper::OnCallStateChanged(IN CallKey nCallKey, IN State eState,
        IN Type /*eType*/, IN IMS_BOOL /*bEmergency*/, IN IMS_SINT32 /*nReason*/)
{
    if (nCallKey != m_nCallKey)
    {
        return;
    }

    if (eState == State::ESTABLISHED || eState == State::TERMINATING)
    {
        m_objContext.GetCallController().ReleaseRedialHelper();
    }
}

PUBLIC VIRTUAL void SilentRedialHelper::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "no timer", 0, 0, 0);
        return;
    }

    if (m_piTimer != piTimer)
    {
        IMS_TRACE_E(0, "invalid timer", 0, 0, 0);
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;

    ReStart();
}

PRIVATE
void SilentRedialHelper::ReStart()
{
    IMS_TRACE_D("ReStart", 0, 0, 0);

    // calltype
    CallType eType = GetCallType();

    // remote number
    const AString strTarget = GetRemoteTarget();

    // MediaInfo
    MediaInfo objMediaInfo = m_objContext.GetMediaManager().GetMediaInfo();

    // SuppService list
    ImsMap<SuppType, SuppService*> objSuppServices;
    const ImsMap<SuppType, SuppService*>& objOriginalSuppServices =
            m_objContext.GetSupplementaryService().GetServices();
    for (IMS_UINT32 i = 0; i < objOriginalSuppServices.GetSize(); i++)
    {
        // TODO: consider the case Supplementary services are changed?
        objSuppServices.Add(objOriginalSuppServices.GetKeyAt(i),
                new SuppService(*objOriginalSuppServices.GetValueAt(i)));
    }

    m_objContext.GetCallManager()
            .GetCallByCallKey(m_nCallKey)
            ->Start(eType, strTarget, objMediaInfo, objSuppServices);
}

PRIVATE
void SilentRedialHelper::SetRedialDetail()
{
    switch (m_nType)
    {
        case EXTRA_CODE_REDIAL_BY_RETRY_AFTER:
            m_nInterval = m_strExtra.ToInt32();
            m_nMaxCount = 1;
            return;
        case EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT:
        {
            const MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();

            m_nInterval = objConfig.GetInt(Feature::SILENT_REDIAL_INTERVAL);
            m_nMaxCount = objConfig.GetInt(Feature::SILENT_REDIAL_MAX_RETRY_COUNT);
        }
            return;
        case EXTRA_CODE_REDIAL_FOR_REDIRECTION:
        case EXTRA_CODE_REDIAL_FOR_SDP_CHANGE:
        case EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF:
            m_nInterval = 0;
            m_nMaxCount = 1;
            return;
        case EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF:
            m_nInterval = 0;
            m_nMaxCount = NO_LIMIT;
            return;
        default:
            return;
    }
}

PRIVATE
void SilentRedialHelper::ReleaseCallResources()
{
    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        m_objContext.RemoveSession(&pSession->GetISession());
    }

    m_objContext.GetMediaManager().DestroyMediaSession();
    StopCallTimers();
}

PRIVATE
void SilentRedialHelper::StopCallTimers()
{
    MtcTimerWrapper& objTimerWrapper = m_objContext.GetTimer();
    if (m_nType == EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF)
    {
        objTimerWrapper.Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT);
        return;
    }

    objTimerWrapper.StopAll();
}

PRIVATE
IMS_BOOL SilentRedialHelper::IsRedialAvailable() const
{
    if (m_nMaxCount <= m_nCount)
    {
        return IMS_FALSE;
    }

    if (m_nInterval == INTERVAL_BY_TYPE)
    {
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PRIVATE
CallType SilentRedialHelper::GetCallType() const
{
    if (m_nType == EXTRA_CODE_REDIAL_FOR_SDP_CHANGE)
    {
        return MtcMediaUtil::GetCallTypeFromMediaTypes(
                MtcMediaUtil::StringToMediaTypes(m_strExtra));
    }

    return m_objContext.GetCallInfo().eInitialCallType;
}

PRIVATE
const AString SilentRedialHelper::GetRemoteTarget() const
{
    if (m_nType == EXTRA_CODE_REDIAL_FOR_REDIRECTION)
    {
        return m_strExtra;
    }

    return m_objContext.GetParticipantInfo().GetRemoteNumber();
}
