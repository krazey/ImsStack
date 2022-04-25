/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090527  toastops@                 Created
    </table>

    Description
     Service is the base class for IMS services, and follows the Generic Connection Framework
    (GCF).
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceNetwork.h"
#include "private/ConfigurationManager.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"
#include "private/SipConfig.h"
#include "private/SubscriberConfig.h"
#include "Feature.h"
#include "Connector.h"
#include "ISIPConnectionNotifier.h"
#include "ISIPHeader.h"
#include "ISIPMessage.h"
#include "ISIPDialog.h"
#include "ISIPClientConnection.h"
#include "ISIPServerConnection.h"
#include "SIPDebug.h"
#include "SIPHeaderName.h"
#include "SIPParsingHelper.h"
#include "SIPStatusCode.h"
#include "SIPParameter.h"
#include "SIPConfigProxy.h"
#include "base/IMS.h"
#include "ImsIdentity.h"
#include "util/CallerCapability.h"
#include "util/CallerPreference.h"
// CALLER_PREFERENCE_MANAGER
#include "util/CallerPreferenceManager.h"
#include "util/PreferenceHeader.h"
#include "PAccessNetworkInfoHeader.h"
#include "util/UserAgentHeader.h"
#include "util/SIPConnectionNotifierManager.h"
#include "SIPTimerValuesHelper.h"
#include "util/MethodManager.h"
#include "base/Method.h"
#include "IServiceManagerListener.h"
#include "IRegBinding.h"
#include "Service.h"

__IMS_TRACE_TAG_IMS__;



PUBLIC
Service::Service(IN CONST AString &strScheme_, IN CONST AString& strAppId_,
        IN CONST AString &strServiceId_, IN CONST SIPAddress *pIMPU_ /* = IMS_NULL */)
    : Connection()
    , strScheme(strScheme_)
    , strAppId(strAppId_)
    , strServiceId(strServiceId_)
    , pAppConfig(IMS_NULL)
    , piServiceManagerListener(IMS_NULL)
    , bImsConnected(IMS_FALSE)
    , piRegBinding(IMS_NULL)
    // MULTI_REG_SIP_PROFILE
    , pSIPProfile(IMS_NULL)
    , bFlag_ProvisionedUserId(IMS_FALSE)
    , objAuthorizedUserIds(IMSList<ISIPHeader*>())
    , nFeatureTags(ISipConfigV::FEATURE_TAG_DEFAULT)
    , objAcceptContacts(IMSList<PreferenceHeader*>())
    , pCallerCapability(IMS_NULL)
    , bFlag_CallerCapabilityChanged(IMS_FALSE)
    , pMethodMngr(IMS_NULL)
    , pFilterCriteria(IMS_NULL)
    // CONTACT_FEATURE_CAPS
    , pFeatureCaps(IMS_NULL)
{
    // If the user id is not provisioned, it needs to be set from the device's configuration.
    pMethodMngr = new MethodManager();
    pFilterCriteria = new ServiceFilterCriteria();
    // CONTACT_FEATURE_CAPS
    pFeatureCaps = new FeatureCaps();

    if (pIMPU_ != IMS_NULL)
    {
        bFlag_ProvisionedUserId = IMS_TRUE;

        objIMPU = (*pIMPU_);
    }
}

