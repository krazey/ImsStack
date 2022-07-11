#ifndef MOCK_I_NETWORK_CONNECTION_H_
#define MOCK_I_NETWORK_CONNECTION_H_

#include <gmock/gmock.h>

#include "AStringArray.h"
#include "IIpcan.h"
#include "INetworkPing.h"
#include "IpAddress.h"

#include "INetworkConnection.h"

class MockINetworkConnection : public INetworkConnection
{
public:
    MOCK_METHOD(RESULT_ENTYPE, Activate, (IN IMS_BOOL bEnableApn), (override));
    MOCK_METHOD(RESULT_ENTYPE, Deactivate, (IN IMS_BOOL bDisableApn), (override));
    MOCK_METHOD(void, GetAccessNetworkInfo, (OUT AccessNetworkInfo & objAccessNetInfo), (override));
    MOCK_METHOD(void, GetLastAccessNetworkInfo,
            (OUT AccessNetworkInfo & objAccessNetInfo, OUT AString& strTimestamp,
                    OUT AString& strCellInfoAge),
            (override));
    MOCK_METHOD(
            IMS_BOOL, GetExtraInfo, (IN const AString& strType, OUT AString& strInfo), (override));
    MOCK_METHOD(IMS_SINT32, GetHostByName,
            (IN const AString& strHostName, OUT IMSList<IPAddress>& objIpAddrs,
                    IN IMS_SINT32 nIpVersion),
            (override));
    MOCK_METHOD(IMS_SINT32, GetIfaceId, (), (const, override));
    MOCK_METHOD(const AString&, GetIfaceName, (), (const, override));
    MOCK_METHOD(const IPAddress&, GetLocalAddress, (IN IMS_SINT32 nIpVersion), (const, override));
    MOCK_METHOD(const AStringArray&, GetPcscfAddress, (IN IMS_SINT32 nIpVersion), (override));
    MOCK_METHOD(STATE_ENTYPE, GetState, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsConnected, (IN IMS_SINT32 nCategory), (const, override));
    MOCK_METHOD(IMS_BOOL, IsePDGEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsMobileDataEnabled, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMtu, (), (const, override));
    MOCK_METHOD(void, SetListener, (IN INetworkConnectionListener * piListener), (override));
    MOCK_METHOD(void, SetPreferredIpVersion, (IN IMS_SINT32 nPreferredIpVersion), (override));
    MOCK_METHOD(INetworkPing*, CreatePing, (), (override));
    MOCK_METHOD(
            void, AddReferenceListener, (IN INetworkConnectionListener * piListener), (override));
    MOCK_METHOD(void, RemoveReferenceListener, (IN INetworkConnectionListener * piListener),
            (override));
};

class MockINetworkConnectionListener : public INetworkConnectionListener
{
public:
    MOCK_METHOD(void, NetworkConnection_OnConnected, (IN INetworkConnection * piConnection),
            (override));
    MOCK_METHOD(void, NetworkConnection_OnDisconnected,
            (IN INetworkConnection * piConnection, IN IMS_SINT32 nErrorCode), (override));
    MOCK_METHOD(void, NetworkConnection_OnConnectionFailed,
            (IN INetworkConnection * piConnection, IN IMS_SINT32 nErrorCode), (override));
    MOCK_METHOD(void, NetworkConnection_OnIpChanged, (IN INetworkConnection * piConnection),
            (override));
    MOCK_METHOD(void, NetworkConnection_OnIpcanChanged, (IN INetworkConnection * piConnection),
            (override));
    MOCK_METHOD(void, NetworkConnection_OnPcscfChanged, (IN INetworkConnection * piConnection),
            (override));
};

#endif  // MOCK_I_NETWORK_CONNECTION_H_
