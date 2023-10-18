/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_I_SIPCONTROLLER_H_
#define MOCK_I_SIPCONTROLLER_H_

#include <gmock/gmock.h>
#include "ISipControllerService.h"
class AString;

class MockISipControllerService : public ISipControllerService
{
public:
    virtual ~MockISipControllerService() {}
    MOCK_METHOD(void, UpdateDelegateRegistration, (IMS_UINTP), (override));
    MOCK_METHOD(void, TriggerDelegateDeregistration, (), (override));
    MOCK_METHOD(void, OpenMessageTracker, (const AString&), (override));
    MOCK_METHOD(void, SendMessage, (IMS_UINTP), (override));
    MOCK_METHOD(void, NotifyMessageReceiveError, (IMS_UINTP), (override));
    MOCK_METHOD(void, CloseSession, (const AString&), (override));

    // IEnablerService
    MOCK_METHOD(void, NotifyJniEnablerSet, (), (override));
};
#endif  // MOCK_I_SIPCONTROLLER_H_
