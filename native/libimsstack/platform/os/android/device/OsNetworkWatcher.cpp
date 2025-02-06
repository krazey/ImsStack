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
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceTrace.h"
#include "device/OsNetworkWatcher.h"
#include "network/OsNetworkConstants.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

PUBLIC
OsNetworkWatcher::OsNetworkWatcher(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_eNetStatusType(NW_REPORT_RADIO_NOSRV),
        m_eNetServiceType(NW_REPORT_SRV_NOSRV),
        m_eNetDomainType(NW_REPORT_DOMAIN_NOSRV)
{
    m_eNetDomainType = NW_REPORT_DOMAIN_CSPS;
    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_NETWORK, this, GetSlotId());

    PostMsgRegisteredThread(GetSlotId());
}

PUBLIC VIRTUAL OsNetworkWatcher::~OsNetworkWatcher()
{
    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_NETWORK, this, GetSlotId());
}

PUBLIC VIRTUAL IMS_UINT32 OsNetworkWatcher::GetNetworkStatus(IN const AString& strProfile)
{
    return ((IMS_UINT32)GetNetRadioTechType(strProfile) | (IMS_UINT32)GetNetServiceType() |
            (IMS_UINT32)GetNetDomainType());
}

PUBLIC VIRTUAL NETRADIO_ENTYPE OsNetworkWatcher::GetNetRadioTechType(
        IN const AString& strProfile, IN IMS_SINT32 nApnType /*= NetworkPolicy::APN_NONE*/)
{
    const NetworkPolicy* pNetworkPolicy = (nApnType != NetworkPolicy::APN_NONE)
            ? NetworkServicePolicy::GetInstance()->GetPolicy(nApnType, GetSlotId())
            : NetworkServicePolicy::GetInstance()->GetPolicy(strProfile, GetSlotId());

    if (pNetworkPolicy == IMS_NULL)
    {
        return NW_REPORT_RADIO_NOSRV;
    }

    if (pNetworkPolicy->IsMobilePolicy())
    {
        IMS_SINT32 nType = GetNetworkType();

        IMS_TRACE_D("GetNetRadioTechType(Mobile) :: RAT(%s)", RadioTechToString(nType), 0, 0);

        m_eNetDomainType = NW_REPORT_DOMAIN_CSPS;

        switch (nType)
        {
            case RADIOTECH_TYPE_EHRPD:
                m_eNetStatusType = NW_REPORT_RADIO_EHRPD;
                break;

            case RADIOTECH_TYPE_LTE:  // FALL-THROUGH
            case RADIOTECH_TYPE_LTE_CA:
                m_eNetStatusType = NW_REPORT_RADIO_LTE;
                break;

            case RADIOTECH_TYPE_NR:
                m_eNetStatusType = NW_REPORT_RADIO_NR;
                break;

            case RADIOTECH_TYPE_UMTS:  // FALL-THROUGH
            case RADIOTECH_TYPE_TD_SCDMA:
                m_eNetStatusType = NW_REPORT_RADIO_WCDMA;
                break;

            case RADIOTECH_TYPE_HSDPA:  // FALL-THROUGH
            case RADIOTECH_TYPE_HSPA:   // FALL-THROUGH
            case RADIOTECH_TYPE_HSUPA:  // FALL-THROUGH
            case RADIOTECH_TYPE_HSPAP:
                m_eNetStatusType = NW_REPORT_RADIO_HSPA;
                break;

            case RADIOTECH_TYPE_GPRS:  // FALL-THROUGH
            case RADIOTECH_TYPE_GSM:
                m_eNetStatusType = NW_REPORT_RADIO_GSM;
                break;

            case RADIOTECH_TYPE_EDGE:
                m_eNetStatusType = NW_REPORT_RADIO_EDGE;
                break;

            // Other Platforms - Need to implement
            default:
            {
                m_eNetStatusType = NW_REPORT_RADIO_NOSRV;
                m_eNetDomainType = NW_REPORT_DOMAIN_NOSRV;
                break;
            }
        }

        return m_eNetStatusType;
    }
    else if (pNetworkPolicy->GetApnType() == NetworkPolicy::APN_WIFI)
    {
        IMS_TRACE_D("GetNetRadioTechType(WiFi)", 0, 0, 0);

        // This operation is provided for mobile-based AoS enabler
        // to verify the service features on WLAN environment.
        m_eNetStatusType = NW_REPORT_RADIO_WLAN;
        m_eNetDomainType = NW_REPORT_DOMAIN_CSPS;

        return m_eNetStatusType;
    }

    return NW_REPORT_RADIO_NOSRV;
}

