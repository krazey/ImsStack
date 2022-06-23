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

#ifndef IDLE_STATE_H_
#define IDLE_STATE_H_

#include "ImsList.h"
#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "helper/block/IMtcBlockChecker.h"
#include "MtcDef.h"
#include "precondition/QosDef.h"

class AString;
class ConfUser;
class IMessage;
class IMtcCallContext;
class JniMtcServiceThread;
class MediaInfo;
class SuppService;

/**
 * Represents the state that any messages have not been sent to the remote.
 */
class IdleState : public MtcCallState
{
public:
    explicit IdleState(IN IMtcCallContext& objContext);
    virtual ~IdleState();
    IdleState(IN const IdleState&) = delete;
    IdleState& operator=(IN const IdleState&) = delete;

    CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMSList<ConfUser*> lstUsers) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN IMSList<ConfUser*> lstUsers) override;
    CallStateName HandleIncoming(
            IN ISession* piSession, IN JniMtcServiceThread* pServiceThread) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;
    CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult) override;
    CallStateName OnAttached() override;

    CallStateName HandleIncomingUssi(
            IN ISession* piSession, IN JniMtcServiceThread* pServiceThread) override;
    CallStateName OnUssiAttached() override;

private:
    enum class ConferenceType
    {
        NOT_CONFERENCE,
        START_CONFERENCE,
        EXPAND,
        MERGE,
    };

    CallStateName ContinueStart(IN MediaInfo* pMediaInfo);
    CallStateName ContinueConference(IN MediaInfo* pMediaInfo, IN IMSList<ConfUser*> lstUsers);
    CallStateName ContinueHandleIncoming();
    CallStateName ContinueStartUssi(IN MediaInfo* pMediaInfo);

    AString GenerateSessionId();
    IMSList<AString> GetEntryUrisFromConferenceUsers(IN const IMSList<ConfUser*>& lstUsers);
    void SetResourceListForConference(
            IN_OUT IMessage& objMessage, IN IMSList<AString>& lstEntryUris);
    IMSList<IMtcBlockRule*> GetIncomingCallBlockRules();
    IMSList<IMtcBlockRule*> GetOutgoingCallBlockRules();
    void SetAcceptContact(IN ISipMessage* piSipMessage);

    ConferenceType m_eConferenceStartType;  // TODO: usage?
    std::unique_ptr<IMtcBlockChecker> m_pBlockChecker;
    std::function<CallStateName()> m_objOperationAfterBlockCheck;
};

#endif
