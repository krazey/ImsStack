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
#include "ISystem.h"
#include "IThread.h"
#include "ImsAccessNetworkInfoType.h"
#include "ImsMessageDef.h"
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServiceEvent.h"
#include "ServiceMemory.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "network/OsNetworkConnection.h"
#include "network/OsNetworkConstants.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

// NR / LTE
#define UTRAN_ANI_ITEM_SIZE        4

#define UTRAN_ANI_MCC_INDEX        0
#define UTRAN_ANI_MNC_INDEX        1
#define UTRAN_ANI_CELLID_INDEX     2
#define UTRAN_ANI_TAC_INDEX        3
#define UTRAN_ANI_MODE_INDEX       4

// eHRPD
#define EHRPD_ANI_ITEM_SIZE        2

#define EHRPD_ANI_SECTOR_ID_INDEX  0
#define EHRPD_ANI_SUBNET_LEN_INDEX 1

// GERAN
#define GERAN_ANI_ITEM_SIZE        4

#define GERAN_ANI_MCC_INDEX        0
#define GERAN_ANI_MNC_INDEX        1
#define GERAN_ANI_CELLID_INDEX     2
#define GERAN_ANI_LAC_INDEX        3

PUBLIC
OsNetworkConnection::OsNetworkConnection(IN IMS_SINT32 nSlotId) :
        ImsNetworkConnection(nSlotId),
        m_nIpcanCategory(IIpcan::CATEGORY_MOBILE),
        m_nState(STATE_IDLE),
        m_nDataState(DATA_DISCONNECTED),
        m_pPolicy(IMS_NULL),
        m_strApn(AString::ConstNull()),
        m_nIfaceId(IMS_NET_IFACE_INVALID_ID),
        m_strIfaceName(AString::ConstNull()),
        m_nPreferredIpVersion(IpAddress::UNKNOWN),
        m_objLocalAddress(IpAddress::NONE),
        m_objLocalAddressIpv4(IpAddress::NONE),
        m_objLocalAddressIpv6(IpAddress::IPv6NONE),
        m_objPcscfsAddress(AStringArray::ConstNull()),
        m_nConnectionHandle(0),
        m_piOwnerThread(IMS_NULL),
        m_piConnectionListener(IMS_NULL),
        m_objReferenceListeners(ImsList<INetworkConnectionListener*>())
{
    IMS_TRACE_D("ctor: OsNetworkConnection", 0, 0, 0);

    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_NETWORK, this, GetSlotId());
}

PUBLIC VIRTUAL OsNetworkConnection::~OsNetworkConnection()
{
    IMS_TRACE_D("dtor: OsNetworkConnection", 0, 0, 0);

    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_NETWORK, this, GetSlotId());

    if (m_pPolicy != IMS_NULL)
    {
        ImsNetworkConnectionState::GetInstance()->DetachHandle(m_pPolicy->GetName(), GetSlotId());

        delete m_pPolicy;
    }
}

PUBLIC VIRTUAL const IpAddress& OsNetworkConnection::GetLocalAddress(
        IN IMS_SINT32 nIpVersion /*= IpAddress::UNKNOWN configuration-based*/) const
{
    if (nIpVersion == IpAddress::UNKNOWN)
    {
        return m_objLocalAddress;
    }
    else if (nIpVersion == IpAddress::IPV4)
    {
        return m_objLocalAddressIpv4;
    }
    else if (nIpVersion == IpAddress::IPV6)
    {
        return m_objLocalAddressIpv6;
    }

    return IpAddress::NONE;
}

PRIVATE VIRTUAL INetworkConnection::RESULT_ENTYPE OsNetworkConnection::Activate(
        IN IMS_BOOL bEnableApn /*= IMS_FALSE*/)
{
    // If the connection with IMS APN is already established,
    // then it sends an event to the application.
    if (IsConnected())
    {
        CacheLocalAddress();

        IMS_TRACE_I("Mobile: Activate - APN (%s) is already activated; state=%d",
                GetProfileName().GetStr(), m_nDataState, 0);

        SetState(STATE_ACTIVE);

        return RESULT_DONE;
    }

    IMS_SINT32 nApnType = GetApnType();

    IMS_TRACE_D("Mobile: Activate - apnType=%d, state=%d", nApnType, m_nDataState, 0);

    if ((nApnType == NetworkPolicy::APN_EMERGENCY) ||
            (bEnableApn && (nApnType == NetworkPolicy::APN_IMS)) ||
            (bEnableApn && (nApnType == NetworkPolicy::APN_INTERNET)))
    {
        if (PlatformContext::GetInstance()->GetSystem()->RequestNetwork(nApnType, GetSlotId()) == 0)
        {
            IMS_TRACE_E(
                    0, "Enabling data connectivity(%s) failed", GetProfileName().GetStr(), 0, 0);

            SetState(STATE_IDLE);

            return RESULT_FAILED;
        }
    }

    SetState(STATE_ACTIVATING);

    return RESULT_DOING;
}

PRIVATE VIRTUAL INetworkConnection::RESULT_ENTYPE OsNetworkConnection::Deactivate(
        IN IMS_BOOL bDisableApn /*= IMS_FALSE*/)
{
    IMS_TRACE_D("Mobile: Deactivate - apnType=%d, state=%d", GetApnType(), m_nDataState, 0);

    if (!Release(bDisableApn))
    {
        SetState(STATE_TERMINATED);
        return RESULT_DONE;
    }

    if (m_nState == STATE_TERMINATED)
    {
        IMS_TRACE_D("Mobile: Deactivate in TERMINATED state", 0, 0, 0);
        return RESULT_DONE;
    }

    SetState(STATE_TERMINATING);

    return RESULT_DOING;
}

/**
 * Byte Array Define
 * 1 byte : total length
 * 1 byte : radio type - 0 : none, 1 : ehrpd, 2 : lte
 * 1 byte : 1st data length n
 * n byte : 1st data
 * 1 byte : 2nd data length l
 * l byte : 2nd data
 * ...
 */
