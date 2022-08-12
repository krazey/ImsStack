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

#ifndef MOCK_I_SIP_MESSAGE_BODY_PART_H_
#define MOCK_I_SIP_MESSAGE_BODY_PART_H_

#include <gmock/gmock.h>

#include "ISipMessageBodyPart.h"

class MockISipMessageBodyPart : public ISipMessageBodyPart
{
public:
    inline MockISipMessageBodyPart() {}
    inline virtual ~MockISipMessageBodyPart() {}

    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(ISipMessageBodyPart*, Clone, (), (const, override));
    MOCK_METHOD(void, CopyFrom, (IN const ISipMessageBodyPart* piBodyPart), (override));
    MOCK_METHOD(AString, GetHeader, (IN IMS_SINT32 nType, IN const AString& strName),
            (const, override));
    MOCK_METHOD(void, SetHeader,
            (IN IMS_SINT32 nType, IN const AString& strValue, IN const AString& strName),
            (override));
    MOCK_METHOD(const ByteArray&, GetContent, (), (const, override));
    MOCK_METHOD(void, SetContent, (IN const ByteArray& objContent), (override));
};

#endif
