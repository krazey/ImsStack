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

#ifndef MOCK_I_CONFERENCE_REFERENCE_H_
#define MOCK_I_CONFERENCE_REFERENCE_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "conferencecall/IConferenceReference.h"
#include <gmock/gmock.h>

class CallConnectionIdManager;

class MockIConferenceReference : public IConferenceReference
{
public:
    MOCK_METHOD(IMS_RESULT, SendInvite,
            (OUT AString& strReferToUri, IN CallConnectionIdManager& objConnectionIdManager),
            (override));
    MOCK_METHOD(IMS_RESULT, SendBye, (IN AString strInvitedUri), (override));
    MOCK_METHOD(IMS_UINT32, GetType, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetResponseCode, (), (const, override));
    MOCK_METHOD(void, SetForceToTerminateInterface, (IN IMS_BOOL bTerminate), (override));
};

#endif
