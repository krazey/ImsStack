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

#ifndef MOCK_I_UCE_JNI_H_
#define MOCK_I_UCE_JNI_H_

#include <gmock/gmock.h>
#include "IUceJni.h"
class AString;

class MockIUceJni : public IUceJni
{
public:
    virtual ~MockIUceJni() {}
    MOCK_METHOD(void, SendPublishCmd,
            (IMS_UINT32, IMS_UINT32, IMS_UINT32, const AString&, const AString&), (override));
    MOCK_METHOD(void, SendSingleSubscribeCmd, (IMS_UINT32, const AString&), (override));
    MOCK_METHOD(void, SendListSubscribeCmd, (IMS_UINT32, const IMSList<AString>&), (override));
    MOCK_METHOD(void, SendOptionsCmd, (IMS_UINT32 key, IMS_UINT32 myCaps, const AString& remoteUri),
            (override));
    MOCK_METHOD(void, SendOptionsRespCmd,
            (IMS_UINT32 key, IMS_SINT32 responseCode, const AString& reason, IMS_UINT32 myCaps),
            (override));
    MOCK_METHOD(void, ImsRegistrationCheck, (), (override));

    // IEnablerService
    MOCK_METHOD(void, NotifyJniEnablerSet, (), (override));
};

#endif
