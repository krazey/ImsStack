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

#include "IIpcan.h"
#include "IImsRadio.h"
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
        m_objMtcTrafficInfos(ImsMap<TrafficType, MtcTrafficInfo*>())
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

    m_objMtcTrafficInfos.Add(IImsRadio::TRAFFIC_TYPE_VOICE,
            new MtcTrafficInfo(IImsRadio::TRAFFIC_TYPE_VOICE, *this));
    m_objMtcTrafficInfos.Add(IImsRadio::TRAFFIC_TYPE_VIDEO,
            new MtcTrafficInfo(IImsRadio::TRAFFIC_TYPE_VIDEO, *this));
    m_objMtcTrafficInfos.Add(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
            new MtcTrafficInfo(IImsRadio::TRAFFIC_TYPE_EMERGENCY, *this));
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::SetTrafficCheckerListener(
        IN IMtcCallTrafficCheckerListener* pListener)
{
    m_piMtcCallTrafficCheckerListener = pListener;
}

PUBLIC VIRTUAL IMS_BOOL MtcCallTrafficChecker::IsTrafficPrepared(
        IN CallType eCallType, IN IMS_BOOL bEmergency) const
{
    return m_objMtcTrafficInfos.GetValue(ConvertCallTypeToTrafficType(eCallType, bEmergency))
            ->m_bTrafficActive;
}

