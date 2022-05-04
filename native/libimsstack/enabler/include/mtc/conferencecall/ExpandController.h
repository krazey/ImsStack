#ifndef EXPAND_CONTROLLER_H_
#define EXPAND_CONTROLLER_H_

#include "conferencecall/ConferenceController.h"

class IMtcCallContext;
class IConferenceReference;
class ConfUser;
class MediaInfo;
class SuppService;
struct CallInfo;
struct CallStartOperationParams;

class ExpandController final : public ConferenceController
{
public:
    explicit ExpandController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager);
    virtual ~ExpandController();
    ExpandController(IN const ExpandController&) = delete;
    ExpandController& operator=(IN const ExpandController&) = delete;

public:
    // void Updated(IN IMS_UINTP nParam) override;
    void OnReferenceStarted(IN IConferenceReference* piConfRef) override;
    void OnReferenceStartFailed(IN IConferenceReference* piConfRef) override;
    void OnReferenceUpdated(IN IConferenceReference* piConfRef, IN SipStatusCode nSipFragCode,
            IN ReferSubscriptionState eState) override;

protected:
    void ProcessExpand(IN IMSList<ConfUser*>& objUsers) override;
    void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation* pOperation) override;
    IMS_BOOL IsStartFinalSipfragWaitTimer() const override;
    void Recover() override;

    void OnCallUpdated(IN IMS_UINT32 nEvent, IN IMS_UINTP nCallKey) override;
    void UpdateUserStatusByReferResult(IN ConfUser* pUser, IN IConferenceReference* piConfRef,
            IN SipStatusCode nStatusCode = SipStatusCode::SC_200) override;

    void NotifyCmdResult() override;

private:
    void StopMedia1to1Session();
    void Resume1to1Session();
    void ProcessJoinAfterExpand();
    void RecoverOnCreating();
    void RecoverOnReferring();
};

#endif
