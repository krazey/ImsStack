#ifndef CONFERENCE_CONTROLLER_H_
#define CONFERENCE_CONTROLLER_H_

#include "ServiceTimer.h"
#include "SipStatusCode.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "conferencecall/IConferenceReferenceListener.h"
#include "call/MtcUiNotifier.h"
#include "conferencecall/IConferenceControllerListener.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/IConferenceOperationQueueListener.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/ConferenceSubscription.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "IMtcCallStateListener.h"

class IMtcContext;
class IConferenceReference;
class ConfUser;
class MediaInfo;
class SuppService;
class CallConnectionIdManager;
struct CallInfo;
struct CallStartOperationParams;

class ConferenceController :
        public IMtcCallStateListener,
        public IConferenceSubscriptionListener,
        public IConferenceReferenceListener,
        public IConferenceController,
        public IConferenceOperationQueueListener,
        public ITimerListener
{
public:
    explicit ConferenceController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager);
    virtual ~ConferenceController();
    ConferenceController(IN const ConferenceController&) = delete;
    ConferenceController& operator=(IN const ConferenceController&) = delete;

public:
    // IMtcCallStateListener interface implementation
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState,
            IN Type eType, IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    inline void OnTotalCallStateChanged(IN State) override {}
    inline IMS_BOOL IsSynchronousCallRequired() { return IMS_TRUE; }

    // IConferenceSubscriptionListener interface implementation
    void OnSubscriptionUpdated(IN SubscriptionUpdateType eType) override;

    // IConferenceReferenceListener interface implementations
    void OnReferenceStarted(IN IConferenceReference* piConfRef) override;
    void OnReferenceStartFailed(IN IConferenceReference* piConfRef) override;
    void OnReferenceUpdated(IN IConferenceReference* piConfRef, IN SIPStatusCode nSipFragCode,
            IN ReferSubscriptionState eState) override;

    // ITimerListener interfaces implementation.
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    inline void SetListener(IConferenceControllerListener* piListener)
    { m_piListener = piListener; }

    // TODO: need to optimize.
    // IConferenceController interfaces implementation
    void ProcessCommand(IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN IMSMap<SuppType, SuppService*>& objSuppServices) override;
    void ProcessCommand(IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers) override;
    IMS_SINT32 GetState() const override;
    IndividualCallState GetCallStatusInConference(IN CallKey nKey) const override;

    // IConferenceOperationQueueListener interfaces implementation
    void OnOperationReady() override;

protected:
    // basic operation set
    inline virtual void ProcessGroupCall(IN IMSList<ConfUser*>&, IN CallInfo&, IN MediaInfo&,
            IN IMSMap<SuppType, SuppService*>&) {}
    inline virtual void ProcessExpand(IN IMSList<ConfUser*>&) {}
    inline virtual void ProcessMerge(IN IMSList<ConfUser*>&) {}
    virtual void ProcessJoin(IN IMSList<ConfUser*>& objUsers);
    virtual void ProcessDrop(IN IMSList<ConfUser*>& objUsers);

    void ProcessSubscribeOnParticipant();

    // ConfUser to Participant
    IMS_UINT32 AddUserToParticipantList(IN IMSList<ConfUser*>& objConfUsers,
            IN IMS_BOOL bReOrder = IMS_FALSE);

    void ClearListForConfUsers(IN IMSList<ConfUser*>& objUsers);

    // real operations.
    virtual IMS_BOOL CreateSubscription();
    virtual void StartSubscription();
    virtual void StopSubscription();
    virtual IConferenceReference* CreateReference(IN ConfUser* pUser);
    virtual IConferenceReference* CreateReference(IN IMSList<ConfUser*>& objUsers);
    void RemoveReference(IN IConferenceReference* piReference);
    void ClearOngoingReferences();
    virtual void UpdateUserStatusByReferResult(IN ConfUser* pUser,
            IN IConferenceReference* piConfRef,
            IN SIPStatusCode nStatusCode = SIPStatusCode::SC_200);

    void NotifyConferenceInfo();
    void NotifyUsersInfo();

    inline virtual void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation*) {}
    virtual void SubscribeConference(IN IMS_BOOL bUnsub = IMS_FALSE);
    virtual void CheckUserEntityConnected(IN ConfUser* pConfUser);
    virtual void InviteParticipants(IN IMSList<ConfUser*> objUsers);
    virtual void RemoveParticipants(IN IMSList<ConfUser*> objUsers);
    virtual void NotifyCmdResult();
    virtual void TerminateIndividualCall(IN IMS_UINT32 nConnectionId);
    virtual void TerminateConference(IN IMS_SINT32 nTerminateReason);

    IMS_BOOL CompleteCurrentAndDoNextOperation(IN IMS_UINT32 nOperationType,
            IN ConfUser* pConfUser = IMS_NULL);
    void DoNextOperation();

    void CheckNStartFinalSipfragWaitTimer(IN IMS_UINT32 nNewCondition);
    void StopFinalSipfragWaitTimer();

    // child class must implement these.
    virtual IMS_BOOL IsStartFinalSipfragWaitTimer() const;
    virtual void Recover();

    virtual void OnCallUpdated(IN IMS_UINT32 nEvent, IN IMS_UINTP nCallKey);
    virtual void OnIndividualCallTerminated(IN IMS_UINTP nCallKey);
    void SendClosed();
    void NotifyResultToConferenceCall();

    void GetFocusAddress(OUT AString& strAddress) const;

    void Init();
    IMtcCall* GetConferenceCall() const;
    IMS_BOOL IsReadyToPerformCmd() const;
    IMS_BOOL IsConditionMet(IN IMS_UINT32 nCondition) const;
    void SetState(IN IMS_SINT32 nState);
    const IMS_CHAR* ConvertStateToString(IN IMS_SINT32 nState) const;

public:
    enum
    {
        STATE_CREATED           = 0,
        STATE_GROUPCALLING      = 1,
        STATE_EXPANDING         = 2,
        STATE_MERGING           = 3,
        STATE_JOINING           = 4,
        STATE_DROPPING          = 5,
        STATE_IDLE              = 6
    };

protected:
    enum
    {
        SESSION_STARTED      = 0,
        SESSION_STARTFAILED  = 1,
        SESSION_TERMINATED   = 2,
        SESSION_DESTROYED    = 3
    };

    enum
    {
        REFER_INVITE_NONE       = 0,
        REFER_INVITE_SINGLE     = 1,
        REFER_INVITE_MULTIPLE   = 2
    };

    enum
    {
        CONDITION_NONE                  = 0,
        CONDITION_SIPFRAG_100_RECEIVED  = 1,
        CONDITION_1TO1_TERMINATED       = 2
    };

    IConferenceControllerListener* m_piListener;

    // FIXME : has IMtcCall reference or has only call id?

    CallKey m_nConfCallKey;
    IMtcContext& m_objContext;
    IMtcCallManager& m_objCallManager;
    CallConnectionIdManager& m_objConnectionIdManager;
    ConferenceParticipantList m_objParticipantList;
    ConferenceEventNotifier m_objNotifier;
    ConferenceOperationQueue m_objOperationQueue;
    ConferenceSubscription* m_pSubscription;
    IMSList<IConferenceReference*> m_objIConfReferences;
    ITimer* m_piTimer;

    IMS_UINT32 m_nConditionFinalSipfragTimer;
    IMS_SINT32 m_nState;

    static const IMS_UINT32 TIME_FINAL_SIPFRAG_WAIT = 3000;
};

#endif