PUBLIC VIRTUAL NETRADIO_ENTYPE OsNetworkWatcher::GetNetRadioTechType()
{
    switch (GetNetworkType())
    {
        case RADIOTECH_TYPE_UNKNOWN:
            m_eNetStatusType = NW_REPORT_RADIO_NOSRV;
            break;

        case RADIOTECH_TYPE_GSM:  // FALL-THROUGH
        case RADIOTECH_TYPE_GPRS:
            m_eNetStatusType = NW_REPORT_RADIO_GSM;
            break;

        case RADIOTECH_TYPE_EDGE:
            m_eNetStatusType = NW_REPORT_RADIO_EDGE;
            break;

        case RADIOTECH_TYPE_UMTS:  // FALL-THROUGH
        case RADIOTECH_TYPE_TD_SCDMA:
            m_eNetStatusType = NW_REPORT_RADIO_WCDMA;
            break;

        case RADIOTECH_TYPE_HSDPA:  // FALL-THROUGH
        case RADIOTECH_TYPE_HSUPA:  // FALL-THROUGH
        case RADIOTECH_TYPE_HSPA:   // FALL-THROUGH
        case RADIOTECH_TYPE_HSPAP:
            m_eNetStatusType = NW_REPORT_RADIO_HSPA;
            break;

        case RADIOTECH_TYPE_EHRPD:
            m_eNetStatusType = NW_REPORT_RADIO_EHRPD;
            break;

        case RADIOTECH_TYPE_LTE:  // FALL-THROUGH
        case RADIOTECH_TYPE_LTE_CA:
            m_eNetStatusType = NW_REPORT_RADIO_LTE;
            break;

        case RADIOTECH_TYPE_NR:
            m_eNetStatusType = NW_REPORT_RADIO_NR;
            break;

        default:
            break;
    }

    return m_eNetStatusType;
}

PUBLIC VIRTUAL NETRADIO_ENTYPE OsNetworkWatcher::GetNetVoiceRadioTechType()
{
    IMS_SINT32 nType =
            PlatformContext::GetInstance()->GetSystem()->GetVoiceNetworkType(GetSlotId());

    NETRADIO_ENTYPE eVoiceNetType = NW_REPORT_RADIO_NOSRV;

    switch (nType)
    {
        case RADIOTECH_TYPE_UNKNOWN:
            eVoiceNetType = NW_REPORT_RADIO_NOSRV;
            break;

        case RADIOTECH_TYPE_GSM:
        case RADIOTECH_TYPE_GPRS:
            eVoiceNetType = NW_REPORT_RADIO_GSM;
            break;

        case RADIOTECH_TYPE_EDGE:
            eVoiceNetType = NW_REPORT_RADIO_EDGE;
            break;

        case RADIOTECH_TYPE_UMTS:  // FALL-THROUGH
        case RADIOTECH_TYPE_TD_SCDMA:
            eVoiceNetType = NW_REPORT_RADIO_WCDMA;
            break;

        case RADIOTECH_TYPE_HSDPA:  // FALL-THROUGH
        case RADIOTECH_TYPE_HSUPA:  // FALL-THROUGH
        case RADIOTECH_TYPE_HSPA:   // FALL-THROUGH
        case RADIOTECH_TYPE_HSPAP:
            eVoiceNetType = NW_REPORT_RADIO_HSPA;
            break;

        case RADIOTECH_TYPE_EHRPD:
            eVoiceNetType = NW_REPORT_RADIO_EHRPD;
            break;

        case RADIOTECH_TYPE_LTE:  // FALL-THROUGH
        case RADIOTECH_TYPE_LTE_CA:
            eVoiceNetType = NW_REPORT_RADIO_LTE;
            break;

        case RADIOTECH_TYPE_NR:
            eVoiceNetType = NW_REPORT_RADIO_NR;
            break;

        default:
            eVoiceNetType = NW_REPORT_RADIO_NOSRV;
            break;
    }

    return eVoiceNetType;
}

