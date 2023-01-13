/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ImsCore.h"
#include "SessionDescriptor.h"
#include "VirtualSessionImpl.h"
#include "base/Ims.h"
#include "media/FramedMediaImpl.h"
#include "media/StreamMediaImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

LOCAL
void virtualSessionImpl_MethodNotSupported()
{
    IMS_TRACE_I("Method call is not supported...", 0, 0, 0);
}

PUBLIC
VirtualSessionImpl::VirtualSessionImpl(IN ISession* piOwnerSession, IN VirtualSession* pSession) :
        m_piOwnerSession(piOwnerSession),
        m_pSession(pSession)
{
    IMS_TRACE_I("VirtualSessionImpl - C", 0, 0, 0);
}

PUBLIC VIRTUAL VirtualSessionImpl::~VirtualSessionImpl()
{
    IMS_TRACE_I("VirtualSessionImpl - D", 0, 0, 0);

    m_piOwnerSession = IMS_NULL;
    m_pSession = IMS_NULL;

    if (!m_objMediaImpls.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objMediaImpls.GetSize(); ++i)
        {
            MediaImpl* pMediaImpl = m_objMediaImpls.GetAt(i);

            if (pMediaImpl != IMS_NULL)
            {
                delete pMediaImpl;
            }
        }

        m_objMediaImpls.Clear();
    }
}

PRIVATE VIRTUAL void VirtualSessionImpl::Destroy()
{
    virtualSessionImpl_MethodNotSupported();
}

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
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetPreviousRequest(nServiceMethod);
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL IMessage* VirtualSessionImpl::GetPreviousResponse(
        IN IMS_SINT32 nServiceMethod) const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetPreviousResponse(nServiceMethod);
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL IMSList<IMessage*> VirtualSessionImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetPreviousResponses(nServiceMethod);
    }

    return IMSList<IMessage*>();
}

PRIVATE VIRTUAL IMSList<AString> VirtualSessionImpl::GetRemoteUserId() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetRemoteUserId();
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

