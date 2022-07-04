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

#include "call/IMtcCall.h"
#include "MtcDef.h"

class IMtcCallContext;
class CallConnectionIdManager;
class ConferenceParticipantList;

class ConferenceEventNotifier
{
public:
    // TODO: update this to use JniMtcCallThread. IMtcCallContext should provide
    // TODO: objContext.GetUiNotifier().GetCallThread()
    explicit ConferenceEventNotifier(IN IMtcCallContext& objConfCallContext,
            IN CallConnectionIdManager& objConnectionIdManager);
    virtual ~ConferenceEventNotifier();
    ConferenceEventNotifier(IN const ConferenceEventNotifier&) = delete;
    ConferenceEventNotifier& operator=(IN const ConferenceEventNotifier&) = delete;

public:
    virtual void NotifyMerged(IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyMergeFailed(IN CallReasonInfo objReason);

    virtual void NotifyGroupCallStarted();
    virtual void NotifyGroupCallFailed(IN CallReasonInfo objReason);

    virtual void NotifyExpanded();
    virtual void NotifyExpandFailed(IN CallReasonInfo objReason);

    virtual void NotifyDropped(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyDropFailed(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);

    virtual void NotifyJoined(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyJoinFailed(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);

    virtual void NotifyConferenceInfo(IN ConferenceParticipantList& objParticipantList);
    virtual void NotifyUsersInfo(IN ConferenceParticipantList& objParticipantList);

    virtual void NotifyIndividualCallTerminated(IN CallKey nKey);

private:
    CallInfo* CloneCallInfo();
    MediaInfo* CloneMediaInfo();

    void CheckDisconnectedConfUsersInfo(
            IN ConferenceParticipantList& objParticipantList, IN_OUT IMSList<ConfUser*>& objUsers);

private:
    IMtcCallContext& m_objConfCallContext;
    CallConnectionIdManager& m_objConnectionIdManager;
};

#endif