PRIVATE VIRTUAL void OsNetworkConnection::GetAccessNetworkInfo(
        OUT AccessNetworkInfo& objAccessNetInfo)
{
    if (m_nIpcanCategory == IIpcan::CATEGORY_WLAN)
    {
        GetAccessNetworkInfoForWiFi(objAccessNetInfo);
        return;
    }

    IMS_SINT32 nDefaultNetworkType = RADIOTECH_TYPE_UNKNOWN;

    if (GetApnType() == NetworkPolicy::APN_EMERGENCY)
    {
        nDefaultNetworkType = RADIOTECH_TYPE_LTE;
    }

    IMS_SINT32 nNetworkType = nDefaultNetworkType;
    AStringArray objCellIdentities;
    PlatformContext::GetInstance()->GetSystem()->GetAccessNetworkInfo(
            nDefaultNetworkType, nNetworkType, objCellIdentities, GetSlotId());

    if (nNetworkType == RADIOTECH_TYPE_UNKNOWN)
    {
        nNetworkType = PlatformContext::GetInstance()->GetSystem()->GetNetworkType(GetSlotId());
        IMS_TRACE_I("PANI: network-type (%d)", nNetworkType, 0, 0);
    }

    objAccessNetInfo = CreateAccessNetworkInfo(nNetworkType, objCellIdentities);
}