PUBLIC VIRTUAL IMS_BOOL MtcCallTrafficChecker::IsTrafficAllowed(
        IN CallType eCallType, IN IMS_BOOL bEmergency) const
{
    return m_piImsRadio->IsImsTrafficAllowed(ConvertCallTypeToTrafficType(eCallType, bEmergency));
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::StartTrafficChecking(
        IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi)
{
    TrafficType eTrafficType = ConvertCallTypeToTrafficType(eCallType, bEmergency);

    m_piImsRadio->StartImsTraffic(
            eTrafficType, ConvertNetworkType(bWifi), m_objMtcTrafficInfos.GetValue(eTrafficType));

    IMS_TRACE_I("StartTrafficChecking TrafficType[%d] ", eTrafficType, 0, 0);
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::StopTrafficChecking(IN TrafficType eTrafficType)
{
    m_piImsRadio->StopImsTraffic(eTrafficType);

    IMS_TRACE_I("StopTrafficChecking TrafficType[%d] ", eTrafficType, 0, 0);
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnIpcanChanged(
        IN IMtcService& objMtcService, IN IMS_UINT32 eIpcan)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objMtcTrafficInfos.GetSize(); nIndex++)
    {
        if (m_objMtcTrafficInfos.GetValueAt(nIndex)->m_objCallKeys.IsEmpty() ||
                !(m_objMtcTrafficInfos.GetValueAt(nIndex)->m_bTrafficActive))
        {
            continue;
        }

        TrafficType eTrafficType = m_objMtcTrafficInfos.GetKeyAt(nIndex);

        IMS_BOOL bEmergency = objMtcService.IsEmergency();
        if ((bEmergency && eTrafficType != IImsRadio::TRAFFIC_TYPE_EMERGENCY) ||
                (!bEmergency && eTrafficType == IImsRadio::TRAFFIC_TYPE_EMERGENCY))
        {
            continue;
        }

        m_piImsRadio->StartImsTraffic(eTrafficType,
                ConvertNetworkType(eIpcan == IIpcan::CATEGORY_WLAN),
                m_objMtcTrafficInfos.GetValue(eTrafficType));
    }
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnCallStateChanged(IN CallKey nCallKey, IN State eState,
        IN Type eType, IN IMS_BOOL bEmergency, IN IMS_SINT32 /* nReason */)
{
    switch (eState)
    {
        case IMtcCall::State::OUTGOING:
        case IMtcCall::State::INCOMING:
        case IMtcCall::State::ALERTING:
            AddCallKeyIfNeeded(ConvertCallTypeToTrafficType(eType, bEmergency), nCallKey);
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
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 /* nCauseCode */,
        IN IMS_UINT32 /* nWaitTimeMillis */)
{
    IMS_TRACE_D("OnConnectionFailed TrafficType[%d] FailureReason[%d]", eTrafficType,
            nFailureReason, 0);

    switch (nFailureReason)
    {
        case IImsRadio::REASON_ACCESS_DENIED:
        case IImsRadio::REASON_INTERNAL_ERROR:
            SetTrafficStatus(eTrafficType, IMS_FALSE);

            NotifyTrafficCheckerListenerIfPossible(IMS_FALSE);
            NotifyRadioConnectionFailedListener(eTrafficType);
            break;

        case IImsRadio::REASON_NAS_FAILURE:
        case IImsRadio::REASON_RACH_FAILURE:
        case IImsRadio::REASON_RLC_FAILURE:
        case IImsRadio::REASON_RRC_REJECT:
        case IImsRadio::REASON_RRC_TIMEOUT:
        case IImsRadio::REASON_NO_SERVICE:
        case IImsRadio::REASON_PDN_NOT_AVAILABLE:
        case IImsRadio::REASON_RF_BUSY:
            NotifyTrafficCheckerListenerIfPossible(IMS_TRUE);
            break;
    }
}

PUBLIC VIRTUAL void MtcCallTrafficChecker::OnConnectionSetupPrepared(IN TrafficType eTrafficType)
{
    IMS_TRACE_D("OnConnectionSetupPrepared TrafficType[%d]", eTrafficType, 0, 0);

    SetTrafficStatus(eTrafficType, IMS_TRUE);

    NotifyTrafficCheckerListenerIfPossible(IMS_TRUE);
}

PUBLIC void MtcCallTrafficChecker::SetTrafficStatus(
        IN TrafficType eTrafficType, IN IMS_BOOL bActive)
{
    m_objMtcTrafficInfos.GetValue(eTrafficType)->m_bTrafficActive = bActive;
}

PUBLIC ImsList<CallKey>& MtcCallTrafficChecker::GetCallKeys(IN TrafficType eTrafficType) const
{
    return m_objMtcTrafficInfos.GetValue(eTrafficType)->m_objCallKeys;
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
        const MtcTrafficInfo* pMtcTrafficInfo = m_objMtcTrafficInfos.GetValueAt(nIndex);
        if (pMtcTrafficInfo)
        {
            delete pMtcTrafficInfo;
        }
    }
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

PRIVATE IMS_UINT32 MtcCallTrafficChecker::ConvertNetworkType(IN IMS_BOOL bWifi)
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
        IN TrafficType eTrafficType, IN CallKey nCallKeyIn)
{
    ImsList<CallKey>& objCallKeys = m_objMtcTrafficInfos.GetValue(eTrafficType)->m_objCallKeys;

    for (IMS_UINT32 nIndex = 0; nIndex < objCallKeys.GetSize(); nIndex++)
    {
        const CallKey nCallKey = objCallKeys.GetAt(nIndex);

        if (nCallKey == nCallKeyIn)
        {
            return;
        }
    }

    objCallKeys.Append(nCallKeyIn);
    IMS_TRACE_D("AddCallKeyIfNeeded TrafficType[%d] CallKey[%d]", eTrafficType, nCallKeyIn, 0);
}

PRIVATE void MtcCallTrafficChecker::RemoveCallKeyAndStopTrafficCheckingIfNeeded(
        IN CallKey nCallKeyIn)
{
    for (IMS_UINT32 nTrafficIndex = 0; nTrafficIndex < m_objMtcTrafficInfos.GetSize();
            nTrafficIndex++)
    {
        ImsList<CallKey>& objCallKeys =
                m_objMtcTrafficInfos.GetValueAt(nTrafficIndex)->m_objCallKeys;
        for (IMS_UINT32 nCallKeyIndex = 0; nCallKeyIndex < objCallKeys.GetSize(); nCallKeyIndex++)
        {
            const CallKey nCallKey = objCallKeys.GetAt(nCallKeyIndex);
            if (nCallKey == nCallKeyIn)
            {
                objCallKeys.RemoveAt(nCallKeyIndex);
                IMS_TRACE_D(
                        "RemoveCallKeyAndStopTrafficCheckingIfNeeded TrafficType[%d] CallKey[%d]",
                        m_objMtcTrafficInfos.GetKeyAt(nTrafficIndex), nCallKey, 0);

                if (objCallKeys.IsEmpty())
                {
                    StopTrafficChecking(m_objMtcTrafficInfos.GetKeyAt(nTrafficIndex));
                }

                return;
            }
        }
    }
}

PRIVATE void MtcCallTrafficChecker::NotifyRadioConnectionFailedListener(IN TrafficType eTrafficType)
{
    ImsList<CallKey> objCallKeys = m_objMtcTrafficInfos.GetValue(eTrafficType)->m_objCallKeys;

    for (IMS_UINT32 nIndex = 0; nIndex < objCallKeys.GetSize(); nIndex++)
    {
        m_objMtcRadioConnectionFailureListener.OnConnectionFailed(objCallKeys.GetValueAt(nIndex));
    }
}

PRIVATE void MtcCallTrafficChecker::NotifyTrafficCheckerListenerIfPossible(IN IMS_BOOL bReady)
{
    if (m_piMtcCallTrafficCheckerListener)
    {
        if (bReady)
        {
            m_piMtcCallTrafficCheckerListener->OnConnectionSetupPrepared();
        }
        else
        {
            m_piMtcCallTrafficCheckerListener->OnConnectionFailed();
        }

        m_piMtcCallTrafficCheckerListener = IMS_NULL;
    }
}

PUBLIC VIRTUAL void MtcTrafficInfo::ImsRadio_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    m_objMtcRadioConnectionListener.OnConnectionFailed(
            m_eTrafficType, nFailureReason, nCauseCode, nWaitTimeMillis);
}

PUBLIC VIRTUAL void MtcTrafficInfo::ImsRadio_OnConnectionSetupPrepared()
{
    m_objMtcRadioConnectionListener.OnConnectionSetupPrepared(m_eTrafficType);
}
