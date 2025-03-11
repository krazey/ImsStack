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

#ifndef MOCK_MEDIA_PROFILE_FACTORY_H_
#define MOCK_MEDIA_PROFILE_FACTORY_H_

#include <gmock/gmock.h>

#include <MediaProfileFactory.h>

class MockMediaProfileFactory : public MediaProfileFactory
{
public:
    MockMediaProfileFactory() {}
    ~MockMediaProfileFactory() {}
    MOCK_METHOD(MediaBaseProfile*, CreateProfile,
            (IN MEDIA_CONTENT_TYPE eType, IN MediaBaseProfile* pProfile), (override));
    MOCK_METHOD(void, DeleteProfile, (IN MediaBaseProfile * pProfile), (override));
    MOCK_METHOD(MediaBaseProfile::BasePayload*, CreatePayload, (IN MEDIA_CONTENT_TYPE eType),
            (override));
    MOCK_METHOD(MediaBaseProfile::BasePayload*, CreatePayload,
            (IN MediaBaseProfile::BasePayload * payload), (override));
    MOCK_METHOD(void, DeletePayload, (IN MediaBaseProfile::BasePayload * pPayload), (override));
};

#endif
