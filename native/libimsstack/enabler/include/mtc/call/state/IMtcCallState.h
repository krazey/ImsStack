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

#ifndef INTERFACE_MTC_CALL_STATE_H_
#define INTERFACE_MTC_CALL_STATE_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockChecker.h"

class AString;
class IMessage;
class IMtcCallContext;
class IReference;
class ISession;
class ISipClientConnection;
class ISipConnection;
class ISipServerConnection;
class JniMediaSessionThread;
class JniMtcServiceThread;
class MediaInfo;
class MtcSession;
enum class QosLossPolicy;
struct CallReasonInfo;
struct ConfUser;

using CallStateName = IMtcCall::State;

class IMtcCallState
{
public:
    virtual ~IMtcCallState() {}

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual CallStateName GetStateName() const = 0;

    virtual CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& lstUsers) = 0;
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& lstUsers) = 0;
    virtual CallStateName HandleIncoming(
            IN ISession* piSession, IN JniMtcServiceThread* pServiceThread) = 0;
    virtual CallStateName HandleUserAlert() = 0;
    virtual CallStateName Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName Reject(IN const CallReasonInfo& objReason) = 0;
    virtual CallStateName Hold(IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName Resume(IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName RejectResume(IN const CallReasonInfo& objReason) = 0;
    virtual CallStateName Update(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName RejectUpdate(IN const CallReasonInfo& objReason) = 0;
    virtual CallStateName CancelUpdate(IN const CallReasonInfo& objReason) = 0;
    virtual CallStateName Terminate(IN const CallReasonInfo& objReason) = 0;
    virtual CallStateName SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration) = 0;
    virtual CallStateName HandleSrvccSuccess() = 0;
    virtual CallStateName HandleSrvccFailure(IN UpdateType eUpdateType) = 0;
    virtual CallStateName HandleIpcanChanged() = 0;

    virtual CallStateName HandleIncomingUssi(
            IN ISession* piSession, IN JniMtcServiceThread* pServiceThread) = 0;
    virtual CallStateName OnUssiAttached() = 0;
    virtual CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;
    virtual CallStateName UssiStarted(IN ISession* piSession) = 0;
    virtual CallStateName TerminateUssi(IN const CallReasonInfo& objReason) = 0;
    virtual CallStateName UssiTerminated(IN ISession* piSession) = 0;

    virtual CallStateName SendUssd(IN const AString& strUssd) = 0;
    virtual CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) = 0;
    virtual CallStateName NotifyResponseToUssiInfo(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) = 0;
    virtual CallStateName NotifyErrorToUssiInfo(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;

    virtual CallStateName SessionAlerting(IN ISession* piSession) = 0;
    virtual CallStateName SessionReferenceReceived(
            IN ISession* piSession, IN IReference* piReference) = 0;
    virtual CallStateName SessionStarted(IN ISession* piSession) = 0;
    virtual CallStateName SessionStartFailed(IN ISession* piSession) = 0;
    virtual CallStateName SessionTerminated(IN ISession* piSession) = 0;
    virtual CallStateName SessionUpdated(IN ISession* piSession) = 0;
    virtual CallStateName SessionUpdateFailed(IN ISession* piSession) = 0;
    virtual CallStateName SessionUpdateReceived(IN ISession* piSession) = 0;
    virtual CallStateName SessionCancelDelivered(IN ISession* piSession) = 0;
    virtual CallStateName SessionCancelDeliveryFailed(IN ISession* piSession) = 0;
    virtual CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) = 0;
    virtual CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) = 0;
    virtual CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) = 0;
    virtual CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) = 0;
    virtual CallStateName SessionPRAckDelivered(IN ISession* piSession) = 0;
    virtual CallStateName SessionPRAckDeliveryFailed(IN ISession* piSession) = 0;
    virtual CallStateName SessionPRAckReceived(IN ISession* piSession) = 0;
    virtual CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) = 0;
    virtual CallStateName SessionRPRDeliveryFailed(IN ISession* piSession) = 0;
    virtual CallStateName SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) = 0;
    virtual CallStateName SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) = 0;

    virtual CallStateName Refresh_NotifyCompleted(IN ISipClientConnection* piScc) = 0;
    virtual CallStateName Refresh_NotifyTerminated() = 0;
    virtual CallStateName Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;

    virtual CallStateName OnTimerExpired(IN IMS_SINT32 nType) = 0;

    virtual CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult) = 0;

    virtual CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;
    virtual CallStateName QosReserveFailed(
            IN ISession* piSession, IN QosLossPolicy eNextAction) = 0;

    virtual CallStateName OnInternalFailure() = 0;
    virtual CallStateName OnAttached() = 0;

    virtual CallStateName ClientConnection_NotifyResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) = 0;
    virtual CallStateName Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;

    virtual CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;
    virtual CallStateName OnVideoLowestBitRate() = 0;
    virtual CallStateName OnReceivingNetworkToneStarted() = 0;
    virtual CallStateName OnReceivingNetworkToneFailed() = 0;
    virtual CallStateName OnMediaFailed(IN CallReasonInfo objReason) = 0;
};

#endif
