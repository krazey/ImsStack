#ifndef CONFERENCE_EVENT_NOTIFIER_H_
#define CONFERENCE_EVENT_NOTIFIER_H_

#include "call/IMtcCall.h"
#include "MtcDef.h"

class IMtcCallContext;
class CallConnectionIdManager;
class ConferenceParticipantList;

class ConferenceEventNotifier final
{
public:
    // TODO: update this to use JniMtcCallThread. IMtcCallContext should provide
    // TODO: objContext.GetUiNotifier().GetCallThread()
    explicit ConferenceEventNotifier(IN IMtcCallContext& objConfCallContext,
            IN CallConnectionIdManager& objConnectionIdManager);
    ~ConferenceEventNotifier();
    ConferenceEventNotifier(IN const ConferenceEventNotifier&) = delete;
    ConferenceEventNotifier& operator=(IN const ConferenceEventNotifier&) = delete;

public:
    void NotifyMerged(IN ConferenceParticipantList& objParticipantList);
    void NotifyMergeFailed(IN CallReasonInfo objReason);

    void NotifyGroupCallStarted();
    void NotifyGroupCallFailed(IN CallReasonInfo objReason);

    void NotifyExpanded();
    void NotifyExpandFailed(IN CallReasonInfo objReason);

    void NotifyDropped(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);
    void NotifyDropFailed(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);

    void NotifyJoined(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);
    void NotifyJoinFailed(
            IN CallReasonInfo objReason, IN ConferenceParticipantList& objParticipantList);

    void NotifyConferenceInfo(IN ConferenceParticipantList& objParticipantList);
    void NotifyUsersInfo(IN ConferenceParticipantList& objParticipantList);

    void NotifyIndividualCallTerminated(IN CallKey nKey);

private:
    CallInfo* CloneCallInfo();
    MediaInfo* CloneMediaInfo();

    void CheckDisconnectedConfUsersInfo(
            IN ConferenceParticipantList& objParticipantList, IN_OUT IMSList<ConfUser*>& objUsers);

private:
    IMtcCallContext& m_objConfCallContext;
    CallConnectionIdManager& m_objConnectionIdManager;
};

#endif