PUBLIC VIRTUAL NETSERVICE_ENTYPE OsNetworkWatcher::GetNetServiceType(
        IN const AString& strProfile, IN IMS_SINT32 nApnType /*= NetworkPolicy::APN_NONE*/)
{
    const NetworkPolicy* pNetworkPolicy = (nApnType != NetworkPolicy::APN_NONE)
            ? NetworkServicePolicy::GetInstance()->GetPolicy(nApnType, GetSlotId())
            : NetworkServicePolicy::GetInstance()->GetPolicy(strProfile, GetSlotId());

    if (pNetworkPolicy != IMS_NULL)
    {
        IMS_TRACE_D("GetNetServiceType :: apnType=%d", pNetworkPolicy->GetApnType(), 0, 0);

        if (pNetworkPolicy->IsMobilePolicy())
        {
            return GetNetServiceType();
        }
        else if (pNetworkPolicy->GetApnType() == NetworkPolicy::APN_WIFI)
        {
            return NW_REPORT_SRV_SRV;
        }
    }

    return NW_REPORT_SRV_NOSRV;
}

PUBLIC VIRTUAL NETSERVICE_ENTYPE OsNetworkWatcher::GetNetServiceType()
{
    switch (GetServiceStateType())
    {
        case STATE_IN_SERVICE:
            m_eNetServiceType = NW_REPORT_SRV_SRV;
            break;

        case STATE_OUT_OF_SERVICE:
            m_eNetServiceType = NW_REPORT_SRV_NOSRV;
            break;

        case STATE_EMERGENCY_ONLY:
            m_eNetServiceType = NW_REPORT_SRV_LIMITED;
            break;

        case STATE_POWER_OFF:
            m_eNetServiceType = NW_REPORT_SRV_PWRSAVE;
            break;

        default:
            break;
    }

    return m_eNetServiceType;
}

PUBLIC VIRTUAL NETSERVICE_ENTYPE OsNetworkWatcher::GetNetVoiceServiceType()
{
    switch (GetVoiceServiceStateType())
    {
        case STATE_IN_SERVICE:
            m_eNetServiceType = NW_REPORT_SRV_SRV;
            break;

        case STATE_OUT_OF_SERVICE:
            m_eNetServiceType = NW_REPORT_SRV_NOSRV;
            break;

        case STATE_EMERGENCY_ONLY:
            m_eNetServiceType = NW_REPORT_SRV_LIMITED;
            break;

        case STATE_POWER_OFF:
            m_eNetServiceType = NW_REPORT_SRV_PWRSAVE;
            break;

        default:
            break;
    }

    return m_eNetServiceType;
}

PUBLIC VIRTUAL NETDOMAIN_ENTYPE OsNetworkWatcher::GetNetDomainType()
{
    return m_eNetDomainType;
}

PUBLIC VIRTUAL void OsNetworkWatcher::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    IMS_TRACE_D(
            "NetWatcherInfo :: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d, nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_AIRPLANE_MODE_CHANGED:
            UpdateAirplaneMode(LONG_TO_SINT(nWParam));
            break;

        case IMS_SYSTEM_DATACONNECTION_STATE_CHANGED:
            UpdateDataConnectionStateChanged(LONG_TO_SINT(nWParam));
            break;

        case IMS_SYSTEM_SERVICE_STATE_CHANGED:
            UpdateServiceState(LONG_TO_SINT(nWParam));
            break;

        case IMS_SYSTEM_RADIOTECH_STATE_CHANGED:
            UpdateRadioTechChanged(LONG_TO_SINT(nWParam));
            break;

        case IMS_SYSTEM_VOICE_RADIOTECH_STATE_CHANGED:
            UpdateVoiceRadioTechChanged(LONG_TO_SINT(nWParam));
            break;

        default:
            break;
    }
}

/**
 * @brief Callback invoked when device service state changes.
 *
 * @see ServiceState#STATE_IN_SERVICE
 * @see ServiceState#STATE_OUT_OF_SERVICE
 * @see ServiceState#STATE_EMERGENCY_ONLY
 * @see ServiceState#STATE_POWER_OFF
 */
