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

#ifndef CONFERENCE_EVENT_NOTIFIER_H_
#define CONFERENCE_EVENT_NOTIFIER_H_

#include "ImsList.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

class IJniMtcCallThread;
class IMtcCallManager;
class CallConnectionIdManager;
class ConferenceParticipantList;

class ConferenceEventNotifier
{
public:
    explicit ConferenceEventNotifier(IN IMtcCallManager& objCallManager,
            IN CallKey nConferenceCallKey, IN CallConnectionIdManager& objConnectionIdManager);
    virtual ~ConferenceEventNotifier();
    ConferenceEventNotifier(IN const ConferenceEventNotifier&) = delete;
    ConferenceEventNotifier& operator=(IN const ConferenceEventNotifier&) = delete;

public:
    virtual void NotifyMerged(
            IN ConferenceParticipantList& objParticipantList, IN IMS_BOOL bSubscribed);
    virtual void NotifyMergeFailed(IN const CallReasonInfo& objReason);

    virtual void NotifyGroupCallStarted();
    virtual void NotifyGroupCallFailed(IN const CallReasonInfo& objReason);

    virtual void NotifyExpanded();
    virtual void NotifyExpandFailed(IN const CallReasonInfo& objReason);

    virtual void NotifyDropped(IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyDropFailed(
            IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList);

    virtual void NotifyJoined(IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyJoinFailed(
            IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList);

    virtual void NotifyConferenceInfo(IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyUsersInfo(IN ConferenceParticipantList& objParticipantList);

    virtual void NotifyIndividualCallTerminated(IN CallKey nKey);

private:
    void CheckDisconnectedConfUsersInfo(
            IN ConferenceParticipantList& objParticipantList, IN_OUT ImsList<ConfUser*>& objUsers);
    IMtcCallContext* GetConferenceCallContext() const;
    IJniMtcCallThread* GetCallThread() const;

private:
    IMtcCallManager& m_objCallManager;
    CallKey m_nConferenceCallKey;
    CallConnectionIdManager& m_objConnectionIdManager;
};

#endif
