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
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "ImsMessageDef.h"
#include "ImsAccessNetworkInfoType.h"
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "IThread.h"
#include "OsUtil.h"
#include "ServiceEvent.h"
#include "ServiceMemory.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "network/OsWifiConnection.h"
#include "system-intf/System.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_ADAPT__;

#define WLAN_DESCRIPT "wlan"
#define WLAN_DESCRIPT_SIZE 4
#define MAX_INTERFACE_REQ 30
#define MAX_IOCTL_LOOKUP 30

PUBLIC
OsWifiConnection::OsWifiConnection() :
        ImsNetworkConnection(IMS_SLOT_0),
        m_nState(STATE_IDLE),
        m_nWifiState(0),
        m_nWifiDetailedState(0),
        m_pPolicy(IMS_NULL),
        m_nIfaceId(IMS_NET_IFACE_INVALID_ID),
        m_strIfaceName(AString::ConstNull()),
        m_nPreferredIpVersion(IPAddress::UNKNOWN),
        m_objLocalAddress(IPAddress::NONE),
        m_objLocalAddressIpv4(IPAddress::NONE),
        m_objLocalAddressIpv6(IPAddress::IPv6NONE),
        m_objPcscfsAddress(AStringArray::ConstNull()),
        m_nConnectionHandle(0),
        m_piOwnerThread(IMS_NULL),
        m_piConnectionListener(IMS_NULL),
        m_objReferenceListeners(IMSList<INetworkConnectionListener*>())
{
    IMS_TRACE_D("Constructor :: WiFi connection", 0, 0, 0);

    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

    System::GetInstance()->AddListener(SystemConstants::CATEGORY_WIFI, this, GetSlotId());
}

PUBLIC VIRTUAL OsWifiConnection::~OsWifiConnection()
{
    IMS_TRACE_D("Destructor :: WiFi connection", 0, 0, 0);

    System::GetInstance()->RemoveListener(SystemConstants::CATEGORY_WIFI, this, GetSlotId());

    if (m_pPolicy != IMS_NULL)
    {
        ImsNetworkConnectionState::GetInstance()->DetachHandle(m_pPolicy->GetName(), GetSlotId());

        delete m_pPolicy;
    }
}

PUBLIC VIRTUAL const IPAddress& OsWifiConnection::GetLocalAddress(
        IN IMS_SINT32 nIpVersion /*= 0 configuration-based*/) const
{
    if (nIpVersion == 0)
    {
        return m_objLocalAddress;
    }
    else if (nIpVersion == IPAddress::IPV4)
    {
        return m_objLocalAddressIpv4;
    }
    else if (nIpVersion == IPAddress::IPV6)
    {
        return m_objLocalAddressIpv6;
    }

    return IPAddress::NONE;
}

PRIVATE VIRTUAL INetworkConnection::RESULT_ENTYPE OsWifiConnection::Activate(
        IN IMS_BOOL /*bEnableApn = IMS_FALSE*/,
        IN IMS_SINT32 nIpcanCategory /*= IIpcan::CATEGORY_MOBILE*/)
{
    (void)nIpcanCategory;

    // If the connection with WiFi network is already established,
    // then it sends an event to the application.
    if (IsConnectedInternal())
    {
        if (!IsConnected())
        {
            m_objLocalAddress = IPAddress::NONE;
            m_objLocalAddressIpv4 = IPAddress::NONE;
            m_objLocalAddressIpv6 = IPAddress::IPv6NONE;
        }

        if (!CacheLocalAddress())
        {
            if (m_nWifiDetailedState == WIFI_NET_DETAILED_STATE_CONNECTED)
            {
                // Try to cache the local address one more.
                // If it's also failed, it means the permanent failure.
                if (!CacheLocalAddress())
                {
                    // Fatal error
                    IMS_TRACE_E(0, "Caching the local IP address failed", 0, 0, 0);
                }
            }
            else if (m_nWifiDetailedState == WIFI_NET_DETAILED_STATE_CAPTIVE_PORTAL_CHECK)
            {
                if (!CacheLocalAddress())
                {
                    IMS_TRACE_D("WiFi :: Waits for CONNECTED on CAPTIVE_PORTAL_CHECK", 0, 0, 0);
                    SetState(STATE_ACTIVATING);
                    return RESULT_DOING;
                }
            }
            else
            {
                IMS_TRACE_D("WiFi :: Waits for CONNECTED on OBTAINING_IPADDR", 0, 0, 0);
                SetState(STATE_ACTIVATING);
                return RESULT_DOING;
            }
        }

        IMS_TRACE_I("WiFi :: Activate() - APN (%s) is already activated; "
                    "state=%d, detailed_state=%d",
                GetProfileName().GetStr(), m_nWifiState, m_nWifiDetailedState);

        SetState(STATE_ACTIVE);

        return RESULT_DONE;
    }

    IMS_TRACE_D("WiFi :: Activate() - state=%d, detailed_state=%d", m_nWifiState,
            m_nWifiDetailedState, 0);

    SetState(STATE_ACTIVATING);

    return RESULT_DOING;
}

