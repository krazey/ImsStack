#ifndef GROUP_CALL_CONTROLLER_H_
#define GROUP_CALL_CONTROLLER_H_

#include "conferencecall/ConferenceController.h"
#include "IMSList.h"

class IMtcCallContext;
class IConferenceReference;
class ConfUser;
class MediaInfo;
class SuppService;
struct CallInfo;
struct CallStartOperationParams;

class GroupCallController final : public ConferenceController
{
public:
    explicit GroupCallController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager);
    virtual ~GroupCallController();
    GroupCallController(IN const GroupCallController&) = delete;
    GroupCallController& operator=(IN const GroupCallController&) = delete;

public:
    void OnReferenceStartFailed(IN IConferenceReference* piConfRef) override;

protected:
    void ProcessGroupCall(IN IMSList<ConfUser*>& objUsers, IN CallInfo& objCallInfo,
            IN MediaInfo& objMediaInfo,
            IN IMSMap<SuppType, SuppService*>& objSuppServices) override;
    void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation* pOperation) override;
    void Recover() override;

private:
    void RecoverOnCreating();
    void RecoverOnReferring();
};

#endif
