/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20201023  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "base/IMS.h"
#include "IMSCore.h"
#include "media/FramedMediaImpl.h"
#include "media/StreamMediaImpl.h"
#include "SessionDescriptor.h"
#include "VirtualSessionImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

LOCAL
void virtualSessionImpl_MethodNotSupported()
{
    IMS_TRACE_I("Method call is not supported...", 0, 0, 0);
}

PUBLIC
VirtualSessionImpl::VirtualSessionImpl(IN ISession* piOwnerSession_, IN VirtualSession* pSession_) :
        piOwnerSession(piOwnerSession_),
        pSession(pSession_)
{
    IMS_TRACE_I("VirtualSessionImpl - C", 0, 0, 0);
}

PUBLIC VIRTUAL VirtualSessionImpl::~VirtualSessionImpl()
{
    IMS_TRACE_I("VirtualSessionImpl - D", 0, 0, 0);

    piOwnerSession = IMS_NULL;
    pSession = IMS_NULL;

    if (!objMediaImpls.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objMediaImpls.GetSize(); ++i)
        {
            MediaImpl* pMediaImpl = objMediaImpls.GetAt(i);

            if (pMediaImpl != IMS_NULL)
            {
                delete pMediaImpl;
            }
        }

        objMediaImpls.Clear();
    }
}

