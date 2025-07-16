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

#ifndef MOCK_MEDIA_MANAGER_H_
#define MOCK_MEDIA_MANAGER_H_

#include <gmock/gmock.h>

#include "media/MediaManager.h"

class MockMediaManager : public MediaManager
{
public:
    // The constructor needs to call the protected base class constructor.
    MockMediaManager() :
            MediaManager("MockMediaManager", 0)
    {
    }
    explicit MockMediaManager(IN const AString& strName, IN IMS_SINT32 nSlotId) :
            MediaManager(strName, nSlotId)
    {
    }
    ~MockMediaManager() override = default;

    MOCK_METHOD(IMediaSession*, CreateSession,
            (MEDIA_NETWORK_TYPE, MEDIA_SERVICE_TYPE, IService*, IMS_SINTP), (override));
    MOCK_METHOD(void, DestroySession, (const IMediaSession*), (override));
    MOCK_METHOD(IMS_BOOL, SendMessage, (IMS_SINT32, IMS_SINTP, IMS_UINTP), (override));
    MOCK_METHOD(
            IMS_BOOL, HandleRequestMsg, (IMS_SINT32, IMS_SINTP, ImsMediaMsgParamBase*), (override));
};

#endif  // MOCK_MEDIA_MANAGER_H_