PRIVATE VIRTUAL void OsNetworkConnection::GetLastAccessNetworkInfo(
        OUT AccessNetworkInfo& objAccessNetInfo, OUT AString& strTimestamp,
        OUT AString& strCellInfoAge)
{
    AStringArray objCellIdentities =
            PlatformContext::GetInstance()->GetSystem()->GetLastAccessNetworkInfo(
                    RADIOTECH_TYPE_UNKNOWN, GetSlotId());

    // 0 : network type
    // 1 : timestamp as UTC format
    // 2 : cell age as seconds format
    // 3..7 : same information in GetAccessNetworkInfo

    if (objCellIdentities.IsEmpty() || (objCellIdentities.GetCount() < 4))
    {
        strTimestamp = AString::ConstNull();
        strCellInfoAge = AString::ConstNull();
        return;
    }

    const AString& strNetworkType = objCellIdentities.GetElementAt(0);
    IMS_SINT32 nNetworkType = strNetworkType.ToInt32();

    strTimestamp = objCellIdentities.GetElementAt(1);
    strCellInfoAge = objCellIdentities.GetElementAt(2);

    // Removes the 1st and 2nd and 3rd element
    objCellIdentities.RemoveElementAt(0);
    objCellIdentities.RemoveElementAt(0);
    objCellIdentities.RemoveElementAt(0);

    objAccessNetInfo = CreateAccessNetworkInfo(nNetworkType, objCellIdentities);
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::GetExtraInfo(
        IN const AString& strType, OUT AString& strInfo)
{
    if (strType.Equals("rat"))
    {
        IMS_SINT32 nRadioType =
                PlatformContext::GetInstance()->GetSystem()->GetNetworkType(GetSlotId());

        if ((nRadioType == RADIOTECH_TYPE_LTE) || (nRadioType == RADIOTECH_TYPE_LTE_CA))
        {
            strInfo = "LTE";
        }
        else if (nRadioType == RADIOTECH_TYPE_NR)
        {
            strInfo = "NR";
        }
        else if ((nRadioType == RADIOTECH_TYPE_UMTS) || (nRadioType == RADIOTECH_TYPE_HSDPA) ||
                (nRadioType == RADIOTECH_TYPE_HSUPA) || (nRadioType == RADIOTECH_TYPE_HSPA) ||
                (nRadioType == RADIOTECH_TYPE_EHRPD) || (nRadioType == RADIOTECH_TYPE_HSPAP))
        {
            strInfo = "3G";
        }
        else if (nRadioType == RADIOTECH_TYPE_UNKNOWN)
        {
            strInfo = "Unknown";
        }
        else
        {
            strInfo = "2G";
        }
    }
    else if (strType.Equals("policy_name"))
    {
        if (m_pPolicy != IMS_NULL)
        {
            strInfo = m_pPolicy->GetName();
        }
    }
    else if (strType.Equals("apn"))
    {
        strInfo = m_strApn;
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_SINT32 OsNetworkConnection::GetHostByName(IN const AString& strHostName,
        OUT ImsList<IpAddress>& objIpAddrs,
        IN IMS_SINT32 nIpVersion /*= 0 default-local-address-based*/)
{
    IMS_TRACE_I("DNS lookup (%d): apnType=%d, domain=%s", nIpVersion, GetApnType(),
            OsUtil::GetInstance()->IsDebugMode() ? strHostName.GetStr() : "***");

    if ((nIpVersion != IpAddress::IPV4) && (nIpVersion != IpAddress::IPV6))
    {
        const IpAddress& objLocalIp = GetLocalAddress();

        if (objLocalIp.IsIPv4Address())
        {
            nIpVersion = IpAddress::IPV4;
        }
        else if (objLocalIp.IsIPv6Address())
        {
            nIpVersion = IpAddress::IPV6;
        }
        else
        {
            IMS_TRACE_E(0, "Address family is unspecified", 0, 0, 0);
            return (-1);
        }
    }

    AStringArray objHostIps = PlatformContext::GetInstance()->GetSystem()->GetHostByName(
            strHostName, nIpVersion, GetApnType(), GetSlotId());

    if (objHostIps.IsEmpty())
    {
        IMS_TRACE_D("No host address or DNS resolution failed", 0, 0, 0);
        return (-1);
    }

    for (IMS_SINT32 i = 0; i < objHostIps.GetCount(); i++)
    {
        const AString& strHostIp = objHostIps.GetElementAt(i);
        IpAddress objIp;

        if (objIp.Parse(strHostIp))
        {
            objIpAddrs.Append(objIp);
        }

        IMS_TRACE_D("gethostbyname: %s >>> %s at index(%d)",
                OsUtil::GetInstance()->IsDebugMode() ? strHostName.GetStr() : "***",
                OsUtil::GetInstance()->IsDebugMode() ? strHostIp.GetStr() : "***", i);
    }

    if (objIpAddrs.IsEmpty())
    {
        return (-1);
    }

    return 1;
}

PRIVATE VIRTUAL IMS_SINT32 OsNetworkConnection::GetIfaceId() const
{
    return m_nIfaceId;
}

PRIVATE VIRTUAL const AString& OsNetworkConnection::GetIfaceName() const
{
    return m_strIfaceName;
}

PRIVATE VIRTUAL const AStringArray& OsNetworkConnection::GetPcscfAddress(
        IN IMS_SINT32 nIpVersion /*= 0 (configuration-based)*/)
{
    m_objPcscfsAddress.RemoveAllElements();
    m_objPcscfsAddress = PlatformContext::GetInstance()->GetSystem()->GetPcscfAddresses(
            GetApnType(), nIpVersion, GetSlotId());

    return m_objPcscfsAddress;
}

PRIVATE VIRTUAL INetworkConnection::STATE_ENTYPE OsNetworkConnection::GetState() const
{
    return (m_nState == STATE_ACTIVE) ? STATE_CONNECTED : STATE_DISCONNECTED;
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::IsConnected(
        IN IMS_SINT32 nCategory /*= IIpcan::CATEGORY_ANY*/) const
{
    if (nCategory == m_nIpcanCategory || nCategory == IIpcan::CATEGORY_ANY)
    {
        return (m_nState == STATE_ACTIVE);
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::IsePDGEnabled() const
{
    return IsConnected(IIpcan::CATEGORY_WLAN);
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::IsIpv6Preferred() const
{
    return PlatformContext::GetInstance()->GetSystem()->IsIpv6Preferred(GetApnType(), GetSlotId());
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::IsMobileDataEnabled() const
{
    return PlatformContext::GetInstance()->GetSystem()->IsMobileDataEnabled(GetSlotId());
}

PRIVATE VIRTUAL IMS_SINT32 OsNetworkConnection::GetMtu() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetMtu(GetApnType(), GetSlotId());
}

PRIVATE VIRTUAL void OsNetworkConnection::SetListener(IN INetworkConnectionListener* piListener)
{
    m_piConnectionListener = piListener;
}

PRIVATE VIRTUAL void OsNetworkConnection::SetPreferredIpVersion(
        IN IMS_SINT32 nPreferredIpVersion /*= 0 default-aos-connection-profile*/)
{
    IMS_TRACE_D("Preferred IP version: %d >> %d", m_nPreferredIpVersion, nPreferredIpVersion, 0);

    if (m_nPreferredIpVersion != nPreferredIpVersion)
    {
        m_nPreferredIpVersion = nPreferredIpVersion;

        if (IsConnected() && (nPreferredIpVersion != IpAddress::UNKNOWN))
        {
            AdjustPreferredLocalAddress();
        }
    }
}

PRIVATE VIRTUAL void OsNetworkConnection::AddReferenceListener(
        IN INetworkConnectionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
    {
        const INetworkConnectionListener* piTmpListener = m_objReferenceListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    m_objReferenceListeners.Append(piListener);
}

PRIVATE VIRTUAL void OsNetworkConnection::RemoveReferenceListener(
        IN INetworkConnectionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
    {
        const INetworkConnectionListener* piTmpListener = m_objReferenceListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objReferenceListeners.RemoveAt(i);
            return;
        }
    }
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::Create(IN const AString& strNetProfile)
{
    const NetworkPolicy* pTmpPolicy =
            NetworkServicePolicy::GetInstance()->GetPolicy(strNetProfile, GetSlotId());

    if (pTmpPolicy == IMS_NULL)
    {
        IMS_TRACE_D("NetworkPolicy (%s) is not present; so, fallback to default APN",
                strNetProfile.GetStr(), 0, 0);

        pTmpPolicy = NetworkServicePolicy::GetInstance()->GetPolicy(
                NetworkPolicy::APN_NONE, GetSlotId());

        if (pTmpPolicy == IMS_NULL)
        {
            IMS_TRACE_E(0, "Default policy is not present", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    m_pPolicy = new NetworkPolicy(*pTmpPolicy);

    m_nConnectionHandle = AttachNetworkConnection();

    return (m_nConnectionHandle != 0) ? IMS_TRUE : IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::Create(IN IMS_SINT32 nApnType)
{
    const NetworkPolicy* pTmpPolicy =
            NetworkServicePolicy::GetInstance()->GetPolicy(nApnType, GetSlotId());

    if (pTmpPolicy == IMS_NULL)
    {
        IMS_TRACE_D(
                "NetworkPolicy (%d) is not present; so, fallback to default APN", nApnType, 0, 0);

        pTmpPolicy = NetworkServicePolicy::GetInstance()->GetPolicy(
                NetworkPolicy::APN_NONE, GetSlotId());

        if (pTmpPolicy == IMS_NULL)
        {
            IMS_TRACE_E(0, "Default policy is not present", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    m_pPolicy = new NetworkPolicy(*pTmpPolicy);

    m_nConnectionHandle = AttachNetworkConnection();

    return (m_nConnectionHandle != 0) ? IMS_TRUE : IMS_FALSE;
}

PRIVATE VIRTUAL void OsNetworkConnection::DispatchServiceMessage(
        IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    (void)nLparam;

    switch (nWparam)
    {
        case NET_CONNECTED:
            NotifyDataConnected(0);
            break;

        case NET_DISCONNECTED:
            NotifyDataDisconnected(0);
            break;

        case NET_CONNECT_FAILED:
            NotifyDataConnectionFailed(0);
            break;

        case NET_IP_CHANGED:
            NotifyIpChanged(0);
            break;

        case NET_IPCAN_CAT_CHANGED:
            NotifyIpcanCatChanged();
            break;

        case NET_PCSCF_CHANGED:
            NotifyPcscfChanged();
            break;

        default:
            break;
    }
}

PRIVATE VIRTUAL IMS_BOOL OsNetworkConnection::Equals(IN const IpAddress& objIpAddr) const
{
    if (objIpAddr.IsIPv4Address())
    {
        return objIpAddr.Equals(m_objLocalAddressIpv4);
    }
    else if (objIpAddr.IsIPv6Address())
    {
        return objIpAddr.Equals(m_objLocalAddressIpv6);
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_CONNECTION OsNetworkConnection::GetHandle() const
{
    return m_nConnectionHandle;
}

PRIVATE VIRTUAL const AString& OsNetworkConnection::GetProfileName() const
{
    return (m_pPolicy != IMS_NULL) ? m_pPolicy->GetName() : AString::ConstNull();
}

PRIVATE VIRTUAL IMS_SINT32 OsNetworkConnection::GetApnType() const
{
    return (m_pPolicy != IMS_NULL) ? m_pPolicy->GetApnType() : NetworkPolicy::APN_IMS;
}

PRIVATE VIRTUAL void OsNetworkConnection::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    IMS_TRACE_D("Mobile: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d, nEvent, nWParam, nLParam);

    // APN Type
    if (GetApnType() != LONG_TO_SINT(nWParam))
    {
#ifdef __IMS_DEBUG__
        IMS_TRACE_D("APN is different - apnType(%d), received apnType(%" PFLS_d ")", GetApnType(),
                nWParam, 0);
#endif
        return;
    }

    switch (nEvent)
    {
        case IMS_SYSTEM_DATACONNECTION_FAILED:
        {
            IMS_TRACE_I("Mobile: data connection state changed in %s - connection failed",
                    StateToString(m_nState), 0, 0);

            PostEvent(NET_CONNECT_FAILED);
            break;
        }
        case IMS_SYSTEM_DATACONNECTION_STATE_CHANGED:
        {
            IMS_TRACE_I("Mobile: data connection state changed in %s - %" PFLS_d,
                    StateToString(m_nState), nLParam, 0);

            if (nLParam == DATA_DISCONNECTED)
            {
                PostEvent(NET_DISCONNECTED);
            }
            else if (nLParam == DATA_CONNECTED)
            {
                PostEvent(NET_CONNECTED);
            }
            else if (nLParam == DATA_CONNECT_FAILED)
            {
                PostEvent(NET_CONNECT_FAILED);
            }
            else if (nLParam == DATA_IPCHANGED)
            {
                PostEvent(NET_IP_CHANGED);
            }
            else if (nLParam == DATA_PCSCF_CHANGED)
            {
                PostEvent(NET_PCSCF_CHANGED);
            }
            else
            {
                IMS_TRACE_E(0, "Unknown data state (%" PFLS_d ")", nLParam, 0, 0);
            }
            break;
        }
        case IMS_SYSTEM_DATACONNECTION_IPCAN_CHANGED:
        {
            if (nLParam == IIpcan::CATEGORY_WLAN)
            {
                SetIpcanCategory(IIpcan::CATEGORY_WLAN);
            }
            else
            {
                SetIpcanCategory(IIpcan::CATEGORY_MOBILE);
            }
            break;
        }
        default:
        {
            IMS_TRACE_D("Event(%d): not handled", nEvent, 0, 0);
            break;
        }
    }
}

PRIVATE
IMS_BOOL OsNetworkConnection::AdjustPreferredLocalAddress()
{
    if (m_nPreferredIpVersion == IpAddress::IPV4)
    {
        if (!m_objLocalAddressIpv4.Equals(IpAddress::NONE) &&
                !m_objLocalAddressIpv4.Equals(m_objLocalAddress))
        {
            IMS_TRACE_D("Preferred local address: %s >> %s", m_objLocalAddress.ToString().GetStr(),
                    m_objLocalAddressIpv4.ToString().GetStr(), 0);

            m_objLocalAddress = m_objLocalAddressIpv4;
        }
    }
    else if (m_nPreferredIpVersion == IpAddress::IPV6)
    {
        if (!m_objLocalAddressIpv6.Equals(IpAddress::IPv6NONE) &&
                !m_objLocalAddressIpv6.Equals(m_objLocalAddress))
        {
            IMS_TRACE_D("Preferred local address: %s >> %s", m_objLocalAddress.ToString().GetStr(),
                    m_objLocalAddressIpv6.ToString().GetStr(), 0);

            m_objLocalAddress = m_objLocalAddressIpv6;
        }
    }

    if (m_objLocalAddress.Equals(IpAddress::NONE) || m_objLocalAddress.Equals(IpAddress::IPv6NONE))
    {
        IMS_TRACE_D("Local address is null", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_CONNECTION OsNetworkConnection::AttachNetworkConnection()
{
    IMS_CONNECTION nHandle = ImsNetworkConnectionState::GetInstance()->GetAndIncrementHandle();

    if (nHandle > 0)
    {
        ImsNetworkConnectionState::GetInstance()->AttachHandle(this);
        return nHandle;
    }

    IMS_TRACE_E(0, "Attaching NetConnection failed", 0, 0, 0);

    return static_cast<IMS_CONNECTION>(0);
}

PUBLIC
IMS_BOOL OsNetworkConnection::CacheLocalAddress()
{
    // IPCAN category
    m_nIpcanCategory = PlatformContext::GetInstance()->GetSystem()->GetIpcanCategory(
            GetApnType(), GetSlotId());

    // nIPVersion with -1 is for caching ip address
    // and selecting ip version based on configuration in java side
    AString strIpAddr = PlatformContext::GetInstance()->GetSystem()->GetLocalAddress(
            GetApnType(), -1, GetSlotId());

    if (!m_objLocalAddress.Parse(strIpAddr))
    {
        m_objLocalAddress = IpAddress::NONE;

        if (m_nPreferredIpVersion == IpAddress::UNKNOWN)
        {
            IMS_TRACE_E(0, "Local Address is null", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        IMS_TRACE_D("Local IP address: APN (%s), IP (%s)", GetProfileName().GetStr(),
                m_objLocalAddress.ToCharString(), 0);
    }

    // IPv4 address
    strIpAddr = PlatformContext::GetInstance()->GetSystem()->GetLocalAddress(
            GetApnType(), IpAddress::IPV4, GetSlotId());

    if (!m_objLocalAddressIpv4.Parse(strIpAddr))
    {
        m_objLocalAddressIpv4 = IpAddress::NONE;
    }

    // IPv6 address
    strIpAddr = PlatformContext::GetInstance()->GetSystem()->GetLocalAddress(
            GetApnType(), IpAddress::IPV6, GetSlotId());

    if (!m_objLocalAddressIpv6.Parse(strIpAddr))
    {
        m_objLocalAddressIpv6 = IpAddress::IPv6NONE;
    }

    if (!AdjustPreferredLocalAddress())
    {
        IMS_TRACE_E(0, "Local Address is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // APN name
    m_strApn = PlatformContext::GetInstance()->GetSystem()->GetApnName(GetApnType(), GetSlotId());

    // Network interface identifier
    m_nIfaceId = PlatformContext::GetInstance()->GetSystem()->GetIfaceId(GetApnType(), GetSlotId());

    // Network interface name
    m_strIfaceName =
            PlatformContext::GetInstance()->GetSystem()->GetIfaceName(GetApnType(), GetSlotId());

    AString strLog;
    strLog.Sprintf("Mobile: ipv4=%s, ipv6=%s, apn=%s, if-id=%d, if-name=%s, ipcan=%d",
            m_objLocalAddressIpv4.ToString().GetStr(), m_objLocalAddressIpv6.ToString().GetStr(),
            m_strApn.GetStr(), m_nIfaceId, m_strIfaceName.GetStr(), m_nIpcanCategory);
    IMS_TRACE_D("%s", strLog.GetStr(), 0, 0);

    return IMS_TRUE;
}

PRIVATE
void OsNetworkConnection::CallReferenceListeners(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nErrorCode /*= 0*/)
{
    switch (nEvent)
    {
        case NET_CONNECTED:
        {
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnConnected(this);
                }
            }
            break;
        }
        case NET_DISCONNECTED:
        {
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnDisconnected(this, nErrorCode);
                }
            }
            break;
        }
        case NET_CONNECT_FAILED:
        {
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnConnectionFailed(this, nErrorCode);
                }
            }
            break;
        }
        case NET_IP_CHANGED:
        {
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnIpChanged(this);
                }
            }
            break;
        }
        case NET_IPCAN_CAT_CHANGED:
        {
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnIpcanChanged(this);
                }
            }
            break;
        }
        case NET_PCSCF_CHANGED:
        {
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnPcscfChanged(this);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

PRIVATE
void OsNetworkConnection::ClearOnDataDisconnected()
{
    // Clear Local addresses
    m_objLocalAddress = IpAddress::NONE;
    m_objLocalAddressIpv4 = IpAddress::NONE;
    m_objLocalAddressIpv6 = IpAddress::IPv6NONE;

    // Clear the previous P-CSCF addresses
    m_objPcscfsAddress.RemoveAllElements();

    // Release all sockets
    ImsSocketState::GetInstance()->DetachAll(m_nConnectionHandle);
}

/**
 * @see #DATA_DISCONNECTED = 0
 * @see #DATA_CONNECTING
 * @see #DATA_CONNECTED
 * @see #DATA_SUSPENDED
 */
PRIVATE
IMS_SINT32 OsNetworkConnection::GetDataState()
{
    m_nDataState = PlatformContext::GetInstance()->GetSystem()->GetDataConnectionState(
            GetApnType(), GetSlotId());

    return m_nDataState;
}

PRIVATE
void OsNetworkConnection::GetAccessNetworkInfoForWiFi(OUT AccessNetworkInfo& objAccessNetInfo)
{
    AString strMacAddress = PlatformContext::GetInstance()->GetSystem()->GetWifiBssId();
    // Fall back to a default MAC address because, in most cases, a valid one is not set
    // due to privacy concerns.
    // - specific case: Cross SIM dialing (no Wi-Fi, other SIM's data connection)
    if (strMacAddress.GetLength() == 0)
    {
        IMS_TRACE_D("Fall back to a default MAC address", 0, 0, 0);
        strMacAddress = WLAN_NULL_MAC;
    }
    ImsList<AString> objTokens = strMacAddress.Split(':');

    if (objTokens.GetSize() == ANI_WLAN_MAX_MAC)
    {
        for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
        {
            const AString& strByte = objTokens.GetAt(i);
            IMS_BOOL bOk = IMS_FALSE;
            IMS_UINT16 nByte = strByte.ToUInt16(&bOk, 16);

            if (bOk)
            {
                objAccessNetInfo.uniAI.i_wlan_node_id.aMAC[i] =
                        static_cast<IMS_UINT8>(0xFF & nByte);
            }
            else
            {
                IMS_TRACE_E(0, "Invalid MAC address value (%s, %d)", strByte.GetStr(), i, 0);
                objAccessNetInfo.uniAI.i_wlan_node_id.aMAC[i] = 0xFF;
            }
        }

        objAccessNetInfo.nType = AccessNetworkInfo::TYPE_IEEE_802_11;
        objAccessNetInfo.nClass = AccessNetworkInfo::CLASS_NONE;
    }
}

PRIVATE
IMS_BOOL OsNetworkConnection::IsConnected()
{
    return (GetDataState() == DATA_CONNECTED);
}

PRIVATE
void OsNetworkConnection::NotifyDataConnected(IN IMS_SINT32 nErrorCode)
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_I("Mobile: Data connection is already activated", 0, 0, 0);

        if (m_objLocalAddress.IsNoneAddress())
        {
            // Even though the data connection is already in active,
            // it had been failed to get the local IP address.
            // In this situation, if the data connection state is in active,
            // and the local address is renewed, it SHOULD be notified to the application.
            NotifyIpChanged(nErrorCode);
        }
        return;
    }

    SetState(STATE_ACTIVE);

    if (!CacheLocalAddress())
    {
        IMS_TRACE_E(0, "Caching the local IP address failed", 0, 0, 0);
    }

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnConnected(this);
    }

    CallReferenceListeners(NET_CONNECTED, nErrorCode);
}

PRIVATE
void OsNetworkConnection::NotifyDataDisconnected(IN IMS_SINT32 nErrorCode)
{
    if (m_nState == STATE_TERMINATING)
    {
        SetState(STATE_TERMINATED);
    }
    else
    {
        SetState(STATE_IDLE);
    }

    ClearOnDataDisconnected();

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnDisconnected(this, nErrorCode);
    }

    CallReferenceListeners(NET_DISCONNECTED, nErrorCode);
}

PRIVATE
void OsNetworkConnection::NotifyDataConnectionFailed(IN IMS_SINT32 nErrorCode)
{
    if (m_nState == STATE_TERMINATING)
    {
        SetState(STATE_TERMINATED);
    }
    else
    {
        SetState(STATE_IDLE);
    }

    ClearOnDataDisconnected();

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnConnectionFailed(this, nErrorCode);
    }

    CallReferenceListeners(NET_CONNECT_FAILED, nErrorCode);
}

PRIVATE
void OsNetworkConnection::NotifyIpChanged(IN IMS_SINT32 nErrorCode)
{
    if (m_nState != STATE_ACTIVE)
    {
        // Ignore the event
        IMS_TRACE_D("Mobile: IP changed - not ACTIVE", 0, 0, 0);
        return;
    }

    if (HandleEmergencyPdnOnIpChanged(nErrorCode))
    {
        // IP-changed event is already handled by above method.
        return;
    }

    ClearOnDataDisconnected();

    if (!CacheLocalAddress())
    {
        IMS_TRACE_E(0, "Caching the local IP address failed", 0, 0, 0);
    }

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnIpChanged(this);
    }

    CallReferenceListeners(NET_IP_CHANGED, nErrorCode);
}

PRIVATE
void OsNetworkConnection::NotifyIpcanCatChanged()
{
    if (m_nState != STATE_ACTIVE)
    {
        // Ignore the event
        IMS_TRACE_D("Mobile: IPCAN category changed - not ACTIVE", 0, 0, 0);
        return;
    }

    m_strIfaceName =
            PlatformContext::GetInstance()->GetSystem()->GetIfaceName(GetApnType(), GetSlotId());

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnIpcanChanged(this);
    }

    CallReferenceListeners(NET_IPCAN_CAT_CHANGED);
}

PRIVATE
void OsNetworkConnection::NotifyPcscfChanged()
{
    if (m_nState != STATE_ACTIVE)
    {
        // Ignore the event
        IMS_TRACE_D("Mobile: P-CSCF changed - not ACTIVE", 0, 0, 0);
        return;
    }

    CacheLocalAddress();

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnPcscfChanged(this);
    }

    CallReferenceListeners(NET_PCSCF_CHANGED);
}

PRIVATE
IMS_BOOL OsNetworkConnection::HandleEmergencyPdnOnIpChanged(IN IMS_SINT32 nErrorCode)
{
    if (GetApnType() != NetworkPolicy::APN_EMERGENCY)
    {
        return IMS_FALSE;
    }

    if (m_objLocalAddress.IsNoneAddress())
    {
        return IMS_FALSE;
    }

    // IPv4 address
    AString strIpAddr = PlatformContext::GetInstance()->GetSystem()->GetLocalAddress(
            GetApnType(), IpAddress::IPV4, GetSlotId());
    if (!m_objLocalAddressIpv4.Parse(strIpAddr))
    {
        m_objLocalAddressIpv4 = IpAddress::NONE;
    }

    // IPv6 address
    strIpAddr = PlatformContext::GetInstance()->GetSystem()->GetLocalAddress(
            GetApnType(), IpAddress::IPV6, GetSlotId());
    if (!m_objLocalAddressIpv6.Parse(strIpAddr))
    {
        m_objLocalAddressIpv6 = IpAddress::IPv6NONE;
    }

    // Default IP address based on configuration
    strIpAddr = PlatformContext::GetInstance()->GetSystem()->GetLocalAddress(
            GetApnType(), -1, GetSlotId());
    if (!m_objLocalAddress.Parse(strIpAddr))
    {
        m_objLocalAddress = IpAddress::NONE;
    }

    AdjustPreferredLocalAddress();

    IMS_TRACE_D("EmergencyPdnOnIpChanged: preferred=%s, ipv4=%s, ipv6=%s",
            m_objLocalAddress.ToString().GetStr(), m_objLocalAddressIpv4.ToString().GetStr(),
            m_objLocalAddressIpv6.ToString().GetStr());

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnIpChanged(this);
    }

    CallReferenceListeners(NET_IP_CHANGED, nErrorCode);

    return IMS_TRUE;
}

PRIVATE
void OsNetworkConnection::PostEvent(IN IMS_UINT32 nEvent)
{
    if (m_piOwnerThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "No owner thread", 0, 0, 0);
        return;
    }

    switch (nEvent)
    {
        case NET_CONNECTED:          // FALL-THROUGH
        case NET_IP_CHANGED:         // FALL-THROUGH
        case NET_DISCONNECTED:       // FALL-THROUGH
        case NET_CONNECT_FAILED:     // FALL-THROUGH
        case NET_IPCAN_CAT_CHANGED:  // FALL-THROUGH
        case NET_PCSCF_CHANGED:
        {
            ImsMessage objMsg(
                    IMS_MSG_NETWORK, nEvent, static_cast<IMS_UINT32>(m_nConnectionHandle));
            m_piOwnerThread->PostMessageI(objMsg);
            break;
        }
        default:
        {
            break;
        }
    }
}

PRIVATE
IMS_BOOL OsNetworkConnection::Release(IN IMS_BOOL bDisableApn /*= IMS_FALSE*/)
{
    if (bDisableApn)
    {
        IMS_TRACE_D("APN (%s) will be explicitly disabled by the application",
                GetProfileName().GetStr(), 0, 0);

        if (PlatformContext::GetInstance()->GetSystem()->ReleaseNetwork(
                    GetApnType(), GetSlotId()) == 0)
        {
            IMS_TRACE_E(0, "Disable data connectivity(%s) failed", GetProfileName().GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
void OsNetworkConnection::SetIpcanCategory(IN IMS_SINT32 nCategory)
{
    if (m_nIpcanCategory != nCategory)
    {
        IMS_TRACE_I("Mobile: IPCAN-category - %d >> %d in %s", m_nIpcanCategory, nCategory,
                StateToString(m_nState));

        m_nIpcanCategory = nCategory;

        PostEvent(NET_IPCAN_CAT_CHANGED);
    }
}

PRIVATE
void OsNetworkConnection::SetState(IN IMS_UINT32 nState)
{
    if (m_nState != nState)
    {
        IMS_TRACE_I("Mobile: connection(%s) - %s >> %s", GetProfileName().GetStr(),
                StateToString(m_nState), StateToString(nState));

        m_nState = nState;
    }
}

PRIVATE GLOBAL AccessNetworkInfo OsNetworkConnection::CreateAccessNetworkInfo(
        IN IMS_SINT32 nNetworkType, IN const AStringArray& objCellIdentities)
{
    AccessNetworkInfo objAni;
    IMS_BOOL bIsNetworkTypeLte =
            ((nNetworkType == RADIOTECH_TYPE_LTE) || (nNetworkType == RADIOTECH_TYPE_LTE_CA));

    if (bIsNetworkTypeLte || (nNetworkType == RADIOTECH_TYPE_UMTS) ||
            (nNetworkType == RADIOTECH_TYPE_HSDPA) || (nNetworkType == RADIOTECH_TYPE_HSUPA) ||
            (nNetworkType == RADIOTECH_TYPE_HSPA) || (nNetworkType == RADIOTECH_TYPE_HSPAP))
    {
        if (bIsNetworkTypeLte)
        {
            objAni.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD;
        }
        else
        {
            objAni.nType = AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD;
        }

        // MCC(3);MNC(2 or 3);CellID(7);TAC(4)
        // 450;06F;1177a00;00d1

        if (objCellIdentities.GetCount() < UTRAN_ANI_ITEM_SIZE)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // MCC / MNC / TAC (Tracking Area Code) / Cell Identity
        const AString& strMcc = objCellIdentities.GetElementAt(UTRAN_ANI_MCC_INDEX);
        const AString& strMnc = objCellIdentities.GetElementAt(UTRAN_ANI_MNC_INDEX);
        const AString& strTac = objCellIdentities.GetElementAt(UTRAN_ANI_TAC_INDEX);
        const AString& strCellId = objCellIdentities.GetElementAt(UTRAN_ANI_CELLID_INDEX);

        // LTE frequency mode: FDD / TDD
        if (bIsNetworkTypeLte && (objCellIdentities.GetCount() > UTRAN_ANI_ITEM_SIZE))
        {
            const AString& strLteMode = objCellIdentities.GetElementAt(UTRAN_ANI_MODE_INDEX);

            if (strLteMode.EqualsIgnoreCase(ANI_MODE_TDD))
            {
                objAni.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD;
            }
            else if (strLteMode.EqualsIgnoreCase(ANI_MODE_FDD))
            {
                objAni.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD;
            }
        }

        if ((strMcc.GetLength() > ANI_3GPP_MCC_MAX_LEN) ||
                (strMnc.GetLength() > ANI_3GPP_MNC_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (MCC=%d, MNC=%d)", strMcc.GetLength(),
                    strMnc.GetLength(), 0);
            return objAni;
        }

        if ((strTac.GetLength() > ANI_3GPP_TAC_MAX_LEN) ||
                (strCellId.GetLength() > ANI_3GPP_UTRAN_CELL_ID_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (TAC=%d, CellIdentity=%d)", strTac.GetLength(),
                    strCellId.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        // MCC (3 digits)
        for (IMS_SINT32 i = 0; i < strMcc.GetLength(); ++i)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMcc[i];
            nIndex++;
        }

        // MNC (2 or 3 digits)
        if (strMnc.GetLength() == ANI_3GPP_MNC_MAX_LEN)
        {
            if ((strMnc[2] == 'F') || (strMnc[2] == 'f') || (strMnc[2] == 0xFF))
            {
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
            }
            else
            {
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[2];
                nIndex++;
            }
        }
        else
        {
            for (IMS_SINT32 i = 0; i < strMnc.GetLength(); ++i)
            {
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[i];
                nIndex++;
            }
        }

        IMS_SINT32 nZeroPadding;

        // TAC (Tracking Area Code, 16bits, 4hex)
        nZeroPadding = ANI_3GPP_TAC_MAX_LEN - strTac.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strTac.GetLength(); ++i)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strTac[i];
            nIndex++;
        }

        // Cell Identity (28bits, 7hex)
        nZeroPadding = ANI_3GPP_UTRAN_CELL_ID_MAX_LEN - strCellId.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strCellId.GetLength(); ++i)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strCellId[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI: %s (%s, %d)", bIsNetworkTypeLte ? "LTE" : "3G",
                OsUtil::GetInstance()->IsDebugMode()
                        ? objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID
                        : "***",
                nIndex);
    }
    else if (nNetworkType == RADIOTECH_TYPE_NR)
    {
        objAni.nType = AccessNetworkInfo::TYPE_3GPP_NR_FDD;

        // MCC(3);MNC(2 or 3);CellID(9);TAC(6)
        // 450;06F;1177a0000;0000d1

        if (objCellIdentities.GetCount() < UTRAN_ANI_ITEM_SIZE)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // MCC / MNC / TAC (Tracking Area Code) / Cell Identity
        const AString& strMcc = objCellIdentities.GetElementAt(UTRAN_ANI_MCC_INDEX);
        const AString& strMnc = objCellIdentities.GetElementAt(UTRAN_ANI_MNC_INDEX);
        const AString& strTac = objCellIdentities.GetElementAt(UTRAN_ANI_TAC_INDEX);
        const AString& strCellId = objCellIdentities.GetElementAt(UTRAN_ANI_CELLID_INDEX);

        // Mode: FDD / TDD
        if (objCellIdentities.GetCount() > UTRAN_ANI_ITEM_SIZE)
        {
            const AString& strMode = objCellIdentities.GetElementAt(UTRAN_ANI_MODE_INDEX);

            if (strMode.EqualsIgnoreCase(ANI_MODE_TDD))
            {
                objAni.nType = AccessNetworkInfo::TYPE_3GPP_NR_TDD;
            }
        }

        if ((strMcc.GetLength() > ANI_3GPP_MCC_MAX_LEN) ||
                (strMnc.GetLength() > ANI_3GPP_MNC_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (MCC=%d, MNC=%d)", strMcc.GetLength(),
                    strMnc.GetLength(), 0);
            return objAni;
        }

        if ((strTac.GetLength() > ANI_3GPP_NR_TAC_MAX_LEN) ||
                (strCellId.GetLength() > ANI_3GPP_NR_CELL_ID_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (TAC=%d, CellIdentity=%d)", strTac.GetLength(),
                    strCellId.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        // MCC (3 digits)
        for (IMS_SINT32 i = 0; i < strMcc.GetLength(); ++i)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMcc[i];
            nIndex++;
        }

        // MNC (2 or 3 digits)
        if (strMnc.GetLength() == ANI_3GPP_MNC_MAX_LEN)
        {
            if ((strMnc[2] == 'F') || (strMnc[2] == 'f') || (strMnc[2] == 0xFF))
            {
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
            }
            else
            {
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[2];
                nIndex++;
            }
        }
        else
        {
            for (IMS_SINT32 i = 0; i < strMnc.GetLength(); ++i)
            {
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[i];
                nIndex++;
            }
        }

        IMS_SINT32 nZeroPadding;

        // TAC (Tracking Area Code, 24bits, 6hex)
        nZeroPadding = ANI_3GPP_NR_TAC_MAX_LEN - strTac.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strTac.GetLength(); ++i)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strTac[i];
            nIndex++;
        }

        // Cell Identity (36bits, 9hex)
        nZeroPadding = ANI_3GPP_NR_CELL_ID_MAX_LEN - strCellId.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strCellId.GetLength(); ++i)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strCellId[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI: NR (%s, %d)",
                OsUtil::GetInstance()->IsDebugMode()
                        ? objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID
                        : "***",
                nIndex, 0);
    }
    else if ((nNetworkType == RADIOTECH_TYPE_GPRS) || (nNetworkType == RADIOTECH_TYPE_EDGE))
    {
        objAni.nType = AccessNetworkInfo::TYPE_3GPP_GERAN;

        // MCC(3);MNC(2 or 3);CellID(4);LAC(4)
        if (objCellIdentities.GetCount() < GERAN_ANI_ITEM_SIZE)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // MCC / MNC / LAC (Location Area Code) / Cell Identity
        const AString& strMcc = objCellIdentities.GetElementAt(GERAN_ANI_MCC_INDEX);
        const AString& strMnc = objCellIdentities.GetElementAt(GERAN_ANI_MNC_INDEX);
        const AString& strLac = objCellIdentities.GetElementAt(GERAN_ANI_LAC_INDEX);
        const AString& strCellId = objCellIdentities.GetElementAt(GERAN_ANI_CELLID_INDEX);

        if ((strMcc.GetLength() > ANI_3GPP_MCC_MAX_LEN) ||
                (strMnc.GetLength() > ANI_3GPP_MNC_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (MCC=%d, MNC=%d)", strMcc.GetLength(),
                    strMnc.GetLength(), 0);
            return objAni;
        }

        if ((strLac.GetLength() > ANI_3GPP_LAC_MAX_LEN) ||
                (strCellId.GetLength() > ANI_3GPP_GERAN_CELL_ID_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (LAC=%d, CellIdentity=%d)", strLac.GetLength(),
                    strCellId.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        for (IMS_SINT32 i = 0; i < strMcc.GetLength(); ++i)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMcc[i];
            nIndex++;
        }

        // MNC (2 or 3 digits)
        if (strMnc.GetLength() == ANI_3GPP_MNC_MAX_LEN)
        {
            if ((strMnc[2] == 'F') || (strMnc[2] == 'f') || (strMnc[2] == 0xFF))
            {
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[1];
                nIndex++;
            }
            else
            {
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[1];
                nIndex++;
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[2];
                nIndex++;
            }
        }
        else
        {
            for (IMS_SINT32 i = 0; i < strMnc.GetLength(); ++i)
            {
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[i];
                nIndex++;
            }
        }

        // LAC (Location Area Code, 16bits, 4hex)
        IMS_SINT32 nZeroPadding = ANI_3GPP_LAC_MAX_LEN - strLac.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strLac.GetLength(); ++i)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strLac[i];
            nIndex++;
        }

        // Cell Identity (16bits, 4hex)
        nZeroPadding = ANI_3GPP_GERAN_CELL_ID_MAX_LEN - strCellId.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strCellId.GetLength(); ++i)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strCellId[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI: GERAN (%s, %d)",
                OsUtil::GetInstance()->IsDebugMode() ? objAni.uniAI.cgi_3gpp.acCGI : "***", nIndex,
                0);
    }
    else if (nNetworkType == RADIOTECH_TYPE_EHRPD)
    {
        objAni.nType = AccessNetworkInfo::TYPE_3GPP2_1X_HRPD;

        // Sector ID (32);SunetLength(2)
        // 8e0a0a0a0a0a0a0a0a0a0a0a0a022382;68
        // ril.ehrpd.netinfo
        if (objCellIdentities.GetCount() < EHRPD_ANI_ITEM_SIZE)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // Sector Id / Subnet Length
        const AString& strSector = objCellIdentities.GetElementAt(EHRPD_ANI_SECTOR_ID_INDEX);
        const AString& strSubnet = objCellIdentities.GetElementAt(EHRPD_ANI_SUBNET_LEN_INDEX);

        if ((strSector.GetLength() > ANI_3GPP2_SECTOR_ID_MAX_LEN) ||
                (strSubnet.GetLength() > ANI_3GPP2_SUBNET_LENGTH_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI: Invalid length (sectorId=%d, subnetLength=%d)",
                    strSector.GetLength(), strSubnet.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        // Sector Id (128bits, 32hex)
        IMS_SINT32 nZeroPadding = ANI_3GPP2_SECTOR_ID_MAX_LEN - strSector.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strSector.GetLength(); ++i)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = strSector[i];
            nIndex++;
        }

        // Subnet Length (8bits, 2hex)
        nZeroPadding = ANI_3GPP2_SUBNET_LENGTH_MAX_LEN - strSubnet.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strSubnet.GetLength(); ++i)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = strSubnet[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI: eHRPD (%s, %d)",
                OsUtil::GetInstance()->IsDebugMode() ? objAni.uniAI.ci_3gpp2.acCI : "***", nIndex,
                0);
    }
    else
    {
        IMS_TRACE_E(0, "Invalid network type (%d)", nNetworkType, 0, 0);
    }

    return objAni;
}

PRIVATE GLOBAL const IMS_CHAR* OsNetworkConnection::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_IDLE:
            return "STATE_IDLE";
        case STATE_PENDING:
            return "STATE_PENDING";
        case STATE_ACTIVATING:
            return "STATE_ACTIVATING";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        case STATE_TERMINATING:
            return "STATE_TERMINATING";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
