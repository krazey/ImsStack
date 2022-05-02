#ifndef OUTGOING_STATE_H_
#define OUTGOING_STATE_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

class AString;
class ConfUser;
class IDialogEvent;
class IMessage;
class IMtcCalContext;
class MediaInfo;
class SuppService;
enum class QosLossPolicy;

class OutgoingState :
        public MtcCallState
{
public:
    explicit OutgoingState(IN IMtcCallContext& objContext);
    virtual ~OutgoingState();
    OutgoingState(IN const OutgoingState&) = delete;
    OutgoingState& operator=(IN const OutgoingState&) = delete;

    virtual void OnEnter() override;

    virtual CallStateName Terminate(IN const FailReason& objReason) override;
    virtual CallStateName HandleSrvccFailure(IN UpdateType eUpdateType) override;

    virtual CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    virtual CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    virtual CallStateName QosReserveFailed(
            IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    virtual CallStateName SessionStarted(IN ISession* piSession) override;
    virtual CallStateName SessionStartFailed(IN ISession* piSession) override;
    virtual CallStateName SessionTerminated(IN ISession* piSession) override;
    virtual CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    virtual CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    virtual CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    virtual CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) override;
    virtual CallStateName SessionPRAckDelivered(IN ISession* piSession) override;
    virtual CallStateName SessionPRAckDeliveryFailed(IN ISession* piSession) override;
    virtual CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    virtual CallStateName SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;


private:
    IMS_RESULT SendPrack(IN ISession* piSession); // TODO: Updating can use this also.
    IMS_RESULT SendAck(IN ISession* piSession); // TODO: differs from UpdatingState::SendAck()?
    void HandleCancel(IN ISession* piSession, IN const FailReason& objReason);
    void HandleRetryAfter(IN const FailReason& objReason);
    IMS_BOOL IsRttCapable(IN IMessage* piMessage);
    void UpdateCallType(IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bPeerView);
    void UpdateRemoteFeatures(IN IMessage* piMessage);
    void HandleCountrySpecificServiceUrn(IN IMessage* piMessage);
    void SendProgressing();
    void OnStarted(IN ISession* piSession);
    void OnStartFailed(IN ISession* piSession, IN const FailReason& objReason);
    void DeleteInactiveSessions();

    IMSMap<ISession*, MtcSession*> m_objSessions;
    IMS_BOOL m_bRemoteAlerted;
};

#endif
