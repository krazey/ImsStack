/**
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

#ifndef MOCK_IMS_IDENTITY_PROXY_H_
#define MOCK_IMS_IDENTITY_PROXY_H_

#include "ImsTypeDef.h"
#include "dialingplan/ImsIdentityProxy.h"
#include <gmock/gmock.h>

class AccessNetworkInfo;

class MockImsIdentityProxy : public ImsIdentityProxy
{
public:
    explicit MockImsIdentityProxy() :
            ImsIdentityProxy()
    {
    }
    ~MockImsIdentityProxy() {}

    MOCK_METHOD(AString, CreateSipUserId,
            (IN const AString&, IN IMS_SINT32, IN IMS_BOOL, IN const AString&), (const, override));
    MOCK_METHOD(AString, CreateSipUserIdWithDialString,
            (IN const AString&, IN IMS_SINT32, IN const AString&), (const, override));
    MOCK_METHOD(AString, CreateSipUserIdWithPhone,
            (IN const AString&, IN IMS_SINT32, IN const AString&), (const, override));
    MOCK_METHOD(const AString, GetPhoneContext,
            (IN IMS_SINT32, IN IMS_SINT32, IN AccessNetworkInfo*, IN const AString&),
            (const, override));
    MOCK_METHOD(const AString, GetHomeDomainName, (IN IMS_SINT32), (const, override));
};

#endif
