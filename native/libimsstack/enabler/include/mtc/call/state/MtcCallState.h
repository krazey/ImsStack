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
class ISIPServerConnection;
class JniMediaSessionThread;
class JniMtcServiceThread;
class MediaInfo;
enum class QosLossPolicy;
struct FailReason;

using CallStateName = IMtcCall::State;

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

    virtual CallStateName Start(
            IN CallType eCallType,
            IN const AString& strTarget,
            IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN JniMediaSessionThread* pJniMediaThread);
    virtual CallStateName StartConference(
            IN CallType eCallType,
            IN const AString& strTarget,
            IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN IMSList<ConfUser*> lstUsers);
    virtual CallStateName StartConference(
            IN CallType eCallType,
            IN const AString& strTarget,
            IN IMSList<ConfUser*> lstUsers);
    virtual CallStateName ExpandToConference(
            IN CallInfo* pCallInfo, IN IMSList<ConfUser*> lstUsers);
    virtual CallStateName MergeToConference(
            IN CallType eCallType, IN CallInfo* pCallInfo, IN IMSList<ConfUser*> lstUsers);
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
            IN ISession* piSession, IN ISIPServerConnection* piSipServerConnection);
    virtual CallStateName MessageMediator_AdjustMessage(
            IN_OUT ISIPMessage *piSipMessage,
            IN IMS_SINT32 nMessage = IMessageMediator::MESSAGE_NORMAL);
    virtual CallStateName OnTimerExpired(IN IMS_SINT32 nType);

    virtual CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult);

    virtual CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType);
    virtual CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction);

    virtual CallStateName OnInternalFailure();

protected:
    enum TimerType
    {
        TIMER_WAIT_TERMINATED,
        TIMER_SRVCC_TERMINATED,
        TIMER_E911_LTE_OPEN,
        TIMER_E911_WIFI_OPEN,
        TIMER_E911_LTE_START,
        TIMER_E911_WIFI_START,
        TIMER_E911_LTE_RINGBACK,
        TIMER_E911_WIFI_RINGBACK,
        TIMER_RETRY_AFTER,
        TIMER_RETRY_MESSAGE,

        TIMER_MO_1XX_WAIT,
        TIMER_MO_NOANSWER,
        TIMER_MT_ALERTING,
        TIMER_MT_PRACK_WAIT,
    };

    void HandleTerminate(IN const FailReason& objReason);
    IMS_SINT32 ConvertTerminateReasonToFailReason(IN IMS_SINT32 eReason);
    CallStateName TransitToTerminating(IN const FailReason& objReason);
    void NotifyHoldResumeState();

    IMtcCallContext& m_objContext;

private:
    const CallStateName m_eStateName;
};

#endif
