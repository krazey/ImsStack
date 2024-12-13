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
#include "ImsMessageDef.h"
#include "ImsAccessNetworkInfoType.h"
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "IThread.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServiceEvent.h"
#include "ServiceMemory.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "network/OsWifiConnection.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

PUBLIC
OsWifiConnection::OsWifiConnection() :
        ImsNetworkConnection(IMS_SLOT_0),
        m_nState(STATE_IDLE),
        m_nWifiState(WIFI_STATE_DISABLED),
        m_nWifiConnectionState(WIFI_CONNECTION_STATE_DISCONNECTED),
        m_pPolicy(IMS_NULL),
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
    IMS_TRACE_D("Constructor :: Wifi connection", 0, 0, 0);

    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_WIFI, this, GetSlotId());
}

PUBLIC VIRTUAL OsWifiConnection::~OsWifiConnection()
{
    IMS_TRACE_D("Destructor :: Wifi connection", 0, 0, 0);

    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_WIFI, this, GetSlotId());

    if (m_pPolicy != IMS_NULL)
    {
        ImsNetworkConnectionState::GetInstance()->DetachHandle(m_pPolicy->GetName(), GetSlotId());

        delete m_pPolicy;
    }
}

PUBLIC VIRTUAL const IpAddress& OsWifiConnection::GetLocalAddress(
        IN IMS_SINT32 nIpVersion /*= 0 configuration-based*/) const
{
    if (nIpVersion == 0)
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

PRIVATE VIRTUAL INetworkConnection::RESULT_ENTYPE OsWifiConnection::Activate(
        IN IMS_BOOL /*bEnableApn = IMS_FALSE*/)
{
    // If the connection with Wi-Fi network is already established,
    // then it sends an event to the application.
    if (IsConnectedInternal())
    {
        if (!IsConnected())
        {
            m_objLocalAddress = IpAddress::NONE;
            m_objLocalAddressIpv4 = IpAddress::NONE;
            m_objLocalAddressIpv6 = IpAddress::IPv6NONE;
        }

        if (!CacheLocalAddress())
        {
            IMS_TRACE_E(0, "Caching the local IP address failed", 0, 0, 0);
        }

        IMS_TRACE_I("Wifi :: Activate() - APN (%s) is already activated; "
                    "state=%d, detailed_state=%d",
                GetProfileName().GetStr(), m_nWifiState, m_nWifiConnectionState);

        SetState(STATE_ACTIVE);

        return RESULT_DONE;
    }

    IMS_TRACE_D("Wifi :: Activate() - state=%d, detailed_state=%d", m_nWifiState,
            m_nWifiConnectionState, 0);

    SetState(STATE_ACTIVATING);

    return RESULT_DOING;
}

PRIVATE VIRTUAL INetworkConnection::RESULT_ENTYPE OsWifiConnection::Deactivate(
        IN IMS_BOOL /*bDisableApn = IMS_FALSE*/)
{
    if (m_nState == STATE_TERMINATED)
    {
        IMS_TRACE_D("Wifi :: Deactivate() in TERMINATED state", 0, 0, 0);
        return RESULT_DONE;
    }

    if (IsDisconnected())
    {
        IMS_TRACE_D("Wifi :: Deactivate() in DATA_DISCONNECTED state", 0, 0, 0);

        SetState(STATE_TERMINATED);
        return RESULT_DONE;
    }

    IMS_TRACE_D("Wifi :: Deactivate() - state=%d, detailed_state=%d", m_nWifiState,
            m_nWifiConnectionState, 0);

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
    AString strMacAddress = PlatformContext::GetInstance()->GetSystem()->GetWifiBssId();
    ImsList<AString> objTokens = strMacAddress.Split(':');

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
        strInfo = PlatformContext::GetInstance()->GetSystem()->GetWifiBssId();

        if (strInfo.GetLength() == 0)
        {
            strInfo = "00:00:00:00:00:00";
        }

        IMS_TRACE_D("MAC ADDRESS (%s)", strInfo.GetStr(), 0, 0);
    }
    else if (strType.Equals("ssid"))
    {
        strInfo = PlatformContext::GetInstance()->GetSystem()->GetWifiSsId();

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
        OUT ImsList<IpAddress>& objIpAddrs,
        IN IMS_SINT32 nIpVersion /*= 0 default-local-address-based*/)
{
    IMS_TRACE_I("DNS lookup (%d) :: apnType=%d, domain=%s", nIpVersion, GetApnType(),
            OsUtil::GetInstance()->IsDebugMode() ? strHostName.GetStr() : "xxx");

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

    // ANDROID_L: android_gethostbynameforiface (KK-OS)
    // ANDROID_P: android_gethostbynamefornet (libc)
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

PRIVATE VIRTUAL IMS_SINT32 OsWifiConnection::GetMtu() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetMtu(GetApnType(), GetSlotId());
}

PRIVATE VIRTUAL void OsWifiConnection::SetPreferredIpVersion(
        IN IMS_SINT32 nPreferredIpVersion /*= 0 default-aos-connection-profile*/)
{
    IMS_TRACE_D("Preferred IP version: %d >> %d", m_nPreferredIpVersion, nPreferredIpVersion, 0);

    if (m_nPreferredIpVersion != nPreferredIpVersion)
    {
        m_nPreferredIpVersion = nPreferredIpVersion;

        if (IsConnected() && (m_nPreferredIpVersion != IpAddress::UNKNOWN))
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
        IMS_TRACE_D("NetworkPolicy (%s) is not present; can not create a Wi-Fi connection",
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
        IMS_TRACE_D("NetworkPolicy (%d) is not present; can not create a Wi-Fi connection",
                nApnType, 0, 0);
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

PRIVATE VIRTUAL IMS_BOOL OsWifiConnection::Equals(IN const IpAddress& objIpAddr) const
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
    IMS_TRACE_D("Wifi :: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d, nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_WIFI_STATE_CHANGED:
        {
            IMS_TRACE_D(
                    "Wifi :: State changed in %s - %" PFLS_d, StateToString(m_nState), nWParam, 0);

            OnWifiStateChanged(LONG_TO_INT(nWParam));
            break;
        }
        case IMS_SYSTEM_WIFI_CONNECTION_STATE_CHANGED:
        {
            IMS_TRACE_I("Wifi :: Connection state changed in %s - %" PFLS_d,
                    StateToString(m_nState), nWParam, 0);

            OnWifiConnectionStateChanged(LONG_TO_INT(nWParam));
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
    if (m_nPreferredIpVersion == IpAddress::IPV4)
    {
        if (!m_objLocalAddressIpv4.Equals(IpAddress::NONE) &&
                !m_objLocalAddressIpv4.Equals(m_objLocalAddress))
        {
            IMS_TRACE_D("Wifi: Preferred local address changed from %s to %s",
                    m_objLocalAddress.ToString().GetStr(),
                    m_objLocalAddressIpv4.ToString().GetStr(), 0);

            m_objLocalAddress = m_objLocalAddressIpv4;
        }
    }
    else if (m_nPreferredIpVersion == IpAddress::IPV6)
    {
        if (!m_objLocalAddressIpv6.Equals(IpAddress::IPv6NONE) &&
                !m_objLocalAddressIpv6.Equals(m_objLocalAddress))
        {
            IMS_TRACE_D("Wifi: Preferred local address changed from %s to %s",
                    m_objLocalAddress.ToString().GetStr(),
                    m_objLocalAddressIpv6.ToString().GetStr(), 0);

            m_objLocalAddress = m_objLocalAddressIpv6;
        }
    }

    if (m_objLocalAddress.Equals(IpAddress::NONE) || m_objLocalAddress.Equals(IpAddress::IPv6NONE))
    {
        // As default, IPv4 is a default address for Wi-Fi network.
        if (!m_objLocalAddressIpv4.Equals(IpAddress::NONE))
        {
            m_objLocalAddress = m_objLocalAddressIpv4;
        }
        else if (!m_objLocalAddressIpv6.Equals(IpAddress::IPv6NONE))
        {
            m_objLocalAddress = m_objLocalAddressIpv6;
        }
        else
        {
            IMS_TRACE_D("Local address is null", 0, 0, 0);
            return IMS_FALSE;
        }
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

    if (!AdjustPreferredLocalAddress())
    {
        IMS_TRACE_E(0, "Local Address is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("Wifi :: LOCAL - IPv4 (%s), IPv6 (%s)", m_objLocalAddressIpv4.ToString().GetStr(),
            m_objLocalAddressIpv6.ToString().GetStr(), 0);

    // Network interface identifier
    m_nIfaceId = PlatformContext::GetInstance()->GetSystem()->GetIfaceId(GetApnType(), GetSlotId());

    // Network interface name
    m_strIfaceName =
            PlatformContext::GetInstance()->GetSystem()->GetIfaceName(GetApnType(), GetSlotId());

    IMS_TRACE_D("Wifi :: IfaceId=%d, IfaceName=%s", m_nIfaceId, m_strIfaceName.GetStr(), 0);

    return IMS_TRUE;
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
    IpAddress objTmpLocalAddress = m_objLocalAddress;
    IpAddress objTmpLocalAddressIpv4 = m_objLocalAddressIpv4;
    IpAddress objTmpLocalAddressIpv6 = m_objLocalAddressIpv6;

    m_objLocalAddress = IpAddress::NONE;
    m_objLocalAddressIpv4 = IpAddress::NONE;
    m_objLocalAddressIpv6 = IpAddress::IPv6NONE;

    if (!CacheLocalAddress())
    {
        // Rollback the existing local addresses
        m_objLocalAddress = objTmpLocalAddress;
        m_objLocalAddressIpv4 = objTmpLocalAddressIpv4;
        m_objLocalAddressIpv6 = objTmpLocalAddressIpv6;

        IMS_TRACE_D("Wifi :: IP_VALIDITY_CHECK - caching local address failed", 0, 0, 0);
        return;
    }

    IMS_BOOL bDefaultChanged = !m_objLocalAddress.Equals(objTmpLocalAddress);
    IMS_BOOL bIpv4Changed = !m_objLocalAddressIpv4.Equals(objTmpLocalAddressIpv4);
    IMS_BOOL bIpv6Changed = !m_objLocalAddressIpv6.Equals(objTmpLocalAddressIpv6);

    if (bDefaultChanged || bIpv4Changed || bIpv6Changed)
    {
        IMS_TRACE_D("Wifi :: IP_VALIDITY_CHECK - change detected; default(%s), v4(%s), v6(%s)",
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
 * @see #WIFI_STATE_DISABLED
 * @see #WIFI_STATE_ENABLED
 */
PRIVATE
IMS_SINT32 OsWifiConnection::GetWifiState()
{
    m_nWifiState = PlatformContext::GetInstance()->GetSystem()->GetWifiState();

    return m_nWifiState;
}

/**
 * @see #WIFI_CONNECTION_STATE_CONNECTED
 * @see #WIFI_CONNECTION_STATE_DISCONNECTED
 */
PRIVATE
IMS_SINT32 OsWifiConnection::GetWifiConnectionState()
{
    m_nWifiConnectionState = PlatformContext::GetInstance()->GetSystem()->GetWifiConnectionState();

    return m_nWifiConnectionState;
}

PRIVATE
IMS_BOOL OsWifiConnection::IsConnectedInternal()
{
    IMS_SINT32 nState = GetWifiState();
    IMS_SINT32 nConnectionState = GetWifiConnectionState();

    return (nState == WIFI_STATE_ENABLED) && (nConnectionState == WIFI_CONNECTION_STATE_CONNECTED);
}

PRIVATE
IMS_BOOL OsWifiConnection::IsDisconnected()
{
    IMS_SINT32 nState = GetWifiState();

    if (nState == WIFI_STATE_DISABLED)
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nConnectionState = GetWifiConnectionState();

    return (nState == WIFI_STATE_ENABLED) &&
            (nConnectionState == WIFI_CONNECTION_STATE_DISCONNECTED);
}

PRIVATE
void OsWifiConnection::NotifyDataConnected(IN IMS_SINT32 nErrorCode)
{
    if (IsConnected())
    {
        IMS_TRACE_I("Wifi :: Network Connection is already activated.", 0, 0, 0);

        CheckValidityForLocalAddress();
        return;
    }

    m_objLocalAddress = IpAddress::NONE;
    m_objLocalAddressIpv4 = IpAddress::NONE;
    m_objLocalAddressIpv6 = IpAddress::IPv6NONE;

    CacheLocalAddress();

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
        IMS_TRACE_D("Wifi :: IP changed - not ACTIVE", 0, 0, 0);
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
 * @see #WIFI_STATE_DISABLED
 * @see #WIFI_STATE_ENABLED
 */
PRIVATE
void OsWifiConnection::OnWifiStateChanged(IN IMS_UINT32 nState)
{
    m_nWifiState = nState;

    if (nState == WIFI_STATE_DISABLED)
    {
        PostEvent(NET_DISCONNECTED);
        return;
    }
}

/**
 * @see #WIFI_CONNECTION_STATE_CONNECTED
 * @see #WIFI_CONNECTION_STATE_DISCONNECTED
 */
PRIVATE
void OsWifiConnection::OnWifiConnectionStateChanged(IN IMS_UINT32 nConnectionState)
{
    m_nWifiConnectionState = nConnectionState;

    if (nConnectionState == WIFI_CONNECTION_STATE_CONNECTED)
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
        IMS_TRACE_I("Wifi :: %s >> %s", StateToString(m_nState), StateToString(nState), 0);
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
