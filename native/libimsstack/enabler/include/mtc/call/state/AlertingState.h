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

#ifndef ALERTING_STATE_H_
#define ALERTING_STATE_H_

#include "ImsList.h"
#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

class AString;
class IDialogEvent;
class IMessage;
class IMtcCalContext;
class MediaInfo;
class SuppService;
enum class QosLossPolicy;

/**
 * Represents the state that a call is incoming and it have alerted to the UI.
 */
class AlertingState : public MtcCallState
{
public:
    explicit AlertingState(IN IMtcCallContext& objContext);
    virtual ~AlertingState();
    AlertingState(IN const AlertingState&) = delete;
    AlertingState& operator=(IN const AlertingState&) = delete;

    CallStateName HandleUserAlert() override;
    CallStateName Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName Reject(IN const CallReasonInfo& objReason) override;
    CallStateName HandleSrvccSuccess() override;
    CallStateName HandleSrvccFailure(IN UpdateType eUpdateType) override;

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    CallStateName SessionStarted(IN ISession* piSession) override;
    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionPRAckReceived(IN ISession* piSession) override;
    CallStateName SessionRPRDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;

    CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName UssiStarted(IN ISession* piSession) override;

    CallStateName OnMediaFailed(IN CallReasonInfo objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

private:
    IMS_RESULT SendAccept();
    IMS_BOOL IsUpdateBySrvcc(IN ISession* piSession) const;
};

#endif
