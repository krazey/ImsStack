/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONFERENCE_CONTROLLER_H_
#define CONFERENCE_CONTROLLER_H_

#include "IMtcCallStateListener.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "SipStatusCode.h"
#include "call/IMtcUiNotifier.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/ConferenceSubscription.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/IConferenceControllerListener.h"
#include "conferencecall/IConferenceOperationQueueListener.h"
#include "conferencecall/IConferenceReferenceListener.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "helper/IMtcTimerListener.h"
#include "helper/MtcTimerWrapper.h"
#include <memory>

class IMtcContext;
class IConferenceReference;
class SuppService;
class CallConnectionIdManager;
class ConferenceFactory;
struct CallInfo;
struct CallStartOperationParams;
struct ConfUser;
struct MediaInfo;

class ConferenceController :
        public IMtcCallStateListener,
        public IConferenceSubscriptionListener,
        public IConferenceReferenceListener,
        public IConferenceController,
        public IConferenceOperationQueueListener,
        public IMtcTimerListener
{
public:
    explicit ConferenceController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory);
    virtual ~ConferenceController();
    ConferenceController(IN const ConferenceController&) = delete;
    ConferenceController& operator=(IN const ConferenceController&) = delete;

public:
    // IMtcCallStateListener interface implementation
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    inline void OnTotalCallStateChanged(IN State) override {}
    inline IMS_BOOL IsSynchronousCallRequired() override { return IMS_TRUE; }

    // IConferenceSubscriptionListener interface implementation
    void OnSubscriptionUpdated(IN SubscriptionUpdateType eType) override;

    // IConferenceReferenceListener interface implementations
    void OnReferenceStarted(IN IConferenceReference* piConfRef) override;
    void OnReferenceStartFailed(IN IConferenceReference* piConfRef) override;
    void OnReferenceUpdated(IN IConferenceReference* piConfRef, IN IMS_SINT32 nSipFragCode,
            IN ReferSubscriptionState eState) override;

    // IMtcTimerListener interfaces implementation.
    void OnTimerExpired(IN IMS_SINT32 nType) override;

    inline void SetListener(IConferenceControllerListener* piListener)
    {
        m_piListener = piListener;
    }

    // TODO: need to optimize.
    // IConferenceController interfaces implementation
    void ProcessCommand(IN IMS_UINT32 nCmd, IN ImsList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void ProcessCommand(IN IMS_UINT32 nCmd, IN ImsList<ConfUser*>& objUsers) override;
    IMS_SINT32 GetState() const override;
    IndividualCallState GetCallStatusInConference(IN CallKey nKey) const override;

    // IConferenceOperationQueueListener interfaces implementation
    void OnOperationReady() override;

protected:
    // basic operation set
    inline virtual void ProcessGroupCall(
            IN ImsList<ConfUser*>&, IN CallInfo&, IN MediaInfo&, IN ImsMap<SuppType, SuppService*>&)
    {
    }
    inline virtual void ProcessExpand(IN ImsList<ConfUser*>&) {}
    inline virtual void ProcessMerge(IN ImsList<ConfUser*>&) {}
    virtual void ProcessJoin(IN ImsList<ConfUser*>& objUsers);
    virtual void ProcessDrop(IN ImsList<ConfUser*>& objUsers);

    void ProcessSubscribeOnParticipant();

    // ConfUser to Participant
    IMS_UINT32 AddUserToParticipantList(
            IN ImsList<ConfUser*>& objConfUsers, IN IMS_BOOL bReOrder = IMS_FALSE);

    static void ClearListForConfUsers(IN ImsList<ConfUser*>& objUsers);

    // real operations.
    virtual IMS_BOOL CreateSubscription();
    virtual void StartSubscription();
    virtual void StopSubscription();
    virtual IConferenceReference* CreateReference(IN ConfUser* pUser);
    virtual IConferenceReference* CreateReference(IN ImsList<ConfUser*>& objUsers);
    void RemoveReference(IN IConferenceReference* piReference);
    void ClearOngoingReferences();
    virtual void UpdateUserStatusByReferResult(IN ConfUser* pUser,
            IN IConferenceReference* piConfRef, IN IMS_SINT32 nStatusCode = SipStatusCode::SC_200);

    void NotifyConferenceInfo();
    void NotifyUsersInfo();

    inline virtual void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation*) {}
    virtual void SubscribeConference(IN IMS_BOOL bUnsub = IMS_FALSE);
    virtual void CheckUserEntityConnected(IN ConfUser* pConfUser);
    virtual void InviteParticipants(IN ImsList<ConfUser*> objUsers);
    virtual void RemoveParticipants(IN ImsList<ConfUser*> objUsers);
    virtual void NotifyCmdResult();
    virtual void TerminateIndividualCall(IN IMS_UINT32 nConnectionId);
    virtual void TerminateConference(IN IMS_SINT32 nTerminateReason);

    IMS_BOOL CompleteCurrentAndDoNextOperation(
            IN IMS_UINT32 nOperationType, IN ConfUser* pConfUser = IMS_NULL);
    void DoNextOperation();

    void CheckNStartFinalSipfragWaitTimer(IN IMS_UINT32 nNewCondition);
    void StopFinalSipfragWaitTimer();

    // child class must implement these.
    virtual IMS_BOOL IsStartFinalSipfragWaitTimer() const;
    virtual void Recover();

    virtual void OnCallUpdated(IN IMS_UINT32 nType, IN IMS_UINTP nCallKey);
    virtual void OnIndividualCallTerminated(IN IMS_UINTP nCallKey);
    void SendClosed();
    void NotifyResultToConferenceCall();

    void GetFocusAddress(OUT AString& strAddress) const;

    void Init();
    IMtcCall* GetConferenceCall() const;
    IMS_BOOL IsReadyToPerformCmd() const;
    IMS_BOOL IsConditionMet(IN IMS_UINT32 nCondition) const;
    void SetState(IN IMS_SINT32 nState);
    static const IMS_CHAR* ConvertStateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_GROUPCALLING = 1,
        STATE_EXPANDING = 2,
        STATE_MERGING = 3,
        STATE_JOINING = 4,
        STATE_DROPPING = 5,
        STATE_IDLE = 6
    };

