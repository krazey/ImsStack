#ifndef MOCK_I_AOS_SUBSCRIBER_MANAGER_LISTENER_H_
#define MOCK_I_AOS_SUBSCRIBER_MANAGER_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosSubscriberManagerListener.h"

class MockIAosSubscriberManagerListener : public IAosSubscriberManagerListener {
public:
    MOCK_METHOD(void, AosSubscriberManager_NotifyState, (IMS_UINT32 nState), (override));
};

#endif // MOCK_I_AOS_SUBSCRIBER_MANAGER_LISTENER_H_
