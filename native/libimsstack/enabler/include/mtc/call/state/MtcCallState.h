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

#ifndef MTC_CALL_STATE_H_
#define MTC_CALL_STATE_H_

#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MtcDef.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/state/IMtcCallState.h"
#include "helper/IMtcAosStateListener.h"
#include "ussi/UssiDef.h"

class AString;
class IMessage;
class IMtcCallContext;
class IMtcSession;
class IReference;
class ISession;
class ISipClientConnection;
class ISipServerConnection;
class MtcSession;
enum class QosLossPolicy;
struct CallReasonInfo;
struct ConfUser;
template <class T>
class ImsList;

class MtcCallState : public IMtcCallState
{
public:
    MtcCallState(IN CallStateName eStateName, IN IMtcCallContext& objContext);
    virtual ~MtcCallState() override;
    MtcCallState(IN const MtcCallState&) = delete;
    MtcCallState& operator=(IN const MtcCallState&) = delete;

    void OnEnter() override;
    void OnExit() override;
    inline CallStateName GetStateName() const override { return m_eStateName; }

    CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& lstUsers) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& lstUsers) override;
    CallStateName HandleIncoming(IN ISession* piSession) override;
    CallStateName HandleUserAlert() override;
    CallStateName Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName Reject(IN const CallReasonInfo& objReason) override;
    CallStateName Hold(IN MediaInfo& objMediaInfo) override;
    CallStateName Resume(IN MediaInfo& objMediaInfo) override;
    CallStateName AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName RejectResume(IN const CallReasonInfo& objReason) override;
    CallStateName Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName RejectUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName CancelUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;

    CallStateName HandleIncomingUssi(IN ISession* piSession) override;
    CallStateName OnUssiAttached() override;
    CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    CallStateName UssiStarted(IN ISession* piSession) override;
    CallStateName TerminateUssi(IN const CallReasonInfo& objReason) override;
    CallStateName UssiTerminated(IN ISession* piSession) override;

    CallStateName SendUssd(IN const AString& strUssd) override;
    CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;

    CallStateName SessionAlerting(IN ISession* piSession) override;
    CallStateName SessionReferenceReceived(
            IN ISession* piSession, IN IReference* piReference) override;
    CallStateName SessionStarted(IN ISession* piSession) override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;
    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdated(IN ISession* piSession) override;
    CallStateName SessionUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionCanceledOnAccepted(IN ISession* piSession) override;
    CallStateName SessionCancelDelivered(IN ISession* piSession) override;
    CallStateName SessionCancelDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) override;
    CallStateName SessionPrackDelivered(IN ISession* piSession) override;
    CallStateName SessionPrackDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionPrackReceived(IN ISession* piSession) override;
    CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName SessionRprDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;

    CallStateName Refresh_NotifyCompleted(IN ISipClientConnection* piScc) override;
    CallStateName Refresh_NotifyTerminated() override;
    CallStateName Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult) override;

    CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    CallStateName OnInternalFailure() override;
    CallStateName OnAttached() override;

    CallStateName OnReceivingMediaDataStarted(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnVideoLowestBitRate() override;
    CallStateName OnReceivingNetworkToneStarted() override;
    CallStateName OnReceivingNetworkToneFailed() override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;
    CallStateName OnSrvccStateUpdated(IN SrvccState eState) override;
    CallStateName OnAosStateChanged(IN MtcAosState eState, IN IMS_UINT32 eAosReason,
            IN IMS_SINT32 nDataFailureReason) override;
    CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) override;
    CallStateName OnRatChanged(IN IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType) override;
    CallStateName OnConnectionFailed(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) override;

    enum TimerType
    {
        TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL,
        TIMER_MO_CALL_INITIATION_TO_18X_WAIT,
        TIMER_MO_18X_WAIT,
        TIMER_MO_NOANSWER,
        TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
        TIMER_MT_ALERTING,
        TIMER_MT_PRACK_WAIT,

        TIMER_RETRY_UPDATE,

        TIMER_CONVERT_USER_RESPONSE,
        TIMER_CONVERT_REMOTE_RESPONSE,

        TIMER_E911_WAIT_SESSION_RELEASED,

        TIMER_DELAY_UPDATE_AFTER_CONNECTED,
    };

protected:
    CallStateName OnReadyToAlert();
    inline virtual CallStateName HandleSrvccStarted() { return GetStateName(); }
    virtual CallStateName SendUpdateBySrvcc(IN UpdateType eType);
    virtual CallStateName HandleAosConnected();
    virtual CallStateName HandleAosDisconnected(
            IN IMS_UINT32 eAosReason, IN IMS_SINT32 nDataFailureReason);
    virtual const CallReasonInfo GetCallReasonInfoByAosDisconnection(
            IN IMS_UINT32 nAosReason, IN IMS_SINT32 nDataFailureReason) const;
    inline virtual CallStateName HandleAosDisconnectedByAllPcscfFailed() { return GetStateName(); }

    void HandleTerminate(IN const CallReasonInfo& objReason) const;
    void NotifyHoldResumeState();

    ISession* GetISession();

    void InitMediaSession(IN const MediaInfo& objMediaInfo);
    const CallReasonInfo HandleReceivedSdp(
            IN ISession* piSession, IN const IMessage* piMessage) const;

    IMS_RESULT SendEarlyUpdate(IN UpdateType eType, IN IMtcSession* piMtcSession);
    CallStateName RejectIncomingAndToTerminating(IN const CallReasonInfo& objReason);

    void SendIncomingUpdateToUi(IN CallType eCallType);

    IMS_BOOL IsNeedToIgnore(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsSdpPreviewModeAllowedByPolicy() const;
    IMS_BOOL IsInvalidOfferAnswer(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsAnswerMandatory(IN ISession* piSession, IN const IMessage* piMessage) const;

    void StartTimer(IN IMS_UINT32 nType) const;
    void StopTimer(IN IMS_UINT32 nType) const;
    IMS_SINT32 GetTimeInMilliseconds(IN IMS_UINT32 nType) const;

    void SendInfoForUssi(
            IN const AString& strUssdString, IN UssiError eErrorCode = UssiError::CODE_NONE);
    static void SendTransactionResponse(IN ISipServerConnection* piSipServerConnection,
            IN IMS_UINT32 nResponseCode, IN const AString& strPhrase = AString::ConstEmpty());

    const CallReasonInfo GetAudioInactivityReasonOnTermination(
            IN const CallReasonInfo& objReason) const;
    const CallReasonInfo GetAudioInactivityReasonOnMediaDataFailed() const;
    IMS_BOOL IsNeedToIgnoreStartFailure() const;
    void StartEpsFallbackWatchdogIfNeeded(IN const IMessage& objMessage) const;
    static const CallReasonInfo GetReasonByNegotiationResult(IN MediaNegoResult eNegoResult);

    IMS_BOOL IsNeedToSendLocalResourceConfirmation(IN ISession* piSession) const;
    IMS_BOOL IsRprRequired() const;
    IMS_BOOL IsPrackRequiredForAlert() const;

    IMtcCallContext& m_objContext;

private:
    inline IMS_BOOL Is18x(IN IMS_SINT32 eStatusCode) const
    {
        return SipStatusCode::SC_180 <= eStatusCode && eStatusCode <= SipStatusCode::SC_183;
    }

    const CallStateName m_eStateName;
};

#endif