void OsNetworkWatcher::UpdateServiceState(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_IN_SERVICE:
            m_eNetServiceType = NW_REPORT_SRV_SRV;
            break;

        case STATE_OUT_OF_SERVICE:
            m_eNetServiceType = NW_REPORT_SRV_NOSRV;
            break;

        case STATE_EMERGENCY_ONLY:
            m_eNetServiceType = NW_REPORT_SRV_LIMITED;
            break;

        case STATE_POWER_OFF:
            m_eNetServiceType = NW_REPORT_SRV_PWRSAVE;
            break;

        default:
            break;
    }

    PostMsgRegisteredThread(GetSlotId());
}

void OsNetworkWatcher::UpdateRadioTechChanged(IN IMS_SINT32 nState)
{
    IMS_TRACE_D("UpdateRadioTechChanged(%d)", nState, 0, 0);

    switch (nState)
    {
        case RADIO_TECH_EHRPD:  // DcNetWatcher.RAT_EHRPD
            m_eNetStatusType = NW_REPORT_RADIO_EHRPD;
            break;

        case RADIO_TECH_LTE:  // DcNetWatcher.RAT_4G
            m_eNetStatusType = NW_REPORT_RADIO_LTE;
            break;

        case RADIO_TECH_WCDMA:  // DcNetWatcher.RAT_3G
            m_eNetStatusType = NW_REPORT_RADIO_WCDMA;
            break;

        case RADIO_TECH_GSM:  // DcNetWatcher.RAT_2G
            m_eNetStatusType = NW_REPORT_RADIO_GSM;
            break;

        case RADIO_TECH_NR:  // DcNetWatcher.RAT_5G
            m_eNetStatusType = NW_REPORT_RADIO_NR;
            break;

        default:
            m_eNetStatusType = NW_REPORT_RADIO_NOSRV;
            break;
    }

    PostMsgRegisteredThread(GetSlotId());
}

void OsNetworkWatcher::UpdateVoiceRadioTechChanged(IN IMS_SINT32 nState)
{
    IMS_TRACE_D("UpdateVoiceRadioTechChanged(%d)", nState, 0, 0);

    PostMsgRegisteredThread(GetSlotId());
}

void OsNetworkWatcher::UpdateAirplaneMode(IN IMS_SINT32 nState)
{
    if (nState != 0)
    {
        m_eNetServiceType = NW_REPORT_SRV_NOSRV;
    }
    else
    {
        m_eNetServiceType = NW_REPORT_SRV_SRV;
    }

    PostMsgRegisteredThread(GetSlotId());
}

/**
 * DATA_INVALID            = -1,
 * DATA_DISCONNECTED        = 0,
 * DATA_CONNECTING            = 1,
 * DATA_CONNECTED            = 2,
 * DATA_SUSPENDED            = 3 ,
 */
void OsNetworkWatcher::UpdateDataConnectionStateChanged(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case DATA_DISCONNECTED:
            m_eNetServiceType = NW_REPORT_SRV_SRV;
            m_eNetDomainType = NW_REPORT_DOMAIN_CS;
            break;
        case DATA_CONNECTED:
            m_eNetServiceType = NW_REPORT_SRV_SRV;
            m_eNetDomainType = NW_REPORT_DOMAIN_CSPS;
            break;
        default:
            return;
    }

    PostMsgRegisteredThread(GetSlotId());
}

PUBLIC VIRTUAL IMS_SINT32 OsNetworkWatcher::GetNetworkType()
{
    return PlatformContext::GetInstance()->GetSystem()->GetNetworkType(GetSlotId());
}

PUBLIC VIRTUAL IMS_SINT32 OsNetworkWatcher::GetRoamingState()
{
    return PlatformContext::GetInstance()->GetSystem()->GetRoamingState(GetSlotId());
}

PUBLIC VIRTUAL IMS_SINT32 OsNetworkWatcher::GetVoiceRoamingType()
{
    return PlatformContext::GetInstance()->GetSystem()->GetVoiceRoamingType(GetSlotId());
}