PUBLIC VIRTUAL
Service::~Service()
{
    //---------------------------------------------------------------------------------------------

    if (pAppConfig != IMS_NULL)
        delete pAppConfig;

    // CONTACT_FEATURE_CAPS
    if (pFeatureCaps != IMS_NULL)
        delete pFeatureCaps;

    if (pFilterCriteria != IMS_NULL)
        delete pFilterCriteria;

    if (pMethodMngr != IMS_NULL)
        delete pMethodMngr;

    for (IMS_UINT32 i = 0; i < objAcceptContacts.GetSize(); ++i)
    {
        PreferenceHeader *pPrefHeader = objAcceptContacts.GetAt(i);

        if (pPrefHeader != IMS_NULL)
            delete pPrefHeader;
    }

    if (pCallerCapability != IMS_NULL)
        delete pCallerCapability;

    for (IMS_UINT32 i = 0; i < objAuthorizedUserIds.GetSize(); ++i)
    {
        ISIPHeader *piHeader = objAuthorizedUserIds.GetAt(i);

        if (piHeader != IMS_NULL)
        {
            piHeader->Destroy();
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
const AString& Service::GetAppId() const
{
    //---------------------------------------------------------------------------------------------

    return strAppId;
}

/*

Remarks

*/
PUBLIC VIRTUAL
const AString& Service::GetScheme() const
{
    //---------------------------------------------------------------------------------------------

    return strScheme;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL Service::CreateConfig(IN CONST AppConfig &objAppConfig)
{
    //---------------------------------------------------------------------------------------------

    pAppConfig = new AppConfig(objAppConfig);

    if (pAppConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating AppConfig failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Creates Contact URI for this service; Move to SetSIPReady(...) method.
    //CreateContactAddress();

    // Constructs a caller capability which sets in Contact header
    pCallerCapability = new CallerCapability(GetServiceCode());

    if (pCallerCapability == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const CoreServiceConfig *pServiceConfig = pAppConfig->GetCoreServiceConfigEx(strServiceId);

    if (!pCallerCapability->Create(pAppConfig, pServiceConfig, GetSipConfigV()))
    {
        IMS_TRACE_E(0, "Creating the features from AppConfig & ServiceConfig failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Create Accept-Contact headers for this service
    if (!CallerPreference::CreateAcceptContactHeaders(
            pAppConfig, pServiceConfig, GetSipConfigV(), objAcceptContacts))
    {
        IMS_TRACE_E(0, "Creating Accept-Contact header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update the current SIP feature tags
    const SipConfigV* pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        nFeatureTags = pSipConfigV->GetFeatureTagOptions();

        IConfigurable *piConfigurable
                = static_cast<const ISipConfigV*>(pSipConfigV)->GetConfigurable();

        if (piConfigurable != IMS_NULL)
        {
            piConfigurable->AddListener(IConfigurable::CP_I_FEATURE_TAG_OPTIONS, this);
            piConfigurable->AddListener(IConfigurable::CP_I_SIP_ALL, this);
        }
    }

    // __IMS_DEBUG__
    if (pServiceConfig != IMS_NULL)
    {
        IMS_TRACE_D("Service :: IARI (%s)", pServiceConfig->GetIARI().ToString().GetStr(), 0, 0);

        const IMSList<ServiceIdentifier> &objICSIs = pServiceConfig->GetICSIs();

        for (IMS_UINT32 i = 0; i < objICSIs.GetSize(); ++i)
        {
            const ServiceIdentifier &objSI = objICSIs.GetAt(i);
            IMS_TRACE_D("Service :: ICSI (%s) at (%d)", objSI.ToString().GetStr(), i, 0);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Service::HandleSessionInvitationReceived(IN Session * /* pSession */)
{
    // The subclass MUST implement this method to handle an incoming INVITE request
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Service::HandlePageMessageReceived(IN PageMessage * /* pPageMessage */)
{
    // The subclass MUST implement this method to handle an incoming MESSAGE request
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Service::HandleReferenceReceived(IN Reference * /* pReference */)
{
    // The subclass MUST implement this method to handle an incoming REFER request
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Service::HandleCapabilityQueryReceived(IN Capabilities * /* pCapabilities */)
{
    // The subclass MUST implement this method to handle an incoming OPTIONS request
}

/*

Remarks

*/
PUBLIC
ISIPClientConnection* Service::CreateConnection(IN CONST SIPAddress *pFrom,
        IN CONST SIPAddress *pTo, IN CONST SIPMethod &objMethod,
        IN IMS_BOOL bPrivacy /* = IMS_FALSE */)
{
    ISIPClientConnection *piSCC;

    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_NULL;
    }

    if ((pFrom == IMS_NULL) || (pTo == IMS_NULL))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks if the method is allowed
    if (objMethod.Equals(SIPMethod::ACK)
            || objMethod.Equals(SIPMethod::CANCEL)
            || objMethod.Equals(SIPMethod::BYE)
            || objMethod.Equals(SIPMethod::INFO)
            || objMethod.Equals(SIPMethod::UPDATE)
            || objMethod.Equals(SIPMethod::PRACK))
    {
        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMS_NULL;
    }

    IMS_BOOL bOverwriteTarget = IMS_FALSE;
    AString strTarget = pTo->ToString();

    IMS_TRACE_D("Service::CreateConnection() - To (%s), Method (%s)",
            SIPDebug::GetUri1(strTarget).GetStr(),
            objMethod.ToString().GetStr(), 0);

    if (!pTo->IsSchemeSIP() && !pTo->IsSchemeSIPS())
    {
        bOverwriteTarget = IMS_TRUE;
        strTarget = ImsIdentity::GetAnonymousUserId();
    }

    piSCC = DYNAMIC_CAST(ISIPClientConnection*, Connector::Open(strTarget));

    if (piSCC == IMS_NULL)
    {
        IMS::SetLastError(IMSError::CONNECTION_NOT_FOUND);

        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    // MULTI_REG_SIP_PROFILE
    piSCC->SetSIPProfile(GetSIPProfile());

    IMS_SINT32 nPortS = piRegBinding->GetPortUS();
    IMS_SINT32 nPortC = piRegBinding->GetPortUC();

    // Sets the transport tuples
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    piSCC->SetTransportTuple(piRegBinding->GetIPAddress(),
            nPortS, nPortC, piRegBinding->GetPortFlowControl(),
            piRegBinding->GetTransportExt());

    // Sets the transaction timer values
    IMS_SINT32 nTxnType = SIPTimerValuesHelper::NON_INVITE_CLIENT;

    if (objMethod.Equals(SIPMethod::INVITE))
    {
        nTxnType = SIPTimerValuesHelper::INVITE_CLIENT;
    }

    piSCC->SetTransactionTimerValues(SIPTimerValuesHelper::GetValues(
            GetSlotId(), GetSIPProfile(), nTxnType));

    if (piSCC->InitRequest(objMethod.ToString(), IMS_NULL) != IMS_SUCCESS)
    {
        piSCC->Close();
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing SIP request failed", 0, 0, 0);
        return IMS_NULL;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Sets From header field
    if (piSIPMsg->SetHeader(ISIPHeader::FROM, pFrom->ToString()) != IMS_SUCCESS)
    {
        piSCC->Close();
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Setting From header failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (bOverwriteTarget)
    {
        strTarget = pTo->ToString();

        if (piSCC->SetRequestURI(strTarget) != IMS_SUCCESS)
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting Request-URI failed", 0, 0, 0);
            return IMS_NULL;
        }

        // Sets To header field
        if (piSIPMsg->SetHeader(ISIPHeader::TO, strTarget) != IMS_SUCCESS)
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting To header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets Contact header fields:
    // INVITE, NOTIFY, OPTIONS, SUBSCRIBE, UPDATE, REGISTER, REFER, PUBLISH
    if (objMethod.Equals(SIPMethod::INVITE)
            || objMethod.Equals(SIPMethod::OPTIONS)
            || objMethod.Equals(SIPMethod::SUBSCRIBE)
            || objMethod.Equals(SIPMethod::REFER)
            || objMethod.Equals(SIPMethod::PUBLISH))
    {
        AString strContact;
        IMS_BOOL bIsContactGRUU = IMS_FALSE;

        FormContactHeader(objMethod, bPrivacy, IMS_TRUE, strContact, bIsContactGRUU);

        if (piSIPMsg->SetHeader(ISIPHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            return IMS_NULL;
        }

        if (bIsContactGRUU)
        {
            piSIPMsg->AddHeader(ISIPHeader::SUPPORTED, SIP::STR_GRUU);
        }
    }

    // Sets Accept-Contact header field
    for (IMS_UINT32 i = 0; i < objAcceptContacts.GetSize(); ++i)
    {
        const PreferenceHeader *pHeader = objAcceptContacts.GetAt(i);

        if (piSIPMsg->AddHeader(ISIPHeader::ACCEPT_CONTACT, pHeader->ToString()) != IMS_SUCCESS)
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Adding Accept-Contact header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    IMS_SINT32 j;

    // Sets Route (except for REGISTER) header fields
    const AStringArray &objServiceRoutes = GetServiceRoutes();

    for (j = 0; j < objServiceRoutes.GetCount(); ++j)
    {
        if (piSIPMsg->AddHeader(ISIPHeader::ROUTE,
                objServiceRoutes.GetElementAt(j)) != IMS_SUCCESS)
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Adding Route header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets Allow header fields
    const SipConfigV *pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray &objMethods = pSipConfigV->GetAllowMethods();

        for (j = 0; j < objMethods.GetCount(); ++j)
        {
            if (piSIPMsg->AddHeader(ISIPHeader::ALLOW, objMethods.GetElementAt(j)) != IMS_SUCCESS)
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        // Sets P-Preferred-Identity (except for REGISTER) header fields
        // RFC 3325, INVITE/BYE/OPTIONS/SUBSCRIBE/NOTIFY/REFER
        // RFC 5876, all requests except for ACK/CANCEL
        if (!SetPPreferredIdentityHeader(pSipConfigV->GetPreferredId(), piSIPMsg))
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting P-Preferred-Identity header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets User-Agent header field
    if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSIPProfile()))
    {
        UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                GetSIPProfile(), GetServiceId(), GetIPAddress(), GetSlotId(), piSIPMsg);
    }

    // Sets P-Access-Network-Info header field
    PAccessNetworkInfoHeader::SetHeader(GetSlotId(), GetIPAddress(), GetSIPProfile(), piSIPMsg);

    // IPSEC {
    {
        // RFC 3329 - SIP Security Agreement:
        // Security related headers (Security-Client / Security-Verify)
        // Do not add Security-Client headers
#if 0
        const AStringArray &objSecurityClients = piRegBinding->GetSecurityClients();

        for (IMS_SINT32 i = 0; i < objSecurityClients.GetCount(); ++i)
        {
            const AString &strHeader = objSecurityClients.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISIPHeader::SECURITY_CLIENT, strHeader) != IMS_SUCCESS)
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Security-Client failed", 0, 0, 0);
                return IMS_NULL;
            }
        }
#endif

        const AStringArray &objSecurityVerifys = piRegBinding->GetSecurityVerifys();

        for (IMS_SINT32 i = 0; i < objSecurityVerifys.GetCount(); ++i)
        {
            const AString &strHeader = objSecurityVerifys.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISIPHeader::SECURITY_VERIFY, strHeader) != IMS_SUCCESS)
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Security-Verify failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        if (!objSecurityVerifys.IsEmpty())
        {
            piSIPMsg->AddHeader(ISIPHeader::REQUIRE, SIP::STR_SEC_AGREE);
            piSIPMsg->AddHeader(ISIPHeader::UNKNOWN,
                    SIP::STR_SEC_AGREE, SIPHeaderName::PROXY_REQUIRE);
        }
    }
    // }

    IMS::SetLastError(IMSError::NO_ERROR);

    return piSCC;
}

/*

Remarks

*/
PUBLIC
ISIPClientConnection* Service::CreateConnection(IN ISIPDialog *piDialog,
        IN CONST SIPMethod &objMethod, IN IMS_BOOL bPrivacy /* = IMS_FALSE */)
{
    ISIPClientConnection *piSCC = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    (void) bPrivacy;

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_NULL;
    }
#endif

    if (piDialog == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks if the method is allowed
    if (objMethod.Equals(SIPMethod::ACK)
            || objMethod.Equals(SIPMethod::CANCEL))
    {
        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMS_NULL;
    }

    // BYE_REQUEST_ON_DIALOG_TERMINATED
    AString strDialogId = piDialog->GetDialogIDEx();

    IMS_TRACE_I("Service::CreateConnection() - Dialog (%s), Method (%s)",
            SIPDebug::GetCharA1(strDialogId.GetStr(), 8, '@'),
            objMethod.ToString().GetStr(), 0);

    piSCC = piDialog->GetNewClientConnection(objMethod.ToString());

    if (piSCC == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating a new SIP connection failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_NULL;
    }

    // MULTI_REG_SIP_PROFILE
    piSCC->SetSIPProfile(GetSIPProfile());

    IMS_SINT32 nPortS = IsRegBindingOnActive() ? \
            piRegBinding->GetPortUS() : objCachedRegBinding.GetPortUS();
    IMS_SINT32 nPortC = IsRegBindingOnActive() ? \
            piRegBinding->GetPortUC() : objCachedRegBinding.GetPortUC();

    // Sets the transport tuples
    if (IsRegBindingOnActive())
    {
        // RFC5626_FLOW_CONTROL
        // MULTI_REG_TRANSPORT
        piSCC->SetTransportTuple(piRegBinding->GetIPAddress(),
                nPortS, nPortC, piRegBinding->GetPortFlowControl(),
                piRegBinding->GetTransportExt());
    }
    else
    {
        // RFC5626_FLOW_CONTROL
        // MULTI_REG_TRANSPORT
        piSCC->SetTransportTuple(objCachedRegBinding.GetIPAddress(),
                nPortS, nPortC, objCachedRegBinding.GetPortFlowControl(),
                objCachedRegBinding.GetTransportExt());
    }

    // Sets the transaction timer values
    IMS_SINT32 nTxnType = SIPTimerValuesHelper::NON_INVITE_CLIENT;

    if (objMethod.Equals(SIPMethod::INVITE))
    {
        nTxnType = SIPTimerValuesHelper::INVITE_CLIENT;
    }

    piSCC->SetTransactionTimerValues(SIPTimerValuesHelper::GetValues(
            GetSlotId(), GetSIPProfile(), nTxnType));

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // CALLER_PREFERENCE_MANAGER
    // Sets Accept-Contact header fields
    if(!(SIPConfigProxy::IsNoAcceptContactHeaderInBYE(GetSlotId(), GetSIPProfile())
            && piSIPMsg->GetMethod().Equals(SIPMethod::BYE)))
    {
        const IMSList<AString> &objAcceptContactsInDialog =
                CallerPreferenceManager::GetInstance()->GetAcceptContacts(strDialogId);

        if (!objAcceptContactsInDialog.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < objAcceptContactsInDialog.GetSize(); ++i)
            {
                const AString &strHeader = objAcceptContactsInDialog.GetAt(i);

                if (piSIPMsg->AddHeader(ISIPHeader::ACCEPT_CONTACT, strHeader) != IMS_SUCCESS)
                {
                    piSCC->Close();
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding Accept-Contact header failed", 0, 0, 0);
                    return IMS_NULL;
                }
            }
        }
        else
        {
            for (IMS_UINT32 i = 0; i < objAcceptContacts.GetSize(); ++i)
            {
                const PreferenceHeader *pHeader = objAcceptContacts.GetAt(i);

                if (piSIPMsg->AddHeader(ISIPHeader::ACCEPT_CONTACT,
                        pHeader->ToString()) != IMS_SUCCESS)
                {
                    piSCC->Close();
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding Accept-Contact header failed", 0, 0, 0);
                    return IMS_NULL;
                }
            }
        }
    }

    // Sets Allow header fields
    const SipConfigV *pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray &objMethods = pSipConfigV->GetAllowMethods();

        for (IMS_SINT32 j = 0; j < objMethods.GetCount(); ++j)
        {
            if (piSIPMsg->AddHeader(ISIPHeader::ALLOW, objMethods.GetElementAt(j)) != IMS_SUCCESS)
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        // Sets P-Preferred-Identity (except for REGISTER) header fields
        // RFC 3325, INVITE/BYE/OPTIONS/SUBSCRIBE/NOTIFY/REFER
        // RFC 5876, all requests except for ACK/CANCEL
        if (!SetPPreferredIdentityHeader(pSipConfigV->GetPreferredId(), piSIPMsg))
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting P-Preferred-Identity header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets User-Agent header field
    if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSIPProfile()))
    {
        UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                GetSIPProfile(), GetServiceId(), GetIPAddress(), GetSlotId(), piSIPMsg);
    }

    // Sets P-Access-Network-Info header field
    PAccessNetworkInfoHeader::SetHeader(GetSlotId(), GetIPAddress(), GetSIPProfile(), piSIPMsg);

    // In case of PRACK method, then do not contain the Security related headers.
    if (objMethod.Equals(SIPMethod::PRACK))
    {
        IMS::SetLastError(IMSError::NO_ERROR);

        return piSCC;
    }

    // IPSEC {
    {
        // RFC 3329 - SIP Security Agreement:
        // Security related headers (Security-Client / Security-Verify)
        // hwangoo.park, 130515, do not add Security-Client headers
#if 0
        const AStringArray &objSecurityClients = IsRegBindingOnActive() ? \
                piRegBinding->GetSecurityClients() : objCachedRegBinding.GetSecurityClients();

        for (IMS_SINT32 i = 0; i < objSecurityClients.GetCount(); ++i)
        {
            const AString &strHeader = objSecurityClients.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISIPHeader::SECURITY_CLIENT, strHeader) != IMS_SUCCESS)
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Security-Client failed", 0, 0, 0);
                return IMS_NULL;
            }
        }
#endif

        const AStringArray &objSecurityVerifys = IsRegBindingOnActive() ? \
                piRegBinding->GetSecurityVerifys() : objCachedRegBinding.GetSecurityVerifys();

        for (IMS_SINT32 i = 0; i < objSecurityVerifys.GetCount(); ++i)
        {
            const AString &strHeader = objSecurityVerifys.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISIPHeader::SECURITY_VERIFY, strHeader) != IMS_SUCCESS)
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Security-Verify failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        if (!objSecurityVerifys.IsEmpty())
        {
            piSIPMsg->AddHeader(ISIPHeader::REQUIRE, SIP::STR_SEC_AGREE);
            piSIPMsg->AddHeader(ISIPHeader::UNKNOWN,
                    SIP::STR_SEC_AGREE, SIPHeaderName::PROXY_REQUIRE);
        }
    }
    // }

    if (objMethod.Equals(SIPMethod::INVITE)
            || objMethod.Equals(SIPMethod::REFER)
            || objMethod.Equals(SIPMethod::SUBSCRIBE)
            || objMethod.Equals(SIPMethod::UPDATE)
            || objMethod.Equals(SIPMethod::NOTIFY))
    {
        SetGRUUOptionTagInMidDialog(piDialog, piSIPMsg);
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return piSCC;
}

/*

Remarks

*/
PUBLIC
ISIPClientConnection* Service::CreateCancelConnection(IN ISIPClientConnection* piSCC)
{
    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISIPClientConnection is null", 0, 0, 0);
        return IMS_NULL;
    }

    ISIPClientConnection *piCancelSCC = piSCC->InitCancel();

    if (piCancelSCC == IMS_NULL)
    {
        return IMS_NULL;
    }

    // IPSEC {
    {
        if (IsRegBindingOnActive())
        {
            const AStringArray &objSecurityVerifys = piRegBinding->GetSecurityVerifys();

            if (!objSecurityVerifys.IsEmpty())
            {
                // Sets the transport tuples
                piCancelSCC->SetTransportTuple(piRegBinding->GetIPAddress(),
                        piRegBinding->GetPortUS(), piRegBinding->GetPortUC(),
                        piRegBinding->GetPortFlowControl(),
                        piRegBinding->GetTransportExt());
            }
        }
    }
    // }

    return piCancelSCC;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::CreateResponse(IN_OUT ISIPServerConnection *piSSC, IN IMS_SINT32 nStatusCode,
        IN CONST AString &strPhrase /* = AString::ConstNull() */,
        IN IMS_BOOL bPrivacy /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Service::CreateResponse() - Method (%s), Status Code (%d)",
            piSSC->GetMethod().ToString().GetStr(), nStatusCode, 0);

    if (piSSC->InitResponse(nStatusCode) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing a SIP response failed - SIPError(%d)",
                SIPError::GetLastError(), 0, 0);

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    // MULTI_REG_SIP_PROFILE
    if (!pSIPProfile.IsNull())
    {
        piSSC->SetSIPProfile(pSIPProfile.Get());
    }

    const SIPMethod &objMethod = piSSC->GetMethod();

    // Sets the transaction timer values
    IMS_SINT32 nTxnType = SIPTimerValuesHelper::NON_INVITE_SERVER;

    if (objMethod.Equals(SIPMethod::INVITE))
    {
        nTxnType = SIPTimerValuesHelper::INVITE_SERVER;
    }

    piSSC->SetTransactionTimerValues(SIPTimerValuesHelper::GetValues(
            GetSlotId(), GetSIPProfile(), nTxnType));

    // Sets the reason phrase if present
    if (!strPhrase.IsNULL())
    {
        piSSC->SetReasonPhrase(strPhrase);
    }

    // In case of CANCEL response, do not perform any more operations.
    if (objMethod.Equals(SIPMethod::CANCEL))
    {
        IMS::SetLastError(IMSError::NO_ERROR);
        return IMS_TRUE;
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    if ((nStatusCode > SIPStatusCode::SC_100)
            && (nStatusCode < SIPStatusCode::SC_300))
    {
        IMS_SINT32 nDialogState = ISIPDialog::STATE_INIT;
        ISIPDialog *piDialog = piSSC->GetDialog();

        if (piDialog != IMS_NULL)
        {
            nDialogState = piDialog->GetState();
        }

        // Sets Contact header fields:
        // INVITE, NOTIFY, OPTIONS, SUBSCRIBE, UPDATE, REGISTER, REFER, PUBLISH
        // OPTIONS case: it will be handled in Capabilities class.
        // In the other situation, J180 will set the Contact header when initializing the response.
        if ((nDialogState != ISIPDialog::STATE_EARLY)
                && (nDialogState != ISIPDialog::STATE_CONFIRMED)
                && (objMethod.Equals(SIPMethod::INVITE)
                    || objMethod.Equals(SIPMethod::REFER)
                    || objMethod.Equals(SIPMethod::SUBSCRIBE)
                    || objMethod.Equals(SIPMethod::NOTIFY)))
        {
            AString strContact;
            IMS_BOOL bIsContactGRUU = IMS_FALSE;

            FormContactHeader(objMethod, bPrivacy, IMS_FALSE, strContact, bIsContactGRUU);

            if (piSIPMsg->SetHeader(ISIPHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
                IMS::SetLastError(IMSError::GENERAL_ERROR);
                return IMS_FALSE;
            }

            if (bIsContactGRUU)
            {
                piSIPMsg->AddHeader(ISIPHeader::SUPPORTED, SIP::STR_GRUU);
            }
        }
        else if (objMethod.Equals(SIPMethod::INVITE)
                || objMethod.Equals(SIPMethod::REFER)
                || objMethod.Equals(SIPMethod::SUBSCRIBE)
                || objMethod.Equals(SIPMethod::UPDATE)
                || objMethod.Equals(SIPMethod::NOTIFY))
        {
            SetGRUUOptionTagInMidDialog(piDialog, piSIPMsg);
        }

        // Sets Allow header fields
         const SipConfigV *pSipConfigV = GetSipConfigV();

        if (pSipConfigV != IMS_NULL)
        {
            const AStringArray &objMethods = pSipConfigV->GetAllowMethods();

            for (IMS_SINT32 j = 0; j < objMethods.GetCount(); ++j)
            {
                if (piSIPMsg->AddHeader(ISIPHeader::ALLOW,
                        objMethods.GetElementAt(j)) != IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                    IMS::SetLastError(IMSError::GENERAL_ERROR);
                    return IMS_FALSE;
                }
            }

            // Sets P-Preferred-Identity (except for REGISTER) header fields
            // This header will not be included in the response except for INVITE
            if (objMethod.Equals(SIPMethod::INVITE))
            {
                if (!SetPPreferredIdentityHeader(pSipConfigV->GetPreferredId(), piSIPMsg))
                {
                    IMS_TRACE_E(0, "Setting P-Preferred-Identity header failed", 0, 0, 0);
                    IMS::SetLastError(IMSError::GENERAL_ERROR);
                    return IMS_FALSE;
                }
            }
        }
    }

    // Sets P-Access-Network-Info header field
    if (!objMethod.Equals(SIPMethod::CANCEL) && (nStatusCode > SIPStatusCode::SC_100))
    {
        PAccessNetworkInfoHeader::SetHeader(GetSlotId(),
                GetIPAddress(), GetSIPProfile(), piSIPMsg);
    }

    // Sets Server header field - User-Agent ?
    if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSIPProfile()))
    {
        if (SIPConfigProxy::IsUserAgentSetByContext(GetSlotId(), GetSIPProfile()))
        {
            UserAgentHeader::SetHeader(SIPHeaderName::SERVER,
                    GetSIPProfile(), GetServiceId(), GetIPAddress(), GetSlotId(), piSIPMsg);
        }
        else
        {
            UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                    GetSIPProfile(), GetServiceId(), GetIPAddress(), GetSlotId(), piSIPMsg);
        }
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::InitAck(IN ISIPClientConnection* piSCC)
{
    if (piSCC->InitAck() != IMS_SUCCESS)
    {
        return IMS_FALSE;
    }

    // IPSEC {
    {
        if (IsRegBindingOnActive())
        {
            const AStringArray &objSecurityVerifys = piRegBinding->GetSecurityVerifys();

            if (!objSecurityVerifys.IsEmpty())
            {
                // Sets the transport tuples
                piSCC->SetTransportTuple(piRegBinding->GetIPAddress(),
                        piRegBinding->GetPortUS(), piRegBinding->GetPortUC(),
                        piRegBinding->GetPortFlowControl(),
                        piRegBinding->GetTransportExt());
            }
        }
    }
    // }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::Equals(IN CONST Service *pService) const
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (GetSlotId() != pService->GetSlotId())
    {
        return IMS_FALSE;
    }

    return strServiceId.Equals(pService->GetServiceId());
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 Service::EvaluateFilterCriteria(IN CONST ISIPMessage *piSIPMsg) const
{
    //---------------------------------------------------------------------------------------------

    if (pFilterCriteria == IMS_NULL)
    {
        return 0;
    }

    if (pFilterCriteria->IsEmpty())
    {
        return 0;
    }

    IMS_TRACE_D("iFC :: Evaluating ... (%s)", GetServiceId().GetStr(), 0, 0);

    IMS_UINT32 nScore = pFilterCriteria->Evaluate(piSIPMsg);

    if (nScore != 0)
    {
        IMS_TRACE_D("iFC :: Match OK (%s : %s : %d)",
                GetAppId().GetStr(), GetServiceId().GetStr(), nScore);
    }

    return nScore;
}

/*

Remarks

*/
PUBLIC
const AString& Service::GetServiceId() const
{
    //---------------------------------------------------------------------------------------------

    return strServiceId;
}

/*

Remarks

*/
PUBLIC
AppConfig* Service::GetAppConfig() const
{
    //---------------------------------------------------------------------------------------------

    return pAppConfig;
}

/*

Remarks

*/
PUBLIC
const CoreServiceConfig* Service::GetServiceConfig() const
{
    //---------------------------------------------------------------------------------------------

    return pAppConfig->GetCoreServiceConfigEx(strServiceId);
}

/*

Remarks

*/
PUBLIC
const IPAddress& Service::GetIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return IPAddress::NONE;
    }

    return piRegBinding->GetIPAddress();
}

/*

Remarks

*/
PUBLIC
const IMSList<PreferenceHeader*>& Service::GetAcceptContactHeaders() const
{
    //---------------------------------------------------------------------------------------------

    return objAcceptContacts;
}

/*

Remarks

*/
PUBLIC
const AString& Service::GetAssociatedURI(IN IMS_SINT32 nScheme) const
{
    const AStringArray &objAssociatedURIs = GetAssociatedURIs();

    //---------------------------------------------------------------------------------------------

    if (objAssociatedURIs.IsEmpty())
    {
        return AString::ConstNull();
    }

    if (nScheme == SIP::URI_SCHEME_TEL)
    {
        for (IMS_UINT32 i = 0; i < objAuthorizedUserIds.GetSize(); ++i)
        {
            ISIPHeader *piHeader = objAuthorizedUserIds.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            const SIPAddress *pAddress = piHeader->GetSIPAddress();

            if (pAddress == IMS_NULL)
                continue;

            if (pAddress->IsSchemeTEL())
            {
                if (static_cast<IMS_SINT32>(i) < objAssociatedURIs.GetCount())
                {
                    return objAssociatedURIs.GetElementAt(static_cast<IMS_SINT32>(i));
                }
            }
        }
    }
    else if (nScheme == SIP::URI_SCHEME_SIP)
    {
        for (IMS_UINT32 i = 0; i < objAuthorizedUserIds.GetSize(); ++i)
        {
            ISIPHeader *piHeader = objAuthorizedUserIds.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            const SIPAddress *pAddress = piHeader->GetSIPAddress();

            if (pAddress == IMS_NULL)
                continue;

            if (pAddress->IsSchemeSIP())
            {
                if (static_cast<IMS_SINT32>(i) < objAssociatedURIs.GetCount())
                {
                    return objAssociatedURIs.GetElementAt(static_cast<IMS_SINT32>(i));
                }
            }
        }
    }

    return AString::ConstNull();
}

/*

Remarks

*/
PUBLIC
const AStringArray& Service::GetAssociatedURIs() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return AStringArray::ConstNull();
    }

    return piRegBinding->GetAssociatedURIs();
}

/*

Remarks

*/
PUBLIC
const SIPAddress& Service::GetAuthorizedUserId() const
{
    //---------------------------------------------------------------------------------------------

    if (IsUserIdProvisioned())
    {
        return GetDefaultUserId();
    }

    if (!IsImsConnected())
    {
        return GetDefaultUserId();
    }

    return piRegBinding->GetAuthorizedAOR();
}

/*

Remarks

*/
PUBLIC
const CallerCapability* Service::GetCallerCapability() const
{
    //---------------------------------------------------------------------------------------------

    return pCallerCapability;
}

/*

Remarks

*/
PUBLIC
const SIPAddress& Service::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return SIPAddress::ConstNull();
    }

    return piRegBinding->GetContactAddress();
}

/*

Remarks

*/
PUBLIC
const SIPAddress* Service::GetContactAddressForOutgoingMessage() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return IMS_NULL;
    }

    return piRegBinding->GetContactAddressForOutgoingMessage();
}

/*

Remarks

*/
PUBLIC
ISIPHeader* Service::GetContactHeader(
        IN IMS_BOOL bPrivacy /* = IMS_FALSE */, IN IMS_BOOL bRequest /* = IMS_TRUE */,
        IN IMS_SINT32 nSIPMethod /* = (-1) SIPMethod::INVALID */) const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return IMS_NULL;
    }

    AString strContact;
    SIPMethod objMethod;

    switch (nSIPMethod)
    {
    case SIPMethod::INVITE:
    case SIPMethod::SUBSCRIBE:
    case SIPMethod::REFER:
    case SIPMethod::NOTIFY:
    case SIPMethod::OPTIONS:
    case SIPMethod::PUBLISH:
        objMethod = nSIPMethod;
        break;
    default:
        objMethod = static_cast<IMS_SINT32>(SIPMethod::INVALID);
        break;
    }

    IMS_BOOL bIsContactGRUU = IMS_FALSE;

    FormContactHeader(objMethod, bPrivacy, bRequest, strContact, bIsContactGRUU);

    (void) bIsContactGRUU;

    return SIPParsingHelper::CreateHeader(ISIPHeader::CONTACT_NORMAL, strContact);
}

