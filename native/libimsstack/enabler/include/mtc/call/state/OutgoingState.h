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

#ifndef OUTGOING_STATE_H_
#define OUTGOING_STATE_H_

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

class OutgoingState : public MtcCallState
{
public:
    explicit OutgoingState(IN IMtcCallContext& objContext);
    virtual ~OutgoingState();
    OutgoingState(IN const OutgoingState&) = delete;
    OutgoingState& operator=(IN const OutgoingState&) = delete;

    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName HandleSrvccFailure(IN UpdateType eUpdateType) override;

    CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    CallStateName SessionStarted(IN ISession* piSession) override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;
    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) override;
    CallStateName SessionPRAckDelivered(IN ISession* piSession) override;
    CallStateName SessionPRAckDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    CallStateName UssiStarted(IN ISession* piSession) override;

    CallStateName OnReceivingNetworkToneStarted() override;
    CallStateName OnReceivingNetworkToneFailed() override;
    CallStateName OnMediaFailed(IN CallReasonInfo objReason) override;

private:
    IMS_RESULT SendPrack(IN ISession* piSession);  // TODO: Updating can use this also.
    IMS_RESULT SendAck(IN ISession* piSession);    // TODO: differs from UpdatingState::SendAck()?
    void HandleCancel(IN ISession* piSession, IN const CallReasonInfo& objReason);
    CallStateName HandleSilentRetry(IN const CallReasonInfo& objReason);
    CallStateName ContinueSilentRetry();
    void HandleCountrySpecificServiceUrn(IN IMessage* piMessage);
    void SendProgressing();
    void OnStarted(IN ISession* piSession);
    void OnStartFailed(IN ISession* piSession, IN const CallReasonInfo& objReason);
    void OnSessionForked(IN ISession* piOriginSession);

    IMS_BOOL m_bRemoteAlerted;
    IMS_SINT32 m_nSilentRedialCount;
};

#endif
