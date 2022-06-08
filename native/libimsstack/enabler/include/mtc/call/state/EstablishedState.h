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

    CallStateName TerminateUssi(IN const FailReason& objReason) override;
    CallStateName UssiTerminated(IN ISession* piSession) override;
    CallStateName SendUssd(IN const AString& strUssd) override;
    CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;
    CallStateName NotifyResponseToUssiInfo(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) override;
    CallStateName NotifyErrorToUssiInfo(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    CallStateName OnReceivingMediaDataFailed(IN IMS_UINT32 eMediaType) override;
    CallStateName OnVideoLowestBitRate() override;
    CallStateName OnMediaFailed(IN FailReason objReason) override;

private:
    IMS_RESULT HandleUpdate(
            IN UpdateType eUpdateType, IN CallType eCallType, IN MediaInfo* pMediaInfo);
    IMS_RESULT HandleReceivedUpdate(OUT CallStateName& eStateName);
    IMS_RESULT HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName);
    IMS_RESULT FormAutoAccept(IN IMS_BOOL bWithoutOffer);
    void AdjustDirectionWithHeldByMe(IN IMS_BOOL bWithoutOffer);
    IMS_BOOL IsConferenceCallParticipant();

    CallStateName TerminateUssiAfterInfoTransaction();
};

#endif
