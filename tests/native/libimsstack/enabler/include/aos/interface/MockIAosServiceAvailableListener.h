#ifndef MOCK_I_AOS_SERVICE_AVAILABLE_LISTENER_H_
#define MOCK_I_AOS_SERVICE_AVAILABLE_LISTENER_H_

#include <gmock/gmock.h>

#include "interface/IAosServiceAvailableListener.h"

class MockIAosServiceAvailableListener : public IAosServiceAvailableListener {
public:
    MOCK_METHOD(void, ServiceAvailable_Changed, (), (override));
    MOCK_METHOD(void, ServiceAvailable_RequestCommand,
            (IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason), (override));
};

#endif // MOCK_I_AOS_SERVICE_AVAILABLE_LISTENER_H_
