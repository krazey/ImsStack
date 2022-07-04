#ifndef MOCK_I_MTC_SIP_INTERFACE_FACTORY_H_
#define MOCK_I_MTC_SIP_INTERFACE_FACTORY_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "IMtcSipInterfaceFactory.h"

class SessionInterfaceHolder;
class ReferenceInterfaceHolder;
class SubscriptionInterfaceHolder;

class MockIMtcSipInterfaceFactory : public IMtcSipInterfaceFactory
{
public:
    ~MockIMtcSipInterfaceFactory() {}
    MOCK_METHOD(SessionInterfaceHolder*, GetISessionHolder, (), (override));
    MOCK_METHOD(ReferenceInterfaceHolder*, GetIReferenceHolder, (), (override));
    MOCK_METHOD(SubscriptionInterfaceHolder*, GetISubscriptionHolder, (), (override));
};

#endif
