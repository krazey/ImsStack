/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100506  hwangoo.park@             Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IPageMessageListener.h"
#include "PageMessageImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PageMessageImpl::PageMessageImpl(IN PageMessage* pPageMessage_) :
        piListener(IMS_NULL),
        pPageMessage(pPageMessage_)
{
    pPageMessage->SetListener(this);
}

PUBLIC VIRTUAL PageMessageImpl::~PageMessageImpl()
{
    if (pPageMessage != IMS_NULL)
    {
        pPageMessage->SetListener(IMS_NULL);
        pPageMessage->Destroy();
    }
}

PRIVATE VIRTUAL void PageMessageImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pPageMessage != IMS_NULL)
    {
        pPageMessage->SetMessageMediator(IMS_NULL);
        pPageMessage->SetListener(IMS_NULL);
        pPageMessage->Destroy();
        pPageMessage = IMS_NULL;
    }

    delete this;
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void PageMessageImpl::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    pPageMessage->SetMessageMediator(piMediator);
}

PRIVATE VIRTUAL IMessage* PageMessageImpl::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetNextRequest();
}

PRIVATE VIRTUAL IMessage* PageMessageImpl::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetNextResponse();
}

PRIVATE VIRTUAL IMessage* PageMessageImpl::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetPreviousResponse(nServiceMethod);
}

PRIVATE VIRTUAL IMessage* PageMessageImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetPreviousRequest(nServiceMethod);
}

PRIVATE VIRTUAL IMSList<IMessage*> PageMessageImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = pPageMessage->GetPreviousResponses(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMSList<AString> PageMessageImpl::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetRemoteUserId();
}

PRIVATE VIRTUAL const ByteArray& PageMessageImpl::GetContent() const
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetContent();
}

PRIVATE VIRTUAL AString PageMessageImpl::GetContentType() const
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetContentType();
}

PRIVATE VIRTUAL IMS_SINT32 PageMessageImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->GetState();
}

PRIVATE VIRTUAL IMS_RESULT PageMessageImpl::Send(
        IN CONST ByteArray& objContent, IN CONST AString& strContentType)
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->Send(objContent, strContentType);
}

PRIVATE VIRTUAL void PageMessageImpl::SetListener(IN IPageMessageListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT PageMessageImpl::Accept(IN IMS_SINT32 nStatusCode /* = 200 */)
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->Accept(nStatusCode);
}

PRIVATE VIRTUAL IMS_RESULT PageMessageImpl::Reject(
        IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pPageMessage->Reject(nStatusCode, nRetryAfter);
}

PRIVATE VIRTUAL void PageMessageImpl::OnPageMessage_Delivered(IN PageMessage* pPageMessage)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPageMessage != pPageMessage)
    {
        IMS_TRACE_E(0, "PAGE MESSAGE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PageMessageDelivered(this);
}

PRIVATE VIRTUAL void PageMessageImpl::OnPageMessage_DeliveryFailed(IN PageMessage* pPageMessage)
{
    //---------------------------------------------------------------------------------------------

    if (this->pPageMessage != pPageMessage)
    {
        IMS_TRACE_E(0, "PAGE MESSAGE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->PageMessageDeliveryFailed(this);
}
