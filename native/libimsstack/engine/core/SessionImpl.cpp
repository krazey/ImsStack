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

#include "CapabilitiesImpl.h"
#include "ISessionListener.h"
#include "ImsCore.h"
#include "PublicationImpl.h"
#include "ReferenceImpl.h"
#include "SessionImpl.h"
#include "SubscriptionImpl.h"
#include "base/Ims.h"
#include "media/FramedMediaImpl.h"
#include "media/StreamMediaImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SessionImpl::SessionImpl(IN SessionEx* pSession) :
        m_pSession(pSession),
        m_piListener(IMS_NULL),
        m_pVirtualSessionImpl(IMS_NULL)
{
    m_pSession->SetListener(this);
    m_pSession->SetExListener(this);
}

PUBLIC VIRTUAL SessionImpl::~SessionImpl()
{
    UpdateVirtualSession(IMS_NULL);

    if (m_pSession != IMS_NULL)
    {
        m_pSession->SetListener(IMS_NULL);
        m_pSession->SetExListener(IMS_NULL);
        m_pSession->Destroy();
    }

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

PRIVATE VIRTUAL void SessionImpl::Destroy()
{
    if (m_pSession != IMS_NULL)
    {
        m_pSession->SetMessageMediator(IMS_NULL);
        m_pSession->SetListener(IMS_NULL);
        m_pSession->SetExListener(IMS_NULL);
        m_pSession->SetRefreshListener(IMS_NULL);
        m_pSession->SetReasonHeaderSetter(IMS_NULL);
        m_pSession->Destroy();
        m_pSession = IMS_NULL;
    }

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

    delete this;
}

PRIVATE VIRTUAL IMSList<IMessage*> SessionImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = m_pSession->GetPreviousResponses(nServiceMethod);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL ICapabilities* SessionImpl::CreateCapabilities()
{
    Capabilities* pCapabilities = m_pSession->CreateCapabilities();

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

PRIVATE VIRTUAL IMedia* SessionImpl::CreateMedia(IN const AString& strType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nCountOfDescriptor /*= 0*/)
{
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

PRIVATE VIRTUAL IReference* SessionImpl::CreateReference(
        IN const AString& strReferTo, IN const AString& strReferMethod)
{
    Reference* pReference = m_pSession->CreateReference(strReferTo, strReferMethod);

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

PRIVATE VIRTUAL IMS_RESULT SessionImpl::RemoveMedia(IN IMedia* piMedia)
{
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

PRIVATE VIRTUAL ISubscription* SessionImpl::CreateSubscription(IN const AString& strEvent)
{
    Subscription* pSubscription = m_pSession->CreateSubscription(strEvent);

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

PRIVATE VIRTUAL IPublication* SessionImpl::CreatePublication(IN const AString& strEvent)
{
    Publication* pPublication = m_pSession->CreatePublication(strEvent);

    if (pPublication == IMS_NULL)
    {
        return IMS_NULL;
    }

    PublicationImpl* pPublicationImpl = new PublicationImpl(pPublication);

    if (pPublicationImpl == IMS_NULL)
    {
        pPublication->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PublicationImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pPublicationImpl;
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Alerting(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionAlerting(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_ReferenceReceived(
        IN Session* pSession, IN Reference* pReference)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
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

    m_piListener->SessionReferenceReceived(this, pReferenceImpl);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Started(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        UpdateVirtualSession(IMS_NULL);
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionStarted(this);

    UpdateVirtualSession(IMS_NULL);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_StartFailed(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionStartFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Terminated(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionTerminated(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_Updated(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionUpdated(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_UpdateFailed(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionUpdateFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_UpdateReceived(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionUpdateReceived(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_CancelDelivered(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionCancelDelivered(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSession_CancelDeliveryFailed(IN Session* pSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionCancelDeliveryFailed(this);
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::OnSession_ForkedResponseReceived(
        IN Session* pSession, IN Session* pForkedSession)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piListener == IMS_NULL)
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

    m_piListener->SessionForkedResponseReceived(this, pForkedSessionImpl);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SessionImpl::OnSession_ProvisionalResponseReceived(
        IN Session* pSession, IN IMS_UINT32 nIndex /*= Session::INDEX_MOST_RECENT_MESSAGE*/)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    UpdateVirtualSession(pSession->GetVirtualEarlySession());

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionProvisionalResponseReceived(this, nIndex);
}

PRIVATE VIRTUAL IMS_BOOL SessionImpl::OnSession_TransactionReceived(
        IN Session* pSession, IN ISipServerConnection* piSsc)
{
    if (m_pSession != pSession)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piListener->SessionTransactionReceived(this, piSsc);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_EarlyMediaUpdated(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionEarlyMediaUpdated(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_EarlyMediaUpdateFailed(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionEarlyMediaUpdateFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_EarlyMediaUpdateReceived(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionEarlyMediaUpdateReceived(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_PrackDelivered(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionPrackDelivered(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_PrackDeliveryFailed(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionPrackDeliveryFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_PrackReceived(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionPrackReceived(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_RprDeliveryFailed(IN SessionEx* pSessionEx)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionRprDeliveryFailed(this);
}

PRIVATE VIRTUAL void SessionImpl::OnSessionEx_RprReceived(IN SessionEx* pSessionEx,
        IN VirtualSession* pVirtualSession,
        IN IMS_UINT32 nIndex /*= Session::INDEX_MOST_RECENT_MESSAGE*/)
{
    if (m_pSession != pSessionEx)
    {
        IMS_TRACE_E(0, "SESSION MISMATCHED", 0, 0, 0);
        return;
    }

    UpdateVirtualSession(pVirtualSession);

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SessionRprReceived(this, nIndex);
}

PRIVATE
void SessionImpl::UpdateVirtualSession(IN VirtualSession* pVirtualSession)
{
    if (pVirtualSession == IMS_NULL)
    {
        if (m_pVirtualSessionImpl != IMS_NULL)
        {
            delete m_pVirtualSessionImpl;
            m_pVirtualSessionImpl = IMS_NULL;
        }
    }
    else
    {
        if (m_pVirtualSessionImpl == IMS_NULL)
        {
            m_pVirtualSessionImpl = new VirtualSessionImpl(this, pVirtualSession);
        }
        else
        {
            m_pVirtualSessionImpl->UpdateSession(pVirtualSession);
        }
    }
}
