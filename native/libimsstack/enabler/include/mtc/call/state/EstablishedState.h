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

#ifndef ESTABLISHED_STATE_H_
#define ESTABLISHED_STATE_H_

#include "CallReasonInfo.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/state/MtcCallState.h"

class ISipClientConnection;

class EstablishedState : public MtcCallState
{
public:
    explicit EstablishedState(IN IMtcCallContext& objContext);
    virtual ~EstablishedState() override;
    EstablishedState(IN const EstablishedState&) = delete;
    EstablishedState& operator=(IN const EstablishedState&) = delete;

public:
    void OnEnter() override;

    CallStateName Hold(IN MediaInfo& objMediaInfo) override;
    CallStateName Resume(IN MediaInfo& objMediaInfo) override;
    CallStateName Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName RejectUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdateReceived(IN ISession* piSession) override;

    CallStateName TerminateUssi(IN const CallReasonInfo& objReason) override;
    CallStateName UssiTerminated(IN ISession* piSession) override;
    CallStateName SendUssd(IN const AString& strUssd) override;
    CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;
    CallStateName Refresh_NotifyCompleted(IN ISipClientConnection* piScc) override;
    CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnVideoLowestBitRate() override;
    CallStateName OnReceivingNetworkToneStarted() override;
    CallStateName OnReceivingNetworkToneFailed() override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;
    CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) override;

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

protected:
    CallStateName SendUpdateBySrvcc(IN UpdateType eType) override;

private:
    IMS_RESULT HandleUpdate(
            IN UpdateType eUpdateType, IN CallType eCallType, IN const MediaInfo& objMediaInfo);
    CallReasonInfo HandleReceivedUpdate(OUT CallStateName& eStateName);
    CallReasonInfo HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName);
    IMS_BOOL IsConferenceCallParticipant() const;
    ImsList<IMtcBlockRule*> GetCallUpdateBlockRules() const;
    CallStateName Downgrade(IN CallType eCallType);
    IMS_BOOL ShouldPendOperation() const;
};

#endif
