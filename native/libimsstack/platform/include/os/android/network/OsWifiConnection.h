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
#ifndef OS_WIFI_CONNECTION_H_
#define OS_WIFI_CONNECTION_H_

#include "ISystemListener.h"
#include "ImsNetworkConnection.h"
#include "OsNetworkConstants.h"

class IThread;
class NetworkPolicy;

class OsWifiConnection : public ImsNetworkConnection, public ISystemListener
{
public:
    OsWifiConnection();
    ~OsWifiConnection() override;

    OsWifiConnection(IN const OsWifiConnection&) = delete;
    OsWifiConnection& operator=(IN const OsWifiConnection&) = delete;

    /// State of Packet Data Connection
    enum
    {
        STATE_IDLE = 0,
        /// Connected with ActiveSync or RRC service is running
        STATE_PENDING,
        STATE_ACTIVATING,
        STATE_ACTIVE,
        STATE_TERMINATED
    };

    enum
    {
        NET_CONNECTED = 100,
        NET_IP_CHANGED,

        NET_DISCONNECTED,
        NET_CONNECT_FAILED,

        NET_TERMINATED
    };

public:
    const IpAddress& GetLocalAddress(
            IN IMS_SINT32 nIpVersion = 0 /*configuration-based*/) const override;

private:
    // INetworkConnection class
    RESULT_ENTYPE Activate(IN IMS_BOOL bEnableApn = IMS_FALSE) override;
    RESULT_ENTYPE Deactivate(IN IMS_BOOL bDisableApn = IMS_FALSE) override;
    void GetAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo) override;
    void GetLastAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo,
            OUT AString& strTimeStamp, OUT AString& strCellInfoAge) override;
    IMS_BOOL GetExtraInfo(IN const AString& strType, OUT AString& strInfo) override;
    IMS_SINT32 GetHostByName(IN const AString& strHostName, OUT ImsList<IpAddress>& objIpAddrs,
            IN IMS_SINT32 nIpVersion = 0 /*default-local-address-based*/) override;
    inline IMS_SINT32 GetIfaceId() const override { return m_nIfaceId; }
    inline const AString& GetIfaceName() const override { return m_strIfaceName; }
    inline const AStringArray& GetPcscfAddress(
            IN IMS_SINT32 nIpVersion = 0 /*configuration-based*/) override
    {
        (void)nIpVersion;
        return m_objPcscfsAddress;
    }
    inline STATE_ENTYPE GetState() const override
    {
        return (m_nState == STATE_ACTIVE) ? STATE_CONNECTED : STATE_DISCONNECTED;
    }
    inline IMS_BOOL IsConnected(IN IMS_SINT32 nCategory = IIpcan::CATEGORY_ANY) const override
    {
        (void)nCategory;
        return m_nState == STATE_ACTIVE;
    }
    inline IMS_BOOL IsePDGEnabled() const override { return IMS_FALSE; }
    inline IMS_BOOL IsIpv6Preferred() const override { return IMS_FALSE; }
    inline IMS_BOOL IsMobileDataEnabled() const override { return IMS_FALSE; }
    IMS_SINT32 GetMtu() const override;
    inline void SetListener(IN INetworkConnectionListener* piListener) override
    {
        m_piConnectionListener = piListener;
    }
    void SetPreferredIpVersion(
            IN IMS_SINT32 nPreferredIpVersion = 0 /*default-aos-connection-profile*/) override;
    void AddReferenceListener(IN INetworkConnectionListener* piListener) override;
    void RemoveReferenceListener(IN INetworkConnectionListener* piListener) override;

    // ImsNetworkConnection class
    IMS_BOOL Create(IN const AString& strNetProfile) override;
    IMS_BOOL Create(IN IMS_SINT32 nApnType) override;
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;
    IMS_BOOL Equals(IN const IpAddress& objIpAddr) const override;
    inline IMS_CONNECTION GetHandle() const override { return m_nConnectionHandle; }
    const AString& GetProfileName() const override;
    IMS_SINT32 GetApnType() const override;

    // ISystemListener interface
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

    IMS_BOOL AdjustPreferredLocalAddress();
    IMS_CONNECTION AttachNetworkConnection();
    IMS_BOOL CacheLocalAddress();
    void CallReferenceListeners(IN IMS_SINT32 nEvent, IN IMS_SINT32 nErrorCode = 0);
    void CheckValidityForLocalAddress();
    void ClearOnDataDisconnected();
    IMS_SINT32 GetWifiConnectionState();
    IMS_SINT32 GetWifiState();
    IMS_BOOL IsConnectedInternal();
    IMS_BOOL IsDisconnected();
    void NotifyDataConnected(IN IMS_SINT32 nErrorCode);
    void NotifyDataDisconnected(IN IMS_SINT32 nErrorCode);
    void NotifyDataConnectionFailed(IN IMS_SINT32 nErrorCode);
    void NotifyIpChanged(IN IMS_SINT32 nErrorCode);
    void OnWifiConnectionStateChanged(IN IMS_UINT32 nConnectionState);
    void OnWifiStateChanged(IN IMS_UINT32 nState);
    void PostEvent(IN IMS_UINT32 nEvent);
    void SetState(IN IMS_UINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    IMS_UINT32 m_nState;
    IMS_SINT32 m_nWifiState;
    IMS_SINT32 m_nWifiConnectionState;

    NetworkPolicy* m_pPolicy;
    // Network interface identifier when data connection is connected
    IMS_SINT32 m_nIfaceId;
    // Network interface name when data connection is connected
    AString m_strIfaceName;

    // Preferred local IP address's version
    IMS_SINT32 m_nPreferredIpVersion;
    // Configuration-based local IP address
    IpAddress m_objLocalAddress;
    IpAddress m_objLocalAddressIpv4;
    IpAddress m_objLocalAddressIpv6;
    AStringArray m_objPcscfsAddress;

    IMS_CONNECTION m_nConnectionHandle;

    IThread* m_piOwnerThread;
    INetworkConnectionListener* m_piConnectionListener;
    ImsList<INetworkConnectionListener*> m_objReferenceListeners;
};

#endif