PUBLIC VIRTUAL IMS_SINT32 OsNetworkWatcher::GetDataRoamingType()
{
    return PlatformContext::GetInstance()->GetSystem()->GetDataRoamingType(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkWatcher::IsImsEmergencyCallSupported()
{
    return PlatformContext::GetInstance()->GetSystem()->IsImsEmergencyCallSupported(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkWatcher::IsImsVoiceCallSupported()
{
    return PlatformContext::GetInstance()->GetSystem()->IsImsVoiceCallSupported(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkWatcher::IsEmergencyOnly()
{
    return PlatformContext::GetInstance()->GetSystem()->IsEmergencyOnly(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkWatcher::IsEmergencyAttachSupported()
{
    return PlatformContext::GetInstance()->GetSystem()->IsEmergencyAttachSupported(GetSlotId());
}

/**
 * @see #STATE_IN_SERVICE
 * @see #STATE_OUT_OF_SERVICE
 * @see #STATE_EMERGENCY_ONLY
 * @see #STATE_POWER_OFF
 */
PUBLIC
IMS_SINT32 OsNetworkWatcher::GetServiceStateType() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetServiceState(GetSlotId());
}

/**
 * @see #STATE_IN_SERVICE
 * @see #STATE_OUT_OF_SERVICE
 * @see #STATE_EMERGENCY_ONLY
 * @see #STATE_POWER_OFF
 */
PUBLIC
IMS_SINT32 OsNetworkWatcher::GetVoiceServiceStateType() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetVoiceServiceState(GetSlotId());
}

PUBLIC VIRTUAL IMS_SINT32 OsNetworkWatcher::GetMocnPlmnInfo()
{
    return PlatformContext::GetInstance()->GetSystem()->GetMocnPlmnInfo(GetSlotId());
}

PRIVATE GLOBAL const IMS_CHAR* OsNetworkWatcher::RadioTechToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RADIOTECH_TYPE_UNKNOWN:
            return "RADIOTECH_TYPE_UNKNOWN";
        case RADIOTECH_TYPE_GPRS:
            return "RADIOTECH_TYPE_GPRS";
        case RADIOTECH_TYPE_EDGE:
            return "RADIOTECH_TYPE_EDGE";
        case RADIOTECH_TYPE_UMTS:
            return "RADIOTECH_TYPE_UMTS";
        case RADIOTECH_TYPE_CDMA:
            return "RADIOTECH_TYPE_CDMA";
        case RADIOTECH_TYPE_EVDO_0:
            return "RADIOTECH_TYPE_EVDO_0";
        case RADIOTECH_TYPE_EVDO_A:
            return "RADIOTECH_TYPE_EVDO_A";
        case RADIOTECH_TYPE_1xRTT:
            return "RADIOTECH_TYPE_1xRTT";
        case RADIOTECH_TYPE_HSDPA:
            return "RADIOTECH_TYPE_HSDPA";
        case RADIOTECH_TYPE_HSUPA:
            return "RADIOTECH_TYPE_HSUPA";
        case RADIOTECH_TYPE_HSPA:
            return "RADIOTECH_TYPE_HSPA";
        case RADIOTECH_TYPE_IDEN:
            return "RADIOTECH_TYPE_IDEN";
        case RADIOTECH_TYPE_EVDO_B:
            return "RADIOTECH_TYPE_EVDO_B";
        case RADIOTECH_TYPE_LTE:
            return "RADIOTECH_TYPE_LTE";
        case RADIOTECH_TYPE_EHRPD:
            return "RADIOTECH_TYPE_EHRPD";
        case RADIOTECH_TYPE_HSPAP:
            return "RADIOTECH_TYPE_HSPAP";
        case RADIOTECH_TYPE_GSM:
            return "RADIOTECH_TYPE_GSM";
        case RADIOTECH_TYPE_TD_SCDMA:
            return "RADIOTECH_TYPE_TD_SCDMA";
        case RADIOTECH_TYPE_IWLAN:
            return "RADIOTECH_TYPE_IWLAN";
        case RADIOTECH_TYPE_LTE_CA:
            return "RADIOTECH_TYPE_LTE_CA";
        case RADIOTECH_TYPE_NR:
            return "RADIOTECH_TYPE_NR";
        default:
            return "__INVALID__";
    }
}
