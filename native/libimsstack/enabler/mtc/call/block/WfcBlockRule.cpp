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

#include "IImsRadio.h"
#include "IMtcImsEventReceiver.h"
#include "IMtcService.h"
#include "INetworkWatcher.h"
#include "ImsEventDef.h"
#include "ServiceImsRadio.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/block/WfcBlockRule.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IPassiveTimerHolder.h"
#include "utility/CallTypeUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
WfcBlockRule::WfcBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_objContext(objContext),
        m_pImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(objContext.GetSlotId())),
        m_eCallType(eCallType)
{
}

PUBLIC VIRTUAL WfcBlockRule::Result WfcBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (!m_objContext.GetService().IsWlanIpCanType() || IsWfcOn() ||
            IsVoiceCallAvailableInCellular())
    {
        // When registered via WLAN but WFC is turned off, if VoPS is enabled on the Cellular
        // network, voice calls are permitted.
        return Result(Result::Status::UNBLOCKED);
    }

    // On WLAN, but WFC is OFF and cellular voice is not available. Only one video call is allowed.
    if (!CallTypeUtil::IsVideoCall(m_eCallType))
    {
        IMS_TRACE_D("Wi-Fi voice call is unavailable", 0, 0, 0);
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF));
    }

    if (HasVideoCall())
    {
        IMS_TRACE_D("Already have a Wi-Fi video call", 0, 0, 0);
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE IMS_BOOL WfcBlockRule::HasVideoCall() const
{
    const ImsList<IMtcCall*>& lstCalls = m_objContext.GetOtherCalls();
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (CallTypeUtil::IsVideoCall(lstCalls.GetAt(nIndex)->GetCallType()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL WfcBlockRule::IsVoiceCallAvailableInCellular() const
{
    IMS_SINT32 eRatType = m_objContext.GetService().GetMobileRatType();
    if (eRatType == INetworkWatcher::RADIOTECH_TYPE_LTE ||
            eRatType == INetworkWatcher::RADIOTECH_TYPE_NR)
    {
        return IsVopsAvailable() && !IsVoiceBlockedBySsac();
    }
    return IMS_FALSE;  // No cellular coverage for voice calls
}

PRIVATE
IMS_BOOL WfcBlockRule::IsVopsAvailable() const
{
    const IMS_UINT32 eVopsState =
            m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE);
    IMS_TRACE_D("IsVopsAvailable : VoPS state[%d]", eVopsState, 0, 0);
    return eVopsState == IMS_VOICE_OVER_PS_SUPPORTED;
}

PRIVATE
IMS_BOOL WfcBlockRule::IsVoiceBlockedBySsac() const
{
    const IMS_SINT32 nBarringFactor = m_pImsRadio->GetSsacInfo().nBarringFactorForVoice;
    IMS_TRACE_D("IsVoiceBlockedBySsac : Voice barring factor[%d]", nBarringFactor, 0, 0);
    return nBarringFactor == 0 ||
            m_objContext.GetPassiveTimerHolder().IsActive(
                    IPassiveTimerHolder::Type::SSAC_VOICE_BARRING);
}

PRIVATE
IMS_BOOL WfcBlockRule::IsWfcOn() const
{
    const IMS_UINT32 eWfcState =
            m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_WFC_SETTING_CHANGED);
    IMS_TRACE_D("IsWfcOn : WFC state[%d]", eWfcState, 0, 0);
    return eWfcState == IMS_WFC_ON;
}