PRIVATE VIRTUAL IMedia* VirtualSessionImpl::CreateMedia(IN const AString& strType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nCountOfDescriptor /* = 0 */)
{
    if (m_pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMS_NULL;
    }

    Media* pMedia = m_pSession->CreateMedia(strType, nDirection, nCountOfDescriptor);

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
        default:
            break;
    }

    if (pMediaImpl == IMS_NULL)
    {
        m_pSession->RemoveMedia(pMedia);

        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!m_objMediaImpls.Append(pMediaImpl))
    {
        delete pMediaImpl;
        m_pSession->RemoveMedia(pMedia);

        Ims::SetLastError(ImsError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pMediaImpl->GetInterface();
}

PRIVATE VIRTUAL IReference* VirtualSessionImpl::CreateReference(
        IN const AString& /*strReferTo*/, IN const AString& /*strReferMethod*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMSList<IMedia*> VirtualSessionImpl::GetMedia()
{
    if (m_pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMSList<IMedia*>();
    }

    const IMSList<Media*>& objMedias = m_pSession->GetMedia();

    if (objMedias.GetSize() > m_objMediaImpls.GetSize())
    {
        MediaImpl* pMediaImpl;

        // Newly added media
        for (IMS_UINT32 i = m_objMediaImpls.GetSize(); i < objMedias.GetSize(); ++i)
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
                default:
                    break;
            }

            m_objMediaImpls.Append(pMediaImpl);

            pMediaImpl = IMS_NULL;
        }
    }

    if (m_objMediaImpls.IsEmpty())
    {
        IMS_TRACE_D("No media in the current session", 0, 0, 0);
        return IMSList<IMedia*>();
    }

    IMSList<IMedia*> objIMedias;

    for (IMS_UINT32 i = 0; i < m_objMediaImpls.GetSize(); ++i)
    {
        MediaImpl* pMediaImpl = m_objMediaImpls.GetAt(i);

        objIMedias.Append(pMediaImpl->GetInterface());
    }

    return objIMedias;
}

PRIVATE VIRTUAL ISessionDescriptor* VirtualSessionImpl::GetSessionDescriptor()
{
    if (m_pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMS_NULL;
    }

    return m_pSession->GetSessionDescriptor();
}

PRIVATE VIRTUAL IMS_SINT32 VirtualSessionImpl::GetState() const
{
    if (m_pSession.IsNull())
    {
        return STATE_TERMINATED;
    }

    return m_pSession->GetState();
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

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::Reject(IN IMS_SINT32 /*nStatusCode*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RejectWithDiversion(
        IN const AString& /*strAlternativeUserAddress*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RemoveMedia(IN IMedia* piMedia)
{
    if (m_pSession.IsNull())
    {
        IMS_TRACE_E(0, "VirtualSession is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (piMedia == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_UINT32 i;

    for (i = 0; i < m_objMediaImpls.GetSize(); ++i)
    {
        const MediaImpl* pMediaImpl = m_objMediaImpls.GetAt(i);

        if (pMediaImpl->Equals(piMedia))
        {
            break;
        }
    }

    if (i >= m_objMediaImpls.GetSize())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    MediaImpl* pMediaImpl = m_objMediaImpls.GetAt(i);

    if (m_pSession->RemoveMedia(pMediaImpl->GetMedia()) != IMS_SUCCESS)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = piMedia->GetState();

    if ((nState == IMedia::STATE_INACTIVE) || (nState == IMedia::STATE_DELETED))
    {
        delete pMediaImpl;

        m_objMediaImpls.RemoveAt(i);
    }

    Ims::SetLastError(ImsError::NO_ERROR);

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

PRIVATE VIRTUAL ISubscription* VirtualSessionImpl::CreateSubscription(
        IN const AString& /*strEvent*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL ISipClientConnection* VirtualSessionImpl::CreateTransaction(
        IN const SipMethod& /*objMethod*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_SINT32 VirtualSessionImpl::GetConfiguration() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetConfiguration();
    }

    return 0;
}

PRIVATE VIRTUAL const ISipHeader* VirtualSessionImpl::GetContactHeader() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetContactHeader();
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL const Replaces* VirtualSessionImpl::GetReplaces() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetReplaces();
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL const AString& VirtualSessionImpl::GetSessionId() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetSessionId();
    }

    return AString::ConstNull();
}

PRIVATE VIRTUAL IMS_SINT32 VirtualSessionImpl::GetTerminationReason() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->GetTerminationReason();
    }

    return ISession::TERMINATION_REASON_INVALID;
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::IsFinalResponseReceivedForInitialInviteRequest() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->IsFinalResponseReceivedForInitialInviteRequest();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::IsReliableProvResponseSupported() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->IsReliableProvResponseSupported();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_BOOL VirtualSessionImpl::IsSdpNegotiationAllowedForNonRpr() const
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->IsSdpNegotiationAllowedForNonRpr();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RejectEx(
        IN IMS_SINT32 /*nStatusCode*/, IN const AString& /*strReasonPhrase = AString::ConstNull()*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RespondToEarlyUpdate(
        IN IMS_SINT32 /*nStatusCode*/, IN const AString& /*strReason = AString::ConstNull()*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::RespondToPrack(
        IN IMS_SINT32 /*nStatusCode*/, IN const AString& /*strReason = AString::ConstNull()*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendAck()
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendPrack()
{
    if (m_piOwnerSession != IMS_NULL)
    {
        return m_piOwnerSession->SendPrack();
    }

    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendProvisionalResponse(
        IN IMS_SINT32 /*nStatusCode*/, IN const AString& /*strReason = AString::ConstNull()*/,
        IN IMS_SINT32 /*nFlags = 0*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SendRpr(IN IMS_SINT32 /*nStatusCode*/,
        IN const AString& /*strReason = AString::ConstNull()*/, IN IMS_BOOL /*bSDP = IMS_TRUE*/,
        IN IMS_SINT32 /*nFlags = 0*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SetCallerPreference(
        IN const IMSList<AString>& /*objCallerPreference*/)
{
    virtualSessionImpl_MethodNotSupported();
    return IMS_FAILURE;
}

PRIVATE VIRTUAL void VirtualSessionImpl::SetConfiguration(IN IMS_SINT32 /*nConfigValue*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::SetContactParameter(
        IN const AString& /*strParameter*/, IN IMS_SINT32 /*nOperation = 0 (0: ADD, 1: REMOVE)*/)
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
        IN IMS_SINT32 /*nCriteriaInterval*/, IN IMS_SINT32 /*nValueEorLt*/,
        IN IMS_SINT32 /*nValueGt*/)
{
    virtualSessionImpl_MethodNotSupported();
}

PRIVATE VIRTUAL IMS_RESULT VirtualSessionImpl::TerminateEx(
        IN IMS_BOOL /*bTerminateMethodBye = IMS_FALSE*/)
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
