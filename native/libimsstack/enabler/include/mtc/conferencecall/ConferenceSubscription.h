#ifndef CONFERENCE_SUBSCRIPTION_H_
#define CONFERENCE_SUBSCRIPTION_H_

#include "ISubscriptionListener.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "call/IMtcCall.h"

class ConferenceParticipantList;
class ISession;
class ICoreService;
class ISubscription;
class IMtcContext;
enum class SubscriptionState;

class ConferenceSubscription final :
        public ISubscriptionListener
{
public:
    explicit ConferenceSubscription(IN IMtcContext& objContext, IN CallKey nConfCallKey,
            IN ConferenceParticipantList& objList, IN IConferenceSubscriptionListener& objListener);
    ~ConferenceSubscription();
    ConferenceSubscription(IN const ConferenceSubscription&) = delete;
    ConferenceSubscription& operator=(IN const ConferenceSubscription&) = delete;

public:
    // ISubscriptionListener implementation
    inline void SubscriptionForkedNotify(IN ISubscription*, IN ISubscription*) override {}
    void SubscriptionNotify(IN ISubscription* piSubscription, IN IMessage* piNotify) override;
    void SubscriptionStarted(IN ISubscription* piSubscription) override;
    void SubscriptionStartFailed(IN ISubscription* piSubscription) override;
    void SubscriptionTerminated(IN ISubscription* piSubscription) override;

    IMS_RESULT Subscribe(IN const AString& strTo);
    void UnSubscribe();
    SubscriptionState GetState();

private:
    void SetState(IN SubscriptionState nState);
    void CreateSubscription();
    IMS_RESULT Subscribe();
    void ReSubscribe();
    void Initialize();
    void SetHeaders();
    void UpdateConferenceInfo(IN IMessage* piNotify);
    void HandleUpdateResult(IN IMS_UINT32 nResult);
    void Notify();

    IMS_BOOL OnReceiving403(IN ISubscription* piSubscription);
    IMS_BOOL OnReceiving423(IN ISubscription* piSubscription);

    void ReleaseISubscription();

private:
    static const IMS_UINT32 MAX_RESUBS_COUNT = 3;

    IMtcContext& m_objContext;
    CallKey m_nConfCallKey;
    ConferenceParticipantList& m_objList;
    IConferenceSubscriptionListener& m_objListener;
    AString m_strTo;
    ISubscription* m_piSubscription;
    SubscriptionState m_nState;
    IMS_UINT32 m_nDialogType;
    IMS_SINT32 m_nExpires;
    IMS_UINT32 m_nReSubsCount;
};

enum class SubscriptionState
{
    IDLE = 0,
    SUBSCRIBING,
    ACTIVE,
    UNSUBSCRIBING
};

#endif
