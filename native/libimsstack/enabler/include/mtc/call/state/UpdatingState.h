#ifndef UPDATING_STATE_H_
#define UPDATING_STATE_H_

#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

class UpdatingState :
        public MtcCallState
{
public:
    UpdatingState(IN IMtcCallContext& objContext);
    virtual ~UpdatingState();
    UpdatingState(IN const UpdatingState&) = delete;
    UpdatingState& operator=(IN const UpdatingState&) = delete;

public:
    void OnExit() override;

    CallStateName AcceptConvert(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName RejectConvert(IN const FailReason& objReason) override;
    CallStateName CancelConvert(IN const FailReason& objReason) override;
    CallStateName Terminate(IN const FailReason& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdated(IN ISession* piSession) override;
    CallStateName SessionUpdateFailed(IN ISession* piSession) override;
    // TODO, PR

    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;

private:
    IMS_RESULT HandleSdpAnswer();
    IMS_RESULT SendAck();
    IMS_RESULT SendUpdate();
    CallStateName HandleModificationSucceeded();
    CallStateName HandleRequestedModificationSucceeded();
    CallStateName HandleReceivedModificationSucceeded();
    void RecoverModificationFailure();
    void StopTimer();

};

#endif
