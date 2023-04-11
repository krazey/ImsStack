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

#ifndef INCOMING_STATE_H_
#define INCOMING_STATE_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/state/MtcCallState.h"

class AString;
class IMessage;
class IMtcCallContext;
class IMtcCallStateManager;
class ISession;
class MtcSession;
class SuppService;

/**
 * Represents the state that a call is incoming but haven't alerted to the UI.
 */
class IncomingState : public MtcCallState
{
public:
    explicit IncomingState(IN IMtcCallContext& objContext);
    virtual ~IncomingState();
    IncomingState(IN const IncomingState&) = delete;
    IncomingState& operator=(IN const IncomingState&) = delete;

    void OnExit() override;

    CallStateName Reject(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionPRAckReceived(IN ISession* piSession) override;
    CallStateName SessionRPRDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;
    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;
    CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) override;
    CallStateName HandleAosConnected() override;
};

#endif
