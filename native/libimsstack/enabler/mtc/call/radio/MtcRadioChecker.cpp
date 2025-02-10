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
#include "call/IMtcCallManager.h"
#include "call/radio/MtcRadioChecker.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcRadioChecker::MtcRadioChecker(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_piNetworkWatcher(PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(
                m_objContext.GetSlotId())),
        m_piImsRadio(ImsRadioService::GetImsRadioService()->GetImsRadio(m_objContext.GetSlotId())),
        m_piMtcRadioCheckerListener(IMS_NULL),
        m_objMtcTrafficInfos(ImsList<MtcTrafficInfo*>())
{
    IMS_TRACE_D("+MtcRadioChecker", 0, 0, 0);
}

PUBLIC MtcRadioChecker::~MtcRadioChecker()
{
    IMS_TRACE_D("~MtcRadioChecker", 0, 0, 0);

    DeInit();
}

PUBLIC void MtcRadioChecker::Init()
{
    m_objContext.GetSipInterfaceFactory().GetISessionHolder().AddListener(this);
    m_objContext.GetServiceByType(ServiceType::NORMAL)->AddAosStateListener(this);
    m_objContext.GetServiceByType(ServiceType::EMERGENCY)->AddAosStateListener(this);
}

PUBLIC VIRTUAL void MtcRadioChecker::SetTrafficCheckerListener(
        IN IMtcRadioCheckerListener* pListener)
{
    m_piMtcRadioCheckerListener = pListener;
}

PUBLIC VIRTUAL CheckResult MtcRadioChecker::Check(IN CallType eCallType, IN IMS_BOOL bEmergency,
        IN PeerType ePeerType, IN IMS_BOOL bWifi, IN CallKey nCallKey)
{
    if (IsTrafficPrepared(eCallType, bEmergency, ePeerType))
    {
        IMS_TRACE_D("Check unblocked - traffic has been prepared", 0, 0, 0);
        return CheckResult::UNBLOCKED;
    }

    if (ePeerType == PeerType::MT)
    {
        StartTrafficChecking(eCallType, bEmergency, ePeerType, bWifi, nCallKey);
        return CheckResult::UNBLOCKED;
    }

    if (IsTrafficAllowed(eCallType, bEmergency) == IMS_FALSE)
    {
        IMS_TRACE_D("Check blocked - traffic hasn't been allowed", 0, 0, 0);
        return CheckResult::BLOCKED;
    }

    StartTrafficChecking(eCallType, bEmergency, ePeerType, bWifi, nCallKey);
    IMS_TRACE_D("Check pending", 0, 0, 0);
    return CheckResult::PENDING;
}

PUBLIC VIRTUAL void MtcRadioChecker::OnIpcanChanged(
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

PUBLIC VIRTUAL void MtcRadioChecker::OnSessionInterfaceReleased(IN CallKey nKey)
{
    RemoveCallKeyAndStopTrafficCheckingIfNeeded(nKey);
}

PUBLIC VIRTUAL void MtcRadioChecker::OnConnectionFailed(IN TrafficType eTrafficType,
        IN CallDirection /* eCallDirection */, IN IMS_UINT32 nFailureReason,
        IN IMS_UINT32 /* nCauseCode */, IN IMS_UINT32 nWaitTimeMillis)
{
    IMS_TRACE_D("OnConnectionFailed TrafficType[%d] FailureReason[%d]", eTrafficType,
            nFailureReason, 0);

    switch (nFailureReason)
    {
        case IImsRadio::REASON_ACCESS_DENIED:
        case IImsRadio::REASON_INTERNAL_ERROR:
        case IImsRadio::REASON_RRC_REJECT:
            if (m_piMtcRadioCheckerListener != IMS_NULL)
            {
                m_piMtcRadioCheckerListener->OnConnectionFailed(nFailureReason, nWaitTimeMillis);
            }
            break;

        case IImsRadio::REASON_NAS_FAILURE:
        case IImsRadio::REASON_RACH_FAILURE:
        case IImsRadio::REASON_RLC_FAILURE:
        case IImsRadio::REASON_RRC_TIMEOUT:
        case IImsRadio::REASON_NO_SERVICE:
        case IImsRadio::REASON_PDN_NOT_AVAILABLE:
        case IImsRadio::REASON_RF_BUSY:
            if (m_piMtcRadioCheckerListener != IMS_NULL)
            {
                // Calls will be blocked in these cases (e.g. message cannot be sent),
                // so they could be handled as success here.
                m_piMtcRadioCheckerListener->OnConnectionSetupPrepared();
            }
            break;
    }
}

PUBLIC VIRTUAL void MtcRadioChecker::OnConnectionSetupPrepared(
        IN TrafficType eTrafficType, IN CallDirection /* eCallDirection */)
{
    IMS_TRACE_D("OnConnectionSetupPrepared TrafficType[%d]", eTrafficType, 0, 0);
    if (m_piMtcRadioCheckerListener != IMS_NULL)
    {
        m_piMtcRadioCheckerListener->OnConnectionSetupPrepared();
    }
}

PUBLIC void MtcRadioChecker::CreateCallTrafficInfoWithGivenValue(IN TrafficType eTrafficType,
        IN CallDirection eCallDirection, IN IMS_BOOL bActive, IN CallKey nCallKeyIn)
{
    MtcTrafficInfo* pMtcTrafficInfo = GetCallTrafficInfo(eTrafficType, eCallDirection);

    if (pMtcTrafficInfo == IMS_NULL)
    {
        pMtcTrafficInfo = CreateCallTrafficInfo(eTrafficType, eCallDirection);
    }

    pMtcTrafficInfo->m_bTrafficActive = bActive;

    if (nCallKeyIn != IMtcCall::CALL_KEY_INVALID)
    {
        pMtcTrafficInfo->m_objCallKeys.Append(nCallKeyIn);
    }
}

PRIVATE void MtcRadioChecker::DeInit()
{
    m_objContext.GetSipInterfaceFactory().GetISessionHolder().RemoveListener(this);
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
        MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetAt(nIndex);
        if (pMtcTrafficInfo)
        {
            StopTrafficChecking(*pMtcTrafficInfo);
            delete pMtcTrafficInfo;
        }
    }

    m_objMtcTrafficInfos.Clear();
}

