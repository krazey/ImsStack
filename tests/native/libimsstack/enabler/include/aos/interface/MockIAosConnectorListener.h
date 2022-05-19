#ifndef MOCK_I_AOS_CONNECTOR_LISTENER_H_
#define MOCK_I_AOS_CONNECTOR_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"

#include "interface/IAosConnectorListener.h"

class MockIAosConnectorListener : public IAosConnectorListener
{
public:
    MOCK_METHOD(void, Connector_Activated, (), (override));
    MOCK_METHOD(void, Connector_Deactivated, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, Connector_Updated, (IN IMS_UINT32 nReason), (override));
};

#endif  // MOCK_I_AOS_CONNECTOR_LISTENER_H_