PRIVATE VIRTUAL INetworkConnection::RESULT_ENTYPE OsWifiConnection::Deactivate(
        IN IMS_BOOL /*bDisableApn = IMS_FALSE*/,
        IN IMS_SINT32 nIpcanCategory /*= IIpcan::CATEGORY_MOBILE*/)
{
    (void)nIpcanCategory;

    if (m_nState == STATE_TERMINATED)
    {
        IMS_TRACE_D("WiFi :: Deactivate() in TERMINATED state", 0, 0, 0);
        return RESULT_DONE;
    }

    if (IsDisconnected())
    {
        IMS_TRACE_D("WiFi :: Deactivate() in DATA_DISCONNECTED state", 0, 0, 0);

        SetState(STATE_TERMINATED);
        return RESULT_DONE;
    }

    IMS_TRACE_D("WiFi :: Deactivate() - state=%d, detailed_state=%d", m_nWifiState,
            m_nWifiDetailedState, 0);

    SetState(STATE_TERMINATED);

    return RESULT_DONE;
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
PRIVATE VIRTUAL void OsWifiConnection::GetAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo)
{
    AString strMacAddress = System::GetInstance()->GetWifiBssId();
    IMSList<AString> objTokens = strMacAddress.Split(':');

    if (objTokens.GetSize() == ANI_WLAN_MAX_MAC)
    {
        for (IMS_UINT32 i = 0; i < objTokens.GetSize(); i++)
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

        objAccessNetInfo.nType = AccessNetworkInfo::TYPE_NONE;
        objAccessNetInfo.nClass = AccessNetworkInfo::CLASS_3GPP_WLAN;
    }
    else
    {
        objAccessNetInfo.nType = AccessNetworkInfo::TYPE_NONE;
        objAccessNetInfo.nClass = AccessNetworkInfo::CLASS_NONE;
    }
}