PRIVATE TrafficType MtcRadioChecker::ConvertCallTypeToTrafficType(
        IN CallType eCallType, IN IMS_BOOL bEmergency)
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
        default:  //  CallType::UNKNOWN:
            return IImsRadio::TRAFFIC_TYPE_VOICE;
    }
}

PRIVATE IMS_UINT32 MtcRadioChecker::ConvertNetworkType(IN IMS_BOOL bWifi) const
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

PRIVATE void MtcRadioChecker::AddCallKey(IN MtcTrafficInfo& pMtcTrafficInfo, IN CallKey nCallKey)
{
    pMtcTrafficInfo.m_objCallKeys.Append(nCallKey);
}

PRIVATE void MtcRadioChecker::RemoveCallKeyAndStopTrafficCheckingIfNeeded(IN CallKey nCallKeyIn)
{
    if (!IsCallTerminated(nCallKeyIn))
    {
        return;  // Silent redialing, no need to stop the traffic
    }

    for (IMS_UINT32 nInfoIndex = 0; nInfoIndex < m_objMtcTrafficInfos.GetSize(); nInfoIndex++)
    {
        MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetAt(nInfoIndex);

        ImsList<CallKey>& objCallKeys = pMtcTrafficInfo->m_objCallKeys;

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
                    StopTrafficChecking(*pMtcTrafficInfo);
                    delete pMtcTrafficInfo;
                    m_objMtcTrafficInfos.RemoveAt(nInfoIndex);
                }
                return;
            }
        }
    }
}

PRIVATE MtcTrafficInfo* MtcRadioChecker::GetCallTrafficInfo(
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

PRIVATE MtcTrafficInfo* MtcRadioChecker::CreateCallTrafficInfo(
        IN TrafficType eTrafficType, IN CallDirection eCallDirection)
{
    MtcTrafficInfo* pMtcTrafficInfo = new MtcTrafficInfo(eTrafficType, eCallDirection, *this);
    m_objMtcTrafficInfos.Append(pMtcTrafficInfo);

    return pMtcTrafficInfo;
}

PRIVATE IMS_BOOL MtcRadioChecker::IsTrafficPrepared(
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

PRIVATE IMS_BOOL MtcRadioChecker::IsTrafficAllowed(
        IN CallType eCallType, IN IMS_BOOL bEmergency) const
{
    return m_piImsRadio->IsImsTrafficAllowed(ConvertCallTypeToTrafficType(eCallType, bEmergency));
}

PRIVATE void MtcRadioChecker::StartTrafficChecking(IN CallType eCallType, IN IMS_BOOL bEmergency,
        IN PeerType ePeerType, IN IMS_BOOL bWifi, IN CallKey nCallKey)
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

    AddCallKey(*pMtcTrafficInfo, nCallKey);

    m_piImsRadio->StartImsTraffic(
            eTrafficType, ConvertNetworkType(bWifi), eCallDirection, pMtcTrafficInfo);

    IMS_TRACE_I("StartTrafficChecking TrafficType[%d] CallDirection[%d] CallKey[%d]", eTrafficType,
            eCallDirection, nCallKey);
}

PRIVATE void MtcRadioChecker::StopTrafficChecking(IN MtcTrafficInfo& objTrafficInfo)
{
    m_piImsRadio->StopImsTraffic(&objTrafficInfo);

    IMS_TRACE_I("StopTrafficChecking TrafficType[%d] CallDirection[%d]",
            objTrafficInfo.m_eTrafficType, objTrafficInfo.m_eCallDirection, 0);
}

PRIVATE IMS_BOOL MtcRadioChecker::IsCallTerminated(IN CallKey nKey)
{
    IMtcCall* pCall = m_objContext.GetCallManager().GetCallByCallKey(nKey);
    return pCall->GetKey() == IMtcCall::CALL_KEY_INVALID ||
            pCall->GetState() == IMtcCall::State::TERMINATING;
}

PRIVATE void MtcTrafficInfo::ImsRadio_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    // TODO: need to check that Radio(moden) gives ImsRadio_OnConnectionFailed or not due to
    // fallback to LTE from NR when does redial.
    m_bTrafficActive = IMS_FALSE;
    m_objMtcRadioConnectionListener.OnConnectionFailed(
            m_eTrafficType, m_eCallDirection, nFailureReason, nCauseCode, nWaitTimeMillis);
}

PUBLIC VIRTUAL void MtcTrafficInfo::ImsRadio_OnConnectionSetupPrepared()
{
    m_bTrafficActive = IMS_TRUE;
    m_objMtcRadioConnectionListener.OnConnectionSetupPrepared(m_eTrafficType, m_eCallDirection);
}
