/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090527  toastops@                 Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "base/Ims.h"
#include "CapabilitiesImpl.h"
#include "PageMessageImpl.h"
#include "PublicationImpl.h"
#include "ReferenceImpl.h"
#include "SessionImpl.h"
#include "SubscriptionImpl.h"
#include "ICoreServiceListener.h"
#include "IDirectCoreServiceListener.h"
#include "CoreServiceImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CoreServiceImpl::CoreServiceImpl(IN CoreService* pCoreService_) :
        piCoreServiceListener(IMS_NULL),
        piDirectCoreServiceListener(IMS_NULL),
        pService(pCoreService_)
{
    pService->SetListener(this);
}

PUBLIC VIRTUAL CoreServiceImpl::~CoreServiceImpl()
{
    if (pService != IMS_NULL)
    {
        pService->SetListener(IMS_NULL);
        pService->SetDirectListener(IMS_NULL);

        pService->Close();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::Close()
{
    //---------------------------------------------------------------------------------------------

    pService->SetListener(IMS_NULL);
    pService->SetDirectListener(IMS_NULL);

    pService->Close();
    pService = IMS_NULL;

    delete this;
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& CoreServiceImpl::GetAppId() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetAppId();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& CoreServiceImpl::GetScheme() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetScheme();
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress& CoreServiceImpl::GetAuthorizedUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetAuthorizedUserId();
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress& CoreServiceImpl::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetContactAddress();
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress* CoreServiceImpl::GetContactAddressForOutgoingMessage() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetContactAddressForOutgoingMessage();
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipHeader* CoreServiceImpl::GetContactHeader(
        IN IMS_BOOL bPrivacy /* = IMS_FALSE */, IN IMS_BOOL bRequest /* = IMS_TRUE */,
        IN IMS_SINT32 nSIPMethod /* = (-1) SipMethod::INVALID */) const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetContactHeader(bPrivacy, bRequest, nSIPMethod);
}

/*

Remarks

*/
PRIVATE VIRTUAL IFeatureCaps* CoreServiceImpl::GetFeatureCaps() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetFeatureCaps();
}

/*

Remarks

*/
PRIVATE VIRTUAL IServiceFilterCriteria* CoreServiceImpl::GetFilterCriteria() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetFilterCriteria();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AStringArray& CoreServiceImpl::GetPathHeaders() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetPathHeaders();
}

/*

Remarks

*/
PRIVATE VIRTUAL const IRegInfo* CoreServiceImpl::GetRegInfo() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetRegInfo();
}

/*

Remarks

*/
PRIVATE VIRTUAL const IPAddress& CoreServiceImpl::GetIpAddress() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetIpAddress();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL SipProfile* CoreServiceImpl::GetSipProfile() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetSipProfile();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AStringArray& CoreServiceImpl::GetUserIdentities() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetAssociatedUris();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& CoreServiceImpl::GetUserIdentity(IN IMS_SINT32 nScheme) const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetAssociatedUri(nScheme);
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipParameter* CoreServiceImpl::GetInstanceParameter() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetInstanceParameter();
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress* CoreServiceImpl::GetPublicGruu() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetPublicGruu();
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress* CoreServiceImpl::GetTemporaryGruu() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetTemporaryGruu();
}

