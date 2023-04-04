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

#ifndef UPDATING_STATE_H_
#define UPDATING_STATE_H_

#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/state/MtcCallState.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/ISession.h"
#include "precondition/QosDef.h"

struct CallReasonInfo;
struct MediaInfo;

class UpdatingState : public MtcCallState
{
public:
    explicit UpdatingState(IN IMtcCallContext& objContext);
    virtual ~UpdatingState();
    UpdatingState(IN const UpdatingState&) = delete;
    UpdatingState& operator=(IN const UpdatingState&) = delete;

public:
    void OnExit() override;

    CallStateName Hold(IN MediaInfo& objMediaInfo) override;
    CallStateName Resume(IN MediaInfo& objMediaInfo) override;
    CallStateName Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName RejectUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName CancelUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdated(IN ISession* piSession) override;
    CallStateName SessionUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionUpdateReceived(IN ISession* piSession) override;

    CallStateName SessionCancelDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionPRAckDelivered(IN ISession* piSession) override;
    CallStateName SessionPRAckDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionPRAckReceived(IN ISession* piSession) override;
    CallStateName SessionRPRDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;

    CallStateName Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnVideoLowestBitRate() override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;
    CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) override;

    static IMS_BOOL IsPreconditionRequired(
            IN const MtcConfigurationProxy& objConfigProxy, IN const UpdatingInfo& objInfo);

protected:
    CallStateName HandleSrvccStarted() override;

private:
    IMS_RESULT HandleSdpAnswer();
    IMS_RESULT SendAck();
    IMS_RESULT SendUpdate();
    CallStateName HandleModificationSucceeded();
    CallStateName HandleRequestedModificationSucceeded();
    CallStateName HandleReceivedModificationSucceeded();
    CallStateName HandleRetry();
    void RecoverModificationFailure();
    void NotifyFailure();
    void StopTimer();
    void UpdateCallType();
    void CheckPreconditionAndNotifyIncomingUpdate(IN ISession* piSession);
};

#endif
