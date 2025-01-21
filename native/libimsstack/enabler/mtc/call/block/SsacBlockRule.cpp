/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "IImsRadio.h"
#include "IMtcService.h"
#include "ImsTypeDef.h"
#include "ServiceImsRadio.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/block/IMtcBlockRule.h"
#include "call/block/SsacBlockRule.h"
#include "helper/ISsacTimerHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SsacBlockRule::SsacBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_objContext(objContext),
        m_eCallType(eCallType),
        m_pImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(m_objContext.GetSlotId()))
{
}

PUBLIC VIRTUAL SsacBlockRule::~SsacBlockRule() {}

PUBLIC VIRTUAL SsacBlockRule::Result SsacBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objContext.GetService().IsNr())
    {
        // AC barring on NR is checked by RadioBlockRule
        return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
    }

    if (m_pImsRadio == IMS_NULL)
    {
        IMS_TRACE_E(0, "SsacBlockRule : IImsRadio is null", 0, 0, 0);
        return Result(
                IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR));
    }

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT ||
            m_objContext.GetCallInfo().IsEmergency() || m_objContext.GetService().IsWlanIpCanType())
    {
        return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
    }

    if (m_objContext.GetService().GetSsacTimerHandler().IsSsacTimerRunning(m_eCallType))
    {
        IMS_TRACE_D("Check blocked - SSAC timer is running", 0, 0, 0);
        return Result(
                IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED));
    }

    if (IsNeedToBar(m_eCallType))
    {
        IMS_TRACE_D("Check blocked - SSAC barred", 0, 0, 0);
        m_objContext.GetService().GetSsacTimerHandler().StartBarringTimer(m_eCallType);
        return Result(
                IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED));
    }

    return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
}

PRIVATE IMS_BOOL SsacBlockRule::IsNeedToBar(IN CallType eCallType) const
{
    if (eCallType == CallType::UNKNOWN)
    {
        return IMS_FALSE;
    }

    const SsacInfo& objSsacInfo = m_pImsRadio->GetSsacInfo();
    IMS_SINT32 nBarringFactor;

    if (eCallType == CallType::VOIP || eCallType == CallType::RTT)
    {
        nBarringFactor = objSsacInfo.nBarringFactorForVoice;
    }
    else
    {
        nBarringFactor = objSsacInfo.nBarringFactorForVideo;
    }

    IMS_TRACE_D("IsNeedToBar BarringFactor[%d]", nBarringFactor, 0, 0);

    if (nBarringFactor >= 100)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nRandom = IMS_SYS_GetRandom(100);
    if (nRandom < static_cast<IMS_UINT32>(nBarringFactor))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
