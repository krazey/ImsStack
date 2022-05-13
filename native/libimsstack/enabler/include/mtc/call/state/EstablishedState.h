#ifndef ESTABLISHED_STATE_H_
#define ESTABLISHED_STATE_H_

#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

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
    CallStateName Convert(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    CallStateName Terminate(IN const FailReason& objReason) override;

    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName SessionUpdateReceived(IN ISession* piSession) override;

private:
    IMS_RESULT HandleUpdate(
            IN UpdateType eUpdateType, IN CallType eCallType, IN MediaInfo* pMediaInfo);
    IMS_RESULT HandleReceivedUpdate(OUT CallStateName& eStateName);
    IMS_RESULT HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName);
    IMS_RESULT FormAutoAccept(IN IMS_BOOL bWithoutOffer);
    void AdjustDirectionWithHeldByMe(IN IMS_BOOL bWithoutOffer);
    IMS_BOOL IsConferenceCallParticipant();
};

#endif
