#ifndef MOCK_I_AOS_SUBSCRIBER_LISTENER_H_
#define MOCK_I_AOS_SUBSCRIBER_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosSubscriberListener.h"

class MockIAosSubscriberListener : public IAosSubscriberListener {
public:
    MOCK_METHOD(void, Subscriber_StateChanged, (IN IMS_UINT32 nState, IN IMS_UINT32 nParam),
            (override));
};

#endif // MOCK_I_AOS_SUBSCRIBER_LISTENER_H_
