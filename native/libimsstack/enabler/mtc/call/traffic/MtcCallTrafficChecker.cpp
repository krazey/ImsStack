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

#include "IImsRadio.h"
#include "IIpcan.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "INetworkWatcher.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ServiceImsRadio.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/traffic/MtcCallTrafficChecker.h"
#include "helper/ICallStateProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcCallTrafficChecker::MtcCallTrafficChecker(IN IMtcContext& objContext,
        IN IMtcRadioConnectionFailureListener& objMtcRadioConnectionFailureListener) :
        m_objContext(objContext),
        m_objMtcRadioConnectionFailureListener(objMtcRadioConnectionFailureListener),
        m_piNetworkWatcher(PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(
                m_objContext.GetSlotId())),
        m_piImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(m_objContext.GetSlotId())),
        m_piMtcCallTrafficCheckerListener(IMS_NULL),
        m_objMtcTrafficInfos(ImsList<MtcTrafficInfo*>())
{
    IMS_TRACE_D("+MtcCallTrafficChecker", 0, 0, 0);
}

PUBLIC MtcCallTrafficChecker::~MtcCallTrafficChecker()
{
    IMS_TRACE_D("~MtcCallTrafficChecker", 0, 0, 0);

    DeInit();
}

PUBLIC void MtcCallTrafficChecker::Init()
{
    m_objContext.GetCallStateProxy().AddListener(this);
    m_objContext.GetServiceByType(ServiceType::NORMAL)->AddAosStateListener(this);
    m_objContext.GetServiceByType(ServiceType::EMERGENCY)->AddAosStateListener(this);
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::SetTrafficCheckerListener(
        IN IMtcCallTrafficCheckerListener* pListener)
{
    m_piMtcCallTrafficCheckerListener = pListener;
}

PUBLIC VIRTUAL CheckResult MtcCallTrafficChecker::Check(
        IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType, IN IMS_BOOL bWifi)
{
    if (IsTrafficPrepared(eCallType, bEmergency, ePeerType))
    {
        return CheckResult::UNBLOCKED;
    }

    if (ePeerType == PeerType::MT)
    {
        StartTrafficChecking(eCallType, bEmergency, ePeerType, bWifi);
        return CheckResult::UNBLOCKED;
    }

    if (IsTrafficAllowed(eCallType, bEmergency) == IMS_FALSE)
    {
        return CheckResult::BLOCKED;
    }

    StartTrafficChecking(eCallType, bEmergency, ePeerType, bWifi);
    return CheckResult::PENDING;
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnIpcanChanged(
        IN IMtcService& objMtcService, IN IMS_UINT32 eIpcan)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objMtcTrafficInfos.GetSize(); nIndex++)
    {
        MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetAt(nIndex);

        if (pMtcTrafficInfo->m_objCallKeys.IsEmpty() || !(pMtcTrafficInfo->m_bTrafficActive))
        {
            continue;
        }

        TrafficType eTrafficType = pMtcTrafficInfo->m_eTrafficType;

        IMS_BOOL bEmergency = objMtcService.IsEmergency();
        if ((bEmergency && eTrafficType != IImsRadio::TRAFFIC_TYPE_EMERGENCY) ||
                (!bEmergency && eTrafficType == IImsRadio::TRAFFIC_TYPE_EMERGENCY))
        {
            continue;
        }

        m_piImsRadio->StartImsTraffic(eTrafficType,
                ConvertNetworkType(eIpcan == IIpcan::CATEGORY_WLAN),
                pMtcTrafficInfo->m_eCallDirection, pMtcTrafficInfo);
    }
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnCallStateChanged(IN CallKey nCallKey, IN State eState,
        IN Type eType, IN IMS_BOOL bEmergency, IN IMS_SINT32 /* nReason */)
{
    switch (eState)
    {
        case IMtcCall::State::OUTGOING:
            AddCallKeyIfNeeded(ConvertCallTypeToTrafficType(eType, bEmergency),
                    IImsRadio::DIRECTION_MO, nCallKey);
            break;

        case IMtcCall::State::INCOMING:
        case IMtcCall::State::ALERTING:
            AddCallKeyIfNeeded(ConvertCallTypeToTrafficType(eType, bEmergency),
                    IImsRadio::DIRECTION_MT, nCallKey);
            break;

        case IMtcCall::State::TERMINATING:
            RemoveCallKeyAndStopTrafficCheckingIfNeeded(nCallKey);
            break;

        case IMtcCall::State::ESTABLISHED:
        case IMtcCall::State::IDLE:
        case IMtcCall::State::UPDATING:
            break;
    }
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnTotalCallStateChanged(IN State /* eState */)
{
    // do nothing
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnConnectionFailed(IN TrafficType eTrafficType,
        IN CallDirection eCallDirection, IN IMS_UINT32 nFailureReason,
        IN IMS_UINT32 /* nCauseCode */, IN IMS_UINT32 /* nWaitTimeMillis */)
{
    IMS_TRACE_D("OnConnectionFailed TrafficType[%d] FailureReason[%d]", eTrafficType,
            nFailureReason, 0);

    switch (nFailureReason)
    {
        case IImsRadio::REASON_ACCESS_DENIED:
        case IImsRadio::REASON_INTERNAL_ERROR:
            NotifyTrafficCheckerListener(IMS_FALSE);
            NotifyRadioConnectionFailedListener(eTrafficType, eCallDirection);
            break;

        case IImsRadio::REASON_NAS_FAILURE:
        case IImsRadio::REASON_RACH_FAILURE:
        case IImsRadio::REASON_RLC_FAILURE:
        case IImsRadio::REASON_RRC_REJECT:
        case IImsRadio::REASON_RRC_TIMEOUT:
        case IImsRadio::REASON_NO_SERVICE:
        case IImsRadio::REASON_PDN_NOT_AVAILABLE:
        case IImsRadio::REASON_RF_BUSY:
            NotifyTrafficCheckerListener(IMS_TRUE);
            break;
    }
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnConnectionSetupPrepared(
        IN TrafficType eTrafficType, IN CallDirection /* eCallDirection */)
{
    IMS_TRACE_D("OnConnectionSetupPrepared TrafficType[%d]", eTrafficType, 0, 0);
    NotifyTrafficCheckerListener(IMS_TRUE);
}

PUBLIC void MtcCallTrafficChecker::CreateCallTrafficInfoWithGivenValue(IN TrafficType eTrafficType,
        IN CallDirection eCallDirection, IN IMS_BOOL bActive, IN CallKey nCallKeyIn)
{
    MtcTrafficInfo* pMtcTrafficInfo = CreateCallTrafficInfo(eTrafficType, eCallDirection);
    pMtcTrafficInfo->m_bTrafficActive = bActive;

    if (nCallKeyIn != IMtcCall::CALL_KEY_INVALID)
    {
        pMtcTrafficInfo->m_objCallKeys.Append(nCallKeyIn);
    }
}

PRIVATE void MtcCallTrafficChecker::DeInit()
{
    m_objContext.GetCallStateProxy().RemoveListener(this);
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (pNormalService)
    {
        pNormalService->RemoveAosStateListener(this);
    }

    IMtcService* pEmergencyService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pEmergencyService)
    {
        pEmergencyService->RemoveAosStateListener(this);
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_objMtcTrafficInfos.GetSize(); nIndex++)
    {
        const MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetAt(nIndex);
        if (pMtcTrafficInfo)
        {
            StopTrafficChecking(pMtcTrafficInfo->m_eTrafficType, pMtcTrafficInfo->m_eCallDirection);
            delete pMtcTrafficInfo;
        }
    }

    m_objMtcTrafficInfos.Clear();
}

PRIVATE TrafficType MtcCallTrafficChecker::ConvertCallTypeToTrafficType(
        IN CallType eCallType, IN IMS_BOOL bEmergency) const
{
    if (bEmergency)
    {
        return IImsRadio::TRAFFIC_TYPE_EMERGENCY;
    }

    switch (eCallType)
    {
        case CallType::VOIP:
        case CallType::RTT:
            return IImsRadio::TRAFFIC_TYPE_VOICE;
        case CallType::VT:
        case CallType::VIDEO_RTT:
            return IImsRadio::TRAFFIC_TYPE_VIDEO;
        case CallType::UNKNOWN:
            return IImsRadio::TRAFFIC_TYPE_VOICE;
    }
}

PRIVATE IMS_UINT32 MtcCallTrafficChecker::ConvertNetworkType(IN IMS_BOOL bWifi) const
{
    if (bWifi)
    {
        return IImsRadio::ACCESS_NETWORK_TYPE_IWLAN;
    }

    switch (m_piNetworkWatcher->GetNetRadioTechType())
    {
        case NW_REPORT_RADIO_NR:
            return IImsRadio::ACCESS_NETWORK_TYPE_NGRAN;
        case NW_REPORT_RADIO_LTE:
            return IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN;
        default:
            return IImsRadio::ACCESS_NETWORK_TYPE_UNKNOWN;
    }
}

PRIVATE void MtcCallTrafficChecker::AddCallKeyIfNeeded(
        IN TrafficType eTrafficType, IN CallDirection eCallDirection, IN CallKey nCallKeyIn)
{
    MtcTrafficInfo* pMtcTrafficInfo = GetCallTrafficInfo(eTrafficType, eCallDirection);

    if (pMtcTrafficInfo == IMS_NULL)
    {
        return;
    }

    ImsList<CallKey>& objCallKeys = pMtcTrafficInfo->m_objCallKeys;

    for (IMS_UINT32 nIndex = 0; nIndex < objCallKeys.GetSize(); nIndex++)
    {
        const CallKey nCallKey = objCallKeys.GetAt(nIndex);

        if (nCallKey == nCallKeyIn)
        {
            return;
        }
    }

    objCallKeys.Append(nCallKeyIn);
    IMS_TRACE_D("AddCallKeyIfNeeded TrafficType[%d] CallDirection[%d] CallKey[%d]", eTrafficType,
            eCallDirection, nCallKeyIn);
}

PRIVATE void MtcCallTrafficChecker::RemoveCallKeyAndStopTrafficCheckingIfNeeded(
        IN CallKey nCallKeyIn)
{
    for (IMS_UINT32 nInfoIndex = 0; nInfoIndex < m_objMtcTrafficInfos.GetSize(); nInfoIndex++)
    {
        MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetAt(nInfoIndex);

        ImsList<CallKey>& objCallKeys = pMtcTrafficInfo->m_objCallKeys;

        if (objCallKeys.IsEmpty())
        {
            delete pMtcTrafficInfo;
            m_objMtcTrafficInfos.RemoveAt(nInfoIndex);
            return;
        }

        for (IMS_UINT32 nCallKeyIndex = 0; nCallKeyIndex < objCallKeys.GetSize(); nCallKeyIndex++)
        {
            const CallKey nCallKey = objCallKeys.GetAt(nCallKeyIndex);
            if (nCallKey == nCallKeyIn)
            {
                objCallKeys.RemoveAt(nCallKeyIndex);
                IMS_TRACE_D(
                        "RemoveCallKeyAndStopTrafficCheckingIfNeeded TrafficType[%d] CallKey[%d]",
                        pMtcTrafficInfo->m_eTrafficType, nCallKey, 0);

                if (objCallKeys.IsEmpty())
                {
                    StopTrafficChecking(
                            pMtcTrafficInfo->m_eTrafficType, pMtcTrafficInfo->m_eCallDirection);
                    delete pMtcTrafficInfo;
                    m_objMtcTrafficInfos.RemoveAt(nInfoIndex);
                }
                return;
            }
        }
    }
}

PRIVATE void MtcCallTrafficChecker::NotifyRadioConnectionFailedListener(
        IN TrafficType eTrafficType, IN CallDirection eCallDirection)
{
    MtcTrafficInfo* pMtcTrafficInfo = GetCallTrafficInfo(eTrafficType, eCallDirection);

    if (pMtcTrafficInfo == IMS_NULL)
    {
        return;
    }

    ImsList<CallKey> objCallKeys = pMtcTrafficInfo->m_objCallKeys;

    for (IMS_UINT32 nIndex = 0; nIndex < objCallKeys.GetSize(); nIndex++)
    {
        m_objMtcRadioConnectionFailureListener.OnConnectionFailed(objCallKeys.GetAt(nIndex));
    }
}

PRIVATE void MtcCallTrafficChecker::NotifyTrafficCheckerListener(IN IMS_BOOL bReady)
{
    if (m_piMtcCallTrafficCheckerListener == IMS_NULL)
    {
        return;
    }

    if (bReady)
    {
        m_piMtcCallTrafficCheckerListener->OnConnectionSetupPrepared();
    }
    else
    {
        m_piMtcCallTrafficCheckerListener->OnConnectionFailed();
    }
}

PRIVATE MtcTrafficInfo* MtcCallTrafficChecker::GetCallTrafficInfo(
        IN TrafficType eTrafficType, IN CallDirection eCallDirection) const
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objMtcTrafficInfos.GetSize(); nIndex++)
    {
        MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetAt(nIndex);

        if (pMtcTrafficInfo->m_eTrafficType == eTrafficType &&
                pMtcTrafficInfo->m_eCallDirection == eCallDirection)
        {
            return pMtcTrafficInfo;
        }
    }

    return IMS_NULL;
}

PRIVATE MtcTrafficInfo* MtcCallTrafficChecker::CreateCallTrafficInfo(
        IN TrafficType eTrafficType, IN CallDirection eCallDirection)
{
    MtcTrafficInfo* pMtcTrafficInfo = new MtcTrafficInfo(eTrafficType, eCallDirection, *this);
    m_objMtcTrafficInfos.Append(pMtcTrafficInfo);

    return pMtcTrafficInfo;
}

PRIVATE IMS_BOOL MtcCallTrafficChecker::IsTrafficPrepared(
        IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType) const
{
    MtcTrafficInfo* pMtcTrafficInfo =
            GetCallTrafficInfo(ConvertCallTypeToTrafficType(eCallType, bEmergency),
                    static_cast<IMS_UINT32>(ePeerType));

    if (pMtcTrafficInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pMtcTrafficInfo->m_bTrafficActive;
}

PRIVATE IMS_BOOL MtcCallTrafficChecker::IsTrafficAllowed(
        IN CallType eCallType, IN IMS_BOOL bEmergency) const
{
    return m_piImsRadio->IsImsTrafficAllowed(ConvertCallTypeToTrafficType(eCallType, bEmergency));
}

PRIVATE void MtcCallTrafficChecker::StartTrafficChecking(
        IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType, IN IMS_BOOL bWifi)
{
    TrafficType eTrafficType = ConvertCallTypeToTrafficType(eCallType, bEmergency);
    const IMS_UINT32 eCallDirection =
            ePeerType == PeerType::MO ? IImsRadio::DIRECTION_MO : IImsRadio::DIRECTION_MT;
    MtcTrafficInfo* pMtcTrafficInfo = GetCallTrafficInfo(eTrafficType, eCallDirection);

    if (pMtcTrafficInfo == IMS_NULL)
    {
        pMtcTrafficInfo = CreateCallTrafficInfo(
                ConvertCallTypeToTrafficType(eCallType, bEmergency), eCallDirection);
    }
    else
    {
        if (!(pMtcTrafficInfo->m_objCallKeys.IsEmpty()))
        {
            return;
        }
    }

    m_piImsRadio->StartImsTraffic(
            eTrafficType, ConvertNetworkType(bWifi), eCallDirection, pMtcTrafficInfo);

    IMS_TRACE_I("StartTrafficChecking TrafficType[%d] CallDirection[%d]", eTrafficType,
            eCallDirection, 0);
}

PRIVATE void MtcCallTrafficChecker::StopTrafficChecking(
        IN TrafficType eTrafficType, IN CallDirection eCallDirection)
{
    MtcTrafficInfo* pMtcTrafficInfo = GetCallTrafficInfo(eTrafficType, eCallDirection);

    if (pMtcTrafficInfo == IMS_NULL)
    {
        return;
    }

    m_piImsRadio->StopImsTraffic(pMtcTrafficInfo);

    IMS_TRACE_I("StopTrafficChecking TrafficType[%d] CallDirection[%d]", eTrafficType,
            eCallDirection, 0);
}

PRIVATE void MtcTrafficInfo::ImsRadio_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    m_bTrafficActive = IMS_FALSE;
    m_objMtcRadioConnectionListener.OnConnectionFailed(
            m_eTrafficType, m_eCallDirection, nFailureReason, nCauseCode, nWaitTimeMillis);
}

PUBLIC VIRTUAL void MtcTrafficInfo::ImsRadio_OnConnectionSetupPrepared()
{
    m_bTrafficActive = IMS_TRUE;
    m_objMtcRadioConnectionListener.OnConnectionSetupPrepared(m_eTrafficType, m_eCallDirection);
}