/*

Remarks

*/
PRIVATE VIRTUAL const IMSList<SipAddress*>& CoreServiceImpl::GetTemporaryGruus() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetTemporaryGruus();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL CoreServiceImpl::IsBehindNat() const
{
    //---------------------------------------------------------------------------------------------

    return pService->IsBehindNat();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL CoreServiceImpl::IsImsConnected() const
{
    //---------------------------------------------------------------------------------------------

    return pService->IsImsConnected();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL CoreServiceImpl::IsWithinTrustDomain() const
{
    //---------------------------------------------------------------------------------------------

    return pService->IsWithinTrustDomain();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL CoreServiceImpl::AddFeatureTags(
        IN CONST IMSList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return pService->AddFeatureTags(objFeatureTags, bRegRequired);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL CoreServiceImpl::RemoveFeatureTags(
        IN CONST IMSList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return pService->RemoveFeatureTags(objFeatureTags, bRegRequired);
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL void CoreServiceImpl::SetSipProfile(IN SipProfile* pProfile)
{
    //---------------------------------------------------------------------------------------------

    pService->SetSipProfile(pProfile);
}

/*

Remarks

*/
PRIVATE VIRTUAL ICapabilities* CoreServiceImpl::CreateCapabilities(
        IN CONST AString& strFrom, IN CONST AString& strTo)
{
    Capabilities* pCapabilities = pService->CreateCapabilities(strFrom, strTo);

    //---------------------------------------------------------------------------------------------

    if (pCapabilities == IMS_NULL)
    {
        return IMS_NULL;
    }

    CapabilitiesImpl* pCapabilitiesImpl = new CapabilitiesImpl(pCapabilities);

    if (pCapabilitiesImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pCapabilities->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating CapabilitiesImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pCapabilitiesImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL IPageMessage* CoreServiceImpl::CreatePageMessage(
        IN CONST AString& strFrom, IN CONST AString& strTo)
{
    PageMessage* pPageMessage = pService->CreatePageMessage(strFrom, strTo);

    //---------------------------------------------------------------------------------------------

    if (pPageMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    PageMessageImpl* pPageMessageImpl = new PageMessageImpl(pPageMessage);

    if (pPageMessageImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pPageMessage->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PageMessageImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pPageMessageImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL IPublication* CoreServiceImpl::CreatePublication(
        IN CONST AString& strFrom, IN CONST AString& strTo, IN CONST AString& strEvent)
{
    Publication* pPublication = pService->CreatePublication(strFrom, strTo, strEvent);

    //---------------------------------------------------------------------------------------------

    if (pPublication == IMS_NULL)
    {
        return IMS_NULL;
    }

    PublicationImpl* pPublicationImpl = new PublicationImpl(pPublication);

    if (pPublicationImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pPublication->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PublicationImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pPublicationImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL IReference* CoreServiceImpl::CreateReference(IN CONST AString& strFrom,
        IN CONST AString& strTo, IN CONST AString& strReferTo, IN CONST AString& strReferMethod)
{
    Reference* pReference = pService->CreateReference(strFrom, strTo, strReferTo, strReferMethod);

    //---------------------------------------------------------------------------------------------

    if (pReference == IMS_NULL)
    {
        return IMS_NULL;
    }

    ReferenceImpl* pReferenceImpl = new ReferenceImpl(pReference);

    if (pReferenceImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pReference->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating ReferenceImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pReferenceImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL ISession* CoreServiceImpl::CreateSession(
        IN CONST AString& strFrom, IN CONST AString& strTo)
{
    SessionEx* pSession = pService->CreateSessionEx(strFrom, strTo);

    //---------------------------------------------------------------------------------------------

    if (pSession == IMS_NULL)
    {
        return IMS_NULL;
    }

    SessionImpl* pSessionImpl = new SessionImpl(pSession);

    if (pSessionImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pSession->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SessionImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pSessionImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL ISubscription* CoreServiceImpl::CreateSubscription(
        IN CONST AString& strFrom, IN CONST AString& strTo, IN CONST AString& strEvent)
{
    Subscription* pSubscription = pService->CreateSubscription(strFrom, strTo, strEvent);

    //---------------------------------------------------------------------------------------------

    if (pSubscription == IMS_NULL)
    {
        return IMS_NULL;
    }

    SubscriptionImpl* pSubscriptionImpl = new SubscriptionImpl(pSubscription);

    if (pSubscriptionImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pSubscription->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SubscriptionImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pSubscriptionImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL AString CoreServiceImpl::GetLocalUserId() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetLocalUserId();
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::SetListener(IN ICoreServiceListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    piCoreServiceListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipConnectionFactory* CoreServiceImpl::CreateSIPConnectionFactory()
{
    //---------------------------------------------------------------------------------------------

    return pService->CreateSIPConnectionFactory();
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::SetDirectListener(IN IDirectCoreServiceListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    piDirectCoreServiceListener = piListener;

    if (piDirectCoreServiceListener != IMS_NULL)
    {
        pService->SetDirectListener(this);
    }
    else
    {
        pService->SetDirectListener(IMS_NULL);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_PageMessageReceived(
        IN CoreService* pService, IN PageMessage* pMessage)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        pMessage->Destroy();
        return;
    }

    PageMessageImpl* pPageMessageImpl = new PageMessageImpl(pMessage);

    if (pPageMessageImpl == IMS_NULL)
    {
        // TODO:: Destroy() or delete ???
        pMessage->Destroy();
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PageMessageImpl failed", 0, 0, 0);
        return;
    }

    piCoreServiceListener->CoreService_PageMessageReceived(this, pPageMessageImpl);
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_ReferenceReceived(
        IN CoreService* pService, IN Reference* pReference)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piCoreServiceListener == IMS_NULL)
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

    piCoreServiceListener->CoreService_ReferenceReceived(this, pReferenceImpl);
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_ServiceClosed(
        IN CoreService* pService, IN ReasonInfo* pReasonInfo)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    piCoreServiceListener->CoreService_ServiceClosed(this, pReasonInfo);
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_SessionInvitationReceived(
        IN CoreService* pService, IN SessionEx* pSession)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piCoreServiceListener == IMS_NULL)
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

    piCoreServiceListener->CoreService_SessionInvitationReceived(this, pSessionImpl);
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_UnsolicitedNotifyReceived(
        IN CoreService* pService, IN Message* pNotify)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    piCoreServiceListener->CoreService_UnsolicitedNotifyReceived(this, pNotify);
}

/*

Remarks

*/
PRIVATE VIRTUAL void CoreServiceImpl::OnCoreService_CapabilityQueryReceived(
        IN CoreService* pService, IN Capabilities* pCapabilities)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return;
    }

    if (piCoreServiceListener == IMS_NULL)
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

    piCoreServiceListener->CoreService_CapabilityQueryReceived(this, pCapabilitiesImpl);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 CoreServiceImpl::OnDirectCoreService_TransactionReceived(
        IN CoreService* pService, IN ISipConnectionFactory* piSCF)
{
    //---------------------------------------------------------------------------------------------

    if (this->pService != pService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    if (piDirectCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO DIRECT CORE SERVICE LISTENER", 0, 0, 0);
        return RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    return piDirectCoreServiceListener->DirectCoreService_TransactionReceived(this, piSCF);
}
