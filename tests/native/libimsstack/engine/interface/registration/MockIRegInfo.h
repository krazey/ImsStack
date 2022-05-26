#ifndef MOCK_I_REG_INFO_H_
#define MOCK_I_REG_INFO_H_

#include <gmock/gmock.h>

#include "IRegInfoRegistration.h"

#include "IRegInfo.h"

class MockIRegInfo : public IRegInfo
{
public:
    MOCK_METHOD(
            IRegInfoRegistration*, GetRegistration, (IN CONST AString & strAOR), (const, override));
    MOCK_METHOD(IRegInfoRegistration*, GetRegistration, (IN CONST SipAddress & objAOR),
            (const, override));
    MOCK_METHOD(IMSList<IRegInfoRegistration*>, GetRegistrations, (), (const, override));
};

#endif  // MOCK_I_REG_INFO_H_
