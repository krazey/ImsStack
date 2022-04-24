/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090305  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_NET_CONNECTION_H_
#define _INTERFACE_IMS_NET_CONNECTION_H_

#include "IPAddress.h"
#include "AStringArray.h"
#include "IIpcan.h"
#include "INetworkPing.h"

class INetworkConnectionListener;
class AccessNetworkInfo;

// Default network id - system default id
#define IMS_NET_IFACE_DEFAULT_ID        0
// Default network id - invalid id
#define IMS_NET_IFACE_INVALID_ID        (-1)

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
        RESULT_DONE = 0,    // ACTIVE, TERMINATED
        RESULT_DOING,       // Activating, Terminating
        RESULT_FAILED       // Error - IDLE
    };

public:
    // bEnableApn (true) : Applied for ims / internet APN; other APNs behave as on-demand.
    virtual RESULT_ENTYPE Activate(IN IMS_BOOL bEnableApn = IMS_FALSE,
            IN IMS_SINT32 nIpcanCategory = IIpcan::CATEGORY_MOBILE) = 0;
    // bDisableApn (true) : Applied for ims / internet APN; other APNs behave as on-demand.
    virtual RESULT_ENTYPE Deactivate(IN IMS_BOOL bDisableApn = IMS_FALSE,
            IN IMS_SINT32 nIpcanCategory = IIpcan::CATEGORY_MOBILE) = 0;
    virtual void GetAccessNetworkInfo(OUT AccessNetworkInfo &objAccessNetInfo) = 0;
    virtual void GetLastAccessNetworkInfo(OUT AccessNetworkInfo &objAccessNetInfo,
            OUT AString &strTimeStamp, OUT AString &strCellInfoAge) = 0;
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
    virtual IMS_BOOL GetExtraInfo(IN const AString &strType, OUT AString &strInfo) = 0;
    virtual IMS_SINT32 GetHostByName(IN const AString &strHostName,
            OUT IMSList<IPAddress> &objIPs,
            IN IMS_SINT32 nIPVersion = 0 /* default-local-address-based */) = 0;
    virtual IMS_SINT32 GetIfaceId() const = 0;
    virtual const AString& GetIfaceName() const = 0;
    // 0 - configuration-based cached address
    // IP version : refer to IPAddress class
    //         4 - IPv4 cached address
    //         6 - IPv6 cached address
    virtual const IPAddress& GetLocalAddress(
            IN IMS_SINT32 nIPVersion = 0 /* configuration-based */) const = 0;
    // IP version : refer to IPAddress class
    virtual const AStringArray& GetPcscfAddress(
            IN IMS_SINT32 nIPVersion = 0 /* configuration-based */) = 0;
    virtual STATE_ENTYPE GetState() const = 0;
    virtual IMS_BOOL IsConnected(IN IMS_SINT32 nCategory = IIpcan::CATEGORY_ANY) const = 0;
    virtual IMS_BOOL SendPingToHostAddress(IN const IPAddress &objHostAddress) = 0;
    virtual IMS_BOOL IsePDGEnabled() const = 0;
    virtual IMS_BOOL IsMobileDataEnabled() const = 0;
    virtual IMS_SINT32 GetMtu() const = 0;
    virtual void SetListener(IN INetworkConnectionListener* piListener) = 0;
    // 0 : configuration-based
    // 4 : IPv4 local address preferred on dual IP
    // 6 : IPv6 local address preferred on dual IP
    virtual void SetPreferredIPVersion(
            IN IMS_SINT32 nPreferredIPVersion = 0 /* default-aos-connection-profile */) = 0;

    virtual INetworkPing* CreatePing() = 0;

    //// For test environment - this method is provided to share the INetworkConnection.
    virtual void AddReferenceListener(IN INetworkConnectionListener* piListener) = 0;
    virtual void RemoveReferenceListener(IN INetworkConnectionListener* piListener) = 0;
};

class INetworkConnectionListener
{
public:
    virtual void NetworkConnection_OnConnected(IN INetworkConnection* piConnection) = 0;
    virtual void NetworkConnection_OnDisconnected(IN INetworkConnection* piConnection,
            IN IMS_SINT32 nErrorCode) = 0;
    virtual void NetworkConnection_OnConnectionFailed(IN INetworkConnection* piConnection,
            IN IMS_SINT32 nErrorCode) = 0;
    virtual void NetworkConnection_OnIpChanged(IN INetworkConnection* piConnection) = 0;
    virtual void NetworkConnection_OnIpcanChanged(IN INetworkConnection* piConnection) = 0;
    virtual void NetworkConnection_OnPcscfChanged(IN INetworkConnection* piConnection) = 0;
};

#endif // _INTERFACE_IMS_NET_CONNECTION_H_
