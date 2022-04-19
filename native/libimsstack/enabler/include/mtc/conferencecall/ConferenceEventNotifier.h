#ifndef CONFERENCE_EVENT_NOTIFIER_H_
#define CONFERENCE_EVENT_NOTIFIER_H_

#include "call/IMtcCall.h"
#include "MtcDef.h"

class IMtcCall;
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
    void NotifyMergeFailed(IN FailReason failReason);

    void NotifyGroupCallStarted();
    void NotifyGroupCallFailed(IN FailReason failReason);

    void NotifyExpanded();
    void NotifyExpandFailed(IN FailReason failReason);

    void NotifyDropped(IN FailReason failReason, IN ConferenceParticipantList& objParticipantList);
    void NotifyDropFailed(IN FailReason failReason,
            IN ConferenceParticipantList& objParticipantList);

    void NotifyJoined(IN FailReason failReason, IN ConferenceParticipantList& objParticipantList);
    void NotifyJoinFailed(IN FailReason failReason,
            IN ConferenceParticipantList& objParticipantList);

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