PRIVATE VIRTUAL void OsWifiConnection::GetLastAccessNetworkInfo(
        OUT AccessNetworkInfo& objAccessNetInfo, OUT AString& strTimeStamp,
        OUT AString& strCellInfoAge)
{
    // no-op
    objAccessNetInfo.nType = AccessNetworkInfo::TYPE_IEEE_802_11;
    objAccessNetInfo.nClass = AccessNetworkInfo::CLASS_NONE;

    strTimeStamp = AString::ConstNull();
    strCellInfoAge = AString::ConstNull();
}

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::GetExtraInfo(
        IN const AString& strType, OUT AString& strInfo)
{
    if (strType.Equals("mac_address"))
    {
        strInfo = System::GetInstance()->GetWifiBssId();

        if (strInfo.GetLength() == 0)
        {
            strInfo = "00:00:00:00:00:00";
        }

        IMS_TRACE_D("MAC ADDRESS (%s)", strInfo.GetStr(), 0, 0);
    }
    else if (strType.Equals("ssid"))
    {
        strInfo = System::GetInstance()->GetWifiSsId();

        IMS_TRACE_D("SSID (%s)", strInfo.GetStr(), 0, 0);
    }
    else if (strType.Equals("rat"))
    {
        strInfo = "WiFi";
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
        strInfo = AString::ConstEmpty();
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_SINT32 OsWifiConnection::GetHostByName(IN const AString& strHostName,
        OUT IMSList<IPAddress>& objIpAddrs,
        IN IMS_SINT32 nIpVersion /*= 0 default-local-address-based*/)
{
    IMS_TRACE_I("DNS lookup (%d) :: apnType=%d, domain=%s", nIpVersion, GetApnType(),
            OsUtil::GetInstance()->IsDebugMode() ? strHostName.GetStr() : "xxx");

    if ((nIpVersion != IPAddress::IPV4) && (nIpVersion != IPAddress::IPV6))
    {
        const IPAddress& objLocalIp = GetLocalAddress();

        if (objLocalIp.IsIPv4Address())
        {
            nIpVersion = IPAddress::IPV4;
        }
        else if (objLocalIp.IsIPv6Address())
        {
            nIpVersion = IPAddress::IPV6;
        }
        else
        {
            IMS_TRACE_E(0, "Address family is unspecified", 0, 0, 0);
            return (-1);
        }
    }

    // ANDROID_L: android_gethostbynameforiface (KK-OS)
    // ANDROID_P: android_gethostbynamefornet (libc)
    AStringArray objHostIps = System::GetInstance()->GetHostByName(
            strHostName, nIpVersion, GetApnType(), GetSlotId());

    if (objHostIps.IsEmpty())
    {
        IMS_TRACE_D("No host address or DNS resolution failed", 0, 0, 0);
        return (-1);
    }

    for (IMS_SINT32 i = 0; i < objHostIps.GetCount(); i++)
    {
        const AString& strHostIp = objHostIps.GetElementAt(i);
        IPAddress objIp;

        if (objIp.Parse(strHostIp))
        {
            objIpAddrs.Append(objIp);
        }

        IMS_TRACE_D("gethostbyname() :: %s >>> %s at index(%d)",
                OsUtil::GetInstance()->IsDebugMode() ? strHostName.GetStr() : "xxx",
                OsUtil::GetInstance()->IsDebugMode() ? strHostIp.GetStr() : "xxx", i);
    }

    if (objIpAddrs.IsEmpty())
    {
        return (-1);
    }

    return 1;
}

PRIVATE VIRTUAL IMS_SINT32 OsWifiConnection::GetIfaceId() const
{
    return m_nIfaceId;
}

PRIVATE VIRTUAL const AString& OsWifiConnection::GetIfaceName() const
{
    return m_strIfaceName;
}

PRIVATE VIRTUAL const AStringArray& OsWifiConnection::GetPcscfAddress(
        IN IMS_SINT32 /*nIpVersion = 0 (configuration-based)*/)
{
    return m_objPcscfsAddress;
}

PRIVATE VIRTUAL INetworkConnection::STATE_ENTYPE OsWifiConnection::GetState() const
{
    return (m_nState == STATE_ACTIVE) ? STATE_CONNECTED : STATE_DISCONNECTED;
}

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::IsConnected(
        IN IMS_SINT32 /*nCategory = IIpcan::CATEGORY_ANY*/) const
{
    return (m_nState == STATE_ACTIVE);
}

PUBLIC VIRTUAL IMS_BOOL OsWifiConnection::SendPingToHostAddress(IN const IPAddress& objHostAddress)
{
    (void)objHostAddress;
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsWifiConnection::IsePDGEnabled() const
{
    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::IsMobileDataEnabled() const
{
    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_SINT32 OsWifiConnection::GetMtu() const
{
    return System::GetInstance()->GetMtu(GetApnType(), GetSlotId());
}

PRIVATE VIRTUAL void OsWifiConnection::SetListener(IN INetworkConnectionListener* piListener)
{
    m_piConnectionListener = piListener;
}

PRIVATE VIRTUAL void OsWifiConnection::SetPreferredIpVersion(
        IN IMS_SINT32 nPreferredIpVersion /*= 0 default-aos-connection-profile*/)
{
    IMS_TRACE_D("Preferred IP version: %d >> %d", m_nPreferredIpVersion, nPreferredIpVersion, 0);

    if (m_nPreferredIpVersion != nPreferredIpVersion)
    {
        m_nPreferredIpVersion = nPreferredIpVersion;

        if (IsConnected() && (m_nPreferredIpVersion != IPAddress::UNKNOWN))
        {
            AdjustPreferredLocalAddress();
        }
    }
}

PRIVATE VIRTUAL void OsWifiConnection::AddReferenceListener(
        IN INetworkConnectionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
    {
        INetworkConnectionListener* piTmpListener = m_objReferenceListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    m_objReferenceListeners.Append(piListener);
}

PRIVATE VIRTUAL void OsWifiConnection::RemoveReferenceListener(
        IN INetworkConnectionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
    {
        INetworkConnectionListener* piTmpListener = m_objReferenceListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objReferenceListeners.RemoveAt(i);
            return;
        }
    }
}

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::Create(IN const AString& strNetProfile)
{
    const NetworkPolicy* pTmpPolicy =
            NetworkServicePolicy::GetInstance()->GetPolicy(strNetProfile, GetSlotId());

    if (pTmpPolicy == IMS_NULL)
    {
        IMS_TRACE_D("NetworkPolicy (%s) is not present; can not create a WiFi connection",
                strNetProfile.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_pPolicy = new NetworkPolicy(*pTmpPolicy);

    m_nConnectionHandle = AttachNetworkConnection();

    return (m_nConnectionHandle != 0) ? IMS_TRUE : IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::Create(IN IMS_SINT32 nApnType)
{
    const NetworkPolicy* pTmpPolicy =
            NetworkServicePolicy::GetInstance()->GetPolicy(nApnType, GetSlotId());

    if (pTmpPolicy == IMS_NULL)
    {
        IMS_TRACE_D("NetworkPolicy (%d) is not present; can not create a WiFi connection", nApnType,
                0, 0);
        return IMS_FALSE;
    }

    m_pPolicy = new NetworkPolicy(*pTmpPolicy);

    m_nConnectionHandle = AttachNetworkConnection();

    return (m_nConnectionHandle != 0) ? IMS_TRUE : IMS_FALSE;
}

PRIVATE VIRTUAL void OsWifiConnection::DispatchServiceMessage(
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

        default:
            break;
    }
}

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::Equals(IN const IPAddress& objIpAddr) const
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

PRIVATE VIRTUAL IMS_CONNECTION OsWifiConnection::GetHandle() const
{
    return m_nConnectionHandle;
}

PRIVATE VIRTUAL const AString& OsWifiConnection::GetProfileName() const
{
    return (m_pPolicy != IMS_NULL) ? m_pPolicy->GetName() : AString::ConstNull();
}

PRIVATE VIRTUAL IMS_SINT32 OsWifiConnection::GetApnType() const
{
    return (m_pPolicy != IMS_NULL) ? m_pPolicy->GetApnType() : NetworkPolicy::APN_WIFI;
}

PRIVATE VIRTUAL void OsWifiConnection::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    IMS_TRACE_D("WiFi :: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d, nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_WIFI_STATE_CHANGED:
        {
            IMS_TRACE_D("WiFi :: wifi state changed in %s - %" PFLS_d, StateToString(m_nState),
                    nWParam, 0);

            OnWifiStateChanged(LONG_TO_INT(nWParam));
            break;
        }
        case IMS_SYSTEM_WIFINETWORK_STATE_CHANGED:
        {
            IMS_TRACE_I("WiFi :: wifi network state changed in %s - %" PFLS_d ", %" PFLS_d,
                    StateToString(m_nState), nWParam, nLParam);

            OnWifiNetworkStateChanged(LONG_TO_INT(nWParam), LONG_TO_INT(nLParam));
            break;
        }
        default:
        {
            IMS_TRACE_D("___ Event (%d) :: Ignored ___", nEvent, 0, 0);
            break;
        }
    }
}

PRIVATE
IMS_BOOL OsWifiConnection::AdjustPreferredLocalAddress()
{
    if (m_nPreferredIpVersion == IPAddress::IPV4)
    {
        if (!m_objLocalAddressIpv4.Equals(IPAddress::NONE) &&
                !m_objLocalAddressIpv4.Equals(m_objLocalAddress))
        {
            IMS_TRACE_D("(WiFi) Preferred local address :: %s >> %s",
                    m_objLocalAddress.ToString().GetStr(),
                    m_objLocalAddressIpv4.ToString().GetStr(), 0);

            m_objLocalAddress = m_objLocalAddressIpv4;
        }
    }
    else if (m_nPreferredIpVersion == IPAddress::IPV6)
    {
        if (!m_objLocalAddressIpv6.Equals(IPAddress::IPv6NONE) &&
                !m_objLocalAddressIpv6.Equals(m_objLocalAddress))
        {
            IMS_TRACE_D("(WiFi) Preferred local address :: %s >> %s",
                    m_objLocalAddress.ToString().GetStr(),
                    m_objLocalAddressIpv6.ToString().GetStr(), 0);

            m_objLocalAddress = m_objLocalAddressIpv6;
        }
    }

    if (m_objLocalAddress.Equals(IPAddress::NONE) || m_objLocalAddress.Equals(IPAddress::IPv6NONE))
    {
        IMS_TRACE_D("Local address is null", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_CONNECTION OsWifiConnection::AttachNetworkConnection()
{
    IMS_CONNECTION nHandle =
            ImsNetworkConnectionState::GetInstance()->GetAndIncrementHandle(IMS_FALSE);

    if (nHandle > 0)
    {
        ImsNetworkConnectionState::GetInstance()->AttachHandle(this);
        return nHandle;
    }

    IMS_TRACE_E(0, "Attaching NetConnection failed", 0, 0, 0);

    return static_cast<IMS_CONNECTION>(0);
}

PRIVATE
IMS_BOOL OsWifiConnection::CacheLocalAddress()
{
    struct ifreq* pIfreq;
    struct ifconf ifcfg;

    IMS_SINT32 nSockFd = socket(AF_INET6, SOCK_DGRAM, 0);

    memset(&ifcfg, 0, sizeof(ifcfg));

    ifcfg.ifc_buf = IMS_NULL;
    ifcfg.ifc_len = sizeof(struct ifreq) * MAX_INTERFACE_REQ;
    ifcfg.ifc_buf = (char*)malloc(ifcfg.ifc_len);

    for (IMS_SINT32 i = 0; i < MAX_IOCTL_LOOKUP; ++i)
    {
        ifcfg.ifc_len = sizeof(struct ifreq) * MAX_INTERFACE_REQ;
        ifcfg.ifc_buf = (char*)realloc(ifcfg.ifc_buf, ifcfg.ifc_len);

        if (ioctl(nSockFd, SIOCGIFCONF, (void*)&ifcfg) < 0)
        {
            IMS_TRACE_E(0, "ioctl(SIOCGIFCONF) (%d-th) error", i, 0, 0);
            continue;
        }

        break;
    }

    if (nSockFd != (-1))
    {
        close(nSockFd);
    }

    IMS_BOOL bIpFound = IMS_FALSE;

    // Look up IPv6 address ...
    pIfreq = ifcfg.ifc_req;
    for (IMS_SINT32 i = 0; i < ifcfg.ifc_len; i += sizeof(struct ifreq))
    {
        if (IMS_StrNCmp(pIfreq->ifr_name, WLAN_DESCRIPT, WLAN_DESCRIPT_SIZE) == 0)
        {
            struct sockaddr* sa = reinterpret_cast<struct sockaddr*>(&pIfreq->ifr_addr);

            if (sa->sa_family != AF_INET6)
            {
                pIfreq++;
                continue;
            }

            struct sockaddr_in6* sin6 = reinterpret_cast<struct sockaddr_in6*>(sa);
            IMS_CHAR acIpv6[64] = {
                    0,
            };
            const IMS_CHAR* pszIpv6 = inet_ntop(
                    AF_INET6, (const void*)&(sin6->sin6_addr.s6_addr), acIpv6, sizeof(acIpv6));

            if (!m_objLocalAddress.Parse(pszIpv6))
            {
                IMS_TRACE_E(0, "Parsing IPv6 address (%s) failed", pszIpv6, 0, 0);

                pIfreq++;
                continue;
            }

            // IPv6 address
            m_objLocalAddressIpv6 = m_objLocalAddress;

            IMS_TRACE_D("GetLocalAddress() on WiFi :: %s", m_objLocalAddress.ToCharString(), 0, 0);

            bIpFound = IMS_TRUE;
            break;
        }

        pIfreq++;
    }

    if (bIpFound)
    {
        // Look up IPv4 address ...
        pIfreq = ifcfg.ifc_req;
        for (IMS_SINT32 i = 0; i < ifcfg.ifc_len; i += sizeof(struct ifreq))
        {
            if (IMS_StrNCmp(pIfreq->ifr_name, WLAN_DESCRIPT, WLAN_DESCRIPT_SIZE) == 0)
            {
                struct sockaddr* sa = reinterpret_cast<struct sockaddr*>(&pIfreq->ifr_addr);

                if (sa->sa_family != AF_INET)
                {
                    pIfreq++;
                    continue;
                }

                struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(sa);
                IMS_CHAR acIpv4[32] = {
                        0,
                };
                const IMS_CHAR* pszIpv4 = inet_ntop(
                        AF_INET, (const void*)&(sin->sin_addr.s_addr), acIpv4, sizeof(acIpv4));

                if (!m_objLocalAddressIpv4.Parse(pszIpv4))
                {
                    IMS_TRACE_E(0, "Parsing IPv4 address (%s) failed", pszIpv4, 0, 0);

                    pIfreq++;
                    continue;
                }
                break;
            }

            pIfreq++;
        }

        IMS_TRACE_D("WiFi :: LOCAL - IPv4 (%s), IPv6 (%s)",
                m_objLocalAddressIpv4.ToString().GetStr(),
                m_objLocalAddressIpv6.ToString().GetStr(), 0);

        goto EXIT_LookupLocalIpAddress;
    }

    // Look up IPv4 address ...
    pIfreq = ifcfg.ifc_req;
    for (IMS_SINT32 i = 0; i < ifcfg.ifc_len; i += sizeof(struct ifreq))
    {
        if (IMS_StrNCmp(pIfreq->ifr_name, WLAN_DESCRIPT, WLAN_DESCRIPT_SIZE) == 0)
        {
            struct sockaddr* sa = reinterpret_cast<struct sockaddr*>(&pIfreq->ifr_addr);

            if (sa->sa_family != AF_INET)
            {
                pIfreq++;
                continue;
            }

            struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(sa);
            IMS_CHAR acIpv4[32] = {
                    0,
            };
            const IMS_CHAR* pszIpv4 = inet_ntop(
                    AF_INET, (const void*)&(sin->sin_addr.s_addr), acIpv4, sizeof(acIpv4));

            if (!m_objLocalAddress.Parse(pszIpv4))
            {
                IMS_TRACE_E(0, "Parsing IPv4 address (%s) failed", pszIpv4, 0, 0);

                pIfreq++;
                continue;
            }

            // IPv4 address
            m_objLocalAddressIpv4 = m_objLocalAddress;

            IMS_TRACE_D(
                    "CacheLocalAddress() on WiFi :: %s", m_objLocalAddress.ToCharString(), 0, 0);

            bIpFound = IMS_TRUE;
            break;
        }

        pIfreq++;
    }

    IMS_TRACE_D("WiFi :: LOCAL - IPv4 (%s), IPv6 (NONE)", m_objLocalAddressIpv4.ToString().GetStr(),
            0, 0);

EXIT_LookupLocalIpAddress:

    if (ifcfg.ifc_buf != IMS_NULL)
    {
        free(ifcfg.ifc_buf);
        ifcfg.ifc_buf = IMS_NULL;
    }

    AdjustPreferredLocalAddress();

    // Network interface identifier
    m_nIfaceId = System::GetInstance()->GetIfaceId(GetApnType(), GetSlotId());

    IMS_TRACE_D("WiFi :: IfaceId=%d", m_nIfaceId, 0, 0);

    return bIpFound;
}

PRIVATE
void OsWifiConnection::CallReferenceListeners(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nErrorCode /*= 0*/)
{
    switch (nEvent)
    {
        case NET_CONNECTED:
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnConnected(this);
                }
            }
            break;

        case NET_DISCONNECTED:
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnDisconnected(this, nErrorCode);
                }
            }
            break;

        case NET_CONNECT_FAILED:
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnConnectionFailed(this, nErrorCode);
                }
            }
            break;

        case NET_IP_CHANGED:
            for (IMS_UINT32 i = 0; i < m_objReferenceListeners.GetSize(); ++i)
            {
                INetworkConnectionListener* piListener = m_objReferenceListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->NetworkConnection_OnIpChanged(this);
                }
            }
            break;

        default:
            break;
    }
}

PRIVATE
void OsWifiConnection::CheckValidityForLocalAddress()
{
    IPAddress objTmpLocalAddress = m_objLocalAddress;
    IPAddress objTmpLocalAddressIpv4 = m_objLocalAddressIpv4;
    IPAddress objTmpLocalAddressIpv6 = m_objLocalAddressIpv6;

    m_objLocalAddress = IPAddress::NONE;
    m_objLocalAddressIpv4 = IPAddress::NONE;
    m_objLocalAddressIpv6 = IPAddress::IPv6NONE;

    if (!CacheLocalAddress())
    {
        // Rollback the existing local addresses
        m_objLocalAddress = objTmpLocalAddress;
        m_objLocalAddressIpv4 = objTmpLocalAddressIpv4;
        m_objLocalAddressIpv6 = objTmpLocalAddressIpv6;

        IMS_TRACE_D("WiFi :: IP_VALIDITY_CHECK - caching local address failed", 0, 0, 0);
        return;
    }

    IMS_BOOL bDefaultChanged = !m_objLocalAddress.Equals(objTmpLocalAddress);
    IMS_BOOL bIpv4Changed = !m_objLocalAddressIpv4.Equals(objTmpLocalAddressIpv4);
    IMS_BOOL bIpv6Changed = !m_objLocalAddressIpv6.Equals(objTmpLocalAddressIpv6);

    if (bDefaultChanged || bIpv4Changed || bIpv6Changed)
    {
        IMS_TRACE_D("WiFi :: IP_VALIDITY_CHECK - change detected; default(%s), v4(%s), v6(%s)",
                _TRACE_B_(bDefaultChanged), _TRACE_B_(bIpv4Changed), _TRACE_B_(bIpv6Changed));
    }
}

PRIVATE
void OsWifiConnection::ClearOnDataDisconnected()
{
    // Clear the previous P-CSCF addresses
    m_objPcscfsAddress.RemoveAllElements();

    // Release all sockets
    ImsSocketState::GetInstance()->DetachAll(m_nConnectionHandle);
}

/**
 * @see #WIFI_STATE_DISABLING = 0
 * @see #WIFI_STATE_DISABLED
 * @see #WIFI_STATE_ENABLING
 * @see #WIFI_STATE_ENABLED
 * @see #WIFI_STATE_UNKNOWN
 */
PRIVATE
IMS_SINT32 OsWifiConnection::GetWifiState()
{
    m_nWifiState = System::GetInstance()->GetWifiState();

    return m_nWifiState;
}

/**
 * IDLE, SCANNING, CONNECTING, AUTHENTICATING,
 * OBTAINING_IPADDR, CONNECTED, SUSPENDED,
 * DISCONNECTING, DISCONNECTED, FAILED
 */
PRIVATE
IMS_SINT32 OsWifiConnection::GetWifiDetailedState()
{
    m_nWifiDetailedState = System::GetInstance()->GetWifiDetailedState();

    return m_nWifiDetailedState;
}

PRIVATE
IMS_BOOL OsWifiConnection::IsConnectedInternal()
{
    IMS_SINT32 nWState = GetWifiState();
    IMS_SINT32 nWDetailedState = GetWifiDetailedState();

    return (((nWState == WIFI_STATE_ENABLED) &&
                    (nWDetailedState == WIFI_NET_DETAILED_STATE_CONNECTED)) ||
            (nWDetailedState == WIFI_NET_DETAILED_STATE_OBTAINING_IPADDR) ||
            (nWDetailedState == WIFI_NET_DETAILED_STATE_CAPTIVE_PORTAL_CHECK));
}

PRIVATE
IMS_BOOL OsWifiConnection::IsDisconnected()
{
    IMS_SINT32 nWState = GetWifiState();

    if (nWState == WIFI_STATE_DISABLED)
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nWDetailedState = GetWifiDetailedState();

    return (nWState == WIFI_STATE_ENABLED) &&
            (nWDetailedState == WIFI_NET_DETAILED_STATE_DISCONNECTED);
}

PRIVATE
void OsWifiConnection::NotifyDataConnected(IN IMS_SINT32 nErrorCode)
{
    if (IsConnected())
    {
        IMS_TRACE_I("WiFi :: Network Connection is already activated.", 0, 0, 0);

        CheckValidityForLocalAddress();
        return;
    }

    m_objLocalAddress = IPAddress::NONE;
    m_objLocalAddressIpv4 = IPAddress::NONE;
    m_objLocalAddressIpv6 = IPAddress::IPv6NONE;

    if (!CacheLocalAddress())
    {
        IMS_SINT32 nWDetailedState = GetWifiDetailedState();

        if (nWDetailedState == WIFI_NET_DETAILED_STATE_CONNECTED)
        {
            // Try to cache the local address one more.
            // If it's also failed, it means the permanent failure.
            if (!CacheLocalAddress())
            {
                // Fatal error
                IMS_TRACE_E(0, "Caching the local IP address failed", 0, 0, 0);
            }
        }
        else if (nWDetailedState == WIFI_NET_DETAILED_STATE_CAPTIVE_PORTAL_CHECK)
        {
            if (!CacheLocalAddress())
            {
                IMS_TRACE_D("WiFi :: Waits for CONNECTED on CAPTIVE_PORTAL_CHECK", 0, 0, 0);
                return;
            }
        }
        else
        {
            IMS_TRACE_D("WiFi :: Waits for CONNECTED on detailed_state(%d)", nWDetailedState, 0, 0);
            return;
        }
    }

    SetState(STATE_ACTIVE);

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnConnected(this);
    }

    CallReferenceListeners(NET_CONNECTED, nErrorCode);
}

PRIVATE
void OsWifiConnection::NotifyDataDisconnected(IN IMS_SINT32 nErrorCode)
{
    if (m_nState != STATE_ACTIVE)
    {
        // Ignore the event
        return;
    }

    SetState(STATE_IDLE);

    ClearOnDataDisconnected();

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnDisconnected(this, nErrorCode);
    }

    CallReferenceListeners(NET_DISCONNECTED, nErrorCode);
}

