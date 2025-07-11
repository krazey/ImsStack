/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_I_SIP_HEADER_H_
#define MOCK_I_SIP_HEADER_H_

#include <gmock/gmock.h>
#include "ISipHeader.h"
#include "SipAddress.h"
#include "SipParameter.h"

class MockISipHeader : public ISipHeader
{
public:
    inline MockISipHeader() {};
    inline virtual ~MockISipHeader() {};

    // ISipObject
    MOCK_METHOD(void, Destroy, (), (override));

    // ISipHeader
    MOCK_METHOD(ISipHeader*, Clone, (), (const, override));
    MOCK_METHOD(IMS_BOOL, Equals, (const ISipHeader* piHeader), (const, override));
    MOCK_METHOD(SipAddress*, GetSipAddress, (), (const, override));
    MOCK_METHOD(AString, GetHeaderValue, (), (const, override));
    MOCK_METHOD(const AString&, GetName, (), (const, override));
    MOCK_METHOD(const SipParameter*, GetParameter, (const AString& strName), (const, override));
    MOCK_METHOD(
            IMS_RESULT, GetParameterNames, (ImsList<AString> & objParamNames), (const, override));
    MOCK_METHOD(const ImsList<SipParameter*>&, GetParameters, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetType, (), (const, override));
    MOCK_METHOD(const AString&, GetValue, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetValueInt, (), (const, override));
    MOCK_METHOD(void, RemoveParameter, (const AString& strName), (override));
    MOCK_METHOD(void, SetName, (const AString& strName), (override));
    MOCK_METHOD(IMS_RESULT, SetParameter, (const AString& strName, const AString& strValue),
            (override));
    MOCK_METHOD(IMS_RESULT, SetHeaderValue, (const AString& strHeaderValue), (override));
    MOCK_METHOD(IMS_RESULT, SetValue, (const AString& strValue), (override));
    MOCK_METHOD(IMS_RESULT, SetValueInt, (IMS_SINT32 nValue), (override));
    MOCK_METHOD(AString, ToString, (), (const, override));
    MOCK_METHOD(AString, ToStringWithoutName, (), (const, override));
};

#endif  // MOCK_I_SIP_HEADER_H_
