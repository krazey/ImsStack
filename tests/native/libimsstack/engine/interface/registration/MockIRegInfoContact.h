#ifndef MOCK_I_REG_INFO_CONTACT_H_
#define MOCK_I_REG_INFO_CONTACT_H_

#include <gmock/gmock.h>

#include "IMSMap.h"
#include "SipAddress.h"

#include "IRegInfoContact.h"

class MockIRegInfoContact : public IRegInfoContact
{
public:
    MOCK_METHOD(IMS_UINT32, GetCSeq, (), (const, override));
    MOCK_METHOD(const AString&, GetDisplayName, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEvent, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetExpiresValue, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetFirstCSeq, (), (const, override));
    MOCK_METHOD(const AString&, GetPublicGRUU, (), (const, override));
    MOCK_METHOD(const AString&, GetTemporaryGRUU, (), (const, override));
    MOCK_METHOD(const AString&, GetQValue, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetRetryAfterValue, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(
            const AString&, GetUnknownParameter, (IN CONST AString & strName), (const, override));
    MOCK_METHOD((const IMSMap<AString, AString>&), GetUnknownParameters, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetURI, (), (const, override));
};

#endif  // MOCK_I_REG_INFO_CONTACT_H_
