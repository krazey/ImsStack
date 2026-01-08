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

#include "ImsTypeDef.h"
#include "call/ISilentRedialHelper.h"
#include "call/state/MtcCallState.h"
#include <memory>

class AString;
class IMessage;
class IMtcCalContext;
class IMtcSession;
class UdpKeepAliveSender;
enum class QosLossPolicy;

class OutgoingState : public MtcCallState
{
public:
    explicit OutgoingState(IN IMtcCallContext& objContext);
    virtual ~OutgoingState() override;
    OutgoingState(IN const OutgoingState&) = delete;
    OutgoingState& operator=(IN const OutgoingState&) = delete;

    void OnExit() override;

    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

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
    CallStateName SessionPrackDelivered(IN ISession* piSession) override;
    CallStateName SessionPrackDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    CallStateName UssiStarted(IN ISession* piSession) override;

    CallStateName OnReceivingMediaDataStarted(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnReceivingNetworkToneStarted() override;
    CallStateName OnReceivingNetworkToneFailed() override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) override;
    CallStateName OnRatChanged(IN IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType) override;
    CallStateName OnConnectionFailed(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) override;

protected:
    CallStateName HandleAosConnected() override;
    const CallReasonInfo GetCallReasonInfoByAosDisconnection(
            IN IMS_UINT32 nAosReason, IN IMS_SINT32 nDataFailureReason) const override;
    CallStateName HandleAosDisconnectedByAllPcscfFailed() override;

private:
    CallStateName On100TryingReceived();
    void HandleCancel(IN ISession* piSession, IN const CallReasonInfo& objReason);
    CallStateName MaySendPreconditionConfirmation(IN ISession& objSession);
    CallReasonInfo MayGetUpdatedReasonByResponseWaitTimeout(IN IMS_SINT32 nReasonCode) const;
    CallStateName HandleSilentRedialReason(IN const CallReasonInfo& objReason);
    CallStateName PerformSilentRedial(
            IN IMS_SINT32 nIntervalInMillis = ISilentRedialHelper::INTERVAL_BY_TYPE);
    IMS_BOOL HasNotRespondedQosConfirmation(IN ISession& objISession) const;
    void OnStarted(IN const IMtcSession& objMtcSession);
    void OnStartFailed(
            IN const CallReasonInfo& objReason, IN IMS_BOOL bReasonFromErrorHandler = IMS_FALSE);
    CallReasonInfo ConvertConnectionFailureToCallReasonInfo(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) const;
    CallStateName HandleAudioPortZero(IN ISession* piSession);

    std::unique_ptr<UdpKeepAliveSender> m_pUdpKeepAliveSender;
    ISilentRedialHelper* m_pSilentRedialHelper;
    IMS_BOOL m_bWaitingServiceConnectedForRedial;
    IMS_BOOL m_bMoResponseTimeoutForReasonTimerExpired;

    static const IMS_SINT32 DEFAULT_RETRY_AFTER = 1;
};

#endif