PRIVATE
void OsWifiConnection::NotifyDataConnectionFailed(IN IMS_SINT32 nErrorCode)
{
    SetState(STATE_IDLE);

    ClearOnDataDisconnected();

    if (m_piConnectionListener != IMS_NULL)
    {
        m_piConnectionListener->NetworkConnection_OnConnectionFailed(this, nErrorCode);
    }

    CallReferenceListeners(NET_CONNECT_FAILED, nErrorCode);
}

PRIVATE
void OsWifiConnection::NotifyIpChanged(IN IMS_SINT32 nErrorCode)
{
    if (m_nState != STATE_ACTIVE)
    {
        // Ignore the event
        IMS_TRACE_D("WiFi :: IP changed - not ACTIVE", 0, 0, 0);
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

/**
 * WIFI_STATE_DISABLING = 0,
 * WIFI_STATE_DISABLED = 1,
 * WIFI_STATE_ENABLING = 2,
 * WIFI_STATE_ENABLED = 3,
 * WIFI_STATE_UNKNOWN = 4,
 */
PRIVATE
void OsWifiConnection::OnWifiStateChanged(IN IMS_UINT32 nState)
{
    if (nState == WIFI_STATE_DISABLED)
    {
        PostEvent(NET_DISCONNECTED);
        return;
    }
}

/**
 *
 * State
 *     WIFI_NET_STATE_CONNECTING = 0,
 *     WIFI_NET_STATE_CONNECTED,
 *     WIFI_NET_STATE_SUSPENDED,
 *     WIFI_NET_STATE_DISCONNECTING,
 *     WIFI_NET_STATE_DISCONNECTED,
 *     WIFI_NET_STATE_UNKNOWN
 *
 * Detailed State
 *     WIFI_NET_DETAILED_STATE_IDLE = 0,
 *     WIFI_NET_DETAILED_STATE_SCANNING,
 *     WIFI_NET_DETAILED_STATE_CONNECTING,
 *     WIFI_NET_DETAILED_STATE_AUTHENTICATING,
 *     WIFI_NET_DETAILED_STATE_OBTAINING_IPADDR,
 *     WIFI_NET_DETAILED_STATE_CONNECTED,
 *     WIFI_NET_DETAILED_STATE_SUSPENDED,
 *     WIFI_NET_DETAILED_STATE_DISCONNECTING,
 *     WIFI_NET_DETAILED_STATE_DISCONNECTED,
 *     WIFI_NET_DETAILED_STATE_FAILED
 */
PRIVATE
void OsWifiConnection::OnWifiNetworkStateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nDetailedState)
{
    (void)nDetailedState;

    if (IsConnected() && (nState == WIFI_NET_DETAILED_STATE_CAPTIVE_PORTAL_CHECK))
    {
        // Keep the current WiFi connection state
        IMS_TRACE_D("CAPTIVE_PORTAL_CHECK :: WiFi is already connected; ignore it.", 0, 0, 0);
    }
    else if (IsConnected() && (nState == WIFI_NET_DETAILED_STATE_VERIFYING_POOR_LINK))
    {
        // Keep the current WiFi connection state
        IMS_TRACE_D("VERIFYING_POOR_LINK :: WiFi is already connected; ignore it.", 0, 0, 0);
    }
    else if ((nState == WIFI_NET_DETAILED_STATE_OBTAINING_IPADDR) ||
            (nState == WIFI_NET_DETAILED_STATE_CONNECTED) ||
            (nState == WIFI_NET_DETAILED_STATE_CAPTIVE_PORTAL_CHECK))
    {
        PostEvent(NET_CONNECTED);
    }
    else
    {
        PostEvent(NET_DISCONNECTED);
    }
}

PRIVATE
void OsWifiConnection::PostEvent(IN IMS_UINT32 nEvent)
{
    if (m_piOwnerThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "No owner thread", 0, 0, 0);
        return;
    }

    switch (nEvent)
    {
        case NET_CONNECTED:     // FALL-THROUGH
        case NET_IP_CHANGED:    // FALL-THROUGH
        case NET_DISCONNECTED:  // FALL-THROUGH
        case NET_CONNECT_FAILED:
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
void OsWifiConnection::SetState(IN IMS_UINT32 nState)
{
    if (m_nState != nState)
    {
        IMS_TRACE_I(
                "WiFi :: connection - %s >> %s", StateToString(m_nState), StateToString(nState), 0);

        m_nState = nState;
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsWifiConnection::StateToString(IN IMS_SINT32 nState)
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
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
