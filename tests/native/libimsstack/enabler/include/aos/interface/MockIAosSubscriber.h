#ifndef MOCK_I_AOS_SUBSCRIBER_H_
#define MOCK_I_AOS_SUBSCRIBER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "AStringArray.h"
#include "interface/IAosSubscriber.h"

class MockIAosSubscriber : public IAosSubscriber
{
public:
    MOCK_METHOD(IMS_BOOL, IsReady, (), (const, override));
    MOCK_METHOD(void, SetListener, (IN IAosSubscriberListener * piListener), (override));
    MOCK_METHOD(const AStringArray&, GetConfiguredImpus, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetFakeImpus, (), (const, override));
    MOCK_METHOD(
            const ISubscriberConfig*, GetSubscriberConfig, (IMS_SINT32 nType), (const, override));
    MOCK_METHOD(IMS_BOOL, Init, (), (override));
    MOCK_METHOD(IMS_BOOL, CleanUp, (), (override));
};

#endif  // MOCK_I_AOS_SUBSCRIBER_H_
