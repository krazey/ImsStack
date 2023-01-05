/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MOCK_I_REG_INFO_CONTACT_H_
#define MOCK_I_REG_INFO_CONTACT_H_

#include <gmock/gmock.h>

#include "ImsMap.h"
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
    MOCK_METHOD(const AString&, GetPublicGruu, (), (const, override));
    MOCK_METHOD(const AString&, GetTemporaryGruu, (), (const, override));
    MOCK_METHOD(const AString&, GetQValue, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetRetryAfterValue, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(
            const AString&, GetUnknownParameter, (IN const AString& strName), (const, override));
    MOCK_METHOD((const IMSMap<AString, AString>&), GetUnknownParameters, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetUri, (), (const, override));
};

#endif  // MOCK_I_REG_INFO_CONTACT_H_
