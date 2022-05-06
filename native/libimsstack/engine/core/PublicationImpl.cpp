/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100423  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IPublicationListener.h"
#include "PublicationImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PublicationImpl::PublicationImpl(IN Publication* pPublication_) :
        piListener(IMS_NULL),
        pPublication(pPublication_)
{
    pPublication->SetListener(this);
}

PUBLIC VIRTUAL PublicationImpl::~PublicationImpl()
{
    if (pPublication != IMS_NULL)
    {
        pPublication->SetListener(IMS_NULL);
        pPublication->Destroy();
    }
}

PRIVATE VIRTUAL void PublicationImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pPublication != IMS_NULL)
    {
        pPublication->SetMessageMediator(IMS_NULL);
        pPublication->SetListener(IMS_NULL);
        pPublication->SetRefreshListener(IMS_NULL);
        pPublication->Destroy();
        pPublication = IMS_NULL;
    }

    delete this;
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void PublicationImpl::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    pPublication->SetMessageMediator(piMediator);
}

PRIVATE VIRTUAL IMessage* PublicationImpl::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetNextRequest();
}

PRIVATE VIRTUAL IMessage* PublicationImpl::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetNextResponse();
}

PRIVATE VIRTUAL IMessage* PublicationImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetPreviousRequest(nServiceMethod);
}

PRIVATE VIRTUAL IMessage* PublicationImpl::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetPreviousResponse(nServiceMethod);
}

PRIVATE VIRTUAL IMSList<IMessage*> PublicationImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = pPublication->GetPreviousResponses(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMSList<AString> PublicationImpl::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetRemoteUserId();
}

PRIVATE VIRTUAL const AString& PublicationImpl::GetEvent() const
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetEvent();
}

PRIVATE VIRTUAL IMS_SINT32 PublicationImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pPublication->GetState();
}

PRIVATE VIRTUAL IMS_RESULT PublicationImpl::Publish(
        IN CONST ByteArray& objState, IN CONST AString& strContentType)
{
    //---------------------------------------------------------------------------------------------

    return pPublication->Publish(objState, strContentType);
}

PRIVATE VIRTUAL void PublicationImpl::SetListener(IN IPublicationListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT PublicationImpl::Unpublish()
{
    //---------------------------------------------------------------------------------------------

    return pPublication->Unpublish();
}

PRIVATE VIRTUAL void PublicationImpl::SetRefreshListener(IN IRefreshListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    pPublication->SetRefreshListener(piListener);
}

PRIVATE VIRTUAL void PublicationImpl::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    pPublication->SetRefreshPolicy(nPolicy, nCriteriaInterval, nValueEorLT, nValueGT);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_Delivered(IN Publication* pPublication)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PublicationDelivered(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_DeliveryFailed(IN Publication* pPublication)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PublicationDeliveryFailed(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_Terminated(IN Publication* pPublication)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PublicationTerminated(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_RefreshStarted(IN Publication* pPublication)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PublicationRefreshStarted(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_RefreshCompleted(IN Publication* pPublication)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PublicationRefreshCompleted(this);
}
