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

#ifndef MOCK_I_MTC_EXTENSION_
#define MOCK_I_MTC_EXTENSION_

#include "ImsTypeDef.h"
#include "call/extension/IMtcExtension.h"
#include <gmock/gmock.h>

class AString;

class MockIMtcExtension : public IMtcExtension
{
public:
    MOCK_METHOD(IMtcExtension*, Clone, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAvailableOnRemote, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredOnRemote, (), (const, override));
    MOCK_METHOD(const AString&, GetOptionTag, (), (const, override));

    MOCK_METHOD(
            void, FormatRequest, (IN RequestType eType, IN_OUT IMessage& objRequest), (override));
    MOCK_METHOD(void, FormatResponse, (IN ResponseType eType, IN_OUT IMessage& objResponse),
            (override));
    MOCK_METHOD(
            void, HandleRequest, (IN RequestType eType, IN const IMessage& objRequest), (override));
    MOCK_METHOD(void, HandleResponse, (IN ResponseType eType, IN const IMessage& objResponse),
            (override));
};

#endif
