/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090719  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "base/Ims.h"
#include "ImsCore.h"
#include "CapabilitiesImpl.h"
#include "ReferenceImpl.h"
#include "SubscriptionImpl.h"
#include "media/FramedMediaImpl.h"
#include "media/StreamMediaImpl.h"
#include "SessionDescriptor.h"
#include "ISessionListener.h"
#include "SessionImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SessionImpl::SessionImpl(IN SessionEx* pSession_) :
        piListener(IMS_NULL),
        pSession(pSession_),
        pVirtualSessionImpl(IMS_NULL)
{
    pSession->SetListener(this);
    pSession->SetExListener(this);
}

PUBLIC VIRTUAL SessionImpl::~SessionImpl()
{
    UpdateVirtualSession(IMS_NULL);

    if (pSession != IMS_NULL)
    {
        pSession->SetListener(IMS_NULL);
        pSession->SetExListener(IMS_NULL);
        pSession->Destroy();
    }

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

PRIVATE VIRTUAL void SessionImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pSession != IMS_NULL)
    {
        pSession->SetMessageMediator(IMS_NULL);
        pSession->SetListener(IMS_NULL);
        pSession->SetExListener(IMS_NULL);
        pSession->SetRefreshListener(IMS_NULL);
        pSession->Destroy();
        pSession = IMS_NULL;
    }

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

    delete this;
}

// SIP_MESSAGE_MEDIATOR
PRIVATE VIRTUAL void SessionImpl::SetMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    pSession->SetMessageMediator(piMediator);
}

PRIVATE VIRTUAL IMessage* SessionImpl::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetNextRequest();
}

PRIVATE VIRTUAL IMessage* SessionImpl::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetNextResponse();
}

PRIVATE VIRTUAL IMessage* SessionImpl::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetPreviousRequest(nServiceMethod);
}

PRIVATE VIRTUAL IMessage* SessionImpl::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    return pSession->GetPreviousResponse(nServiceMethod);
}

