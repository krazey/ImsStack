#ifndef MOCK_I_AOS_CONNECTION_LISTENER_H_
#define MOCK_I_AOS_CONNECTION_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"

#include "interface/IAosConnectionListener.h"

class MockIAosConnectionListener : public IAosConnectionListener
{
public:
    MOCK_METHOD(void, AosConnection_StateChanged, (IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, AosConnection_IpChanged, (), (override));
    MOCK_METHOD(void, AosConnection_IpcanCatChanged, (), (override));
    MOCK_METHOD(void, AosConnection_PcscfChanged, (), (override));
    MOCK_METHOD(void, AosConnection_ConnectionFailed, (), (override));
};

#endif  // MOCK_I_AOS_CONNECTION_LISTENER_H_
