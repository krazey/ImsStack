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
#include "ImsTypeDef.h"
#include "ServiceImsRadio.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/block/IMtcBlockRule.h"
#include "call/block/SsacBlockRule.h"
#include "helper/IPassiveTimerHolder.h"

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
            m_objContext.GetCallInfo().bEmergency || m_objContext.GetService().IsWlanIpCanType())
    {
        return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
    }

    if (IsSsacTimerRunning(m_eCallType) || StartSsacTimer(m_eCallType))
    {
        IMS_TRACE_D("Check blocked - SSAC barred", 0, 0, 0);
        return Result(
                IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED));
    }

    return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
}

PRIVATE IMS_BOOL SsacBlockRule::IsSsacTimerRunning(IN CallType eCallType) const
{
    if (m_objContext.GetPassiveTimerHolder().IsActive(
                IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
    {
        return IMS_TRUE;
    }

    if (eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT)
    {
        return m_objContext.GetPassiveTimerHolder().IsActive(
                IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING);
    }

    return IMS_FALSE;
}

PRIVATE IMS_BOOL SsacBlockRule::StartSsacTimer(IN CallType eCallType)
{
    if (eCallType == CallType::UNKNOWN)
    {
        return IMS_FALSE;
    }

    const SsacInfo& objSsacInfo = m_pImsRadio->GetSsacInfo();
    IMS_SINT32 nBarringFactor;
    IMS_SINT32 nBarringTimeSec;
    IPassiveTimerHolder::Type nSsacType;

    if (eCallType == CallType::VOIP || eCallType == CallType::RTT)
    {
        nBarringFactor = objSsacInfo.nBarringFactorForVoice;
        nBarringTimeSec = objSsacInfo.nBarringTimeSecForVoice;
        nSsacType = IPassiveTimerHolder::Type::SSAC_VOICE_BARRING;
    }
    else
    {
        nBarringFactor = objSsacInfo.nBarringFactorForVideo;
        nBarringTimeSec = objSsacInfo.nBarringTimeSecForVideo;
        nSsacType = IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING;
    }

    IMS_TRACE_D("StartSsacTimer BarringFactor[%d] BarringTimeSec[%d]", nBarringFactor,
            nBarringTimeSec, 0);

    if (nBarringFactor >= 100 || (nBarringFactor != 0 && nBarringTimeSec <= 0))
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nRandom = IMS_SYS_GetRandom(100);
    if (nRandom < static_cast<IMS_UINT32>(nBarringFactor))
    {
        return IMS_FALSE;
    }

    nRandom = IMS_SYS_GetRandom(10);
    IMS_DOUBLE nCalculatedBarringTime = (0.7 + 0.6 * (nRandom / 10.0)) * nBarringTimeSec;

    m_objContext.GetPassiveTimerHolder().AddTimer(nSsacType, nCalculatedBarringTime * 1000);

    return IMS_TRUE;
}
