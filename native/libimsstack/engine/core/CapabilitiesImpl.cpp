/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091201  toastops@                 Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ICapabilitiesListener.h"
#include "CapabilitiesImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CapabilitiesImpl::CapabilitiesImpl(IN Capabilities* pCapabilities_) :
        piListener(IMS_NULL),
        pCapabilities(pCapabilities_)
{
    pCapabilities->SetListener(this);
}

PUBLIC VIRTUAL CapabilitiesImpl::~CapabilitiesImpl()
{
    //---------------------------------------------------------------------------------------------

    if (pCapabilities != IMS_NULL)
    {
        pCapabilities->SetListener(IMS_NULL);
        pCapabilities->Destroy();
    }
}

PRIVATE VIRTUAL void CapabilitiesImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pCapabilities != IMS_NULL)
    {
        pCapabilities->SetMessageMediator(IMS_NULL);
        pCapabilities->SetListener(IMS_NULL);
        pCapabilities->Destroy();
        pCapabilities = IMS_NULL;
    }

    delete this;
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void CapabilitiesImpl::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    pCapabilities->SetMessageMediator(piMediator);
}

PRIVATE VIRTUAL IMessage* CapabilitiesImpl::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetNextRequest();
}

PRIVATE VIRTUAL IMessage* CapabilitiesImpl::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetNextResponse();
}

PRIVATE VIRTUAL IMessage* CapabilitiesImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetPreviousRequest(nServiceMethod);
}

PRIVATE VIRTUAL IMessage* CapabilitiesImpl::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetPreviousResponse(nServiceMethod);
}

PRIVATE VIRTUAL IMSList<IMessage*> CapabilitiesImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = pCapabilities->GetPreviousResponses(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMSList<AString> CapabilitiesImpl::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetRemoteUserId();
}

PRIVATE VIRTUAL IMSList<AString> CapabilitiesImpl::GetRemoteUserIdentities() const
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetRemoteUserIdentities();
}

PRIVATE VIRTUAL IMS_SINT32 CapabilitiesImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->GetState();
}

PRIVATE VIRTUAL IMS_BOOL CapabilitiesImpl::HasCapabilities(IN CONST AString& strConnection) const
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->HasCapabilities(strConnection);
}

PRIVATE VIRTUAL IMS_RESULT CapabilitiesImpl::QueryCapabilities(IN IMS_BOOL bSDPInRequest,
        IN IMS_BOOL bContactInRequest /* = IMS_TRUE */, IN IMS_BOOL bCheckSupport /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->QueryCapabilities(bSDPInRequest, bContactInRequest, bCheckSupport);
}

PRIVATE VIRTUAL IMS_RESULT CapabilitiesImpl::QueryCapabilitiesEx()
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->QueryCapabilitiesEx();
}

PRIVATE VIRTUAL void CapabilitiesImpl::SetListener(IN ICapabilitiesListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT CapabilitiesImpl::Accept(
        IN IMS_BOOL bFeatureInContact /* = IMS_TRUE */, IN IMS_BOOL bCheckSupport /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->Accept(bFeatureInContact, bCheckSupport);
}

PRIVATE VIRTUAL IMS_RESULT CapabilitiesImpl::AcceptEx()
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->AcceptEx();
}

PRIVATE VIRTUAL IMS_RESULT CapabilitiesImpl::Reject(
        IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pCapabilities->Reject(nStatusCode, nRetryAfter);
}

PRIVATE VIRTUAL void CapabilitiesImpl::OnCapabilities_QueryDelivered(IN Capabilities* pCapabilities)
{
    //---------------------------------------------------------------------------------------------

    if (this->pCapabilities != pCapabilities)
    {
        IMS_TRACE_E(0, "CAPABILITIES MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->CapabilityQueryDelivered(this);
}

PRIVATE VIRTUAL void CapabilitiesImpl::OnCapabilities_QueryDeliveryFailed(
        IN Capabilities* pCapabilities)
{
    //---------------------------------------------------------------------------------------------

    if (this->pCapabilities != pCapabilities)
    {
        IMS_TRACE_E(0, "CAPABILITIES MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->CapabilityQueryDeliveryFailed(this);
}
