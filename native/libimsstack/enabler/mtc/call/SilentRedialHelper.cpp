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
#include "IMessage.h"
#include "ISession.h"
#include "IMtcCallController.h"
#include "ImsAosParameter.h"
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
#include "call/termination/StartErrorHandler.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ICallStateProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "precondition/MtcPreconditionManager.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SilentRedialHelper::SilentRedialHelper(
        IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason) :
        m_objContext(objContext),
        m_nCallKey(objContext.GetCallKey()),
        m_nType(objReason.nExtraCode),
        m_nMaxDuration(0),
        m_nInterval(INTERVAL_BY_TYPE),
        m_nMaxCount(0),
        m_nCount(0),
        m_nTotalRetryDuration(0),
        m_strExtra(objReason.strExtraMessage),
        m_piRetryTimer(IMS_NULL),
        m_objMediaInfo(
                objContext.GetMediaManager().GetMediaInfo(objContext.GetSession()->GetISession()))
{
    m_objContext.GetCallStateProxy().AddListener(this);
    SetRedialDetail();
    IMS_TRACE_D("+SilentRedialHelper type[%d] maxCount[%d] maxDuration[%d]", m_nType, m_nMaxCount,
            m_nMaxDuration);
}

PUBLIC VIRTUAL SilentRedialHelper::~SilentRedialHelper()
{
    IMS_TRACE_D("~SilentRedialHelper[%d]", m_nCallKey, 0, 0);
    if (m_piRetryTimer)
    {
        m_piRetryTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piRetryTimer);
    }
    m_objContext.GetCallStateProxy().RemoveListener(this);
}

PUBLIC VIRTUAL CallReasonInfo SilentRedialHelper::Redial(IN IMS_SINT32 nIntervalInMillis)
{
    if (nIntervalInMillis != INTERVAL_BY_TYPE)
    {
        // in case of EXTRA_CODE_REDIAL_BY_RETRY_AFTER, nIntervalInMillis must be specified.
        m_nInterval = nIntervalInMillis;
    }

    m_nTotalRetryDuration += m_nInterval;

    if (IsRedialAvailable() == IMS_FALSE)
    {
        IMtcSession* pSession = m_objContext.GetSession();
        return pSession ? HandleFailure(*pSession) : CallReasonInfo(CODE_NONE);
    }

    ReleaseCallResources();
    m_nCount += 1;

    IMS_TRACE_D("Redial count[%d] interval[%d]", m_nCount, m_nInterval, 0);

    // In case m_nInterval = 0, the operation will be done asynchronously without delay.
    m_piRetryTimer = TimerService::GetTimerService()->CreateTimer();
    m_piRetryTimer->SetTimer(m_nInterval, this);

    return CallReasonInfo(CODE_NONE);
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
    if (m_piRetryTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "no timer", 0, 0, 0);
        return;
    }

    if (m_piRetryTimer != piTimer)
    {
        IMS_TRACE_E(0, "invalid timer", 0, 0, 0);
        return;
    }

    m_piRetryTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piRetryTimer);
    m_piRetryTimer = IMS_NULL;

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
    MediaInfo objMediaInfo = m_objMediaInfo;
    MtcMediaUtil::RefineMediaInfoByCallType(eType, objMediaInfo);

    // SuppService list
    ImsList<SuppService*> objSuppServices;
    const ImsList<SuppService*>& objOriginalSuppServices =
            m_objContext.GetSupplementaryService().GetServices();
    for (IMS_UINT32 i = 0; i < objOriginalSuppServices.GetSize(); i++)
    {
        objSuppServices.Append(new SuppService(*objOriginalSuppServices.GetAt(i)));
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
            // in this case, m_nInterval will be specified when redial() is called
            LoadRetryLimitsFromConfiguration();
            return;
        case EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT:
        case EXTRA_CODE_REDIAL_BY_ERROR_RESPONSE:
            m_nInterval = m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT);
            LoadRetryLimitsFromConfiguration();
            return;
        case EXTRA_CODE_REDIAL_FOR_REDIRECTION:
        case EXTRA_CODE_REDIAL_BY_EPS_FALLBACK:
        case EXTRA_CODE_REDIAL_BY_EPS_FALLBACK_WITH_REG:
        case EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION:
        case EXTRA_CODE_REDIAL_EMERGENCY_WITH_ANONYMOUS:
            m_nInterval = 0;
            m_nMaxCount = 3;
            return;
        case EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF:
        case EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF:
            m_nInterval = 0;
            m_nMaxCount = NO_LIMIT;
            return;
        case EXTRA_CODE_REDIAL_FOR_SDP_CHANGE:
        case EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF_ONCE:
            m_nInterval = 0;
            m_nMaxCount = 1;
            return;
        default:
            return;
    }
}

PRIVATE
void SilentRedialHelper::LoadRetryLimitsFromConfiguration()
{
    m_nMaxCount = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT);
    m_nMaxDuration = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT);
}

PRIVATE
void SilentRedialHelper::ReleaseCallResources()
{
    // MediaSession must be destroyed first, then MtcSession should be destroyed
    // to ensure for Media to send “closeSession” to ImsMedia before AudioSession is destroyed
    m_objContext.GetMediaManager().DestroyMediaSession();
    m_objContext.RemoveAllSessions();
    m_objContext.GetPreconditionManager().InitializeMobileRatInformation();
    StopCallTimers();
}

PRIVATE
void SilentRedialHelper::StopCallTimers()
{
    if (m_nType == EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF)
    {
        return;
    }

    m_objContext.GetTimer().StopAll();
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

    return m_nMaxDuration == 0 || m_nTotalRetryDuration < m_nMaxDuration;
}

PRIVATE
CallReasonInfo SilentRedialHelper::HandleFailure(IN IMtcSession& objMtcSession) const
{
    IMS_SINT32 eFailureAction = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT);

    IMS_TRACE_I("HandleFailure action[%d]", eFailureAction, 0, 0);
    switch (eFailureAction)
    {
        case ConfigVoice::SILENT_REDIAL_FAILURE_ACTION_CSFB:
            if (m_objContext.IsCsfbAvailable())
            {
                return CallReasonInfo(
                        CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
            }
            break;

        case ConfigVoice::SILENT_REDIAL_FAILURE_ACTION_REGISTRATION:
            ControlAos(ImsAosControl::REGISTER_REINITIATE);
            break;

        default:  // ConfigVoice::SILENT_REDIAL_FAILURE_ACTION_TERMINATE
            break;
    }

    const IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
            &objMtcSession.GetISession(), IMessage::SESSION_START);
    if (piResponse == IMS_NULL)
    {
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
    }

    return StartErrorHandler::GetDefaultCallReasonInfo(m_objContext, *piResponse);
}

PRIVATE
CallType SilentRedialHelper::GetCallType() const
{
    if (m_nType == EXTRA_CODE_REDIAL_FOR_SDP_CHANGE)
    {
        return MtcMediaUtil::GetCallTypeFromMediaTypes(
                MtcMediaUtil::StringToMediaTypes(m_strExtra));
    }

    if (m_nType == EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION)
    {
        return CallType::VOIP;
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

PRIVATE
void SilentRedialHelper::ControlAos(IN IMS_UINT32 eCommand) const
{
    const IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(eCommand);
    }
}
