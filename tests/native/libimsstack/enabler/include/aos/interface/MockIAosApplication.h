#ifndef MOCK_I_AOS_APPLICATION_H_
#define MOCK_I_AOS_APPLICATION_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"

#include "interface/IAosApplication.h"

class MockIAosApplication : public IAosApplication
{
public:
    MOCK_METHOD(void, Reconfig, (), (override));
    MOCK_METHOD(IMS_BOOL, RequestCmd, (IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(const AString&, GetActivityName, (), (override));
    MOCK_METHOD(void, GetProperty,
            (IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue), (override));
    MOCK_METHOD(IMS_UINT32, GetAppState, (), (override));
    MOCK_METHOD(IMS_UINT32, GetOffReason, (), (override));
    MOCK_METHOD(IMS_BOOL, IsActivated, (), (override));
    MOCK_METHOD(IMS_BOOL, IsOn, (), (override));
    MOCK_METHOD(void, SetActivation, (IN IMS_BOOL bActivation), (override));
    MOCK_METHOD(void, NotifyPublishState, (IN IMS_BOOL bStart), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

#endif  // MOCK_I_AOS_APPLICATION_H_
