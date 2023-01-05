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

#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcUiNotifier.h"
#include <gmock/gmock.h>

class ParticipantInfo;
class SuppService;
struct CallInfo;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;

class MockIMtcUiNotifier : public IMtcUiNotifier
{
public:
    virtual ~MockIMtcUiNotifier() {}

    MOCK_METHOD(void, SendPreIncomingCallReceived, (IN CallKey), (override));
    MOCK_METHOD(void, SendIncomingCallReceived,
            (IN CallKey, IN CallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&), IN ParticipantInfo&),
            (override));
    MOCK_METHOD(void, SendStarted,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendStartFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendProgressing,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&),
                    IN IMS_BOOL),
            (override));
    MOCK_METHOD(void, SendHeld,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendHoldFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendResumed,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendResumeFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendHeldBy,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendResumedBy,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendTerminated, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendIncomingResume,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendIncomingUpdate,
            (IN CallType, IN CallInfo*, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendUpdated,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendUpdateFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendUpdatedBy,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendNotifyInfo,
            (IN IMS_UINT32, IN const AString&, IN IMS_SINT32, IN IMS_BOOL), (override));
    MOCK_METHOD(void, SendExpanded,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, SendExpandFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendExpandedBy,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&),
                    IN IMS_SINTP),
            (override));
    MOCK_METHOD(void, SendMerged,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&),
                    IN ImsList<ConfUser*>&),
            (override));
    MOCK_METHOD(void, SendMergeFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendJoined, (IN IMS_BOOL, IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendDropped, (IN IMS_BOOL, IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendNotifyUsersInfo, (IN ImsList<ConfUser*>&), (override));
    MOCK_METHOD(void, SendNotifyConfInfo,
            (IN AString, IN AString, IN IMS_SINT32, IN IMS_UINT32, IN const AString), (override));
    MOCK_METHOD(void, SendReplacedBy,
            (IN CallInfo*, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&),
                    IN IMS_SINTP, IN IMS_UINTP),
            (override));
    MOCK_METHOD(void, SendEctCompleted, (IN IMS_RESULT, IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, SendCallPushCompleted, (IN IMS_BOOL, IN const CallReasonInfo&), (override));
};

#endif
