#ifndef MOCK_I_REG_SUBSCRIPTION_H_
#define MOCK_I_REG_SUBSCRIPTION_H_

#include <gmock/gmock.h>

#include "IRegBase.h"
#include "IRegInfo.h"
class IRegSubscriptionListener;

#include "IRegSubscription.h"

class MockIRegSubscription : public IRegSubscription
{
public:
    MOCK_METHOD(ISipMessage*, GetNextRequest, (), (override));
    MOCK_METHOD(ISipMessage*, GetPreviousRequest, (), (const, override));
    MOCK_METHOD(ISipMessage*, GetPreviousResponse, (), (const, override));
    MOCK_METHOD(void, SetSipMessageMediator, (IN IMessageMediator * piMediator), (override));

    MOCK_METHOD(void, DestroyEx, (), (override));
    MOCK_METHOD(IMS_SINT32, DisableFeatures, (IN IMS_SINT32 nFeatures), (override));
    MOCK_METHOD(IMS_SINT32, EnableFeatures, (IN IMS_SINT32 nFeatures), (override));
    MOCK_METHOD(IMS_UINT32, GetExpires, (), (const, override));
    MOCK_METHOD(const IRegInfo*, GetRegInfo, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_RESULT, SetContactParameter,
            (IN const AString& strParameter, IN IMS_SINT32 nOperation), (override));
    MOCK_METHOD(void, SetExpires, (IN IMS_UINT32 nExpires), (override));
    MOCK_METHOD(void, SetListener, (IN IRegSubscriptionListener * piListener), (override));
    MOCK_METHOD(void, SetRefreshPolicy,
            (IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT,
                    IN IMS_SINT32 nValueGT),
            (override));
    MOCK_METHOD(IMS_RESULT, Subscribe, (), (override));
    MOCK_METHOD(IMS_RESULT, Unsubscribe, (), (override));
};

#endif  // MOCK_I_REG_SUBSCRIPTION_H_
