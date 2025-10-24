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

#include "ImsTypeDef.h"
#include "call/state/MtcCallState.h"
#include <memory>

class AString;
class IMessage;
class IMtcCalContext;
class SuppService;
class UdpKeepAliveSender;
enum class CallType;
enum class QosLossPolicy;
struct MediaInfo;

/**
 * Represents the state that a call is incoming and it have alerted to the UI.
 */
class AlertingState : public MtcCallState
{
public:
    explicit AlertingState(IN IMtcCallContext& objContext);
    virtual ~AlertingState() override;
    AlertingState(IN const AlertingState&) = delete;
    AlertingState& operator=(IN const AlertingState&) = delete;

    void OnEnter() override;
    void OnExit() override;

    CallStateName HandleUserAlert() override;
    CallStateName Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName Reject(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;
    CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName UssiStarted(IN ISession* piSession) override;
    CallStateName SessionStarted(IN ISession* piSession) override;
    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionCanceledOnAccepted(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionPrackReceived(IN ISession* piSession) override;
    CallStateName SessionRprDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;
    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) override;

private:
    std::unique_ptr<UdpKeepAliveSender> m_pUdpKeepAliveSender;
};

#endif
