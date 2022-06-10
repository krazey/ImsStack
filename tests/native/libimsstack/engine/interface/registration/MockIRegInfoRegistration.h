#ifndef MOCK_I_REG_INFO_REGISTRATION_H_
#define MOCK_I_REG_INFO_REGISTRATION_H_

#include <gmock/gmock.h>

#include "IRegInfoContact.h"

#include "IRegInfoRegistration.h"

class MockIRegInfoRegistration : public IRegInfoRegistration
{
public:
    MOCK_METHOD(const SipAddress&, GetAor, (), (const, override));
    MOCK_METHOD(
            IRegInfoContact*, GetContact, (IN CONST SipAddress & objContactUri), (const, override));
    MOCK_METHOD(IMSList<IRegInfoContact*>, GetContacts, (), (const, override));
    MOCK_METHOD(IRegInfoContact*, GetPriorContact, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
};

#endif  // MOCK_I_REG_INFO_REGISTRATION_H_
