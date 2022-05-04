#ifndef MOCK_I_AOS_CONNECTION_H_
#define MOCK_I_AOS_CONNECTION_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "AStringArray.h"
#include "IPAddress.h"
#include "interface/IAosConnection.h"

class MockIAosConnection : public IAosConnection {
public:
    MOCK_METHOD(IMS_BOOL, Activate, (), (override));
    MOCK_METHOD(void, Deactivate, (), (override));
    MOCK_METHOD(IMS_UINT32, GetState, (), (override));
    MOCK_METHOD(IMS_SINT32, GetConnectionType, (), (override));
    MOCK_METHOD(IMS_SINT32, GetPreferredIpVersion, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosConnectionListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosConnectionListener* piListener), (override));
    MOCK_METHOD(IMS_SINT32, GetMtu, (), (override));
    MOCK_METHOD(const IPAddress&, GetLocalAddress, (IN IMS_SINT32 nIpVersion), (override));
    MOCK_METHOD(const AStringArray&, GetPcscfAddress, (IN IMS_SINT32 nIpVersion), (override));
    MOCK_METHOD(IMS_SINT32, GetHostByName, (IN const AString& strHostName,
            OUT IMSList<IPAddress>& objIps, IN IMS_SINT32 nIpVersion), (override));
    MOCK_METHOD(const AString&, GetIfaceName, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEpdgEnabled, (), (override));
    MOCK_METHOD(IMS_SINT32, GetIpcanCategory, (), (override));
    MOCK_METHOD(IMS_BOOL, SendPingToHostAddress, (IN const IPAddress& objHostAddress), (override));
};

#endif // MOCK_I_AOS_CONNECTION_H_
