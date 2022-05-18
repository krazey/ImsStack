#ifndef MOCK_I_AOS_SUBSCRIPTION_LISTENER_H_
#define MOCK_I_AOS_SUBSCRIPTION_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"

#include "interface/IAosSubscriptionListener.h"

class MockIAosSubscriptionListener : public IAosSubscriptionListener
{
public:
    MOCK_METHOD(void, Subscription_StateChanged, (IN IMS_SINT32 nState, IN IMS_SINT32 nReason),
            (override));
    MOCK_METHOD(IMS_BOOL, Subscription_CanBeTransmitted, (), (override));
    MOCK_METHOD(void, Subscription_NotifyReceived, (IN IMS_SINT32 nEvent), (override));
    MOCK_METHOD(void, Subscription_Request, (IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter),
            (override));
};

#endif  // MOCK_I_AOS_SUBSCRIPTION_LISTENER_H_
