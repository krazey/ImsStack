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

#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

class UpdatingState : public MtcCallState
{
public:
    UpdatingState(IN IMtcCallContext& objContext);
    virtual ~UpdatingState();
    UpdatingState(IN const UpdatingState&) = delete;
    UpdatingState& operator=(IN const UpdatingState&) = delete;

public:
    void OnExit() override;

    CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName RejectUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName CancelUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdated(IN ISession* piSession) override;
    CallStateName SessionUpdateFailed(IN ISession* piSession) override;
    // TODO, PR

    CallStateName Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnMediaFailed(IN CallReasonInfo objReason) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction);

private:
    IMS_RESULT HandleSdpAnswer();
    IMS_RESULT SendAck();
    IMS_RESULT SendUpdate();
    CallStateName HandleModificationSucceeded();
    CallStateName HandleRequestedModificationSucceeded();
    CallStateName HandleReceivedModificationSucceeded();
    void RecoverModificationFailure();
    void StopTimer();
    void UpdateCallType();
};

#endif
