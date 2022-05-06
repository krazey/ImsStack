/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100326  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SUBSCRIPTION_IMPL_H_
#define _SUBSCRIPTION_IMPL_H_

#include "ISubscription.h"
#include "IOnSubscriptionListener.h"
#include "Subscription.h"

class SubscriptionImpl : public ISubscription, public IOnSubscriptionListener
{
public:
    explicit SubscriptionImpl(IN Subscription* pSubscription_);
    virtual ~SubscriptionImpl();

private:
    SubscriptionImpl(IN CONST SubscriptionImpl& objRHS);
    SubscriptionImpl& operator=(IN CONST SubscriptionImpl& objRHS);

private:
    // IMethod interface
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // IServiceMethod interface
    virtual IMessage* GetNextRequest();
    virtual IMessage* GetNextResponse();
    virtual IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const;
    virtual IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const;
    virtual IMSList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const;
    virtual IMSList<AString> GetRemoteUserId() const;

    // ISubscription interface
    virtual const AString& GetEvent() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_RESULT Poll();
    virtual void SetListener(IN ISubscriptionListener* piListener);
    virtual IMS_RESULT Subscribe();
    virtual IMS_RESULT Unsubscribe();

    //// IMS extensions
    virtual void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);
    virtual void SetRefreshListener(IN IRefreshListener* piListener);
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);

    // IOnSubscriptionListener interface
    virtual IMS_BOOL OnSubscription_ForkedNotifyReceived(
            IN Subscription* pSubscription, IN Subscription* pForkedSubscription);
    virtual void OnSubscription_NotifyReceived(
            IN Subscription* pSubscription, IN Message* pNotify, OUT IMS_BOOL& bDestroyNotify);
    virtual void OnSubscription_Started(IN Subscription* pSubscription);
    virtual void OnSubscription_StartFailed(IN Subscription* pSubscription);
    virtual void OnSubscription_Terminated(IN Subscription* pSubscription);

private:
    ISubscriptionListener* piListener;

    Subscription* pSubscription;
};

#endif  // _SUBSCRIPTION_IMPL_H_
