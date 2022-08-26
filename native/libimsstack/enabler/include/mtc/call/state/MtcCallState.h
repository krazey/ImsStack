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

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ISessionListener.h"
#include "MtcDef.h"
#include "base/IMessageMediator.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/state/IMtcCallState.h"
#include "ussi/UssiDef.h"

class AString;
class IMessage;
class IMtcCallContext;
class IReference;
class ISession;
class ISipClientConnection;
class ISipConnection;
class ISipServerConnection;
class MediaInfo;
class MtcSession;
enum class QosLossPolicy;
struct CallReasonInfo;
struct ConfUser;

class MtcCallState : public IMtcCallState
{
public:
    MtcCallState(IN CallStateName eStateName, IN IMtcCallContext& objContext);
    virtual ~MtcCallState();
    MtcCallState(IN const MtcCallState&) = delete;
    MtcCallState& operator=(IN const MtcCallState&) = delete;

    void OnEnter() override;
    void OnExit() override;
    inline CallStateName GetStateName() const override { return m_eStateName; }

    CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& lstUsers) override;
    CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& lstUsers) override;
    CallStateName HandleIncoming(IN ISession* piSession) override;
    CallStateName HandleUserAlert() override;
    CallStateName Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName Reject(IN const CallReasonInfo& objReason) override;
    CallStateName Hold(IN MediaInfo* pMediaInfo) override;
    CallStateName Resume(IN MediaInfo* pMediaInfo) override;
    CallStateName AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName RejectResume(IN const CallReasonInfo& objReason) override;
    CallStateName Update(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName RejectUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName CancelUpdate(IN const CallReasonInfo& objReason) override;
    CallStateName Terminate(IN const CallReasonInfo& objReason) override;
    CallStateName SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration) override;
    CallStateName HandleSrvccSuccess() override;
    CallStateName HandleSrvccFailure(IN UpdateType eUpdateType) override;
    CallStateName HandleIpcanChanged() override;

    CallStateName HandleIncomingUssi(IN ISession* piSession) override;
    CallStateName OnUssiAttached() override;
    CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName UssiStarted(IN ISession* piSession) override;
    CallStateName TerminateUssi(IN const CallReasonInfo& objReason) override;
    CallStateName UssiTerminated(IN ISession* piSession) override;

    CallStateName SendUssd(IN const AString& strUssd) override;
    CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;
    CallStateName NotifyResponseToUssiInfo(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) override;
    CallStateName NotifyErrorToUssiInfo(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    CallStateName SessionAlerting(IN ISession* piSession) override;
    CallStateName SessionReferenceReceived(
            IN ISession* piSession, IN IReference* piReference) override;
    CallStateName SessionStarted(IN ISession* piSession) override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;
    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdated(IN ISession* piSession) override;
    CallStateName SessionUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionCancelDelivered(IN ISession* piSession) override;
    CallStateName SessionCancelDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) override;
    CallStateName SessionPRAckDelivered(IN ISession* piSession) override;
    CallStateName SessionPRAckDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionPRAckReceived(IN ISession* piSession) override;
    CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    CallStateName SessionRPRDeliveryFailed(IN ISession* piSession) override;
    CallStateName SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
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

    CallStateName ClientConnection_NotifyResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) override;
    CallStateName Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    CallStateName OnVideoLowestBitRate() override;
    CallStateName OnReceivingNetworkToneStarted() override;
    CallStateName OnReceivingNetworkToneFailed() override;
    CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) override;

    enum TimerType
    {
        TIMER_MO_100_WAIT,
        TIMER_MO_18X_WAIT,
        TIMER_MO_NOANSWER,
        TIMER_MT_ALERTING,

        TIMER_RETRY_AFTER,

        TIMER_CONVERT_USER_RESPONSE,
        TIMER_CONVERT_REMOTE_RESPONSE,

        TIMER_E911_LTE_OPEN,
        TIMER_E911_WIFI_OPEN,
        TIMER_E911_LTE_START,
        TIMER_E911_WIFI_START,
    };

protected:
    void HandleTerminate(IN const CallReasonInfo& objReason);
    void NotifyHoldResumeState();

    ISession* GetISession();

    void InitMediaSession();
    IMS_SINT32 OnSdpReceived(IN ISession* piSession, IN IMessage* piMessage);
    void RunMedia(IN ISession* piSession, IN IMessage* piMessage);

    CallStateName RejectIncomingAndToTerminating(IN const CallReasonInfo& objReason);

    void SendPreIncomingCallReceived();
    void SendIncomingCallReceived();
    void SendStarted();
    void SendIncomingUpdate(IN CallType eCallType);

    // TODO: bCheckSdp to be TRUE for all cases??
    void UpdatePreconditionCapability(
            IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bCheckeSdp = IMS_TRUE);
    void SetLocalQosAvailableForWifiCalling(IN ISession* piSession);

    IMS_BOOL IsRprSupported() const;
    IMS_BOOL IsNeedToIgnore(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsInvalidOfferAnswer(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsPreviewOfAnswer(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsAnswerMandatory(IN ISession* piSession, IN const IMessage* piMessage) const;

    // TODO: move these into MtcTimerWrapper? Is it used by All MTC classes?
    void StartTimer(IN IMS_UINT32 nType) const;
    void StopTimer(IN IMS_UINT32 nType) const;
    IMS_SINT32 GetTimeInMilliseconds(IN IMS_UINT32 nType) const;

    void SendInfoForUssi(
            IN const AString& strUssdString, IN UssiError eErrorCode = UssiError::CODE_NONE);
    void SendTransactionResponse(IN ISipServerConnection* piSipServerConnection,
            IN IMS_UINT32 nResponseCode, IN const AString& strPhrase = AString::ConstEmpty());

    IMS_BOOL IsCallEndNeededByAudioInactivity(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) const;
    CallReasonInfo GetAudioInactivityReasonOnTermination(IN const CallReasonInfo& objReason);

    IMtcCallContext& m_objContext;

private:
    const CallStateName m_eStateName;
};

#endif
