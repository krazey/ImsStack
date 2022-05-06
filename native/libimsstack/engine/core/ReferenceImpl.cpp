/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100413  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "base/IMS.h"
#include "INotificationListener.h"
#include "IReferenceListener.h"
#include "SessionImpl.h"
#include "ReferenceImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
ReferenceImpl::ReferenceImpl(IN Reference* pReference_) :
        piListener(IMS_NULL),
        piNotificationListener(IMS_NULL),
        pReference(pReference_)
{
    pReference->SetListener(this);
}

PUBLIC VIRTUAL ReferenceImpl::~ReferenceImpl()
{
    //---------------------------------------------------------------------------------------------

    if (pReference != IMS_NULL)
    {
        pReference->SetListener(IMS_NULL);
        pReference->Destroy();
    }
}

PRIVATE VIRTUAL void ReferenceImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pReference != IMS_NULL)
    {
        pReference->SetMessageMediator(IMS_NULL);
        pReference->SetNotificationListener(IMS_NULL);
        pReference->SetListener(IMS_NULL);
        pReference->Destroy();
        pReference = IMS_NULL;
    }

    delete this;
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void ReferenceImpl::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    pReference->SetMessageMediator(piMediator);
}

PRIVATE VIRTUAL IMessage* ReferenceImpl::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetNextRequest();
}

PRIVATE VIRTUAL IMessage* ReferenceImpl::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetNextResponse();
}

PRIVATE VIRTUAL IMessage* ReferenceImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetPreviousRequest(nServiceMethod);
}

PRIVATE VIRTUAL IMessage* ReferenceImpl::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetPreviousResponse(nServiceMethod);
}

PRIVATE VIRTUAL IMSList<IMessage*> ReferenceImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = pReference->GetPreviousResponses(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMSList<AString> ReferenceImpl::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetRemoteUserId();
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::Accept()
{
    //---------------------------------------------------------------------------------------------

    return pReference->Accept();
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::ConnectReferMethod(IN IServiceMethod* piServiceMethod)
{
    //---------------------------------------------------------------------------------------------

    if (piServiceMethod == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    const SipMethod& objMethod = pReference->GetReferMethod();
    Method* pMethod = IMS_NULL;

    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::BYE))
    {
        SessionImpl* pSessionImpl = DYNAMIC_CAST(SessionImpl*, piServiceMethod);

        pMethod = pSessionImpl->GetSession();
    }

    // FIXME: If any other method needs to be handled, then add the code below...

    return pReference->ConnectReferMethod(pMethod);
}

PRIVATE VIRTUAL const AString& ReferenceImpl::GetReferMethod() const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetReferMethod().ToString();
}

PRIVATE VIRTUAL const AString& ReferenceImpl::GetReferToUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetReferToUserId();
}

PRIVATE VIRTUAL const AString& ReferenceImpl::GetReplaces() const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetReplaces();
}

PRIVATE VIRTUAL IMS_SINT32 ReferenceImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pReference->GetState();
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::Refer(IN IMS_BOOL bImplicitSubscription)
{
    //---------------------------------------------------------------------------------------------

    return pReference->Refer(bImplicitSubscription);
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::Reject()
{
    //---------------------------------------------------------------------------------------------

    return pReference->Reject();
}

PRIVATE VIRTUAL void ReferenceImpl::SetListener(IN IReferenceListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::SetReplaces(IN CONST AString& strSessionId)
{
    //---------------------------------------------------------------------------------------------

    return pReference->SetReplaces(strSessionId);
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::AcceptEx(
        IN IMS_SINT32 nStatusCode /* = 202 */, IN IMS_BOOL b100Trying /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return pReference->AcceptEx(nStatusCode, b100Trying);
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::ReferEx(IN IMS_BOOL bImplicitSubscription,
        IN CONST AString& strHeadersForReferTo /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    return pReference->ReferEx(bImplicitSubscription, strHeadersForReferTo);
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::RejectEx(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    return pReference->RejectEx(nStatusCode);
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::SendNotification(IN IMS_SINT32 nSubState,
        IN CONST ByteArray& objContent,
        IN IMS_SINT32 nReason /* = ISubscriptionState::REASON_NONE */,
        IN IMS_SINT32 nExpires /* = (-1) */)
{
    //---------------------------------------------------------------------------------------------

    return pReference->SendNotification(nSubState, objContent, nReason, nExpires);
}

PRIVATE VIRTUAL void ReferenceImpl::SetNotificationListener(IN INotificationListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    piNotificationListener = piListener;

    if (piNotificationListener != IMS_NULL)
    {
        pReference->SetNotificationListener(this);
    }
    else
    {
        pReference->SetNotificationListener(IMS_NULL);
    }
}

PRIVATE VIRTUAL void ReferenceImpl::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    //---------------------------------------------------------------------------------------------

    return pReference->SetImplicitRoutingRequired(bFlag);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_Delivered(IN Reference* pReference)
{
    //---------------------------------------------------------------------------------------------

    if (this->pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ReferenceDelivered(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_DeliveryFailed(IN Reference* pReference)
{
    //---------------------------------------------------------------------------------------------

    if (this->pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ReferenceDeliveryFailed(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_NotifyReceived(
        IN Reference* pReference, IN Message* pNotify)
{
    //---------------------------------------------------------------------------------------------

    if (this->pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ReferenceNotify(this, pNotify);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_Terminated(IN Reference* pReference)
{
    //---------------------------------------------------------------------------------------------

    if (this->pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ReferenceTerminated(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnNotification_Delivered(IN ServiceMethod* pMethod)
{
    Reference* pReference = DYNAMIC_CAST(Reference*, pMethod);

    //---------------------------------------------------------------------------------------------

    if (this->pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piNotificationListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER (INotificationListener)", 0, 0, 0);
        return;
    }

    piNotificationListener->NotificationDelivered(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnNotification_DeliveryFailed(
        IN ServiceMethod* pMethod, IN IMS_SINT32 nStatusCode)
{
    Reference* pReference = DYNAMIC_CAST(Reference*, pMethod);

    //---------------------------------------------------------------------------------------------

    if (this->pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piNotificationListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER (INotificationListener)", 0, 0, 0);
        return;
    }

    piNotificationListener->NotificationDeliveryFailed(this, nStatusCode);
}
