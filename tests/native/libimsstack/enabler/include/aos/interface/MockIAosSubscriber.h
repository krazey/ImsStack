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

#ifndef MOCK_I_AOS_SUBSCRIBER_H_
#define MOCK_I_AOS_SUBSCRIBER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "AStringArray.h"
#include "interface/IAosSubscriber.h"

class MockIAosSubscriber : public IAosSubscriber
{
public:
    MOCK_METHOD(IMS_BOOL, IsReady, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIsim, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUsim, (), (const, override));
    MOCK_METHOD(void, SetListener, (IN IAosSubscriberListener * piListener), (override));
    MOCK_METHOD(const AStringArray&, GetConfiguredImpus, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetFakeImpus, (), (const, override));
    MOCK_METHOD(
            const ISubscriberConfig*, GetSubscriberConfig, (IMS_SINT32 nType), (const, override));
    MOCK_METHOD(void, CreateTemporaryPublicUserIdForGiba, (), (override));
    MOCK_METHOD(void, ClearTemporaryPublicUserIdForGiba, (), (override));
    MOCK_METHOD(IMS_BOOL, HasValidTemporaryPublicUserIdForGiba, (), (const override));
    MOCK_METHOD(const AString&, GetTemporaryPublicUserIdForGiba, (), (const override));
    MOCK_METHOD(IMS_BOOL, Init, (), (override));
    MOCK_METHOD(IMS_BOOL, CleanUp, (), (override));
};

#endif  // MOCK_I_AOS_SUBSCRIBER_H_
