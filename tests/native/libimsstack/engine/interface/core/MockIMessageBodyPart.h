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

#ifndef MOCK_I_MESSAGE_BODY_PART_H_
#define MOCK_I_MESSAGE_BODY_PART_H_

#include <gmock/gmock.h>

#include "IMessageBodyPart.h"

class MockIMessageBodyPart : public IMessageBodyPart
{
public:
    ~MockIMessageBodyPart() override = default;

    MOCK_METHOD(const ByteArray&, GetContent, (), (const, override));
    MOCK_METHOD(AString, GetHeader, (IN const AString& strName), (const, override));
    MOCK_METHOD(IMS_RESULT, SetContent, (IN const ByteArray& objContent), (override));
    MOCK_METHOD(IMS_RESULT, SetHeader, (IN const AString& strName, IN const AString& strValue),
            (override));
};

#endif  // MOCK_I_MESSAGE_BODY_PART_H_
