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

#ifndef MOCK_I_JNI_MTC_CALL_THREAD_H_
#define MOCK_I_JNI_MTC_CALL_THREAD_H_

#include <gmock/gmock.h>
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "ImsMap.h"
#include "ImsList.h"
#include "conferencecall/ConferenceDef.h"

#include "IJniMtcCallThread.h"

class SuppService;
struct CallReasonInfo;
struct JniCallInfo;

class MockIJniMtcCallThread : public IJniMtcCallThread
{
public:
    inline virtual ~MockIJniMtcCallThread() {}

    MOCK_METHOD(void, OnStarted,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnStartFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnInitiating, (IN const JniCallInfo&, IN const MediaInfo&, IN IMS_SINT32),
            (override));
    MOCK_METHOD(void, OnProgressing,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnHeld,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnHoldFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnResumed,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnResumeFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnHeldBy,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnResumedBy,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnTerminated, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnIncomingResume,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnIncomingUpdate,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnUpdated,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));
    MOCK_METHOD(void, OnUpdateFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnUpdatedBy,
            (IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&)),
            (override));

    MOCK_METHOD(void, OnMerged,
            (IN const JniCallInfo&, IN const MediaInfo&, (IN const ImsMap<SuppType, SuppService*>&),
                    IN const ImsList<ConfUser*>&),
            (override));
    MOCK_METHOD(void, OnMergeFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnConferenceParticipantAdded, (), (override));
    MOCK_METHOD(void, OnConferenceParticipantAddFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnConferenceParticipantRemoved, (), (override));
    MOCK_METHOD(void, OnConferenceParticipantRemoveFailed, (IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnConferenceInfoChanged, (IN const AString&, IN const AString, IN IMS_UINT32,
            IN IMS_UINT32, IN const AString&), (override));
    MOCK_METHOD(void, OnConferenceParticipantsInfoChanged, (IN const ImsList<ConfUser*>&),
            (override));

    MOCK_METHOD(void, OnEctCompleted, (IN IMS_RESULT, IN const CallReasonInfo&), (override));
    MOCK_METHOD(void, OnIncomingCallReceived,
            (IN IMS_UINTP, IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&), IN OipType, IN const AString&,
                    IN IMS_SINT32),
            (override));
    MOCK_METHOD(void, OnInformationNotificationReceived, (IN IMS_UINT32, IN const AString,
            IN IMS_SINT32, IN IMS_BOOL), (override));
    MOCK_METHOD(void, OnRatChanged, (IN IMS_SINT32), (override));
};

#endif