protected:
    enum
    {
        CALL_STARTED = 0,
        CALL_STARTFAILED = 1,
        CALL_TERMINATED = 2,
        CALL_DESTROYED = 3
    };

    enum
    {
        REFER_INVITE_NONE = 0,
        REFER_INVITE_SINGLE = 1,
        REFER_INVITE_MULTIPLE = 2
    };

    enum
    {
        CONDITION_NONE = 0,
        CONDITION_SIPFRAG_100_RECEIVED = 1,
        CONDITION_1TO1_TERMINATED = 2
    };

    IConferenceControllerListener* m_piListener;

    // FIXME : has IMtcCall reference or has only call id?

    CallKey m_nConfCallKey;
    IMtcContext& m_objContext;
    IMtcCallManager& m_objCallManager;
    CallConnectionIdManager& m_objConnectionIdManager;
    ConferenceFactory& m_objFactory;
    std::unique_ptr<ConferenceParticipantList> m_pParticipantList;
    std::unique_ptr<ConferenceEventNotifier> m_pNotifier;
    std::unique_ptr<ConferenceOperationQueue> m_pOperationQueue;
    ConferenceSubscription* m_pSubscription;
    ImsList<IConferenceReference*> m_objIConfReferences;
    std::unique_ptr<MtcTimerWrapper> m_pTimer;

    IMS_UINT32 m_nConditionFinalSipfragTimer;
    IMS_SINT32 m_nState;

    static const IMS_UINT32 TIMER_FINAL_SIPFRAG_WAIT = 0;
    static const IMS_UINT32 TIME_FINAL_SIPFRAG_WAIT = 3000;
};

#endif
