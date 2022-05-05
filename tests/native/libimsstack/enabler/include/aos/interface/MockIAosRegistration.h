#ifndef MOCK_I_AOS_REGISTRATION_H_
#define MOCK_I_AOS_REGISTRATION_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosRegistration.h"

class MockIAosRegistration : public IAosRegistration {
public:
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(void, Update, (IN IMS_BOOL bIgnoreRetryTimer, IN IMS_BOOL bExplicitUpdate),
            (override));
    MOCK_METHOD(void, Reconfig, (), (override));
    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosRegistrationListener* piRegListener), (override));
    MOCK_METHOD(void, RequestCmd, (IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(IMS_UINT32, GetMode, (), (override));
    MOCK_METHOD(IMS_UINT32, GetProperty, (IN IMS_UINT32 nType, OUT IMS_UINT32& nValue,
            OUT AString& strValue), (override));
    MOCK_METHOD(IMS_UINT32, GetState, (), (override));
    MOCK_METHOD(AosRegistrationType, GetRegType, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRegistered, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRefreshing, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRetryTimer, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRetryHeld, (), (override));
    MOCK_METHOD(IMS_BOOL, IsTerminated, (), (override));
    MOCK_METHOD(void, SetAppReady, (IN IMS_BOOL bReady), (override));

    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

#endif // MOCK_I_AOS_REGISTRATION_H_
