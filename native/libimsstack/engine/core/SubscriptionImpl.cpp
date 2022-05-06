/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISubscriptionListener.h"
#include "SubscriptionImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SubscriptionImpl::SubscriptionImpl(IN Subscription* pSubscription_) :
        piListener(IMS_NULL),
        pSubscription(pSubscription_)
{
    pSubscription->SetListener(this);
}

PUBLIC VIRTUAL SubscriptionImpl::~SubscriptionImpl()
{
    if (pSubscription != IMS_NULL)
    {
        pSubscription->SetListener(IMS_NULL);
        pSubscription->Destroy();
    }
}

PRIVATE VIRTUAL void SubscriptionImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pSubscription != IMS_NULL)
    {
        pSubscription->SetMessageMediator(IMS_NULL);
        pSubscription->SetListener(IMS_NULL);
        pSubscription->SetRefreshListener(IMS_NULL);
        pSubscription->Destroy();
        pSubscription = IMS_NULL;
    }

    delete this;
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void SubscriptionImpl::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    pSubscription->SetMessageMediator(piMediator);
}

PRIVATE VIRTUAL IMessage* SubscriptionImpl::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetNextRequest();
}

PRIVATE VIRTUAL IMessage* SubscriptionImpl::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetNextResponse();
}

PRIVATE VIRTUAL IMessage* SubscriptionImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetPreviousRequest(nServiceMethod);
}

PRIVATE VIRTUAL IMessage* SubscriptionImpl::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetPreviousResponse(nServiceMethod);
}

PRIVATE VIRTUAL IMSList<IMessage*> SubscriptionImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = pSubscription->GetPreviousResponses(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMSList<AString> SubscriptionImpl::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetRemoteUserId();
}

PRIVATE VIRTUAL const AString& SubscriptionImpl::GetEvent() const
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetEvent();
}

PRIVATE VIRTUAL IMS_SINT32 SubscriptionImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->GetState();
}

PRIVATE VIRTUAL IMS_RESULT SubscriptionImpl::Poll()
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->Poll();
}

PRIVATE VIRTUAL void SubscriptionImpl::SetListener(IN ISubscriptionListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT SubscriptionImpl::Subscribe()
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->Subscribe();
}

PRIVATE VIRTUAL IMS_RESULT SubscriptionImpl::Unsubscribe()
{
    //---------------------------------------------------------------------------------------------

    return pSubscription->Unsubscribe();
}

PRIVATE VIRTUAL void SubscriptionImpl::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    //---------------------------------------------------------------------------------------------

    pSubscription->SetImplicitRoutingRequired(bFlag);
}

PRIVATE VIRTUAL void SubscriptionImpl::SetRefreshListener(IN IRefreshListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    pSubscription->SetRefreshListener(piListener);
}

PRIVATE VIRTUAL void SubscriptionImpl::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    pSubscription->SetRefreshPolicy(nPolicy, nCriteriaInterval, nValueEorLT, nValueGT);
}

PRIVATE VIRTUAL IMS_BOOL SubscriptionImpl::OnSubscription_ForkedNotifyReceived(
        IN Subscription* pSubscription, IN Subscription* pForkedSubscription)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return IMS_FALSE;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pForkedSubscription == IMS_NULL)
    {
        IMS_TRACE_E(0, "No forked subscription", 0, 0, 0);
        return IMS_FALSE;
    }

    SubscriptionImpl* pSubscriptionImpl = new SubscriptionImpl(pForkedSubscription);

    if (pSubscriptionImpl == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a forked SubscriptionImpl failed", 0, 0, 0);
        return IMS_FALSE;
    }

    piListener->SubscriptionForkedNotify(this, pSubscriptionImpl);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_NotifyReceived(
        IN Subscription* pSubscription, IN Message* pNotify, OUT IMS_BOOL& /* bDestroyNotify */)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SubscriptionNotify(this, pNotify);
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_Started(IN Subscription* pSubscription)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SubscriptionStarted(this);
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_StartFailed(IN Subscription* pSubscription)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SubscriptionStartFailed(this);
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_Terminated(IN Subscription* pSubscription)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SubscriptionTerminated(this);
}