PRIVATE VIRTUAL IMSList<IMessage*> SessionImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = pSession->GetPreviousResponses(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMSList<AString> SessionImpl::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetRemoteUserId();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Accept()
{
    //---------------------------------------------------------------------------------------------

    return pSession->Accept();
}

PRIVATE VIRTUAL ICapabilities* SessionImpl::CreateCapabilities()
{
    Capabilities* pCapabilities = pSession->CreateCapabilities();

    //---------------------------------------------------------------------------------------------

    if (pCapabilities == IMS_NULL)
    {
        return IMS_NULL;
    }

    CapabilitiesImpl* pCapabilitiesImpl = new CapabilitiesImpl(pCapabilities);

    if (pCapabilitiesImpl == IMS_NULL)
    {
        pCapabilities->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating CapabilitiesImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pCapabilitiesImpl;
}

PRIVATE VIRTUAL IMedia* SessionImpl::CreateMedia(IN CONST AString& strType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nCountOfDescriptor /* = 0 */,
        IN IMS_BOOL bIMSExtension /* = IMS_TRUE */)
{
    Media* pMedia = pSession->CreateMedia(strType, nDirection, nCountOfDescriptor, bIMSExtension);

    //---------------------------------------------------------------------------------------------

    if (pMedia == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaImpl* pMediaImpl = IMS_NULL;

    switch (pMedia->GetType())
    {
        case ImsCore::MEDIA_TYPE_STREAM:
            pMediaImpl = new StreamMediaImpl(DYNAMIC_CAST(StreamMedia*, pMedia));
            break;

        case ImsCore::MEDIA_TYPE_FRAMED:
            pMediaImpl = new FramedMediaImpl(DYNAMIC_CAST(FramedMedia*, pMedia));
            break;

        case ImsCore::MEDIA_TYPE_BASIC_RELIABLE:
            break;

        case ImsCore::MEDIA_TYPE_BASIC_UNRELIABLE:
            break;

        default:
            break;
    }

    if (pMediaImpl == IMS_NULL)
    {
        pSession->RemoveMedia(pMedia);

        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!objMediaImpls.Append(pMediaImpl))
    {
        delete pMediaImpl;
        pSession->RemoveMedia(pMedia);

        Ims::SetLastError(ImsError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pMediaImpl->GetInterface();
}

PRIVATE VIRTUAL IReference* SessionImpl::CreateReference(
        IN CONST AString& strReferTo, IN CONST AString& strReferMethod)
{
    Reference* pReference = pSession->CreateReference(strReferTo, strReferMethod);

    //---------------------------------------------------------------------------------------------

    if (pReference == IMS_NULL)
    {
        return IMS_NULL;
    }

    ReferenceImpl* pReferenceImpl = new ReferenceImpl(pReference);

    if (pReferenceImpl == IMS_NULL)
    {
        pReference->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating ReferenceImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pReferenceImpl;
}

PRIVATE VIRTUAL IMSList<IMedia*> SessionImpl::GetMedia()
{
    // TODO:: later change the below code
    const IMSList<Media*>& objMedias = pSession->GetMedia();

    //---------------------------------------------------------------------------------------------

    if (objMedias.GetSize() > objMediaImpls.GetSize())
    {
        MediaImpl* pMediaImpl;

        // Newly added media
        for (IMS_UINT32 i = objMediaImpls.GetSize(); i < objMedias.GetSize(); ++i)
        {
            Media* pMedia = objMedias.GetAt(i);

            switch (pMedia->GetType())
            {
                case ImsCore::MEDIA_TYPE_STREAM:
                    pMediaImpl = new StreamMediaImpl(DYNAMIC_CAST(StreamMedia*, pMedia));
                    break;

                case ImsCore::MEDIA_TYPE_FRAMED:
                    pMediaImpl = new FramedMediaImpl(DYNAMIC_CAST(FramedMedia*, pMedia));
                    break;

                case ImsCore::MEDIA_TYPE_BASIC_RELIABLE:
                    break;

                case ImsCore::MEDIA_TYPE_BASIC_UNRELIABLE:
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

PRIVATE VIRTUAL ISessionDescriptor* SessionImpl::GetSessionDescriptor()
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetSessionDescriptor();
}

PRIVATE VIRTUAL IMS_SINT32 SessionImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetState();
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::HasPendingUpdate() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->HasPendingUpdate();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Reject()
{
    //---------------------------------------------------------------------------------------------

    return pSession->Reject();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Reject(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    return pSession->Reject(nStatusCode);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::RejectWithDiversion(
        IN CONST AString& strAlternativeUserAddress)
{
    //---------------------------------------------------------------------------------------------

    return pSession->RejectWithDiversion(strAlternativeUserAddress);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::RemoveMedia(IN IMedia* piMedia)
{
    //---------------------------------------------------------------------------------------------

    if (piMedia == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
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
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    MediaImpl* pMediaImpl = objMediaImpls.GetAt(i);

    if (pSession->RemoveMedia(pMediaImpl->GetMedia()) != IMS_SUCCESS)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = piMedia->GetState();

    if ((nState == IMedia::STATE_INACTIVE) || (nState == IMedia::STATE_DELETED))
    {
        delete pMediaImpl;

        objMediaImpls.RemoveAt(i);
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Restore()
{
    //---------------------------------------------------------------------------------------------

    return pSession->Restore();
}

PRIVATE VIRTUAL void SessionImpl::SetListener(IN ISessionListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Start()
{
    //---------------------------------------------------------------------------------------------

    return pSession->Start();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Terminate()
{
    //---------------------------------------------------------------------------------------------

    return pSession->Terminate();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::Update()
{
    //---------------------------------------------------------------------------------------------

    return pSession->Update();
}

//// IMS extensions

PRIVATE VIRTUAL ISubscription* SessionImpl::CreateSubscription(IN CONST AString& strEvent)
{
    //---------------------------------------------------------------------------------------------

    Subscription* pSubscription = pSession->CreateSubscription(strEvent);

    if (pSubscription == IMS_NULL)
    {
        return IMS_NULL;
    }

    SubscriptionImpl* pSubscriptionImpl = new SubscriptionImpl(pSubscription);

    if (pSubscriptionImpl == IMS_NULL)
    {
        pSubscription->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SubscriptionImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pSubscriptionImpl;
}

PRIVATE VIRTUAL ISipClientConnection* SessionImpl::CreateTransaction(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    return pSession->CreateTransaction(objMethod);
}

PRIVATE VIRTUAL IMS_SINT32 SessionImpl::GetConfiguration() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetConfiguration();
}

PRIVATE VIRTUAL const ISipHeader* SessionImpl::GetContactHeader() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetContactHeader();
}

PRIVATE VIRTUAL const Replaces* SessionImpl::GetReplaces() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetReplaces();
}

PRIVATE VIRTUAL const AString& SessionImpl::GetSessionId() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetSessionId();
}

PRIVATE VIRTUAL IMS_SINT32 SessionImpl::GetTerminationReason() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->GetTerminationReason();
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::IsFinalResponseReceivedForInitialInviteRequest() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->IsFinalResponseReceivedForInitialInviteRequest();
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::IsReliableProvResponseSupported() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->IsReliableProvResponseSupported();
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::IsSDPNegotiationAllowedForNonRPR() const
{
    //---------------------------------------------------------------------------------------------

    return pSession->IsSDPNegotiationAllowedForNonRPR();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::RejectEx(
        IN IMS_SINT32 nStatusCode, IN CONST AString& strReasonPhrase /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->RejectEx(nStatusCode, strReasonPhrase);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::RespondToEarlyUpdate(
        IN IMS_SINT32 nStatusCode, IN CONST AString& strReason /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->RespondToEarlyUpdate(nStatusCode, strReason);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::RespondToPRAck(
        IN IMS_SINT32 nStatusCode, IN CONST AString& strReason /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->RespondToPRAck(nStatusCode, strReason);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::SendAck()
{
    //---------------------------------------------------------------------------------------------

    return pSession->SendAck();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::SendPRAck()
{
    //---------------------------------------------------------------------------------------------

    return pSession->SendPRAck();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
        IN CONST AString& strReason /* = AString::ConstNull() */, IN IMS_SINT32 nFlags /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->SendProvisionalResponse(nStatusCode, strReason, nFlags);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::SendRPR(IN IMS_SINT32 nStatusCode,
        IN CONST AString& strReason /* = AString::ConstNull() */, IN IMS_BOOL bSDP /* = IMS_TRUE */,
        IN IMS_SINT32 nFlags /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->SendRPR(nStatusCode, strReason, bSDP, nFlags);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::SetCallerPreference(
        IN CONST IMSList<AString>& objCallerPreference)
{
    //---------------------------------------------------------------------------------------------

    return pSession->SetCallerPreference(objCallerPreference);
}

PRIVATE VIRTUAL void SessionImpl::SetConfiguration(IN IMS_SINT32 nConfigValue)
{
    //---------------------------------------------------------------------------------------------

    pSession->SetConfiguration(nConfigValue);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::SetContactParameter(
        IN CONST AString& strParameter, IN IMS_SINT32 nOperation /* = 0 (0: ADD, 1: REMOVE) */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->SetContactParameter(strParameter, nOperation);
}

PRIVATE VIRTUAL void SessionImpl::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    //---------------------------------------------------------------------------------------------

    return pSession->SetImplicitRoutingRequired(bFlag);
}

PRIVATE VIRTUAL void SessionImpl::SetReasonForCallTermination(IN IMS_SINT32 nReason)
{
    //---------------------------------------------------------------------------------------------

    return pSession->SetReasonForCallTermination(nReason);
}

PRIVATE VIRTUAL void SessionImpl::SetRefreshListener(IN IRefreshListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    pSession->SetRefreshListener(piListener);
}

PRIVATE VIRTUAL void SessionImpl::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    pSession->SetRefreshPolicy(nPolicy, nCriteriaInterval, nValueEorLT, nValueGT);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::TerminateEx(
        IN IMS_BOOL bTerminateMethodBYE /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->TerminateEx(bTerminateMethodBYE);
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::UpdateEarlyMedia()
{
    //---------------------------------------------------------------------------------------------

    return pSession->UpdateEarlyMedia();
}

PRIVATE VIRTUAL IMS_RESULT SessionImpl::UpdateEx(IN IMS_SINT32 nMethod /* = SipMethod::INVALID */,
        IN IMS_BOOL bSessionRefresh /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    return pSession->UpdateEx(nMethod, bSessionRefresh);
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
PRIVATE VIRTUAL IMS_RESULT SessionImpl::CreateFailureSdp()
{
    return pSession->CreateFailureSdp();
}

PRIVATE VIRTUAL void SessionImpl::DestroyFailureSdp()
{
    pSession->DestroyFailureSdp();
}

PRIVATE VIRTUAL ISessionParameter* SessionImpl::GetFailureSdp() const
{
    return pSession->GetFailureSdp();
}
// }

PRIVATE VIRTUAL void SessionImpl::OnSession_Alerting(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionAlerting(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_ReferenceReceived(
        IN Session* pSession, IN Reference* pReference)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    ReferenceImpl* pReferenceImpl = new ReferenceImpl(pReference);

    if (pReferenceImpl == IMS_NULL)
    {
        pReference->Destroy();
        return;
    }

    piListener->SessionReferenceReceived(this, pReferenceImpl);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Started(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        UpdateVirtualSession(IMS_NULL);
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionStarted(this);

    UpdateVirtualSession(IMS_NULL);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_StartFailed(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionStartFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Terminated(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionTerminated(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Updated(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionUpdated(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_UpdateFailed(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionUpdateFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_UpdateReceived(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionUpdateReceived(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_CancelDelivered(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionCancelDelivered(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_CancelDeliveryFailed(IN Session* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionCancelDeliveryFailed(this);
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::OnSession_ForkedResponseReceived(
        IN Session* pSession, IN Session* pForkedSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return IMS_FALSE;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return IMS_FALSE;
    }

    SessionEx* pSessionEx = DYNAMIC_CAST(SessionEx*, pForkedSession);

    if (pSessionEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "Forked Session is null", 0, 0, 0);
        return IMS_FALSE;
    }

    SessionImpl* pForkedSessionImpl = new SessionImpl(pSessionEx);

    if (pForkedSessionImpl == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a forked SessionImpl failed", 0, 0, 0);
        return IMS_FALSE;
    }

    piListener->SessionForkedResponseReceived(this, pForkedSessionImpl);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SessionImpl::OnSession_ProvisionalResponseReceived(
        IN Session* pSession, IN IMS_UINT32 nIndex /* = 0xFFFFFFFF */)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    UpdateVirtualSession(pSession->GetVirtualEarlySession());

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionProvisionalResponseReceived(this, nIndex);
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::OnSession_TransactionReceived(
        IN Session* pSession, IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return IMS_FALSE;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return IMS_FALSE;
    }

    piListener->SessionTransactionReceived(this, piSSC);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_EarlyMediaUpdated(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionEarlyMediaUpdated(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_EarlyMediaUpdateFailed(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionEarlyMediaUpdateFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_EarlyMediaUpdateReceived(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionEarlyMediaUpdateReceived(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_PRAckDelivered(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionPRAckDelivered(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_PRAckDeliveryFailed(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionPRAckDeliveryFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_PRAckReceived(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionPRAckReceived(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_RPRDeliveryFailed(IN SessionEx* pSessionEx)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionRPRDeliveryFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_RPRReceived(IN SessionEx* pSessionEx,
        IN VirtualSession* pVirtualSession, IN IMS_UINT32 nIndex /* = 0xFFFFFFFF */)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    UpdateVirtualSession(pVirtualSession);

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->SessionRPRReceived(this, nIndex);
}

PRIVATE
void SessionImpl::UpdateVirtualSession(IN VirtualSession* pVirtualSession)
{
    if (pVirtualSession == IMS_NULL)
    {
        if (pVirtualSessionImpl != IMS_NULL)
        {
            delete pVirtualSessionImpl;
            pVirtualSessionImpl = IMS_NULL;
        }
    }
    else
    {
        if (pVirtualSessionImpl == IMS_NULL)
        {
            pVirtualSessionImpl = new VirtualSessionImpl(this, pVirtualSession);
        }
        else
        {
            pVirtualSessionImpl->UpdateSession(pVirtualSession);
        }
    }
}
