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

#ifndef MOCK_CALL_CONNECTION_ID_MANAGER_H_
#define MOCK_CALL_CONNECTION_ID_MANAGER_H_

#include "ImsTypeDef.h"
#include "call/CallConnectionIdManager.h"
#include <gmock/gmock.h>

class IMtcContext;

class MockCallConnectionIdManager : public CallConnectionIdManager
{
public:
    explicit MockCallConnectionIdManager(IN IMtcContext& objContext) :
            CallConnectionIdManager(objContext)
    {
    }
    ~MockCallConnectionIdManager() {}
    MOCK_METHOD(
            void, OnConferenceParticipantDisconnected, (IN IMS_UINT32 nConnectionId), (override));
    MOCK_METHOD(CallKey, GetCallKey, (IN IMS_UINT32 nConnectionId), (const, override));
};

#endif
