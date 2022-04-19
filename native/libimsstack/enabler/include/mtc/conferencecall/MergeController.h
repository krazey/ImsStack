#ifndef MERGE_CONTROLLER_H_
#define MERGE_CONTROLLER_H_

#include "conferencecall/ConferenceController.h"
#include "IMSList.h"

class IMtcCallContext;
class MediaInfo;
class SuppService;
class ConfUser;
struct CallInfo;
struct CallStartOperationParams;

class MergeController final :
        public ConferenceController
{
public:
    explicit MergeController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager);
    virtual ~MergeController();
    MergeController(IN const MergeController&) = delete;
    MergeController& operator=(IN const MergeController&) = delete;

protected:
    void ProcessMerge(IN IMSList<ConfUser*>& objUsers) override;
    void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation* pOperation) override;
    IMS_BOOL IsStartFinalSipfragWaitTimer() const override;
    void Recover() override;
    void OnIndividualCallTerminated(IN IMS_UINTP nCallKey) override;

private:
    void ProcessMergeWithoutRefer(IN IMSList<ConfUser*>& objUsers);
    void UpdateUserStateBySessionTerminated(IN IMS_UINTP nCallKey);
    void RecoverOnCreating();
    void RecoverOnReferring();
    void RecoverOnSubscribing();
    IMS_BOOL RecoverOnConferenceCallFailed();
    void ClearIndividualCallOnMergeFailed();
};

#endif
