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
#include "CoreServiceImpl.h"
#include "ICoreServiceListener.h"
#include "IDirectCoreServiceListener.h"
#include "PageMessageImpl.h"
#include "PublicationImpl.h"
#include "ReferenceImpl.h"
#include "SessionImpl.h"
#include "SubscriptionImpl.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CoreServiceImpl::CoreServiceImpl(IN CoreService* pCoreService) :
        m_pService(pCoreService),
        m_piCoreServiceListener(IMS_NULL),
        m_piDirectCoreServiceListener(IMS_NULL)
{
    m_pService->SetListener(this);
}

PUBLIC VIRTUAL CoreServiceImpl::~CoreServiceImpl()
{
    if (m_pService != IMS_NULL)
    {
        m_pService->SetListener(IMS_NULL);
        m_pService->SetDirectListener(IMS_NULL);
        m_pService->Close();
    }
}

PRIVATE VIRTUAL void CoreServiceImpl::Close()
{
    m_pService->SetListener(IMS_NULL);
    m_pService->SetDirectListener(IMS_NULL);

    m_pService->Close();
    m_pService = IMS_NULL;

    delete this;
}

PRIVATE VIRTUAL ICapabilities* CoreServiceImpl::CreateCapabilities(
        IN const AString& strFrom, IN const AString& strTo)
{
    Capabilities* pCapabilities = m_pService->CreateCapabilities(strFrom, strTo);

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

PRIVATE VIRTUAL IPageMessage* CoreServiceImpl::CreatePageMessage(
        IN const AString& strFrom, IN const AString& strTo)
{
    PageMessage* pPageMessage = m_pService->CreatePageMessage(strFrom, strTo);

    if (pPageMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    PageMessageImpl* pPageMessageImpl = new PageMessageImpl(pPageMessage);

    if (pPageMessageImpl == IMS_NULL)
    {
        pPageMessage->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PageMessageImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pPageMessageImpl;
}

PRIVATE VIRTUAL IPublication* CoreServiceImpl::CreatePublication(
        IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent)
{
    Publication* pPublication = m_pService->CreatePublication(strFrom, strTo, strEvent);

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

PRIVATE VIRTUAL IReference* CoreServiceImpl::CreateReference(IN const AString& strFrom,
        IN const AString& strTo, IN const AString& strReferTo, IN const AString& strReferMethod)
{
    Reference* pReference = m_pService->CreateReference(strFrom, strTo, strReferTo, strReferMethod);

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

PRIVATE VIRTUAL ISession* CoreServiceImpl::CreateSession(
        IN const AString& strFrom, IN const AString& strTo)
{
    SessionEx* pSession = m_pService->CreateSessionEx(strFrom, strTo);

    if (pSession == IMS_NULL)
    {
        return IMS_NULL;
    }

    SessionImpl* pSessionImpl = new SessionImpl(pSession);

    if (pSessionImpl == IMS_NULL)
    {
        pSession->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SessionImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pSessionImpl;
}

PRIVATE VIRTUAL ISubscription* CoreServiceImpl::CreateSubscription(
        IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent)
{
    Subscription* pSubscription = m_pService->CreateSubscription(strFrom, strTo, strEvent);

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

PRIVATE VIRTUAL void CoreServiceImpl::SetDirectListener(IN IDirectCoreServiceListener* piListener)
{
    m_piDirectCoreServiceListener = piListener;

    if (m_piDirectCoreServiceListener != IMS_NULL)
    {
        m_pService->SetDirectListener(this);
    }
    else
    {
        m_pService->SetDirectListener(IMS_NULL);
    }
}

PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_PageMessageReceived(
        IN CoreService* pService, IN PageMessage* pMessage)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        pMessage->Destroy();
        return;
    }

    PageMessageImpl* pPageMessageImpl = new PageMessageImpl(pMessage);

    if (pPageMessageImpl == IMS_NULL)
    {
        pMessage->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PageMessageImpl failed", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->CoreService_PageMessageReceived(this, pPageMessageImpl);
}

PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_ReferenceReceived(
        IN CoreService* pService, IN Reference* pReference)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        pReference->Reject();
        pReference->Destroy();
        return;
    }

    ReferenceImpl* pReferenceImpl = new ReferenceImpl(pReference);

    if (pReferenceImpl == IMS_NULL)
    {
        pReference->Reject();
        pReference->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating ReferenceImpl failed", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->CoreService_ReferenceReceived(this, pReferenceImpl);
}

PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_ServiceClosed(
        IN CoreService* pService, IN ReasonInfo* pReasonInfo)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->CoreService_ServiceClosed(this, pReasonInfo);
}

PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_SessionInvitationReceived(
        IN CoreService* pService, IN SessionEx* pSession)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        pSession->Reject(ISession::STATUSCODE_480_TEMPORARILY_UNAVAILABLE);
        pSession->Destroy();
        return;
    }

    SessionImpl* pSessionImpl = new SessionImpl(pSession);

    if (pSessionImpl == IMS_NULL)
    {
        pSession->Reject(ISession::STATUSCODE_480_TEMPORARILY_UNAVAILABLE);
        pSession->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SessionImpl failed", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->CoreService_SessionInvitationReceived(this, pSessionImpl);
}

PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_UnsolicitedNotifyReceived(
        IN CoreService* pService, IN Message* pNotify)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->CoreService_UnsolicitedNotifyReceived(this, pNotify);
}

PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_CapabilityQueryReceived(
        IN CoreService* pService, IN Capabilities* pCapabilities)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        pCapabilities->Reject(SipStatusCode::SC_480);
        pCapabilities->Destroy();
        return;
    }

    CapabilitiesImpl* pCapabilitiesImpl = new CapabilitiesImpl(pCapabilities);

    if (pCapabilitiesImpl == IMS_NULL)
    {
        pCapabilities->Reject(SipStatusCode::SC_480);
        pCapabilities->Destroy();

        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating CapabilitiesImpl failed", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->CoreService_CapabilityQueryReceived(this, pCapabilitiesImpl);
}

PRIVATE VIRTUAL IMS_SINT32 CoreServiceImpl::OnDirectCoreService_TransactionReceived(
        IN CoreService* pService, IN ISipConnectionFactory* piScf)
{
    if (m_pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    if (m_piDirectCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO DIRECT CORE SERVICE LISTENER", 0, 0, 0);
        return RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    return m_piDirectCoreServiceListener->DirectCoreService_TransactionReceived(this, piScf);
}
