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

#ifndef MOCK_CONFERENCE_EVENT_NOTIFIER_H_
#define MOCK_CONFERENCE_EVENT_NOTIFIER_H_

#include "CallReasonInfo.h"
#include "call/IMtcCall.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include <gmock/gmock.h>

class IMtcCallContext;
class IMtcCallManager;
class CallConnectionIdManager;
class ConferenceParticipantList;

class MockConferenceEventNotifier : public ConferenceEventNotifier
{
public:
    explicit MockConferenceEventNotifier(IN IMtcCallManager& objCallManager,
            IN CallKey nConferenceCallKey, IN CallConnectionIdManager& objConnectionIdManager) :
            ConferenceEventNotifier(objCallManager, nConferenceCallKey, objConnectionIdManager)
    {
    }
    ~MockConferenceEventNotifier() {}
    MOCK_METHOD(void, NotifyMerged,
            (IN ConferenceParticipantList & objParticipantList, IN IMS_BOOL bSubscribed),
            (override));
    MOCK_METHOD(void, NotifyMergeFailed, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, NotifyGroupCallStarted, (), (override));
    MOCK_METHOD(void, NotifyGroupCallFailed, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, NotifyExpanded, (), (override));
    MOCK_METHOD(void, NotifyExpandFailed, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(
            void, NotifyDropped, (IN ConferenceParticipantList & objParticipantList), (override));
    MOCK_METHOD(void, NotifyDropFailed,
            (IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList),
            (override));
    MOCK_METHOD(
            void, NotifyJoined, (IN ConferenceParticipantList & objParticipantList), (override));
    MOCK_METHOD(void, NotifyJoinFailed,
            (IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList),
            (override));
    MOCK_METHOD(void, NotifyConferenceInfo, (IN ConferenceParticipantList& objParticipantList),
            (override));
    MOCK_METHOD(
            void, NotifyUsersInfo, (IN ConferenceParticipantList& objParticipantList), (override));
    MOCK_METHOD(void, NotifyIndividualCallTerminated, (IN CallKey nKey), (override));
};

#endif