/*

Remarks

*/
PUBLIC
const SIPAddress& Service::GetDefaultUserId() const
{
    //---------------------------------------------------------------------------------------------

    return objIMPU;
}

/*

Remarks
 CONTACT_FEATURE_CAPS
*/
PUBLIC
IFeatureCaps* Service::GetFeatureCaps() const
{
    //---------------------------------------------------------------------------------------------

    return pFeatureCaps;
}

/*

Remarks

*/
PUBLIC
ServiceFilterCriteria* Service::GetFilterCriteria() const
{
    //---------------------------------------------------------------------------------------------

    return pFilterCriteria;
}

/*

Remarks

*/
PUBLIC
const SIPParameter* Service::GetInstanceParameter() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piRegBinding->GetInstanceParameter();
}

/*

Remarks

*/
PUBLIC
const AStringArray& Service::GetPathHeaders() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return AStringArray::ConstNull();
    }

    return piRegBinding->GetPathHeaders();
}

/*

Remarks

*/
PUBLIC
const IRegInfo* Service::GetRegInfo() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return IMS_NULL;
    }

    return piRegBinding->GetRegInfo();
}

/*

Remarks

*/
PUBLIC
const AStringArray& Service::GetServiceRoutes() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsImsConnected())
    {
        return AStringArray::ConstNull();
    }

    return piRegBinding->GetServiceRoutes();
}

