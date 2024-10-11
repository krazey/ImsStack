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

#ifndef MOCK_I_MTC_UI_NOTIFIER_H_
#define MOCK_I_MTC_UI_NOTIFIER_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcUiNotifier.h"
#include <gmock/gmock.h>

class AString;
struct CallReasonInfo;

class MockIMtcUiNotifier : public IMtcUiNotifier
{
public:
    virtual ~MockIMtcUiNotifier() {}

    MOCK_METHOD(void, SendPreIncomingCallReceived, (), (override));
    MOCK_METHOD(void, SendIncomingCallReceived, (), (override));
    MOCK_METHOD(void, SendIncomingCallRejected, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendStarted, (), (override));
    MOCK_METHOD(void, SendStartFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendInitiating, (), (override));
    MOCK_METHOD(void, SendProgressing, (), (override));
    MOCK_METHOD(void, SendHeld, (), (override));
    MOCK_METHOD(void, SendHoldFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendResumed, (), (override));
    MOCK_METHOD(void, SendResumeFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendHeldBy, (), (override));
    MOCK_METHOD(void, SendResumedBy, (), (override));
    MOCK_METHOD(void, SendTerminated, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendIncomingResume, (), (override));
    MOCK_METHOD(void, SendIncomingUpdate, (IN CallType), (override));
    MOCK_METHOD(void, SendUpdated, (), (override));
    MOCK_METHOD(void, SendUpdateFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendUpdatedBy, (), (override));
    MOCK_METHOD(void, SendNotifyInfo,
            (IN IMS_UINT32, IN const AString&, IN IMS_SINT32, IN IMS_BOOL), (override));
    MOCK_METHOD(void, SendReplacedBy, (IN IMS_SINTP, IN IMS_UINTP), (override));
    MOCK_METHOD(void, SendEctCompleted, (IN IMS_RESULT, IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendCallPushCompleted, (IN IMS_RESULT, IN const CallReasonInfo&), (override));
};

#endif
