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

#include "IMSTypeDef.h"
#include "MtcDef.h"
#include "call/state/MtcCallState.h"

class EstablishedState : public MtcCallState
{
public:
    EstablishedState(IN IMtcCallContext& objContext);
    virtual ~EstablishedState();
    EstablishedState(IN const EstablishedState&) = delete;
    EstablishedState& operator=(IN const EstablishedState&) = delete;

public:
    CallStateName Hold(IN MediaInfo* pMediaInfo) override;
    CallStateName Resume(IN MediaInfo* pMediaInfo) override;
    CallStateName Update(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdateReceived(IN ISession* piSession) override;

    CallStateName TerminateUssi(IN const CallReasonInfo& objReason) override;
    CallStateName UssiTerminated(IN ISession* piSession) override;
    CallStateName SendUssd(IN const AString& strUssd) override;
    CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;
    CallStateName NotifyResponseToUssiInfo(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) override;
    CallStateName NotifyErrorToUssiInfo(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnVideoLowestBitRate() override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName HandleIpcanChanged() override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction);

protected:
    CallStateName SendUpdateBySrvcc(IN UpdateType eType) override;

private:
    IMS_RESULT HandleUpdate(
            IN UpdateType eUpdateType, IN CallType eCallType, IN MediaInfo* pMediaInfo);
    IMS_RESULT HandleReceivedUpdate(OUT CallStateName& eStateName);
    IMS_RESULT HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName);
    IMS_RESULT FormAutoAccept(IN IMS_BOOL bWithoutOffer);
    void AdjustDirectionWithHeldByMe(IN IMS_BOOL bWithoutOffer);
    IMS_BOOL IsConferenceCallParticipant();
    IMSList<IMtcBlockRule*> GetCallUpdateBlockRules() const;

    CallStateName TerminateUssiAfterInfoTransaction();
};

#endif
