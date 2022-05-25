#ifndef INCOMING_STATE_H_
#define INCOMING_STATE_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

class AString;
class ConfUser;
class IDialogEvent;
class IMessage;
class IMtcCallContext;
class IMtcCallStateManager;
class ISession;
class MediaInfo;
class MtcSession;
class SuppService;

/**
 * Represents the state that a call is incoming but haven't alerted to the UI.
 */
class IncomingState : public MtcCallState
{
public:
    IncomingState(IN IMtcCallContext& objContext);
    virtual ~IncomingState();
    IncomingState(IN const IncomingState&) = delete;
    IncomingState& operator=(IN const IncomingState&) = delete;

    CallStateName HandleSrvccSuccess() override;
    CallStateName HandleSrvccFailure(IN UpdateType eUpdateType) override;

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

    CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    CallStateName QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    CallStateName SessionPRAckReceived(IN ISession* piSession) override;
    CallStateName SessionRPRDeliveryFailed(IN ISession* piSession) override;
};

#endif