PRIVATE VIRTUAL void VirtualSessionImpl::Destroy()
{
    virtualSessionImpl_MethodNotSupported();
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void VirtualSessionImpl::SetMessageMediator(IN IMessageMediator* /*piMediator*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL IMessage* VirtualSessionImpl::GetNextRequest()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMessage* VirtualSessionImpl::GetNextResponse()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMessage* VirtualSessionImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetPreviousRequest(nServiceMethod);
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL IMessage* VirtualSessionImpl::GetPreviousResponse(
        IN IMS_SINT32 nServiceMethod) const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetPreviousResponse(nServiceMethod);
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL IMSList<IMessage*> VirtualSessionImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetPreviousResponses(nServiceMethod);
    }

    return IMSList<IMessage*>();
}

PRIVATE VIRTUAL IMSList<AString> VirtualSessionImpl::GetRemoteUserId() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetRemoteUserId();
    }

    return IMSList<AString>();
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Accept()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL ICapabilities* VirtualSessionImpl::CreateCapabilities()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMedia* VirtualSessionImpl::CreateMedia(IN CONST AString& strType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nCountOfDescriptor /* = 0 */,
        IN IMS_BOOL bIMSExtension /* = IMS_TRUE */)
{
    if (pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMS_NULL;
    }

    Media* pMedia = pSession->CreateMedia(strType, nDirection, nCountOfDescriptor, bIMSExtension);

    if (pMedia == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaImpl* pMediaImpl = IMS_NULL;

    switch (pMedia->GetType())
    {
        case IMSCore::MEDIA_TYPE_STREAM:
            pMediaImpl = new StreamMediaImpl(DYNAMIC_CAST(StreamMedia*, pMedia));
            break;

        case IMSCore::MEDIA_TYPE_FRAMED:
            pMediaImpl = new FramedMediaImpl(DYNAMIC_CAST(FramedMedia*, pMedia));
            break;

        case IMSCore::MEDIA_TYPE_BASIC_RELIABLE:
            break;

        case IMSCore::MEDIA_TYPE_BASIC_UNRELIABLE:
            break;

        default:
            break;
    }

    if (pMediaImpl == IMS_NULL)
    {
        pSession->RemoveMedia(pMedia);

        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!objMediaImpls.Append(pMediaImpl))
    {
        delete pMediaImpl;
        pSession->RemoveMedia(pMedia);

        IMS::SetLastError(IMSError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pMediaImpl->GetInterface();
}

PRIVATE VIRTUAL IReference* VirtualSessionImpl::CreateReference(
        IN CONST AString& /*strReferTo*/, IN CONST AString& /*strReferMethod*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMSList<IMedia*> VirtualSessionImpl::GetMedia()
{
    if (pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMSList<IMedia*>();
    }

    const IMSList<Media*>& objMedias = pSession->GetMedia();

    if (objMedias.GetSize() > objMediaImpls.GetSize())
    {
        MediaImpl* pMediaImpl;

        // Newly added media
        for (IMS_UINT32 i = objMediaImpls.GetSize(); i < objMedias.GetSize(); ++i)
        {
            Media* pMedia = objMedias.GetAt(i);

            switch (pMedia->GetType())
            {
                case IMSCore::MEDIA_TYPE_STREAM:
                    pMediaImpl = new StreamMediaImpl(DYNAMIC_CAST(StreamMedia*, pMedia));
                    break;

                case IMSCore::MEDIA_TYPE_FRAMED:
                    pMediaImpl = new FramedMediaImpl(DYNAMIC_CAST(FramedMedia*, pMedia));
                    break;

                case IMSCore::MEDIA_TYPE_BASIC_RELIABLE:
                    break;

                case IMSCore::MEDIA_TYPE_BASIC_UNRELIABLE:
                    break;

                default:
                    break;
            }

            objMediaImpls.Append(pMediaImpl);

            pMediaImpl = IMS_NULL;
        }
    }

    if (objMediaImpls.IsEmpty())
    {
        IMS_TRACE_D("No media in the current session", 0, 0, 0);
        return IMSList<IMedia*>();
    }

    IMSList<IMedia*> objIMedias;

    for (IMS_UINT32 i = 0; i < objMediaImpls.GetSize(); ++i)
    {
        MediaImpl* pMediaImpl = objMediaImpls.GetAt(i);

        objIMedias.Append(pMediaImpl->GetInterface());
    }

    return objIMedias;
}

PRIVATE VIRTUAL ISessionDescriptor* VirtualSessionImpl::GetSessionDescriptor()
{
    if (pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMS_NULL;
    }

    return pSession->GetSessionDescriptor();
}

PRIVATE VIRTUAL IMS_SINT32 VirtualSessionImpl::GetState() const
{
    if (pSession.IsNull())
    {
        return STATE_TERMINATED;
    }

    return pSession->GetState();
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::HasPendingUpdate() const
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Reject()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Reject(IN IMS_SINT32 /* nStatusCode*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RejectWithDiversion(
        IN CONST AString& /*strAlternativeUserAddress*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RemoveMedia(IN IMedia* piMedia)
{
    if (pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (piMedia == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_UINT32 i;

    for (i = 0; i < objMediaImpls.GetSize(); ++i)
    {
        const MediaImpl* pMediaImpl = objMediaImpls.GetAt(i);

        if (pMediaImpl->Equals(piMedia))
        {
            break;
        }
    }

    if (i >= objMediaImpls.GetSize())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    MediaImpl* pMediaImpl = objMediaImpls.GetAt(i);

    if (pSession->RemoveMedia(pMediaImpl->GetMedia()) != IMS_SUCCESS)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = piMedia->GetState();

    if ((nState == IMedia::STATE_INACTIVE) || (nState == IMedia::STATE_DELETED))
    {
        delete pMediaImpl;

        objMediaImpls.RemoveAt(i);
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Restore()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetListener(IN ISessionListener* /*piListener*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Start()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Terminate()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Update()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

//// IMS extensions

PRIVATE VIRTUAL ISubscription* VirtualSessionImpl::CreateSubscription(
        IN CONST AString& /*strEvent*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL ISipClientConnection* VirtualSessionImpl::CreateTransaction(
        IN CONST SipMethod& /*objMethod*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_SINT32 VirtualSessionImpl::GetConfiguration() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetConfiguration();
    }

    return 0;
}

PRIVATE VIRTUAL const ISipHeader* VirtualSessionImpl::GetContactHeader() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetContactHeader();
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL const Replaces* VirtualSessionImpl::GetReplaces() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetReplaces();
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL const AString& VirtualSessionImpl::GetSessionId() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetSessionId();
    }

    return AString::ConstNull();
}

PRIVATE VIRTUAL IMS_SINT32 VirtualSessionImpl::GetTerminationReason() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->GetTerminationReason();
    }

    return ISession::TERMINATION_REASON_INVALID;
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::IsFinalResponseReceivedForInitialInviteRequest() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->IsFinalResponseReceivedForInitialInviteRequest();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::IsReliableProvResponseSupported() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->IsReliableProvResponseSupported();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::IsSDPNegotiationAllowedForNonRPR() const
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->IsSDPNegotiationAllowedForNonRPR();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RejectEx(IN IMS_SINT32 /*nStatusCode*/,
        IN CONST AString& /*strReasonPhrase = AString::ConstNull() */)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RespondToEarlyUpdate(
        IN IMS_SINT32 /*nStatusCode*/, IN CONST AString& /*strReason = AString::ConstNull() */)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RespondToPRAck(
        IN IMS_SINT32 /*nStatusCode*/, IN CONST AString& /*strReason = AString::ConstNull() */)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendAck()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendPRAck()
{
    if (piOwnerSession != IMS_NULL)
    {
        return piOwnerSession->SendPRAck();
    }

    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendProvisionalResponse(
        IN IMS_SINT32 /*nStatusCode*/, IN CONST AString& /*strReason = AString::ConstNull()*/,
        IN IMS_SINT32 /*nFlags = 0*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendRPR(IN IMS_SINT32 /*nStatusCode*/,
        IN CONST AString& /*strReason = AString::ConstNull()*/, IN IMS_BOOL /*bSDP = IMS_TRUE*/,
        IN IMS_SINT32 /*nFlags = 0*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SetCallerPreference(
        IN CONST IMSList<AString>& /*objCallerPreference*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetConfiguration(IN IMS_SINT32 /*nConfigValue*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SetContactParameter(
        IN CONST AString& /*strParameter*/, IN IMS_SINT32 /*nOperation = 0 (0: ADD, 1: REMOVE)*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetImplicitRoutingRequired(IN IMS_BOOL /*bFlag*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetReasonForCallTermination(IN IMS_SINT32 /*nReason*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetRefreshListener(IN IRefreshListener* /*piListener*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetRefreshPolicy(IN IMS_SINT32 /*nPolicy*/,
        IN IMS_SINT32 /*nCriteriaInterval*/, IN IMS_SINT32 /*nValueEorLT*/,
        IN IMS_SINT32 /*nValueGT*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::TerminateEx(
        IN IMS_BOOL /*bTerminateMethodBYE = IMS_FALSE*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::UpdateEarlyMedia()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::UpdateEx(
        IN IMS_SINT32 /*nMethod = SipMethod::INVALID*/, IN IMS_BOOL /*bSessionRefresh = IMS_FALSE*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::CreateFailureSdp()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL void VirtualSessionImpl::DestroyFailureSdp()
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL ISessionParameter* VirtualSessionImpl::GetFailureSdp() const
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}
// }
