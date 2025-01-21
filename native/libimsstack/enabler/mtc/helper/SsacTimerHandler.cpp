/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "IMtcContext.h"
#include "INetworkWatcher.h"
#include "ServiceImsRadio.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/SsacTimerHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SsacTimerHandler::SsacTimerHandler(IN IMtcContext& objContext) :
        m_objContext(objContext)
{
    IMS_TRACE_I("+SsacTimerHandler", 0, 0, 0);
}

PUBLIC VIRTUAL SsacTimerHandler::~SsacTimerHandler()
{
    IMS_TRACE_I("~SsacTimerHandler", 0, 0, 0);

    Clear();
}

PUBLIC VIRTUAL void SsacTimerHandler::OnPassiveTimerExpired(
        IN [[maybe_unused]] IPassiveTimerHolder::Type eType)
{
    if (IsSsacTimerRunning())
    {
        return;
    }

    m_objContext.GetServiceByType(ServiceType::NORMAL)->RemoveNetworkWatcherListener(this);
}

PUBLIC VIRTUAL void SsacTimerHandler::OnRatChanged(IN [[maybe_unused]] ServiceType eServiceType,
        IN [[maybe_unused]] IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType)
{
    if (eRatType != INetworkWatcher::RADIOTECH_TYPE_LTE)
    {
        Clear();
    }
}

PUBLIC VIRTUAL IMS_BOOL SsacTimerHandler::IsSsacTimerRunning(IN CallType eCallType) const
{
    switch (eCallType)
    {
        case CallType::UNKNOWN:
        case CallType::VOIP:
        case CallType::RTT:
            return m_objContext.GetPassiveTimerHolder().IsActive(
                    IPassiveTimerHolder::Type::SSAC_VOICE_BARRING);

        case CallType::VT:
        case CallType::VIDEO_RTT:
            return m_objContext.GetPassiveTimerHolder().IsActive(
                           IPassiveTimerHolder::Type::SSAC_VOICE_BARRING) ||
                    m_objContext.GetPassiveTimerHolder().IsActive(
                            IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING);
    }
}

PUBLIC VIRTUAL void SsacTimerHandler::StartBarringTimer(IN CallType eCallType)
{
    IImsRadio* pImsRadio =
            ImsRadioService::GetImsRadioService()->GetImsRadio(m_objContext.GetSlotId());
    if (!pImsRadio)
    {
        return;
    }

    const SsacInfo& objSsacInfo = pImsRadio->GetSsacInfo();
    IMS_SINT32 nBarringTimeSec;
    IPassiveTimerHolder::Type eSsacTimertype;

    if (eCallType == CallType::VOIP || eCallType == CallType::RTT)
    {
        nBarringTimeSec = objSsacInfo.nBarringTimeSecForVoice;
        eSsacTimertype = IPassiveTimerHolder::Type::SSAC_VOICE_BARRING;
    }
    else
    {
        nBarringTimeSec = objSsacInfo.nBarringTimeSecForVideo;
        eSsacTimertype = IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING;
    }

    if (nBarringTimeSec <= 0)
    {
        return;
    }

    if (!IsSsacTimerRunning())
    {
        m_objContext.GetServiceByType(ServiceType::NORMAL)->AddNetworkWatcherListener(this);
    }

    IMS_UINT32 nRandom = IMS_SYS_GetRandom(10);
    IMS_DOUBLE nCalculatedBarringTime = (0.7 + 0.6 * (nRandom / 10.0)) * nBarringTimeSec;
    m_objContext.GetPassiveTimerHolder().AddTimer(eSsacTimertype, nCalculatedBarringTime * 1000);
}

PRIVATE void SsacTimerHandler::Clear()
{
    if (!IsSsacTimerRunning())
    {
        return;
    }

    m_objContext.GetPassiveTimerHolder().RemoveTimer(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING);
    m_objContext.GetPassiveTimerHolder().RemoveTimer(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING);

    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (pNormalService)
    {
        pNormalService->RemoveNetworkWatcherListener(this);
    }
}

PRIVATE IMS_BOOL SsacTimerHandler::IsSsacTimerRunning() const
{
    return m_objContext.GetPassiveTimerHolder().IsActive(
                   IPassiveTimerHolder::Type::SSAC_VOICE_BARRING) ||
            m_objContext.GetPassiveTimerHolder().IsActive(
                    IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING);
}
