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
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/state/MtcCallState.h"
#include "conferencecall/ConferenceDef.h"
#include "precondition/QosDef.h"
#include <functional>
#include <memory>

class AString;
class IMessage;
class IMtcCallContext;
class SuppService;
struct MediaInfo;

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
            IN MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& lstUsers) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& lstUsers) override;
    CallStateName HandleIncoming(IN ISession* piSession) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;
    CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult) override;
    CallStateName OnAttached() override;

    CallStateName HandleIncomingUssi(IN ISession* piSession) override;
    CallStateName OnUssiAttached() override;

protected:
    CallStateName HandleAosConnected() override;

private:
    CallStateName ContinueStart();
    CallStateName ContinueConference();
    CallStateName ContinueHandleIncoming();
    CallStateName ContinueStartUssi();

    void SetResourceListForConference(IN_OUT IMessage& objMessage);
    ImsList<IMtcBlockRule*> GetIncomingCallBlockRules();
    ImsList<IMtcBlockRule*> GetOutgoingCallBlockRules();
    ImsList<IMtcBlockRule*> GetBlockRulesAfterEpsFallback();
    IMS_BOOL IsCallPull() const;
    IMS_RESULT HandleCallPull();
    void CopyConfUserListForAsynchronousHandling(const ImsList<ConfUser*> objUsers);
    AString RemoveCallerIdServiceCodeAndUpdateSuppService(IN const AString& strTarget);
    CallReasonInfo GetInternalErrorReason() const;

    std::unique_ptr<IMtcBlockChecker> m_pBlockChecker;
    std::function<CallStateName()> m_objOperationAfterBlockCheck;
    ImsList<std::shared_ptr<ConfUser>> m_pConfUsers;
};

#endif
