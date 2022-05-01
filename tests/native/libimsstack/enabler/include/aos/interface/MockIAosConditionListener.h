#ifndef MOCK_I_AOS_CONDITION_LISTENER_H_
#define MOCK_I_AOS_CONDITION_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosConditionListener.h"

class MockIAosConditionListener : public IAosConditionListener {
public:
    MOCK_METHOD(void, Condition_Changed, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, Condition_RequestCommand, (IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason),
            (override));
};

#endif // MOCK_I_AOS_CONDITION_LISTENER_H_
