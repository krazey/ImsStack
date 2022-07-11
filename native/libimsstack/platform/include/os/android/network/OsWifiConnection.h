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

#include "ImsNetworkConnection.h"
#include "OsNetworkConstants.h"
#include "system-intf/ISystemListener.h"

class IThread;
class NetworkPolicy;

class OsWifiConnection : public ImsNetworkConnection, public ISystemListener
{
public:
    OsWifiConnection();
    virtual ~OsWifiConnection();

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
    const IPAddress& GetLocalAddress(
            IN IMS_SINT32 nIpVersion = 0 /*configuration-based*/) const override;

private:
    // INetworkConnection class
    RESULT_ENTYPE Activate(IN IMS_BOOL bEnableApn = IMS_FALSE) override;
    RESULT_ENTYPE Deactivate(IN IMS_BOOL bDisableApn = IMS_FALSE) override;
    void GetAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo) override;
    void GetLastAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo,
            OUT AString& strTimeStamp, OUT AString& strCellInfoAge) override;
    IMS_BOOL GetExtraInfo(IN const AString& strType, OUT AString& strInfo) override;
    IMS_SINT32 GetHostByName(IN const AString& strHostName, OUT IMSList<IPAddress>& objIpAddrs,
            IN IMS_SINT32 nIpVersion = 0 /*default-local-address-based*/) override;
    IMS_SINT32 GetIfaceId() const override;
    const AString& GetIfaceName() const override;
    const AStringArray& GetPcscfAddress(
            IN IMS_SINT32 nIpVersion = 0 /*configuration-based*/) override;
    STATE_ENTYPE GetState() const override;
    IMS_BOOL IsConnected(IN IMS_SINT32 nCategory = IIpcan::CATEGORY_ANY) const override;
    IMS_BOOL IsePDGEnabled() const override;
    IMS_BOOL IsMobileDataEnabled() const override;
    IMS_SINT32 GetMtu() const override;
    void SetListener(IN INetworkConnectionListener* piListener) override;
    void SetPreferredIpVersion(
            IN IMS_SINT32 nPreferredIpVersion = 0 /*default-aos-connection-profile*/) override;
    void AddReferenceListener(IN INetworkConnectionListener* piListener) override;
    void RemoveReferenceListener(IN INetworkConnectionListener* piListener) override;

    // ImsNetworkConnection class
    IMS_BOOL Create(IN const AString& strNetProfile) override;
    IMS_BOOL Create(IN IMS_SINT32 nApnType) override;
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;
    IMS_BOOL Equals(IN const IPAddress& objIpAddr) const override;
    IMS_CONNECTION GetHandle() const override;
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
    IMS_SINT32 GetWifiDetailedState();
    IMS_SINT32 GetWifiState();
    IMS_BOOL IsConnectedInternal();
    IMS_BOOL IsDisconnected();
    void NotifyDataConnected(IN IMS_SINT32 nErrorCode);
    void NotifyDataDisconnected(IN IMS_SINT32 nErrorCode);
    void NotifyDataConnectionFailed(IN IMS_SINT32 nErrorCode);
    void NotifyIpChanged(IN IMS_SINT32 nErrorCode);
    void OnWifiNetworkStateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nDetailedState);
    void OnWifiStateChanged(IN IMS_UINT32 nState);
    void PostEvent(IN IMS_UINT32 nEvent);
    void SetState(IN IMS_UINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    IMS_UINT32 m_nState;
    IMS_SINT32 m_nWifiState;
    IMS_SINT32 m_nWifiDetailedState;

    NetworkPolicy* m_pPolicy;
    // Network interface identifier when data connection is connected
    IMS_SINT32 m_nIfaceId;
    // Network interface name when data connection is connected
    AString m_strIfaceName;

    // Preferred local IP address's version
    IMS_SINT32 m_nPreferredIpVersion;
    // Configuration-based local IP address
    IPAddress m_objLocalAddress;
    IPAddress m_objLocalAddressIpv4;
    IPAddress m_objLocalAddressIpv6;
    AStringArray m_objPcscfsAddress;

    IMS_CONNECTION m_nConnectionHandle;

    IThread* m_piOwnerThread;
    INetworkConnectionListener* m_piConnectionListener;
    IMSList<INetworkConnectionListener*> m_objReferenceListeners;
};

#endif