/*

Remarks

*/
PROTECTED
const ISipConfigV* Service::GetISipConfigV() const
{
    return SIPConfigProxy::GetSipConfigV(GetSlotId());
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
SIPProfile* Service::GetSIPProfile() const
{
    //---------------------------------------------------------------------------------------------

    if (pSIPProfile.IsNull())
    {
        return (piRegBinding != IMS_NULL) ? piRegBinding->GetSIPProfile() : IMS_NULL;
    }

    return pSIPProfile.Get();
}

/*

Remarks
 MULTI_SUBS
*/
PUBLIC
const AString& Service::GetSubscriberId() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return piRegBinding->GetSubscriberId();
}

/*

Remarks

*/
PUBLIC
const SIPAddress* Service::GetPublicGRUU() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piRegBinding->GetPublicGRUU();
}

/*

Remarks

*/
PUBLIC
const SIPAddress* Service::GetTemporaryGRUU() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piRegBinding->GetTemporaryGRUU();
}

/*

Remarks

*/
PUBLIC
const IMSList<SIPAddress*>& Service::GetTemporaryGRUUs() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
    {
        return SIPAddress::ConstEmptyList();
    }

    return piRegBinding->GetTemporaryGRUUs();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::IsBehindNAT() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
        return IMS_FALSE;

    return piRegBinding->IsBehindNAT();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::IsEventPackageSupported(IN CONST AString &strEvent) const
{
    //---------------------------------------------------------------------------------------------

    if (pAppConfig == IMS_NULL)
        return IMS_FALSE;

    return pAppConfig->IsEventPackageSupported(strEvent);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::IsImsConnected() const
{
    //---------------------------------------------------------------------------------------------

    return bImsConnected;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::IsWithinTrustDomain() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
        return IMS_FALSE;

    return piRegBinding->IsWithinTrustDomain();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::AddFeatureTags(IN CONST IMSList<AString> &objFeatureTags,
        IN IMS_BOOL bRegRequired /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return UpdateFeatureTags(objFeatureTags, bRegRequired, 1);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::RemoveFeatureTags(IN CONST IMSList<AString> &objFeatureTags,
        IN IMS_BOOL bRegRequired /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    return UpdateFeatureTags(objFeatureTags, bRegRequired, 2);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::UpdateFeatureTags(IN CONST IMSList<AString> &objFeatureTags,
        IN IMS_BOOL bRegRequired /* = IMS_TRUE */, IN IMS_SINT32 nOP /* = 1 (1: add, 2: remove) */)
{
    //---------------------------------------------------------------------------------------------

    if (objFeatureTags.IsEmpty())
    {
        return IMS_FALSE;
    }

    AString strName;
    AString strValue;
    AString strTemp;
    FeatureSet *pFeatureSet;
    IMS_BOOL bCallerCapabilityChanged = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objFeatureTags.GetSize(); ++i)
    {
        strTemp = objFeatureTags.GetAt(i).MakeLower();
        IMS_SINT32 nCount = strTemp.SplitF(TextParser::CHAR_EQUAL, strName, strValue);

        if (strName.GetLength() == 0)
            continue;

        if (nCount == 1)
        {
            pFeatureSet = new FeatureSet(strName);
            pFeatureSet->Add(strName);
        }
        else if (nCount == 2)
        {
            pFeatureSet = new FeatureSet(strName, strValue);
        }
        else
        {
            continue;
        }

        // ADD
        if (nOP == 1)
        {
            if (pCallerCapability->AddFeature(pFeatureSet))
            {
                bCallerCapabilityChanged = IMS_TRUE;
            }
        }
        // REMOVE
        else if (nOP == 2)
        {
            if (pCallerCapability->RemoveFeature(pFeatureSet))
            {
                bCallerCapabilityChanged = IMS_TRUE;
            }
        }

        if (pFeatureSet != IMS_NULL)
        {
            delete pFeatureSet;
        }
    }

    if (bCallerCapabilityChanged)
    {
        IMS_TRACE_D("Service(%s) :: caller capability is updated (%s)",
                GetServiceId().GetStr(), (nOP == 1) ? "ADD" : "REMOVE", 0);

        bFlag_CallerCapabilityChanged = IMS_TRUE;
    }

    if ((piRegBinding != IMS_NULL) && bFlag_CallerCapabilityChanged && bRegRequired)
    {
        bFlag_CallerCapabilityChanged = IMS_FALSE;
        piRegBinding->NotifyCallerCapabilityChanged();
    }

    return bCallerCapabilityChanged;
}

/*

Remarks

*/
PUBLIC
void Service::NotifyError(IN IMS_SINT32 nErrorCode)
{
    // The subclass MUST implement this method to handle a loss of network,
    // no communication with proxy.
    const IMSList<Method*> &objMethods = pMethodMngr->GetMethods();

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); ++i)
    {
        Method *pMethod = objMethods.GetAt(i);

        if (pMethod == IMS_NULL)
            continue;

        pMethod->Exception_NotifyError(nErrorCode);
    }

    Exception_NotifyError(nErrorCode);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::NotifyRequest(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    return ServerConnection_NotifyRequest(piSSC);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::SendResponse(IN ISIPServerConnection *piSSC, IN IMS_SINT32 nStatusCode,
        IN CONST AString &strPhrase /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ Sending %d response to %s ...",
            nStatusCode, piSSC->GetMethod().ToString().GetStr(), 0);

    if (!CreateResponse(piSSC, nStatusCode, strPhrase))
    {
        return IMS_FALSE;
    }

    if (piSSC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending SIP response failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void Service::SetServiceManagerListener(IN IServiceManagerListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piServiceManagerListener = piListener;
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
void Service::SetSIPProfile(IN SIPProfile *pProfile)
{
    //---------------------------------------------------------------------------------------------

    pSIPProfile = pProfile;
}

/*

Remarks

*/
PUBLIC
void Service::RegisterMethod(IN Method *pMethod)
{
    //---------------------------------------------------------------------------------------------

    pMethodMngr->AddMethod(pMethod);
}

/*

Remarks

*/
PUBLIC
void Service::DeregisterMethod(IN Method *pMethod)
{
    //---------------------------------------------------------------------------------------------

    pMethodMngr->RemoveMethod(pMethod);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::ValidateMethod(IN CONST SIPMethod &objMethod)
{
    //---------------------------------------------------------------------------------------------

    // Handle a specific behavior for an incoming INVITE request
    if (objMethod.Equals(SIPMethod::INVITE))
    {
        // If the service does not support any media, it can't receive any incoming INVITE request
        if (!(pAppConfig->IsStreamMediaAudioSupported()
                || pAppConfig->IsStreamMediaVideoSupported()
                || pAppConfig->IsFramedMediaSupported()
                || pAppConfig->IsBasicMediaSupported()))
        {
            return IMS_FALSE;
        }
    }

    const SipConfigV *pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray &objMethods = pSipConfigV->GetAllowMethods();

        return objMethods.Contains(objMethod.ToString());
    }
    else
    {
        return IMS_FALSE;
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::ValidateRequestURI(IN CONST SIPAddress &objRequestURI,
        IN ISIPDialog *piDialog /* = IMS_NULL */,
        IN IMS_BOOL bIsMidDialogRequest /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (bIsMidDialogRequest)
    {
        // Checks GRUU identities if it is supported
        if (SIPConfigProxy::IsGRUUConfigured(GetSlotId(), GetSIPProfile()))
        {
            static const AString GR("gr");
            const SIPParameter *pParamGR = objRequestURI.GetParameter(GR);

            if (pParamGR != IMS_NULL)
            {
                if (pParamGR->GetValue().GetLength() > 0)
                {
                    // If the pub-gruu is available, then compare it with the Request-URI
                    const SIPAddress *pPubGRUU =
                            IsRegBindingOnActive() ? \
                            GetPublicGRUU() : objCachedRegBinding.GetPublicGRUU();

                    if (pPubGRUU != IMS_NULL)
                    {
                        if (pPubGRUU->Equals(objRequestURI))
                        {
                            IMS_TRACE_D("pub-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }
                else
                {
                    //4 List ?
                    // If the temp-gruu is available, then compare it with the Request-URI
                    const SIPAddress *pTempGRUU =
                            IsRegBindingOnActive() ? \
                            GetTemporaryGRUU() : objCachedRegBinding.GetTemporaryGRUU();

                    if (pTempGRUU != IMS_NULL)
                    {
                        if (pTempGRUU->Equals(objRequestURI))
                        {
                            IMS_TRACE_D("temp-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }

                IMS_TRACE_D("Request-URI contains a 'gr' parameter & GRUU ids are not matched",
                        0, 0, 0);

                return IMS_FALSE;
            }
        }

        // Checks if the contact address matches or not.
        const SIPAddress *pContact = IMS_NULL;

        if (piDialog != IMS_NULL)
        {
            const ISIPHeader *piContactHeader = piDialog->GetContactHeader();

            if (piContactHeader != IMS_NULL)
            {
                pContact = piContactHeader->GetSIPAddress();
            }
        }

        if (pContact == IMS_NULL)
        {
            IMS_TRACE_D("Contact header is not present in the dialog; " \
                    "use the default contact address...", 0, 0, 0);

            if (IsRegBindingOnActive())
            {
                pContact = GetContactAddressForOutgoingMessage();
            }
            else
            {
                pContact = objCachedRegBinding.GetContactAddressForOutgoingMessage();
            }
        }

        if ((pContact != IMS_NULL) && pContact->Equals(objRequestURI))
        {
            return IMS_TRUE;
        }

        const SIPAddress &objContact =
                IsRegBindingOnActive() ? \
                GetContactAddress() : objCachedRegBinding.GetContactAddress();

        if (objContact.Equals(objRequestURI))
        {
            return IMS_TRUE;
        }
    }
    else
    {
        // Checks GRUU identities if it is supported
        if (SIPConfigProxy::IsGRUUConfigured(GetSlotId(), GetSIPProfile()))
        {
            static const AString GR("gr");
            const SIPParameter *pParamGR = objRequestURI.GetParameter(GR);

            if (pParamGR != IMS_NULL)
            {
                if (pParamGR->GetValue().GetLength() > 0)
                {
                    // If the pub-gruu is available, then compare it with the Request-URI
                    const SIPAddress *pPubGRUU = GetPublicGRUU();

                    if (pPubGRUU != IMS_NULL)
                    {
                        if (pPubGRUU->Equals(objRequestURI))
                        {
                            IMS_TRACE_D("pub-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }
                else
                {
                    //4 List ?
                    // If the temp-gruu is available, then compare it with the Request-URI
                    const SIPAddress *pTempGRUU = GetTemporaryGRUU();

                    if (pTempGRUU != IMS_NULL)
                    {
                        if (pTempGRUU->Equals(objRequestURI))
                        {
                            IMS_TRACE_D("temp-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }

                IMS_TRACE_D("Request-URI contains a 'gr' parameter & GRUU ids are not matched",
                        0, 0, 0);

                return IMS_FALSE;
            }
        }

        // Checks if the contact address matches or not.
        const SIPAddress &objContact = GetContactAddress();

        if (objContact.Equals(objRequestURI))
        {
            return IMS_TRUE;
        }

        // Checks if the contact address for outgoing message matches or not.
        const SIPAddress *pContact = GetContactAddressForOutgoingMessage();

        if ((pContact != IMS_NULL) && pContact->Equals(objRequestURI))
        {
            IMS_TRACE_D("Request-URI matches the contact address for outgoing message", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    // Checks if the public user identity matches or not
    for (IMS_UINT32 i = 0; i < objAuthorizedUserIds.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objAuthorizedUserIds.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        const SIPAddress *pAddress = piHeader->GetSIPAddress();

        if (pAddress == IMS_NULL)
            continue;

        if (pAddress->Equals(objRequestURI))
        {
            // The public user identity is matched, so this message can be routed to this service.
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Service::ValidateRequestURIForIPAndPort(IN CONST SIPAddress &objRequestURI,
        IN ISIPDialog *piDialog/* = IMS_NULL */,
        IN IMS_BOOL bIsMidDialogRequest/* = IMS_FALSE*/)
{
    IPAddress objIPForRURI(objRequestURI.GetHost());

    //---------------------------------------------------------------------------------------------

    if (!objIPForRURI.IsIPv4Address() && !objIPForRURI.IsIPv6Address())
    {
        IMS_TRACE_D("R-URI is not a format of IP address", 0, 0, 0);
        return IMS_FALSE;
    }

    if (bIsMidDialogRequest)
    {
        // Checks if the contact address matches or not.
        const SIPAddress *pContact = IMS_NULL;

        if (piDialog != IMS_NULL)
        {
            const ISIPHeader *piContactHeader = piDialog->GetContactHeader();

            if (piContactHeader != IMS_NULL)
            {
                pContact = piContactHeader->GetSIPAddress();
            }
        }

        if (pContact == IMS_NULL)
        {
            IMS_TRACE_D("Contact header is not present in the dialog; " \
                    "use the default contact address...", 0, 0, 0);

            if (IsRegBindingOnActive())
            {
                pContact = GetContactAddressForOutgoingMessage();
            }
            else
            {
                pContact = objCachedRegBinding.GetContactAddressForOutgoingMessage();
            }
        }

        if (pContact != IMS_NULL)
        {
            IPAddress objIPForContact(pContact->GetHost());

            if (objIPForRURI.Equals(objIPForContact)
                    && (objRequestURI.GetPort() == pContact->GetPort()))
            {
                IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
                return IMS_TRUE;
            }
        }

        const SIPAddress &objContact =
                IsRegBindingOnActive() ? \
                GetContactAddress() : objCachedRegBinding.GetContactAddress();

        IPAddress objIPForContact(objContact.GetHost());

        if (objIPForRURI.Equals(objIPForContact)
                && (objRequestURI.GetPort() == objContact.GetPort()))
        {
            IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    else
    {
        // Checks if the contact address matches or not.
        const SIPAddress &objContact = GetContactAddress();

        IPAddress objIPForContact(objContact.GetHost());

        if (objIPForRURI.Equals(objIPForContact)
                && (objRequestURI.GetPort() == objContact.GetPort()))
        {
            IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
            return IMS_TRUE;
        }

        // Checks if the contact address for outgoing message matches or not.
        const SIPAddress *pContact = GetContactAddressForOutgoingMessage();

        if (pContact != IMS_NULL)
        {
            IPAddress objIPForContact(pContact->GetHost());

            if (objIPForRURI.Equals(objIPForContact)
                    && (objRequestURI.GetPort() == pContact->GetPort()))
            {
                IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC GLOBAL
IMS_BOOL Service::ValidateFromAndTo(IN CONST AString &strFrom, IN CONST AString &strTo,
        IN IMS_BOOL bToLenient)
{
    SIPAddress objAddress;

    //---------------------------------------------------------------------------------------------

    // Validates From field
    if (!strFrom.IsNULL())
    {
        if (!objAddress.Create(strFrom))
        {
            IMS_TRACE_E(0, "ILLEGAL ARGUMENT - From (%s)",
                    SIPDebug::GetUri1(strFrom).GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    // Validates To field
    if (bToLenient && strTo.IsNULL())
    {
        return IMS_TRUE;
    }

    if (strTo.IsNULL())
    {
        IMS_TRACE_E(0, "ILLEGAL ARGUMENT - To", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!objAddress.Create(strTo))
    {
        IMS_TRACE_E(0, "ILLEGAL ARGUMENT - To (%s)",
                SIPDebug::GetUri1(strTo).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*
 Checks the arguments of Refer-To header

Remarks

*/
PUBLIC GLOBAL
IMS_BOOL Service::ValidateReferTo(IN CONST AString &strURI, IN CONST AString &strMethod)
{
    // Checks the body of Refer-To header
    SIPAddress objURI;

    //---------------------------------------------------------------------------------------------

    if (!objURI.Create(strURI))
    {
        IMS_TRACE_E(0, "ILLEGAL ARGUMENT - Refer-To (%s)",
                SIPDebug::GetUri1(strURI).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // Checks method parameter
    if (strMethod.GetLength() == 0)
    {
        return IMS_TRUE;
    }

    for (IMS_SINT32 i = 0; i < strMethod.GetLength(); ++i)
    {
        const IMS_CHAR ch = strMethod[i];

        if ((ch < 'A') || (ch > 'Z'))
        {
            IMS_TRACE_E(0, "ILLEGAL ARGUMENT - Method (%s) in Refer-To", strMethod.GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::Close()
{
    //---------------------------------------------------------------------------------------------

    // Destroy a service-specific configurations .....
    SipConfig *pSipConfig = const_cast<SipConfig *>(
            ConfigurationManager::GetInstance()->GetSipConfig(GetSlotId()));

    // Remove the configuration update listener
    const SipConfigV *pSipConfigV = pSipConfig->GetServiceConfig();

    if (pSipConfigV != IMS_NULL)
    {
        IConfigurable *piConfigurable
                = static_cast<const ISipConfigV*>(pSipConfigV)->GetConfigurable();

        if (piConfigurable != IMS_NULL)
        {
            piConfigurable->RemoveListener(IConfigurable::CP_I_FEATURE_TAG_OPTIONS, this);
            piConfigurable->RemoveListener(IConfigurable::CP_I_SIP_ALL, this);
        }
    }

    if (piServiceManagerListener != IMS_NULL)
    {
        piServiceManagerListener->ServiceClosed(this);
    }

    if (piRegBinding != IMS_NULL)
    {
        piRegBinding->SetListener(IMS_NULL);
    }

    PostMessage(AMSG_DESTROY, 0, 0);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Service::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCPI,
        IN const AString &strConfName /* = AString::ConstNull() */,
        IN const AString &strExtraParam /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    (void) strConfName;
    (void) strExtraParam;

    if ((nCPI == IConfigurable::CP_I_FEATURE_TAG_OPTIONS)
            || (nCPI == IConfigurable::CP_I_SIP_ALL))
    {
        IMS_UINT32 nTmpFTs = nFeatureTags;
        const ISipConfigV *piSipConfigV = GetSipConfigV();

        if (piSipConfigV != IMS_NULL)
        {
            nTmpFTs = piSipConfigV->GetFeatureTagOptions();
        }

        if (nTmpFTs != nFeatureTags)
        {
            IMS_TRACE_D("SIP feature-tags is updated :: %08X >> %08X", nFeatureTags, nTmpFTs, 0);

            nFeatureTags = nTmpFTs;

            UpdateCallerCapabilityNPreference();
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::RegBinding_OnActive()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Service (%s) :: OnActive()", strServiceId.GetStr(), 0, 0);

    bImsConnected = IMS_TRUE;

    UpdateAuthorizedUserIds();
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    UpdateRegBindings();

    // CONTACT_FEATURE_CAPS
    pFeatureCaps->UpdateRegCaps(pCallerCapability);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::RegBinding_OnDestroy()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Service (%s) :: OnDestroy()", strServiceId.GetStr(), 0, 0);

    if (bImsConnected)
    {
        NotifyError(IMSError::SERVICE_CLOSED);
    }

    bImsConnected = IMS_FALSE;
    piRegBinding = IMS_NULL;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::RegBinding_OnInit(IN CONST SIPAddress *pAOR)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Service (%s) :: OnInit()", strServiceId.GetStr(), 0, 0);

    if (pAOR != IMS_NULL)
    {
        objIMPU = *pAOR;
    }
    else
    {
        CreateDefaultPublicUserId();
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::RegBinding_OnQueryCapability(OUT CallerCapability *&pCapability)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Service (%s) :: OnQueryCapability()", strServiceId.GetStr(), 0, 0);

    pCapability = pCallerCapability;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::RegBinding_OnQueryRegistrationHeaders(OUT AStringArray &objHeaders)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Service (%s) :: OnQueryRegistrationHeaders()", strServiceId.GetStr(), 0, 0);

    if (pAppConfig == IMS_NULL)
    {
        return;
    }

    const CoreServiceConfig *pServiceConfig = pAppConfig->GetCoreServiceConfigEx(GetServiceId());

    if (pServiceConfig == IMS_NULL)
    {
        return;
    }

    objHeaders = pServiceConfig->GetRegistrationHeaders();
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::RegBinding_OnTerminated()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Service (%s) :: OnTerminated()", strServiceId.GetStr(), 0, 0);

    if (bImsConnected)
    {
        NotifyError(IMSError::SERVICE_CLOSED);
    }

    bImsConnected = IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Service::Abort()
{
    //---------------------------------------------------------------------------------------------
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Service::ServerConnection_NotifyRequest(IN ISIPServerConnection * /* piSSC */)
{
    //---------------------------------------------------------------------------------------------

    // The subclass MUST implement this method to handle new incoming SIP requests

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED
void Service::FormContactHeader(IN CONST SIPMethod &objMethod, IN IMS_BOOL bPrivacy,
        IN IMS_BOOL bRequest, OUT AString &strContact, OUT IMS_BOOL &bIsContactGRUU) const
{
    IMS_BOOL bRegBindingOnActive = IsRegBindingOnActive();

    //---------------------------------------------------------------------------------------------

    bIsContactGRUU = IMS_FALSE;

    if (SIPConfigProxy::IsGRUUConfigured(GetSlotId(), GetSIPProfile()))
    {
        if (bRequest && SIPConfigProxy::IsMultipleRegConfigured(GetSlotId(), GetSIPProfile()))
        {
            SIPAddress objContact;

            if (!bPrivacy)
            {
                // If the pub-gruu is available, then add it to the Contact header
                const SIPAddress *pPubGRUU = GetPublicGRUU();

                if (pPubGRUU != IMS_NULL)
                {
                    bIsContactGRUU = IMS_TRUE;
                    objContact = *pPubGRUU;
                }
                else
                {
                    const SIPAddress *pContact = GetContactAddressForOutgoingMessage();

                    objContact = (pContact != IMS_NULL) ? *pContact : GetContactAddress();
                }
            }
            else
            {
                // If the temp-gruu is available, then add it to the Contact header
                const SIPAddress *pTempGRUU = GetTemporaryGRUU();

                if (pTempGRUU != IMS_NULL)
                {
                    bIsContactGRUU = IMS_TRUE;
                    objContact = *pTempGRUU;
                }
                else
                {
                    const SIPAddress *pContact = GetContactAddressForOutgoingMessage();

                    objContact = (pContact != IMS_NULL) ? *pContact : GetContactAddress();
                }
            }

            objContact.AddParameter(SIP::STR_OB, AString::ConstNull());

            strContact = objContact.ToString();
        }
        else
        {
            if (!bPrivacy)
            {
                // If the pub-gruu is available, then add it to the Contact header
                const SIPAddress *pPubGRUU = bRegBindingOnActive ? \
                        GetPublicGRUU() : objCachedRegBinding.GetPublicGRUU();

                if (pPubGRUU != IMS_NULL)
                {
                    bIsContactGRUU = IMS_TRUE;
                    strContact = pPubGRUU->ToString();
                }
            }
            else
            {
                // If the temp-gruu is available, then add it to the Contact header
                const SIPAddress *pTempGRUU = bRegBindingOnActive ? \
                        GetTemporaryGRUU() : objCachedRegBinding.GetTemporaryGRUU();

                if (pTempGRUU != IMS_NULL)
                {
                    bIsContactGRUU = IMS_TRUE;
                    strContact = pTempGRUU->ToString();
                }
            }

            if (!bIsContactGRUU)
            {
                const SIPAddress *pContact = bRegBindingOnActive ? \
                        GetContactAddressForOutgoingMessage() : \
                        objCachedRegBinding.GetContactAddressForOutgoingMessage();

                if (pContact != IMS_NULL)
                {
                    strContact = pContact->ToString();
                }
                else
                {
                    strContact = bRegBindingOnActive ? \
                            GetContactAddress().ToString() : \
                            objCachedRegBinding.GetContactAddress().ToString();
                }
            }
        }
    }
    else
    {
        if (bRequest && SIPConfigProxy::IsMultipleRegConfigured(GetSlotId(), GetSIPProfile()))
        {
            const SIPAddress *pContact = GetContactAddressForOutgoingMessage();
            SIPAddress objContact = (pContact != IMS_NULL) ? *pContact : GetContactAddress();

            objContact.AddParameter(SIP::STR_OB, AString::ConstNull());

            strContact = objContact.ToString();
        }
        else
        {
            const SIPAddress *pContact = bRegBindingOnActive ? \
                    GetContactAddressForOutgoingMessage() : \
                    objCachedRegBinding.GetContactAddressForOutgoingMessage();

            if (pContact != IMS_NULL)
            {
                strContact = pContact->ToString();
            }
            else
            {
                strContact = bRegBindingOnActive ? \
                        GetContactAddress().ToString() : \
                        objCachedRegBinding.GetContactAddress().ToString();
            }
        }
    }

    if (!strContact.EndsWith(TextParser::CHAR_RAQUOT))
    {
        strContact.Prepend(TextParser::CHAR_LAQUOT);
        strContact.Append(TextParser::CHAR_RAQUOT);
    }

    // CONTACT_FEATURE_CAPS
    AString strContactFeatures(AString::ConstNull());

    if (!objMethod.Equals(SIPMethod::INVALID)
            && pFeatureCaps->FormContactFeatures(objMethod.ToInt(), bRequest, strContactFeatures))
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(strContactFeatures);
    }
    else if (!pCallerCapability->IsEmpty())
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(pCallerCapability->ToString());
    }

    IMS_BOOL bDeviceIdRequired = IMS_FALSE;

    // "+sip.instance" parameter can be inserted
    // if UE detects that the destination of requests & responses is a trusted intermediary.
    if (IsWithinTrustDomain())
    {
        bDeviceIdRequired = IMS_TRUE;
    }

    // CONTACT_FEATURE_CAPS
    if (bDeviceIdRequired && strContact.Contains(SIP::STR_SIP_INSTANCE))
    {
        IMS_TRACE_D("FeatureCaps :: Device id is directly formed by the application; ignore it",
                0, 0, 0);
        bDeviceIdRequired = IMS_FALSE;
    }

    if (bDeviceIdRequired)
    {
        // Add the '+sip.instance' parameter
        const SIPParameter *pParameter = GetInstanceParameter();

        if (pParameter != IMS_NULL)
        {
            strContact.Append(TextParser::CHAR_SEMICOLON);
            strContact.Append(pParameter->ToString());
        }
    }
}

/*

Remarks

*/
PROTECTED
IRegBinding* Service::GetRegBinding() const
{
    //---------------------------------------------------------------------------------------------

    return piRegBinding;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Service::IsUserIdProvisioned() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_ProvisionedUserId;
}

/*

Remarks

*/
PROTECTED
void Service::SetGRUUOptionTagInMidDialog(IN ISIPDialog *piDialog, IN_OUT ISIPMessage *&piSIPMsg)
{
    const ISIPHeader *piContactHeader = (piDialog != IMS_NULL) ? \
            piDialog->GetContactHeader() : IMS_NULL;
    const SIPAddress *pContact = (piContactHeader != IMS_NULL) ? \
            piContactHeader->GetSIPAddress() : IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pContact != IMS_NULL)
    {
        if (pContact->GetParameter("gr") != IMS_NULL)
        {
            piSIPMsg->AddHeader(ISIPHeader::SUPPORTED, SIP::STR_GRUU);
        }
    }
}

/*

Remarks

*/
PRIVATE
void Service::CreateDefaultPublicUserId()
{
    ConfigurationManager *pConfigMngr = ConfigurationManager::GetInstance();

    //---------------------------------------------------------------------------------------------

    // Read a default public user identity
    if (!bFlag_ProvisionedUserId)
    {
        const SubscriberConfig *pSubsConfig
                = pConfigMngr->GetSubscriberConfig(SubscriberConfig::GetDefaultId(), GetSlotId());

        if (pSubsConfig == IMS_NULL)
            return;

        if (!objIMPU.Create(pSubsConfig->GetPublicUserId()))
        {
            IMS_TRACE_E(0, "Creating a default public user identity of the device failed",
                    0, 0, 0);
        }
    }
}

/*

Remarks

*/
PRIVATE
IMS_UINT32 Service::GetServiceCode() const
{
    AString strServiceCode = strAppId + strServiceId;

    //---------------------------------------------------------------------------------------------

    return strServiceCode.GetHashCode();
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Service::IsRegBindingOnActive() const
{
    //---------------------------------------------------------------------------------------------

    return (IsImsConnected() && (piRegBinding != IMS_NULL));
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Service::SetPPreferredIdentityHeader(IN IMS_SINT32 nPreferredId,
        IN_OUT ISIPMessage *&piSIPMsg)
{
    const AStringArray &objAssociatedURIs = IsRegBindingOnActive() ? \
            GetAssociatedURIs() : objCachedRegBinding.GetAssociatedURIs();

    //---------------------------------------------------------------------------------------------

    if (objAssociatedURIs.IsEmpty())
    {
        return IMS_TRUE;
    }

    if (nPreferredId == SipConfigV::PREFERRED_ID_DEFAULT)
    {
        if (piSIPMsg->AddHeader(ISIPHeader::P_PREFERRED_IDENTITY,
                objAssociatedURIs.GetElementAt(0)) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else if (nPreferredId == SipConfigV::PREFERRED_ID_ALL)
    {
        for (IMS_SINT32 i = 0; i < objAssociatedURIs.GetCount(); ++i)
        {
            if (piSIPMsg->AddHeader(ISIPHeader::P_PREFERRED_IDENTITY,
                    objAssociatedURIs.GetElementAt(i)) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
    }
    else
    {
        IMS_BOOL bTopmostSIP = IMS_TRUE;
        IMS_BOOL bTelURIFound = IMS_FALSE;
        IMS_SINT32 nCount = 1;

        if ((nPreferredId == SipConfigV::PREFERRED_ID_TEL)
                || (nPreferredId == SipConfigV::PREFERRED_ID_TEL_SIP))
        {
            bTopmostSIP = IMS_FALSE;
        }

        if ((nPreferredId == SipConfigV::PREFERRED_ID_SIP_TEL)
                || (nPreferredId == SipConfigV::PREFERRED_ID_TEL_SIP))
        {
            nCount += 1;
        }

        for (IMS_SINT32 i = 0; i < objAssociatedURIs.GetCount(); ++i)
        {
            const AString &strId = objAssociatedURIs.GetElementAt(i);

            if ((bTopmostSIP && (strId.Contains("sip:") || strId.Contains("sips:")))
                    || (!bTopmostSIP && strId.Contains("tel:")))
            {
                if (piSIPMsg->AddHeader(ISIPHeader::P_PREFERRED_IDENTITY, strId) != IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                    return IMS_FALSE;
                }

                if (!bTopmostSIP)
                {
                    bTelURIFound = IMS_TRUE;
                }
                break;
            }
        }

        if (nCount > 1)
        {
            for (IMS_SINT32 i = 0; i < objAssociatedURIs.GetCount(); ++i)
            {
                const AString &strId = objAssociatedURIs.GetElementAt(i);

                if ((bTopmostSIP && strId.Contains("tel:"))
                        || (!bTopmostSIP && (strId.Contains("sip:") || strId.Contains("sips:"))))
                {
                    if (piSIPMsg->AddHeader(ISIPHeader::P_PREFERRED_IDENTITY,
                            strId) != IMS_SUCCESS)
                    {
                        IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                        return IMS_FALSE;
                    }

                    break;
                }
            }
        }
        else
        {
            if (!bTopmostSIP && !bTelURIFound)
            {
                for (IMS_SINT32 i = 0; i < objAssociatedURIs.GetCount(); ++i)
                {
                    const AString &strId = objAssociatedURIs.GetElementAt(i);

                    if (strId.Contains("sip:") || strId.Contains("sips:"))
                    {
                        if (piSIPMsg->AddHeader(ISIPHeader::P_PREFERRED_IDENTITY,
                                strId) != IMS_SUCCESS)
                        {
                            IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                            return IMS_FALSE;
                        }
                        break;
                    }
                }
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void Service::SetRegBinding(IN IRegBinding *piRegBinding)
{
    //---------------------------------------------------------------------------------------------

    if (this->piRegBinding != IMS_NULL)
    {
        this->piRegBinding->SetListener(IMS_NULL);
    }

    this->piRegBinding = piRegBinding;

    if (this->piRegBinding != IMS_NULL)
    {
        this->piRegBinding->SetListener(this);
    }
}

/*

Remarks

*/
PRIVATE
void Service::UpdateAuthorizedUserIds()
{
    //---------------------------------------------------------------------------------------------

    // Clear the previous public user identities
    for (IMS_UINT32 i = 0; i < objAuthorizedUserIds.GetSize(); ++i)
    {
        ISIPHeader *piHeader = objAuthorizedUserIds.GetAt(i);

        if (piHeader != IMS_NULL)
        {
            piHeader->Destroy();
        }
    }

    objAuthorizedUserIds.Clear();

    const AStringArray& objAssociatedURIs = GetAssociatedURIs();

    for (IMS_SINT32 i = 0; i < objAssociatedURIs.GetCount(); ++i)
    {
        ISIPHeader *piHeader = SIPParsingHelper::CreateHeader(
                ISIPHeader::P_ASSOCIATED_URI, objAssociatedURIs.GetElementAt(i));

        if (piHeader != IMS_NULL)
        {
            objAuthorizedUserIds.Append(piHeader);
        }
    }

    IMS_TRACE_D("Service :: AssociatedURIs (%d), AuthorizedUserIds (%d)",
            objAssociatedURIs.GetCount(), objAuthorizedUserIds.GetSize(), 0);
}

/*

Remarks

*/
PRIVATE
void Service::UpdateCallerCapabilityNPreference()
{
    //---------------------------------------------------------------------------------------------

    const CoreServiceConfig *pServiceConfig = pAppConfig->GetCoreServiceConfigEx(strServiceId);

    pCallerCapability->Clear();

    // Updates the caller capabilities
    if (!pCallerCapability->Create(pAppConfig, pServiceConfig, GetSipConfigV()))
    {
        IMS_TRACE_E(0, "Creating the features from AppConfig & ServiceConfig failed", 0, 0, 0);
        return;
    }

    // Removes & updates the caller preference
    for (IMS_UINT32 i = 0; i < objAcceptContacts.GetSize(); ++i)
    {
        PreferenceHeader *pHeader = objAcceptContacts.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            delete pHeader;
        }
    }

    objAcceptContacts.Clear();

    if (!CallerPreference::CreateAcceptContactHeaders(
            pAppConfig, pServiceConfig, GetSipConfigV(), objAcceptContacts))
    {
        IMS_TRACE_E(0, "Creating Accept-Contact header failed", 0, 0, 0);
        return;
    }
}

/*

Remarks

*/
PRIVATE
void Service::UpdateRegBindings()
{
    //---------------------------------------------------------------------------------------------

    if (piRegBinding == IMS_NULL)
    {
        objCachedRegBinding.SetPortUC(SIP::PORT_UNSPECIFIED);
        objCachedRegBinding.SetPortUS(SIP::PORT_UNSPECIFIED);
        // RFC5626_FLOW_CONTROL
        objCachedRegBinding.SetPortFlowControl(SIP::PORT_UNSPECIFIED);
        // MULTI_REG_TRANSPORT
        objCachedRegBinding.SetTransportExt(SIP::TRANSPORT_EXT_ANY);
        objCachedRegBinding.SetIPAddress(IPAddress::NONE);
        objCachedRegBinding.SetContactAddress(SIPAddress::ConstNull());
        objCachedRegBinding.SetContactAddressForOutgoingMessage(IMS_NULL);
        // IPSEC {
        objCachedRegBinding.SetSecurityClients(AStringArray::ConstNull());
        objCachedRegBinding.SetSecurityVerifys(AStringArray::ConstNull());
        // }
        // GRUU {
        objCachedRegBinding.SetPublicGRUU(IMS_NULL);
        objCachedRegBinding.SetTemporaryGRUU(IMS_NULL);
        // }
        objCachedRegBinding.SetAssociatedURIs(AStringArray::ConstNull());
    }
    else
    {
        objCachedRegBinding.SetPortUC(piRegBinding->GetPortUC());
        objCachedRegBinding.SetPortUS(piRegBinding->GetPortUS());
        // RFC5626_FLOW_CONTROL
        objCachedRegBinding.SetPortFlowControl(piRegBinding->GetPortFlowControl());
        // MULTI_REG_TRANSPORT
        objCachedRegBinding.SetTransportExt(piRegBinding->GetTransportExt());
        objCachedRegBinding.SetIPAddress(piRegBinding->GetIPAddress());
        objCachedRegBinding.SetContactAddress(piRegBinding->GetContactAddress());
        objCachedRegBinding.SetContactAddressForOutgoingMessage(
                piRegBinding->GetContactAddressForOutgoingMessage());
        // IPSEC {
        objCachedRegBinding.SetSecurityClients(piRegBinding->GetSecurityClients());
        objCachedRegBinding.SetSecurityVerifys(piRegBinding->GetSecurityVerifys());
        // }
        // GRUU {
        objCachedRegBinding.SetPublicGRUU(piRegBinding->GetPublicGRUU());
        objCachedRegBinding.SetTemporaryGRUU(piRegBinding->GetTemporaryGRUU());
        // }
        objCachedRegBinding.SetAssociatedURIs(piRegBinding->GetAssociatedURIs());
    }
}
