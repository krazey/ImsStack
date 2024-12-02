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

#include "IMtcImsEventReceiver.h"
#include "IMtcService.h"
#include "ImsEventDef.h"
#include "ServiceImsRadio.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/block/WfcBlockRule.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IPassiveTimerHolder.h"

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
    if (!m_objContext.GetService().IsWlanIpCanType() || IsWfcOn())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_eCallType == CallType::VOIP && !IsVoiceCallAvailableInCellular())
    {
        IMS_TRACE_D("WFC voice call is unavailable", 0, 0, 0);
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
IMS_BOOL WfcBlockRule::IsVoiceCallAvailableInCellular() const
{
    return IsVopsAvailable() && !IsVoiceBlockedBySsac();
}

PRIVATE
IMS_BOOL WfcBlockRule::IsVopsAvailable() const
{
    return m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE) ==
            IMS_VOICE_OVER_PS_SUPPORTED;
}

PRIVATE
IMS_BOOL WfcBlockRule::IsVoiceBlockedBySsac() const
{
    return m_pImsRadio->GetSsacInfo().nBarringFactorForVoice == 0 ||
            m_objContext.GetPassiveTimerHolder().IsActive(
                    IPassiveTimerHolder::Type::SSAC_VOICE_BARRING);
}

PRIVATE
IMS_BOOL WfcBlockRule::IsWfcOn() const
{
    return m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_WFC_SETTING_CHANGED) ==
            IMS_WFC_ON;
}
