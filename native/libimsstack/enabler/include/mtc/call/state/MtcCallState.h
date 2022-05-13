#ifndef MTC_CALL_STATE_H_
#define MTC_CALL_STATE_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "ISessionListener.h"
#include "base/IMessageMediator.h"
#include "call/IMtcCall.h"
#include "helper/block/IMtcBlockChecker.h"
#include "CallInfo.h"
#include "MtcDef.h"

class AString;
class IMtcCallContext;
class IReference;
class ISession;
class IMessage;
class ISipServerConnection;
class JniMediaSessionThread;
class JniMtcServiceThread;
class MediaInfo;
class MtcSession;
enum class QosLossPolicy;
struct FailReason;

using CallStateName = IMtcCall::State;

enum class ResultSetSdp
{
    NO_SDP,
    FAILURE,
    SUCCESS
};

class MtcCallState
{
public:
    MtcCallState(IN CallStateName eStateName, IN IMtcCallContext& objContext);
    virtual ~MtcCallState();
    MtcCallState(IN const MtcCallState&) = delete;
    MtcCallState& operator=(IN const MtcCallState&) = delete;

    virtual void OnEnter();
    virtual void OnExit();
    inline CallStateName GetStateName() const { return m_eStateName; }

    virtual CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMSList<ConfUser*> lstUsers);
    virtual CallStateName StartConference(
            IN CallType eCallType, IN const AString& strTarget, IN IMSList<ConfUser*> lstUsers);
    virtual CallStateName HandleIncoming(
            IN ISession* piSession, IN JniMtcServiceThread* pServiceThread);
    virtual CallStateName HandleUserAlert();
    virtual CallStateName Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo);
    virtual CallStateName Reject(IN const FailReason& objReason);
    virtual CallStateName Hold(IN MediaInfo* pMediaInfo);
    virtual CallStateName Resume(IN MediaInfo* pMediaInfo);
    virtual CallStateName AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo);
    virtual CallStateName RejectResume(IN const FailReason& objReason);
    virtual CallStateName Convert(IN CallType eCallType, IN MediaInfo* pMediaInfo);
    virtual CallStateName AcceptConvert(IN CallType eCallType, IN MediaInfo* pMediaInfo);
    virtual CallStateName RejectConvert(IN const FailReason& objReason);
    virtual CallStateName CancelConvert(IN const FailReason& objReason);
    virtual CallStateName Terminate(IN const FailReason& objReason);
    virtual CallStateName SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration);
    virtual CallStateName SendUssi(IN const AString& strUssi);
    virtual CallStateName HandleSrvccSuccess();
    virtual CallStateName HandleSrvccFailure(IN UpdateType eUpdateType);

    virtual CallStateName SessionAlerting(IN ISession* piSession);
    virtual CallStateName SessionReferenceReceived(
            IN ISession* piSession, IN IReference* piReference);
    virtual CallStateName SessionStarted(IN ISession* piSession);
    virtual CallStateName SessionStartFailed(IN ISession* piSession);
    virtual CallStateName SessionTerminated(IN ISession* piSession);
    virtual CallStateName SessionUpdated(IN ISession* piSession);
    virtual CallStateName SessionUpdateFailed(IN ISession* piSession);
    virtual CallStateName SessionUpdateReceived(IN ISession* piSession);
    virtual CallStateName SessionCancelDelivered(IN ISession* piSession);
    virtual CallStateName SessionCancelDeliveryFailed(IN ISession* piSession);
    virtual CallStateName SessionEarlyMediaUpdated(IN ISession* piSession);
    virtual CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession);
    virtual CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession);
    virtual CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession);
    virtual CallStateName SessionPRAckDelivered(IN ISession* piSession);
    virtual CallStateName SessionPRAckDeliveryFailed(IN ISession* piSession);
    virtual CallStateName SessionPRAckReceived(IN ISession* piSession);
    virtual CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex);
    virtual CallStateName SessionRPRDeliveryFailed(IN ISession* piSession);
    virtual CallStateName SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex);
    virtual CallStateName SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection);
    virtual CallStateName MessageMediator_AdjustMessage(IN_OUT ISipMessage* piSipMessage,
            IN IMS_SINT32 nMessage = IMessageMediator::MESSAGE_NORMAL);
    virtual CallStateName OnTimerExpired(IN IMS_SINT32 nType);

    virtual CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult);

    virtual CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType);
    virtual CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction);

    virtual CallStateName OnInternalFailure();

protected:
    enum TimerType
    {
        // TODO: differentiate 100 and 183?
        // Currently, MTC isn't being notified about 100 Trying response.
        TIMER_MO_1XX_WAIT,
        TIMER_MO_NOANSWER,
        TIMER_MT_ALERTING,
        TIMER_MT_PRACK_WAIT,

        TIMER_RETRY_AFTER,

        TIMER_CONVERT_USER_RESPONSE,
        TIMER_CONVERT_REMOTE_RESPONSE,

        TIMER_E911_LTE_OPEN,
        TIMER_E911_WIFI_OPEN,
        TIMER_E911_LTE_START,
        TIMER_E911_WIFI_START,
    };

    void HandleTerminate(IN const FailReason& objReason);
    IMS_SINT32 ConvertTerminateReasonToFailReason(IN IMS_SINT32 eReason);
    void NotifyHoldResumeState();

    IMS_RESULT CreateISession();
    ISession* GetISession();

    void InitMediaSession(IN MediaInfo* pMediaInfo = IMS_NULL);
    IMS_SINT32 OnSdpReceived(IN ISession* piSession, IN IMessage* piMessage);
    ResultSetSdp SetSdpToSend(IN IMS_BOOL bAllowReOffer, IN ISession* piSession = IMS_NULL);
    void RunMedia(IN ISession* piSession, IN IMessage* piMessage);

    IMS_RESULT SendProvisionalResponse(IN IMS_BOOL bUserAlert);
    IMS_RESULT SendEarlyUpdate(IN MtcSession* pMtcSession);
    IMS_RESULT SendResponseToEarlyUpdate(IN IMS_SINT32 eStatusCode, IN MtcSession* pMtcSession);
    IMS_RESULT SendResponseToPrack(IN IMS_SINT32 eStatusCode);

    CallStateName RejectIncomingAndToTerminating(IN const FailReason& objFailReason);

    void SendPreIncomingCallReceived();
    void SendIncomingCallReceived();
    void SendStarted();
    void SendIncomingUpdate(IN CallType eCallType);

    // TODO: bCheckSdp to be TRUE for all cases??
    void UpdatePreconditionCapability(
            IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bCheckeSdp = IMS_TRUE);
    void SetLocalQosAvailableForWifiCalling(IN ISession* piSession);
    IMS_RESULT NegotiateExtension(
            IN MtcSession* pMtcSession, IN IMessage* piMessage, IN IMS_UINT32 eMethod);

    IMS_BOOL IsRprSupported() const;
    IMS_BOOL IsNeedToIgnore(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsInvalidOfferAnswer(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsPreviewOfAnswer(IN ISession* piSession, IN const IMessage* piMessage) const;
    IMS_BOOL IsCallWaiting() const;
    IMS_BOOL IsNeedToReliable(IN IMS_BOOL bIncludeSdp) const;

    // TODO: move these into MtcTimerWrapper? Is it used by All MTC classes?
    void StartTimer(IN IMS_UINT32 nType) const;
    void StopTimer(IN IMS_UINT32 nType) const;
    IMS_SINT32 GetTimeInMilliseconds(IN IMS_UINT32 nType) const;

    IMtcCallContext& m_objContext;

private:
    const CallStateName m_eStateName;
};

#endif
