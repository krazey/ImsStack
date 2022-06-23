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
#ifndef INTERFACE_NETWORK_CONNECTION_H_
#define INTERFACE_NETWORK_CONNECTION_H_

#include "AStringArray.h"
#include "IIpcan.h"
#include "INetworkPing.h"
#include "IpAddress.h"

class INetworkConnectionListener;
class AccessNetworkInfo;

// Default network id - system default id
#define IMS_NET_IFACE_DEFAULT_ID 0
// Default network id - invalid id
#define IMS_NET_IFACE_INVALID_ID (-1)

// All the listener methods will be invoked in the caller's thread execution flow.
// Let's share the network connection using the profile name such as ...
class INetworkConnection
{
public:
    enum STATE_ENTYPE
    {
        STATE_CONNECTED = 0,
        STATE_DISCONNECTED,

    };

    enum RESULT_ENTYPE
    {
        RESULT_DONE = 0,  // ACTIVE, TERMINATED
        RESULT_DOING,     // Activating, Terminating
        RESULT_FAILED     // Error - IDLE
    };

public:
    // bEnableApn (true) : Applied for ims / internet APN; other APNs behave as on-demand.
    virtual RESULT_ENTYPE Activate(IN IMS_BOOL bEnableApn = IMS_FALSE,
            IN IMS_SINT32 nIpcanCategory = IIpcan::CATEGORY_MOBILE) = 0;
    // bDisableApn (true) : Applied for ims / internet APN; other APNs behave as on-demand.
    virtual RESULT_ENTYPE Deactivate(IN IMS_BOOL bDisableApn = IMS_FALSE,
            IN IMS_SINT32 nIpcanCategory = IIpcan::CATEGORY_MOBILE) = 0;
    virtual void GetAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo) = 0;
    virtual void GetLastAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo,
            OUT AString& strTimestamp, OUT AString& strCellInfoAge) = 0;
    // Pre-defined types
    // - Common
    //      "rat" : "4G"/"3G"/"2G"/"WiFi"/"Unknown"
    //      "policy_name" : name of data connection profile
    //
    // - Mobile
    //      "apn" : apn host when the data connection is connected
    //
    // - WiFi
    //      "mac_address" : MAC address (BSSID, 00:00:00:00:00:00)
    //      "ssid" : SSID (string)
    //
    virtual IMS_BOOL GetExtraInfo(IN const AString& strType, OUT AString& strInfo) = 0;
    virtual IMS_SINT32 GetHostByName(IN const AString& strHostName,
            OUT IMSList<IPAddress>& objIpAddrs,
            IN IMS_SINT32 nIpVersion = 0 /* default-local-address-based */) = 0;
    virtual IMS_SINT32 GetIfaceId() const = 0;
    virtual const AString& GetIfaceName() const = 0;
    // 0 - configuration-based cached address
    // IP version : refer to IPAddress class
    //         4 - IPv4 cached address
    //         6 - IPv6 cached address
    virtual const IPAddress& GetLocalAddress(
            IN IMS_SINT32 nIpVersion = 0 /* configuration-based */) const = 0;
    // IP version : refer to IPAddress class
    virtual const AStringArray& GetPcscfAddress(
            IN IMS_SINT32 nIpVersion = 0 /* configuration-based */) = 0;
    virtual STATE_ENTYPE GetState() const = 0;
    virtual IMS_BOOL IsConnected(IN IMS_SINT32 nCategory = IIpcan::CATEGORY_ANY) const = 0;
    virtual IMS_BOOL SendPingToHostAddress(IN const IPAddress& objHostAddress) = 0;
    virtual IMS_BOOL IsePDGEnabled() const = 0;
    virtual IMS_BOOL IsMobileDataEnabled() const = 0;
    virtual IMS_SINT32 GetMtu() const = 0;
    virtual void SetListener(IN INetworkConnectionListener* piListener) = 0;
    // 0 : configuration-based
    // 4 : IPv4 local address preferred on dual IP
    // 6 : IPv6 local address preferred on dual IP
    virtual void SetPreferredIpVersion(
            IN IMS_SINT32 nPreferredIpVersion = 0 /* default-aos-connection-profile */) = 0;

    virtual INetworkPing* CreatePing() = 0;

    //// For test environment - this method is provided to share the INetworkConnection.
    virtual void AddReferenceListener(IN INetworkConnectionListener* piListener) = 0;
    virtual void RemoveReferenceListener(IN INetworkConnectionListener* piListener) = 0;
};

class INetworkConnectionListener
{
public:
    virtual void NetworkConnection_OnConnected(IN INetworkConnection* piConnection) = 0;
    virtual void NetworkConnection_OnDisconnected(
            IN INetworkConnection* piConnection, IN IMS_SINT32 nErrorCode) = 0;
    virtual void NetworkConnection_OnConnectionFailed(
            IN INetworkConnection* piConnection, IN IMS_SINT32 nErrorCode) = 0;
    virtual void NetworkConnection_OnIpChanged(IN INetworkConnection* piConnection) = 0;
    virtual void NetworkConnection_OnIpcanChanged(IN INetworkConnection* piConnection) = 0;
    virtual void NetworkConnection_OnPcscfChanged(IN INetworkConnection* piConnection) = 0;
};

#endif
