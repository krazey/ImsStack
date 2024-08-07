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
#include "ServiceNetwork.h"
#include "ServiceTrace.h"

#include "Feature.h"
#include "ImsIdentity.h"
#include "private/ConfigurationManager.h"
#include "private/SipConfig.h"
#include "private/SubscriberConfig.h"

#include "Connector.h"
#include "IRegBinding.h"
#include "IServiceCloseListener.h"
#include "ISipClientConnection.h"
#include "ISipConnectionNotifier.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImsCoreContext.h"
#include "PAccessNetworkInfoHeader.h"
#include "Service.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipTimerValuesHelper.h"
#include "base/Ims.h"
#include "base/Method.h"
#include "util/CallerCapability.h"
#include "util/CallerPreference.h"
// CALLER_PREFERENCE_MANAGER
#include "util/CallerPreferenceManager.h"
#include "util/MethodManager.h"
#include "util/PreferenceHeader.h"
#include "util/UserAgentHeader.h"

__IMS_TRACE_TAG_IMS__;

PUBLIC
Service::Service(IN const AString& strScheme, IN const AString& strAppId,
        IN const AString& strServiceId, IN const SipAddress* pImpu /*= IMS_NULL*/) :
        Connection(),
        m_strScheme(strScheme),
        m_strAppId(strAppId),
        m_strServiceId(strServiceId),
        m_pAppConfig(IMS_NULL),
        m_piServiceCloseListener(IMS_NULL),
        m_bImsConnected(IMS_FALSE),
        m_piRegBinding(IMS_NULL),
        m_pSipProfile(IMS_NULL),
        m_bProvisionedUserId(IMS_FALSE),
        m_objAuthorizedUserIds(ImsList<ISipHeader*>()),
        m_nFeatureTags(ISipConfigV::FEATURE_TAG_DEFAULT),
        m_objAcceptContacts(ImsList<PreferenceHeader*>()),
        m_pCallerCapability(IMS_NULL),
        m_bCallerCapabilityChanged(IMS_FALSE),
        m_pMethodMngr(IMS_NULL),
        m_pFilterCriteria(IMS_NULL),
        // CONTACT_FEATURE_CAPS
        m_pFeatureCaps(IMS_NULL)
{
    m_pMethodMngr = new MethodManager();
    m_pFilterCriteria = new ServiceFilterCriteria();
    // CONTACT_FEATURE_CAPS
    m_pFeatureCaps = new FeatureCaps();

    // If the user id is not provisioned, it needs to be set from the device's configuration.
    if (pImpu != IMS_NULL)
    {
        m_bProvisionedUserId = IMS_TRUE;
        m_objImpu = (*pImpu);
    }
}

PUBLIC VIRTUAL Service::~Service()
{
    if (m_pAppConfig != IMS_NULL)
    {
        delete m_pAppConfig;
    }

    // CONTACT_FEATURE_CAPS
    if (m_pFeatureCaps != IMS_NULL)
    {
        delete m_pFeatureCaps;
    }

    if (m_pFilterCriteria != IMS_NULL)
    {
        delete m_pFilterCriteria;
    }

    if (m_pMethodMngr != IMS_NULL)
    {
        delete m_pMethodMngr;
    }

    for (IMS_UINT32 i = 0; i < m_objAcceptContacts.GetSize(); ++i)
    {
        PreferenceHeader* pPrefHeader = m_objAcceptContacts.GetAt(i);

        if (pPrefHeader != IMS_NULL)
        {
            delete pPrefHeader;
        }
    }

    if (m_pCallerCapability != IMS_NULL)
    {
        delete m_pCallerCapability;
    }

    for (IMS_UINT32 i = 0; i < m_objAuthorizedUserIds.GetSize(); ++i)
    {
        ISipHeader* piHeader = m_objAuthorizedUserIds.GetAt(i);

        if (piHeader != IMS_NULL)
        {
            piHeader->Destroy();
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL Service::CreateConfig(IN const AppConfig& objAppConfig)
{
    m_pAppConfig = new AppConfig(objAppConfig);

    if (m_pAppConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating AppConfig failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Creates Contact URI for this service; Move to SetSIPReady(...) method.
    // CreateContactAddress();

    // Constructs a caller capability which sets in Contact header
    m_pCallerCapability = new CallerCapability(GetServiceCode());

    if (m_pCallerCapability == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const CoreServiceConfig* pServiceConfig = m_pAppConfig->GetCoreServiceConfigEx(m_strServiceId);

    if (!m_pCallerCapability->Create(m_pAppConfig, pServiceConfig, GetSipConfigV()))
    {
        IMS_TRACE_E(0, "Creating the features from AppConfig & ServiceConfig failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Create Accept-Contact headers for this service
    if (!CallerPreference::CreateAcceptContactHeaders(
                m_pAppConfig, pServiceConfig, GetSipConfigV(), m_objAcceptContacts))
    {
        IMS_TRACE_E(0, "Creating Accept-Contact header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update the current SIP feature tags
    const SipConfigV* pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        m_nFeatureTags = pSipConfigV->GetFeatureTagOptions();

        IConfigurable* piConfigurable =
                static_cast<const ISipConfigV*>(pSipConfigV)->GetConfigurable();

        if (piConfigurable != IMS_NULL)
        {
            piConfigurable->AddListener(IConfigurable::CP_I_FEATURE_TAG_OPTIONS, this);
            piConfigurable->AddListener(IConfigurable::CP_I_SIP_ALL, this);
        }
    }

    // __IMS_DEBUG__
    if (pServiceConfig != IMS_NULL)
    {
        IMS_TRACE_D("Service :: IARI (%s)", pServiceConfig->GetIari().ToString().GetStr(), 0, 0);

        const ImsList<ServiceIdentifier>& objIcsis = pServiceConfig->GetIcsis();

        for (IMS_UINT32 i = 0; i < objIcsis.GetSize(); ++i)
        {
            const ServiceIdentifier& objSi = objIcsis.GetAt(i);
            IMS_TRACE_D("Service :: ICSI (%s) at (%d)", objSi.ToString().GetStr(), i, 0);
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL ISipClientConnection* Service::CreateConnection(IN const SipAddress* pFrom,
        IN const SipAddress* pTo, IN const SipMethod& objMethod,
        IN IMS_BOOL bPrivacy /*= IMS_FALSE*/)
{
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_NULL;
    }

    if ((pFrom == IMS_NULL) || (pTo == IMS_NULL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks if the method is allowed
    if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::CANCEL) ||
            objMethod.Equals(SipMethod::BYE) || objMethod.Equals(SipMethod::INFO) ||
            objMethod.Equals(SipMethod::UPDATE) || objMethod.Equals(SipMethod::PRACK))
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_NULL;
    }

    IMS_BOOL bOverwriteTarget = IMS_FALSE;
    AString strTarget = pTo->ToString();

    IMS_TRACE_D("Service::CreateConnection() - To (%s), Method (%s)",
            SipDebug::GetUri1(strTarget).GetStr(), objMethod.ToString().GetStr(), 0);

    if (!pTo->IsSchemeSip() && !pTo->IsSchemeSips())
    {
        bOverwriteTarget = IMS_TRUE;
        strTarget = ImsIdentity::GetAnonymousUserId();
    }

    ISipClientConnection* piScc = DYNAMIC_CAST(ISipClientConnection*, Connector::Open(strTarget));

    if (piScc == IMS_NULL)
    {
        Ims::SetLastError(ImsError::CONNECTION_NOT_FOUND);

        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    piScc->SetSipProfile(GetSipProfile());

    IMS_SINT32 nPortS = m_piRegBinding->GetPortUs();
    IMS_SINT32 nPortC = m_piRegBinding->GetPortUc();

    // Sets the transport tuples
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    piScc->SetTransportTuple(m_piRegBinding->GetIpAddress(), nPortS, nPortC,
            m_piRegBinding->GetPortFlowControl(), m_piRegBinding->GetTransportExt());

    // Sets the transaction timer values
    IMS_SINT32 nTxnType = SipTimerValuesHelper::NON_INVITE_CLIENT;

    if (objMethod.Equals(SipMethod::INVITE))
    {
        nTxnType = SipTimerValuesHelper::INVITE_CLIENT;
    }

    piScc->SetTransactionTimerValues(
            SipTimerValuesHelper::GetValues(GetSlotId(), GetSipProfile(), nTxnType));

    if (piScc->InitRequest(objMethod.ToString(), IMS_NULL) != IMS_SUCCESS)
    {
        piScc->Close();
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing SIP request failed", 0, 0, 0);
        return IMS_NULL;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Sets From header field
    if (piSipMsg->SetHeader(ISipHeader::FROM, pFrom->ToString()) != IMS_SUCCESS)
    {
        piScc->Close();
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Setting From header failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (bOverwriteTarget)
    {
        strTarget = pTo->ToString();

        if (piScc->SetRequestUri(strTarget) != IMS_SUCCESS)
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting Request-URI failed", 0, 0, 0);
            return IMS_NULL;
        }

        // Sets To header field
        if (piSipMsg->SetHeader(ISipHeader::TO, strTarget) != IMS_SUCCESS)
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting To header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets Contact header fields:
    // INVITE, NOTIFY, OPTIONS, SUBSCRIBE, UPDATE, REGISTER, REFER, PUBLISH
    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::OPTIONS) ||
            objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER) ||
            objMethod.Equals(SipMethod::PUBLISH))
    {
        AString strContact;
        IMS_BOOL bIsContactGruu = IMS_FALSE;

        FormContactHeader(objMethod, bPrivacy, IMS_TRUE, strContact, bIsContactGruu);

        if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            return IMS_NULL;
        }

        if (bIsContactGruu)
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
        }
    }

    // Sets Accept-Contact header field
    for (IMS_UINT32 i = 0; i < m_objAcceptContacts.GetSize(); ++i)
    {
        const PreferenceHeader* pHeader = m_objAcceptContacts.GetAt(i);

        if (piSipMsg->AddHeader(ISipHeader::ACCEPT_CONTACT, pHeader->ToString()) != IMS_SUCCESS)
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Adding Accept-Contact header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    IMS_SINT32 j;

    // Sets Route (except for REGISTER) header fields
    const AStringArray& objServiceRoutes = GetServiceRoutes();

    for (j = 0; j < objServiceRoutes.GetCount(); ++j)
    {
        if (piSipMsg->AddHeader(ISipHeader::ROUTE, objServiceRoutes.GetElementAt(j)) != IMS_SUCCESS)
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Adding Route header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets Allow header fields
    const SipConfigV* pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray& objMethods = pSipConfigV->GetAllowMethods();

        for (j = 0; j < objMethods.GetCount(); ++j)
        {
            if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(j)) != IMS_SUCCESS)
            {
                piScc->Close();
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        // Sets P-Preferred-Identity (except for REGISTER) header fields
        // RFC 3325, INVITE/BYE/OPTIONS/SUBSCRIBE/NOTIFY/REFER
        // RFC 5876, all requests except for ACK/CANCEL
        if (!SetPPreferredIdentityHeader(pSipConfigV->GetPreferredId(), piSipMsg))
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting P-Preferred-Identity header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets User-Agent header field
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSipProfile()))
    {
        UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, GetSipProfile(), GetServiceId(),
                GetIpAddress(), GetSlotId(), piSipMsg);
    }

    // Sets P-Access-Network-Info header field
    PAccessNetworkInfoHeader::SetHeader(GetSlotId(), GetIpAddress(), GetSipProfile(), piSipMsg);

    // IPSEC {
    {
        // RFC 3329 - SIP Security Agreement:
        // Security related headers (Security-Client / Security-Verify)
        // Do not add Security-Client headers
        const AStringArray& objSecurityVerifys = m_piRegBinding->GetSecurityVerifys();

        for (IMS_SINT32 i = 0; i < objSecurityVerifys.GetCount(); ++i)
        {
            const AString& strHeader = objSecurityVerifys.GetElementAt(i);

            if (piSipMsg->AddHeader(ISipHeader::SECURITY_VERIFY, strHeader) != IMS_SUCCESS)
            {
                piScc->Close();
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Security-Verify failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        if (!objSecurityVerifys.IsEmpty())
        {
            piSipMsg->AddHeader(ISipHeader::REQUIRE, Sip::STR_SEC_AGREE);
            piSipMsg->AddHeader(ISipHeader::PROXY_REQUIRE, Sip::STR_SEC_AGREE);
        }
    }
    // }

    Ims::SetLastError(ImsError::NO_ERROR);

    return piScc;
}

PUBLIC VIRTUAL ISipClientConnection* Service::CreateConnection(IN ISipDialog* piDialog,
        IN const SipMethod& objMethod, IN IMS_BOOL bPrivacy /*= IMS_FALSE*/)
{
    (void)bPrivacy;

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG: do not check if Ims is connected or not.

    if (piDialog == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks if the method is allowed
    if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::CANCEL))
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_NULL;
    }

    // BYE_REQUEST_ON_DIALOG_TERMINATED
    AString strDialogId = piDialog->GetDialogIdEx();

    IMS_TRACE_I("Service::CreateConnection() - Dialog (%s), Method (%s)",
            SipDebug::GetCharA1(strDialogId.GetStr(), 8, '@'), objMethod.ToString().GetStr(), 0);

    ISipClientConnection* piScc = piDialog->GetNewClientConnection(objMethod.ToString());

    if (piScc == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating a new SIP connection failed - SipError (%d)",
                SipError::GetLastError(), 0, 0);
        return IMS_NULL;
    }

    piScc->SetSipProfile(GetSipProfile());

    IMS_SINT32 nPortS = IsRegBindingOnActive() ? m_piRegBinding->GetPortUs()
                                               : m_objCachedRegBinding.GetPortUs();
    IMS_SINT32 nPortC = IsRegBindingOnActive() ? m_piRegBinding->GetPortUc()
                                               : m_objCachedRegBinding.GetPortUc();

    // Sets the transport tuples
    if (IsRegBindingOnActive())
    {
        // RFC5626_FLOW_CONTROL
        // MULTI_REG_TRANSPORT
        piScc->SetTransportTuple(m_piRegBinding->GetIpAddress(), nPortS, nPortC,
                m_piRegBinding->GetPortFlowControl(), m_piRegBinding->GetTransportExt());
    }
    else
    {
        // RFC5626_FLOW_CONTROL
        // MULTI_REG_TRANSPORT
        piScc->SetTransportTuple(m_objCachedRegBinding.GetIpAddress(), nPortS, nPortC,
                m_objCachedRegBinding.GetPortFlowControl(),
                m_objCachedRegBinding.GetTransportExt());
    }

    // Sets the transaction timer values
    IMS_SINT32 nTxnType = SipTimerValuesHelper::NON_INVITE_CLIENT;

    if (objMethod.Equals(SipMethod::INVITE))
    {
        nTxnType = SipTimerValuesHelper::INVITE_CLIENT;
    }

    piScc->SetTransactionTimerValues(
            SipTimerValuesHelper::GetValues(GetSlotId(), GetSipProfile(), nTxnType));

    ISipMessage* piSipMsg = piScc->GetMessage();

    // CALLER_PREFERENCE_MANAGER
    // Sets Accept-Contact header fields
    if (!(SipConfigProxy::IsNoAcceptContactHeaderInBye(GetSlotId(), GetSipProfile()) &&
                piSipMsg->GetMethod().Equals(SipMethod::BYE)))
    {
        const ImsList<AString>& objAcceptContactsInDialog =
                ImsCoreContext::GetInstance()->GetCallerPreferenceManager()->GetAcceptContacts(
                        strDialogId);

        if (!objAcceptContactsInDialog.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < objAcceptContactsInDialog.GetSize(); ++i)
            {
                const AString& strHeader = objAcceptContactsInDialog.GetAt(i);

                if (piSipMsg->AddHeader(ISipHeader::ACCEPT_CONTACT, strHeader) != IMS_SUCCESS)
                {
                    piScc->Close();
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding Accept-Contact header failed", 0, 0, 0);
                    return IMS_NULL;
                }
            }
        }
        else
        {
            for (IMS_UINT32 i = 0; i < m_objAcceptContacts.GetSize(); ++i)
            {
                const PreferenceHeader* pHeader = m_objAcceptContacts.GetAt(i);

                if (piSipMsg->AddHeader(ISipHeader::ACCEPT_CONTACT, pHeader->ToString()) !=
                        IMS_SUCCESS)
                {
                    piScc->Close();
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding Accept-Contact header failed", 0, 0, 0);
                    return IMS_NULL;
                }
            }
        }
    }

    // Sets Allow header fields
    const SipConfigV* pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray& objMethods = pSipConfigV->GetAllowMethods();

        for (IMS_SINT32 j = 0; j < objMethods.GetCount(); ++j)
        {
            if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(j)) != IMS_SUCCESS)
            {
                piScc->Close();
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        // Sets P-Preferred-Identity (except for REGISTER) header fields
        // RFC 3325, INVITE/BYE/OPTIONS/SUBSCRIBE/NOTIFY/REFER
        // RFC 5876, all requests except for ACK/CANCEL
        if (!SetPPreferredIdentityHeader(pSipConfigV->GetPreferredId(), piSipMsg))
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Setting P-Preferred-Identity header failed", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Sets User-Agent header field
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSipProfile()))
    {
        UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, GetSipProfile(), GetServiceId(),
                GetIpAddress(), GetSlotId(), piSipMsg);
    }

    // Sets P-Access-Network-Info header field
    PAccessNetworkInfoHeader::SetHeader(GetSlotId(), GetIpAddress(), GetSipProfile(), piSipMsg);

    // In case of PRACK method, then do not contain the Security related headers.
    if (objMethod.Equals(SipMethod::PRACK))
    {
        Ims::SetLastError(ImsError::NO_ERROR);

        return piScc;
    }

    // IPSEC {
    {
        // RFC 3329 - SIP Security Agreement:
        // Security related headers (Security-Client / Security-Verify)
        // do not add Security-Client headers
        const AStringArray& objSecurityVerifys = IsRegBindingOnActive()
                ? m_piRegBinding->GetSecurityVerifys()
                : m_objCachedRegBinding.GetSecurityVerifys();

        for (IMS_SINT32 i = 0; i < objSecurityVerifys.GetCount(); ++i)
        {
            const AString& strHeader = objSecurityVerifys.GetElementAt(i);

            if (piSipMsg->AddHeader(ISipHeader::SECURITY_VERIFY, strHeader) != IMS_SUCCESS)
            {
                piScc->Close();
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding Security-Verify failed", 0, 0, 0);
                return IMS_NULL;
            }
        }

        if (!objSecurityVerifys.IsEmpty())
        {
            piSipMsg->AddHeader(ISipHeader::REQUIRE, Sip::STR_SEC_AGREE);
            piSipMsg->AddHeader(ISipHeader::PROXY_REQUIRE, Sip::STR_SEC_AGREE);
        }
    }
    // }

    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::REFER) ||
            objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::UPDATE) ||
            objMethod.Equals(SipMethod::NOTIFY))
    {
        SetGruuOptionTagInMidDialog(piDialog, piSipMsg);
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return piScc;
}

PUBLIC VIRTUAL ISipClientConnection* Service::CreateCancelConnection(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISipClientConnection is null", 0, 0, 0);
        return IMS_NULL;
    }

    ISipClientConnection* piCancelScc = piScc->InitCancel();

    if (piCancelScc == IMS_NULL)
    {
        return IMS_NULL;
    }

    // IPSEC {
    {
        if (IsRegBindingOnActive())
        {
            const AStringArray& objSecurityVerifys = m_piRegBinding->GetSecurityVerifys();

            if (!objSecurityVerifys.IsEmpty())
            {
                // Sets the transport tuples
                piCancelScc->SetTransportTuple(m_piRegBinding->GetIpAddress(),
                        m_piRegBinding->GetPortUs(), m_piRegBinding->GetPortUc(),
                        m_piRegBinding->GetPortFlowControl(), m_piRegBinding->GetTransportExt());
            }
        }
    }
    // }

    return piCancelScc;
}

PUBLIC VIRTUAL IMS_BOOL Service::CreateResponse(IN_OUT ISipServerConnection* piSsc,
        IN IMS_SINT32 nStatusCode, IN const AString& strPhrase /*= AString::ConstNull()*/,
        IN IMS_BOOL bPrivacy /*= IMS_FALSE*/)
{
    IMS_TRACE_I("Service::CreateResponse() - Method (%s), Status Code (%d)",
            piSsc->GetMethod().ToString().GetStr(), nStatusCode, 0);

    if (piSsc->InitResponse(nStatusCode) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing a SIP response failed - SipError(%d)",
                SipError::GetLastError(), 0, 0);

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    // MULTI_REG_SIP_PROFILE
    if (!m_pSipProfile.IsNull())
    {
        piSsc->SetSipProfile(m_pSipProfile.Get());
    }

    const SipMethod& objMethod = piSsc->GetMethod();

    // Sets the transaction timer values
    IMS_SINT32 nTxnType = SipTimerValuesHelper::NON_INVITE_SERVER;

    if (objMethod.Equals(SipMethod::INVITE))
    {
        nTxnType = SipTimerValuesHelper::INVITE_SERVER;
    }

    piSsc->SetTransactionTimerValues(
            SipTimerValuesHelper::GetValues(GetSlotId(), GetSipProfile(), nTxnType));

    // Sets the reason phrase if present
    if (!strPhrase.IsNULL())
    {
        piSsc->SetReasonPhrase(strPhrase);
    }

    // In case of CANCEL response, do not perform any more operations.
    if (objMethod.Equals(SipMethod::CANCEL))
    {
        Ims::SetLastError(ImsError::NO_ERROR);
        return IMS_TRUE;
    }

    ISipMessage* piSipMsg = piSsc->GetMessage();

    if ((nStatusCode > SipStatusCode::SC_100) && (nStatusCode < SipStatusCode::SC_300))
    {
        IMS_SINT32 nDialogState = ISipDialog::STATE_INIT;
        ISipDialog* piDialog = piSsc->GetDialog();

        if (piDialog != IMS_NULL)
        {
            nDialogState = piDialog->GetState();
        }

        // Sets Contact header fields:
        // INVITE, NOTIFY, OPTIONS, SUBSCRIBE, UPDATE, REGISTER, REFER, PUBLISH
        // OPTIONS case: it will be handled in Capabilities class.
        // In the other situation, J180 will set the Contact header when initializing the response.
        if ((nDialogState != ISipDialog::STATE_EARLY) &&
                (nDialogState != ISipDialog::STATE_CONFIRMED) &&
                (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::REFER) ||
                        objMethod.Equals(SipMethod::SUBSCRIBE) ||
                        objMethod.Equals(SipMethod::NOTIFY)))
        {
            AString strContact;
            IMS_BOOL bIsContactGruu = IMS_FALSE;

            FormContactHeader(objMethod, bPrivacy, IMS_FALSE, strContact, bIsContactGruu);

            if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
                Ims::SetLastError(ImsError::GENERAL_ERROR);
                return IMS_FALSE;
            }

            if (bIsContactGruu)
            {
                piSipMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
            }
        }
        else if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::REFER) ||
                objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::UPDATE) ||
                objMethod.Equals(SipMethod::NOTIFY))
        {
            SetGruuOptionTagInMidDialog(piDialog, piSipMsg);
        }

        // Sets Allow header fields
        const SipConfigV* pSipConfigV = GetSipConfigV();

        if (pSipConfigV != IMS_NULL)
        {
            const AStringArray& objMethods = pSipConfigV->GetAllowMethods();

            for (IMS_SINT32 j = 0; j < objMethods.GetCount(); ++j)
            {
                if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(j)) !=
                        IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                    Ims::SetLastError(ImsError::GENERAL_ERROR);
                    return IMS_FALSE;
                }
            }

            // Sets P-Preferred-Identity (except for REGISTER) header fields
            // This header will not be included in the response except for INVITE
            if (objMethod.Equals(SipMethod::INVITE))
            {
                if (!SetPPreferredIdentityHeader(pSipConfigV->GetPreferredId(), piSipMsg))
                {
                    IMS_TRACE_E(0, "Setting P-Preferred-Identity header failed", 0, 0, 0);
                    Ims::SetLastError(ImsError::GENERAL_ERROR);
                    return IMS_FALSE;
                }
            }
        }
    }

    // Sets P-Access-Network-Info header field
    if (!objMethod.Equals(SipMethod::CANCEL) && (nStatusCode > SipStatusCode::SC_100))
    {
        PAccessNetworkInfoHeader::SetHeader(GetSlotId(), GetIpAddress(), GetSipProfile(), piSipMsg);
    }

    // Sets Server header field - User-Agent ?
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetSipProfile()))
    {
        if (SipConfigProxy::IsUserAgentSetByContext(GetSlotId(), GetSipProfile()))
        {
            UserAgentHeader::SetHeader(ISipHeader::SERVER, GetSipProfile(), GetServiceId(),
                    GetIpAddress(), GetSlotId(), piSipMsg);
        }
        else
        {
            UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, GetSipProfile(), GetServiceId(),
                    GetIpAddress(), GetSlotId(), piSipMsg);
        }
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL Service::InitAck(IN ISipClientConnection* piScc)
{
    if (piScc->InitAck() != IMS_SUCCESS)
    {
        return IMS_FALSE;
    }

    // IPSEC {
    if (IsRegBindingOnActive())
    {
        const AStringArray& objSecurityVerifys = m_piRegBinding->GetSecurityVerifys();

        if (!objSecurityVerifys.IsEmpty())
        {
            // Sets the transport tuples
            piScc->SetTransportTuple(m_piRegBinding->GetIpAddress(), m_piRegBinding->GetPortUs(),
                    m_piRegBinding->GetPortUc(), m_piRegBinding->GetPortFlowControl(),
                    m_piRegBinding->GetTransportExt());
        }
    }
    // }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL Service::Equals(IN const Service* pService) const
{
    if (pService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (GetSlotId() != pService->GetSlotId())
    {
        return IMS_FALSE;
    }

    return m_strServiceId.Equals(pService->GetServiceId());
}

PUBLIC
IMS_UINT32 Service::EvaluateFilterCriteria(IN const ISipMessage* piSipMsg) const
{
    if (m_pFilterCriteria == IMS_NULL)
    {
        return 0;
    }

    if (m_pFilterCriteria->IsEmpty())
    {
        return 0;
    }

    IMS_TRACE_D("iFC :: Evaluating ... (%s)", GetServiceId().GetStr(), 0, 0);

    IMS_UINT32 nScore = m_pFilterCriteria->Evaluate(piSipMsg);

    if (nScore != 0)
    {
        IMS_TRACE_D("iFC :: Match OK (%s : %s : %d)", GetAppId().GetStr(), GetServiceId().GetStr(),
                nScore);
    }

    return nScore;
}

PUBLIC
const IpAddress& Service::GetIpAddress() const
{
    if (!IsImsConnected())
    {
        return IpAddress::NONE;
    }

    return m_piRegBinding->GetIpAddress();
}

PUBLIC
const AString& Service::GetAssociatedUri(IN IMS_SINT32 nScheme) const
{
    const AStringArray& objAssociatedUris = GetAssociatedUris();

    if (objAssociatedUris.IsEmpty())
    {
        return AString::ConstNull();
    }

    if (nScheme == Sip::URI_SCHEME_TEL)
    {
        for (IMS_UINT32 i = 0; i < m_objAuthorizedUserIds.GetSize(); ++i)
        {
            ISipHeader* piHeader = m_objAuthorizedUserIds.GetAt(i);

            if (piHeader == IMS_NULL)
            {
                continue;
            }

            const SipAddress* pAddress = piHeader->GetSipAddress();

            if (pAddress == IMS_NULL)
            {
                continue;
            }

            if (pAddress->IsSchemeTel())
            {
                if (static_cast<IMS_SINT32>(i) < objAssociatedUris.GetCount())
                {
                    return objAssociatedUris.GetElementAt(static_cast<IMS_SINT32>(i));
                }
            }
        }
    }
    else if (nScheme == Sip::URI_SCHEME_SIP)
    {
        for (IMS_UINT32 i = 0; i < m_objAuthorizedUserIds.GetSize(); ++i)
        {
            ISipHeader* piHeader = m_objAuthorizedUserIds.GetAt(i);

            if (piHeader == IMS_NULL)
            {
                continue;
            }

            const SipAddress* pAddress = piHeader->GetSipAddress();

            if (pAddress == IMS_NULL)
            {
                continue;
            }

            if (pAddress->IsSchemeSip())
            {
                if (static_cast<IMS_SINT32>(i) < objAssociatedUris.GetCount())
                {
                    return objAssociatedUris.GetElementAt(static_cast<IMS_SINT32>(i));
                }
            }
        }
    }

    return AString::ConstNull();
}

PUBLIC
const AStringArray& Service::GetAssociatedUris() const
{
    if (!IsImsConnected())
    {
        return AStringArray::ConstNull();
    }

    return m_piRegBinding->GetAssociatedUris();
}

PUBLIC
const SipAddress& Service::GetAuthorizedUserId() const
{
    if (IsUserIdProvisioned())
    {
        return GetDefaultUserId();
    }

    if (!IsImsConnected())
    {
        return GetDefaultUserId();
    }

    return m_piRegBinding->GetAuthorizedAor();
}

PUBLIC
const SipAddress& Service::GetContactAddress() const
{
    if (!IsImsConnected())
    {
        return SipAddress::ConstNull();
    }

    return m_piRegBinding->GetContactAddress();
}

PUBLIC
const SipAddress* Service::GetContactAddressForOutgoingMessage() const
{
    if (!IsImsConnected())
    {
        return IMS_NULL;
    }

    return m_piRegBinding->GetContactAddressForOutgoingMessage();
}

PUBLIC
ISipHeader* Service::GetContactHeader(IN IMS_BOOL bPrivacy /*= IMS_FALSE*/,
        IN IMS_BOOL bRequest /*= IMS_TRUE*/,
        IN IMS_SINT32 nSipMethod /*= SipMethod::INVALID*/) const
{
    if (!IsImsConnected())
    {
        return IMS_NULL;
    }

    AString strContact;
    SipMethod objMethod;

    switch (nSipMethod)
    {
        case SipMethod::INVITE:     // FALL-THROUGH
        case SipMethod::SUBSCRIBE:  // FALL-THROUGH
        case SipMethod::REFER:      // FALL-THROUGH
        case SipMethod::NOTIFY:     // FALL-THROUGH
        case SipMethod::OPTIONS:    // FALL-THROUGH
        case SipMethod::PUBLISH:
            objMethod = nSipMethod;
            break;
        default:
            objMethod = static_cast<IMS_SINT32>(SipMethod::INVALID);
            break;
    }

    IMS_BOOL bIsContactGruu = IMS_FALSE;

    FormContactHeader(objMethod, bPrivacy, bRequest, strContact, bIsContactGruu);

    (void)bIsContactGruu;

    return SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, strContact);
}

PUBLIC
const SipParameter* Service::GetInstanceParameter() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_piRegBinding->GetInstanceParameter();
}

PUBLIC
const AStringArray& Service::GetPathHeaders() const
{
    if (!IsImsConnected())
    {
        return AStringArray::ConstNull();
    }

    return m_piRegBinding->GetPathHeaders();
}

PUBLIC
const IRegInfo* Service::GetRegInfo() const
{
    if (!IsImsConnected())
    {
        return IMS_NULL;
    }

    return m_piRegBinding->GetRegInfo();
}

PUBLIC
const AStringArray& Service::GetServiceRoutes() const
{
    if (!IsImsConnected())
    {
        return AStringArray::ConstNull();
    }

    return m_piRegBinding->GetServiceRoutes();
}

PROTECTED
const ISipConfigV* Service::GetISipConfigV() const
{
    return SipConfigProxy::GetSipConfigV(GetSlotId());
}

PUBLIC
SipProfile* Service::GetSipProfile() const
{
    if (m_pSipProfile.IsNull())
    {
        return (m_piRegBinding != IMS_NULL) ? m_piRegBinding->GetSipProfile() : IMS_NULL;
    }

    return m_pSipProfile.Get();
}

PUBLIC
const AString& Service::GetSubscriberId() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return m_piRegBinding->GetSubscriberId();
}

PUBLIC
const SipAddress* Service::GetPublicGruu() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_piRegBinding->GetPublicGruu();
}

PUBLIC
const SipAddress* Service::GetTemporaryGruu() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_piRegBinding->GetTemporaryGruu();
}

PUBLIC
const ImsList<SipAddress*>& Service::GetTemporaryGruus() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return SipAddress::ConstEmptyList();
    }

    return m_piRegBinding->GetTemporaryGruus();
}

PUBLIC
IMS_BOOL Service::IsBehindNat() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_piRegBinding->IsBehindNat();
}

PUBLIC
IMS_BOOL Service::IsEventPackageSupported(IN const AString& strEvent) const
{
    if (m_pAppConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pAppConfig->IsEventPackageSupported(strEvent);
}

PUBLIC
IMS_BOOL Service::IsWithinTrustDomain() const
{
    if (m_piRegBinding == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_piRegBinding->IsWithinTrustDomain();
}

PUBLIC
IMS_BOOL Service::UpdateFeatureTags(IN const ImsList<AString>& objFeatureTags,
        IN IMS_BOOL bRegRequired, IN IMS_SINT32 nOperation)
{
    if (objFeatureTags.IsEmpty())
    {
        return IMS_FALSE;
    }

    AString strName;
    AString strValue;
    AString strTemp;
    FeatureSet* pFeatureSet;
    IMS_BOOL bCallerCapabilityChanged = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objFeatureTags.GetSize(); ++i)
    {
        strTemp = objFeatureTags.GetAt(i).MakeLower();
        IMS_SINT32 nCount = strTemp.SplitF(TextParser::CHAR_EQUAL, strName, strValue);

        if (strName.GetLength() == 0)
        {
            continue;
        }

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
        if (nOperation == FEATURE_TAG_OP_ADD)
        {
            if (m_pCallerCapability->AddFeature(pFeatureSet))
            {
                bCallerCapabilityChanged = IMS_TRUE;
            }
        }
        // REMOVE
        else if (nOperation == FEATURE_TAG_OP_REMOVE)
        {
            if (m_pCallerCapability->RemoveFeature(pFeatureSet))
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
        IMS_TRACE_D("Service(%s) :: caller capability is updated (%s)", GetServiceId().GetStr(),
                (nOperation == FEATURE_TAG_OP_ADD) ? "ADD" : "REMOVE", 0);

        m_bCallerCapabilityChanged = IMS_TRUE;
    }

    if ((m_piRegBinding != IMS_NULL) && m_bCallerCapabilityChanged && bRegRequired)
    {
        m_bCallerCapabilityChanged = IMS_FALSE;
        m_piRegBinding->NotifyCallerCapabilityChanged();
    }

    return bCallerCapabilityChanged;
}

PUBLIC
void Service::NotifyError(IN IMS_SINT32 nErrorCode)
{
    // The subclass MUST implement this method to handle a loss of network,
    // no communication with proxy.
    const ImsList<Method*>& objMethods = m_pMethodMngr->GetMethods();

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); ++i)
    {
        Method* pMethod = objMethods.GetAt(i);

        if (pMethod == IMS_NULL)
        {
            continue;
        }

        pMethod->Exception_NotifyError(nErrorCode);
    }

    Exception_NotifyError(nErrorCode);
}

PUBLIC
IMS_BOOL Service::SendResponse(IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
        IN const AString& strPhrase /*= AString::ConstNull()*/)
{
    IMS_TRACE_I("___ Sending %d response to %s ...", nStatusCode,
            piSsc->GetMethod().ToString().GetStr(), 0);

    if (!CreateResponse(piSsc, nStatusCode, strPhrase))
    {
        return IMS_FALSE;
    }

    if (piSsc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending SIP response failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void Service::RegisterMethod(IN Method* pMethod)
{
    m_pMethodMngr->AddMethod(pMethod);
}

PUBLIC
void Service::DeregisterMethod(IN Method* pMethod)
{
    m_pMethodMngr->RemoveMethod(pMethod);
}

PUBLIC
IMS_BOOL Service::ValidateMethod(IN const SipMethod& objMethod) const
{
    // Handle a specific behavior for an incoming INVITE request
    if (objMethod.Equals(SipMethod::INVITE))
    {
        // If the service does not support any media, it can't receive any incoming INVITE request
        if (!(m_pAppConfig->IsStreamMediaAudioSupported() ||
                    m_pAppConfig->IsStreamMediaVideoSupported() ||
                    m_pAppConfig->IsFramedMediaSupported() ||
                    m_pAppConfig->IsBasicMediaSupported()))
        {
            return IMS_FALSE;
        }
    }

    const SipConfigV* pSipConfigV = GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray& objMethods = pSipConfigV->GetAllowMethods();

        return objMethods.Contains(objMethod.ToString());
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL Service::ValidateRequestUri(IN const SipAddress& objRequestUri,
        IN ISipDialog* piDialog /*= IMS_NULL*/, IN IMS_BOOL bIsMidDialogRequest /*= IMS_FALSE*/)
{
    if (bIsMidDialogRequest)
    {
        // Checks GRUU identities if it is supported
        if (SipConfigProxy::IsGruuConfigured(GetSlotId(), GetSipProfile()))
        {
            const AString strGr("gr");
            const SipParameter* pParamGr = objRequestUri.GetParameter(strGr);

            if (pParamGr != IMS_NULL)
            {
                if (pParamGr->GetValue().GetLength() > 0)
                {
                    // If the pub-gruu is available, then compare it with the Request-URI
                    const SipAddress* pPubGruu = IsRegBindingOnActive()
                            ? GetPublicGruu()
                            : m_objCachedRegBinding.GetPublicGruu();

                    if (pPubGruu != IMS_NULL)
                    {
                        if (pPubGruu->Equals(objRequestUri))
                        {
                            IMS_TRACE_D("pub-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }
                else
                {
                    // 4 List ?
                    //  If the temp-gruu is available, then compare it with the Request-URI
                    const SipAddress* pTempGruu = IsRegBindingOnActive()
                            ? GetTemporaryGruu()
                            : m_objCachedRegBinding.GetTemporaryGruu();

                    if (pTempGruu != IMS_NULL)
                    {
                        if (pTempGruu->Equals(objRequestUri))
                        {
                            IMS_TRACE_D("temp-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }

                IMS_TRACE_D("Request-URI contains a 'gr' parameter & GRUU ids are not matched", 0,
                        0, 0);

                return IMS_FALSE;
            }
        }

        // Checks if the contact address matches or not.
        const SipAddress* pContact = IMS_NULL;

        if (piDialog != IMS_NULL)
        {
            const ISipHeader* piContactHeader = piDialog->GetContactHeader();

            if (piContactHeader != IMS_NULL)
            {
                pContact = piContactHeader->GetSipAddress();
            }
        }

        if (pContact == IMS_NULL)
        {
            IMS_TRACE_D("No contacts in the dialog; use the default contact address...", 0, 0, 0);

            if (IsRegBindingOnActive())
            {
                pContact = GetContactAddressForOutgoingMessage();
            }
            else
            {
                pContact = m_objCachedRegBinding.GetContactAddressForOutgoingMessage();
            }
        }

        if ((pContact != IMS_NULL) && pContact->Equals(objRequestUri))
        {
            return IMS_TRUE;
        }

        const SipAddress& objContact = IsRegBindingOnActive()
                ? GetContactAddress()
                : m_objCachedRegBinding.GetContactAddress();

        if (objContact.Equals(objRequestUri))
        {
            return IMS_TRUE;
        }

        // Checks if the contact address for outgoing message matches or not.
        pContact = IsRegBindingOnActive()
                ? GetContactAddressForOutgoingMessage()
                : m_objCachedRegBinding.GetContactAddressForOutgoingMessage();

        if ((pContact != IMS_NULL) && pContact->Equals(objRequestUri))
        {
            IMS_TRACE_D("Request-URI matches the contact address for outgoing message", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    else
    {
        // Checks GRUU identities if it is supported
        if (SipConfigProxy::IsGruuConfigured(GetSlotId(), GetSipProfile()))
        {
            const AString strGr("gr");
            const SipParameter* pParamGr = objRequestUri.GetParameter(strGr);

            if (pParamGr != IMS_NULL)
            {
                if (pParamGr->GetValue().GetLength() > 0)
                {
                    // If the pub-gruu is available, then compare it with the Request-URI
                    const SipAddress* pPubGruu = GetPublicGruu();

                    if (pPubGruu != IMS_NULL)
                    {
                        if (pPubGruu->Equals(objRequestUri))
                        {
                            IMS_TRACE_D("pub-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }
                else
                {
                    // 4 List ?
                    //  If the temp-gruu is available, then compare it with the Request-URI
                    const SipAddress* pTempGruu = GetTemporaryGruu();

                    if (pTempGruu != IMS_NULL)
                    {
                        if (pTempGruu->Equals(objRequestUri))
                        {
                            IMS_TRACE_D("temp-gruu is matched", 0, 0, 0);
                            return IMS_TRUE;
                        }
                    }
                }

                IMS_TRACE_D("Request-URI contains a 'gr' parameter & GRUU ids are not matched", 0,
                        0, 0);

                return IMS_FALSE;
            }
        }

        // Checks if the contact address matches or not.
        const SipAddress& objContact = GetContactAddress();

        if (objContact.Equals(objRequestUri))
        {
            return IMS_TRUE;
        }

        // Checks if the contact address for outgoing message matches or not.
        const SipAddress* pContact = GetContactAddressForOutgoingMessage();

        if ((pContact != IMS_NULL) && pContact->Equals(objRequestUri))
        {
            IMS_TRACE_D("Request-URI matches the contact address for outgoing message", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    // Checks if the public user identity matches or not
    for (IMS_UINT32 i = 0; i < m_objAuthorizedUserIds.GetSize(); ++i)
    {
        const ISipHeader* piHeader = m_objAuthorizedUserIds.GetAt(i);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        const SipAddress* pAddress = piHeader->GetSipAddress();

        if (pAddress == IMS_NULL)
        {
            continue;
        }

        if (pAddress->Equals(objRequestUri))
        {
            // The public user identity is matched, so this message can be routed to this service.
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL Service::ValidateRequestUriForIpAndPort(IN const SipAddress& objRequestUri,
        IN ISipDialog* piDialog /*= IMS_NULL*/, IN IMS_BOOL bIsMidDialogRequest /*= IMS_FALSE*/)
{
    IpAddress objIpForRUri(objRequestUri.GetHost());

    if (!objIpForRUri.IsIPv4Address() && !objIpForRUri.IsIPv6Address())
    {
        IMS_TRACE_D("R-URI is not a format of IP address", 0, 0, 0);
        return IMS_FALSE;
    }

    if (bIsMidDialogRequest)
    {
        // Checks if the contact address matches or not.
        const SipAddress* pContact = IMS_NULL;

        if (piDialog != IMS_NULL)
        {
            const ISipHeader* piContactHeader = piDialog->GetContactHeader();

            if (piContactHeader != IMS_NULL)
            {
                pContact = piContactHeader->GetSipAddress();
            }
        }

        if (pContact == IMS_NULL)
        {
            IMS_TRACE_D("Contact header is not present in the dialog; "
                        "use the default contact address...",
                    0, 0, 0);

            if (IsRegBindingOnActive())
            {
                pContact = GetContactAddressForOutgoingMessage();
            }
            else
            {
                pContact = m_objCachedRegBinding.GetContactAddressForOutgoingMessage();
            }
        }

        if (pContact != IMS_NULL)
        {
            IpAddress objIpForContact(pContact->GetHost());

            if (objIpForRUri.Equals(objIpForContact) &&
                    (objRequestUri.GetPort() == pContact->GetPort()))
            {
                IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
                return IMS_TRUE;
            }
        }

        const SipAddress& objContact = IsRegBindingOnActive()
                ? GetContactAddress()
                : m_objCachedRegBinding.GetContactAddress();

        IpAddress objIpForContact(objContact.GetHost());

        if (objIpForRUri.Equals(objIpForContact) &&
                (objRequestUri.GetPort() == objContact.GetPort()))
        {
            IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    else
    {
        // Checks if the contact address matches or not.
        const SipAddress& objContact = GetContactAddress();

        IpAddress objIpForContact(objContact.GetHost());

        if (objIpForRUri.Equals(objIpForContact) &&
                (objRequestUri.GetPort() == objContact.GetPort()))
        {
            IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
            return IMS_TRUE;
        }

        // Checks if the contact address for outgoing message matches or not.
        const SipAddress* pContact = GetContactAddressForOutgoingMessage();

        if (pContact != IMS_NULL)
        {
            IpAddress objIpForContact2(pContact->GetHost());

            if (objIpForRUri.Equals(objIpForContact2) &&
                    (objRequestUri.GetPort() == pContact->GetPort()))
            {
                IMS_TRACE_D("R-URI matches for IP/port", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL Service::ValidateFromAndTo(
        IN const AString& strFrom, IN const AString& strTo, IN IMS_BOOL bToLenient)
{
    SipAddress objAddress;

    // Validates From field
    if (!strFrom.IsNULL())
    {
        if (!objAddress.Create(strFrom))
        {
            IMS_TRACE_E(
                    0, "ILLEGAL ARGUMENT - From (%s)", SipDebug::GetUri1(strFrom).GetStr(), 0, 0);
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
        IMS_TRACE_E(0, "ILLEGAL ARGUMENT - To (%s)", SipDebug::GetUri1(strTo).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/**
 * @brief Checks the arguments of Refer-To header.
 */
PUBLIC GLOBAL IMS_BOOL Service::ValidateReferTo(
        IN const AString& strUri, IN const AString& strMethod)
{
    // Checks the body of Refer-To header
    SipAddress objUri;

    if (!objUri.Create(strUri))
    {
        IMS_TRACE_E(
                0, "ILLEGAL ARGUMENT - Refer-To (%s)", SipDebug::GetUri1(strUri).GetStr(), 0, 0);
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

PROTECTED VIRTUAL void Service::Close()
{
    // Destroy a service-specific configurations .....
    SipConfig* pSipConfig =
            const_cast<SipConfig*>(ConfigurationManager::GetInstance()->GetSipConfig(GetSlotId()));

    if (pSipConfig != IMS_NULL)
    {
        // Remove the configuration update listener
        const SipConfigV* pSipConfigV = pSipConfig->GetServiceConfig();

        if (pSipConfigV != IMS_NULL)
        {
            IConfigurable* piConfigurable =
                    static_cast<const ISipConfigV*>(pSipConfigV)->GetConfigurable();

            if (piConfigurable != IMS_NULL)
            {
                piConfigurable->RemoveListener(IConfigurable::CP_I_FEATURE_TAG_OPTIONS, this);
                piConfigurable->RemoveListener(IConfigurable::CP_I_SIP_ALL, this);
            }
        }
    }

    if (m_piServiceCloseListener != IMS_NULL)
    {
        m_piServiceCloseListener->ServiceClosed(this);
    }

    if (m_piRegBinding != IMS_NULL)
    {
        m_piRegBinding->SetListener(IMS_NULL);
    }

    PostMessage(AMSG_DESTROY, 0, 0);
}

PROTECTED VIRTUAL IMS_BOOL Service::DispatchMessage(IN ImsMessage& objMsg)
{
    return EngineActivity::DispatchMessage(objMsg);
}

PROTECTED VIRTUAL void Service::ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCpi,
        IN const AString& strConfName /*= AString::ConstNull()*/,
        IN const AString& strExtraParam /*= AString::ConstNull()*/)
{
    (void)strConfName;
    (void)strExtraParam;

    if ((nCpi == IConfigurable::CP_I_FEATURE_TAG_OPTIONS) || (nCpi == IConfigurable::CP_I_SIP_ALL))
    {
        IMS_UINT32 nTmpFts = m_nFeatureTags;
        const ISipConfigV* piSipConfigV = GetSipConfigV();

        if (piSipConfigV != IMS_NULL)
        {
            nTmpFts = piSipConfigV->GetFeatureTagOptions();
        }

        if (nTmpFts != m_nFeatureTags)
        {
            IMS_TRACE_D("SIP feature-tags is updated :: %08X >> %08X", m_nFeatureTags, nTmpFts, 0);

            m_nFeatureTags = nTmpFts;

            UpdateCallerCapabilityNPreference();
        }
    }
}

PROTECTED VIRTUAL void Service::RegBinding_OnActive()
{
    IMS_TRACE_D("Service (%s) :: OnActive()", m_strServiceId.GetStr(), 0, 0);

    SetImsConnected(IMS_TRUE);

    UpdateAuthorizedUserIds();
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    UpdateRegBindings();

    // CONTACT_FEATURE_CAPS
    m_pFeatureCaps->UpdateRegCaps(m_pCallerCapability);
}

PROTECTED VIRTUAL void Service::RegBinding_OnDestroy()
{
    IMS_TRACE_D("Service (%s) :: OnDestroy()", m_strServiceId.GetStr(), 0, 0);

    if (IsImsConnected())
    {
        NotifyError(ImsError::SERVICE_CLOSED);
    }

    SetImsConnected(IMS_FALSE);
    m_piRegBinding = IMS_NULL;
}

PROTECTED VIRTUAL void Service::RegBinding_OnInit(IN const SipAddress* pAor)
{
    IMS_TRACE_D("Service (%s) :: OnInit()", m_strServiceId.GetStr(), 0, 0);

    if (pAor != IMS_NULL)
    {
        m_objImpu = *pAor;
    }
    else
    {
        CreateDefaultPublicUserId();
    }
}

PROTECTED VIRTUAL void Service::RegBinding_OnQueryCapability(OUT CallerCapability*& pCapability)
{
    IMS_TRACE_D("Service (%s) :: OnQueryCapability()", m_strServiceId.GetStr(), 0, 0);

    pCapability = m_pCallerCapability;
}

PROTECTED VIRTUAL void Service::RegBinding_OnQueryRegistrationHeaders(OUT AStringArray& objHeaders)
{
    IMS_TRACE_D("Service (%s) :: OnQueryRegistrationHeaders()", m_strServiceId.GetStr(), 0, 0);

    if (m_pAppConfig == IMS_NULL)
    {
        return;
    }

    const CoreServiceConfig* pServiceConfig = m_pAppConfig->GetCoreServiceConfigEx(GetServiceId());

    if (pServiceConfig == IMS_NULL)
    {
        return;
    }

    objHeaders = pServiceConfig->GetRegistrationHeaders();
}

PROTECTED VIRTUAL void Service::RegBinding_OnTerminated()
{
    IMS_TRACE_D("Service (%s) :: OnTerminated()", m_strServiceId.GetStr(), 0, 0);

    if (IsImsConnected())
    {
        NotifyError(ImsError::SERVICE_CLOSED);
    }

    SetImsConnected(IMS_FALSE);
}

PROTECTED
void Service::FormContactHeader(IN const SipMethod& objMethod, IN IMS_BOOL bPrivacy,
        IN IMS_BOOL bRequest, OUT AString& strContact, OUT IMS_BOOL& bIsContactGruu) const
{
    IMS_BOOL bRegBindingOnActive = IsRegBindingOnActive();

    bIsContactGruu = IMS_FALSE;

    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), GetSipProfile()))
    {
        if (bRequest && SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), GetSipProfile()))
        {
            SipAddress objContact;

            if (!bPrivacy)
            {
                // If the pub-gruu is available, then add it to the Contact header
                const SipAddress* pPubGruu = GetPublicGruu();

                if (pPubGruu != IMS_NULL)
                {
                    bIsContactGruu = IMS_TRUE;
                    objContact = *pPubGruu;
                }
                else
                {
                    const SipAddress* pContact = GetContactAddressForOutgoingMessage();

                    objContact = (pContact != IMS_NULL) ? *pContact : GetContactAddress();
                }
            }
            else
            {
                // If the temp-gruu is available, then add it to the Contact header
                const SipAddress* pTempGruu = GetTemporaryGruu();

                if (pTempGruu != IMS_NULL)
                {
                    bIsContactGruu = IMS_TRUE;
                    objContact = *pTempGruu;
                }
                else
                {
                    const SipAddress* pContact = GetContactAddressForOutgoingMessage();

                    objContact = (pContact != IMS_NULL) ? *pContact : GetContactAddress();
                }
            }

            objContact.AddParameter(Sip::STR_OB, AString::ConstNull());

            strContact = objContact.ToString();
        }
        else
        {
            if (!bPrivacy)
            {
                // If the pub-gruu is available, then add it to the Contact header
                const SipAddress* pPubGruu = bRegBindingOnActive
                        ? GetPublicGruu()
                        : m_objCachedRegBinding.GetPublicGruu();

                if (pPubGruu != IMS_NULL)
                {
                    bIsContactGruu = IMS_TRUE;
                    strContact = pPubGruu->ToString();
                }
            }
            else
            {
                // If the temp-gruu is available, then add it to the Contact header
                const SipAddress* pTempGruu = bRegBindingOnActive
                        ? GetTemporaryGruu()
                        : m_objCachedRegBinding.GetTemporaryGruu();

                if (pTempGruu != IMS_NULL)
                {
                    bIsContactGruu = IMS_TRUE;
                    strContact = pTempGruu->ToString();
                }
            }

            if (!bIsContactGruu)
            {
                const SipAddress* pContact = bRegBindingOnActive
                        ? GetContactAddressForOutgoingMessage()
                        : m_objCachedRegBinding.GetContactAddressForOutgoingMessage();

                if (pContact != IMS_NULL)
                {
                    strContact = pContact->ToString();
                }
                else
                {
                    strContact = bRegBindingOnActive
                            ? GetContactAddress().ToString()
                            : m_objCachedRegBinding.GetContactAddress().ToString();
                }
            }
        }
    }
    else
    {
        if (bRequest && SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), GetSipProfile()))
        {
            const SipAddress* pContact = GetContactAddressForOutgoingMessage();
            SipAddress objContact = (pContact != IMS_NULL) ? *pContact : GetContactAddress();

            objContact.AddParameter(Sip::STR_OB, AString::ConstNull());

            strContact = objContact.ToString();
        }
        else
        {
            const SipAddress* pContact = bRegBindingOnActive
                    ? GetContactAddressForOutgoingMessage()
                    : m_objCachedRegBinding.GetContactAddressForOutgoingMessage();

            if (pContact != IMS_NULL)
            {
                strContact = pContact->ToString();
            }
            else
            {
                strContact = bRegBindingOnActive
                        ? GetContactAddress().ToString()
                        : m_objCachedRegBinding.GetContactAddress().ToString();
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

    if (!objMethod.Equals(SipMethod::INVALID) &&
            m_pFeatureCaps->FormContactFeatures(objMethod.ToInt(), bRequest, strContactFeatures))
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(strContactFeatures);
    }
    else if (!m_pCallerCapability->IsEmpty())
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(m_pCallerCapability->ToString());
    }

    IMS_BOOL bDeviceIdRequired = IMS_FALSE;

    // "+sip.instance" parameter can be inserted
    // if UE detects that the destination of requests & responses is a trusted intermediary.
    if (IsWithinTrustDomain())
    {
        bDeviceIdRequired = IMS_TRUE;
    }

    // CONTACT_FEATURE_CAPS
    if (bDeviceIdRequired && strContact.Contains(Sip::STR_SIP_INSTANCE))
    {
        IMS_TRACE_D("FeatureCaps :: Device id is directly formed by the application; ignore it", 0,
                0, 0);
        bDeviceIdRequired = IMS_FALSE;
    }

    if (bDeviceIdRequired)
    {
        // Add the '+sip.instance' parameter
        const SipParameter* pParameter = GetInstanceParameter();

        if (pParameter != IMS_NULL)
        {
            strContact.Append(TextParser::CHAR_SEMICOLON);
            strContact.Append(pParameter->ToString());
        }
    }
}

PROTECTED
void Service::SetGruuOptionTagInMidDialog(IN ISipDialog* piDialog, IN_OUT ISipMessage*& piSipMsg)
{
    const ISipHeader* piContactHeader =
            (piDialog != IMS_NULL) ? piDialog->GetContactHeader() : IMS_NULL;
    const SipAddress* pContact =
            (piContactHeader != IMS_NULL) ? piContactHeader->GetSipAddress() : IMS_NULL;

    if (pContact != IMS_NULL)
    {
        if (pContact->GetParameter("gr") != IMS_NULL)
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
        }
    }
}

PRIVATE
void Service::CreateDefaultPublicUserId()
{
    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    // Read a default public user identity
    if (!m_bProvisionedUserId)
    {
        const SubscriberConfig* pSubsConfig =
                pConfigMngr->GetSubscriberConfig(SubscriberConfig::GetDefaultId(), GetSlotId());

        if (pSubsConfig == IMS_NULL)
        {
            return;
        }

        if (!m_objImpu.Create(pSubsConfig->GetPublicUserId()))
        {
            IMS_TRACE_E(0, "Creating a default public user identity of the device failed", 0, 0, 0);
        }
    }
}

PRIVATE
IMS_BOOL Service::SetPPreferredIdentityHeader(
        IN IMS_SINT32 nPreferredId, IN_OUT ISipMessage*& piSipMsg) const
{
    const AStringArray& objAssociatedUris = IsRegBindingOnActive()
            ? GetAssociatedUris()
            : m_objCachedRegBinding.GetAssociatedUris();

    if (objAssociatedUris.IsEmpty())
    {
        return IMS_TRUE;
    }

    if (nPreferredId == SipConfigV::PREFERRED_ID_DEFAULT)
    {
        if (piSipMsg->AddHeader(ISipHeader::P_PREFERRED_IDENTITY,
                    objAssociatedUris.GetElementAt(0)) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }
    else if (nPreferredId == SipConfigV::PREFERRED_ID_ALL)
    {
        for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount(); ++i)
        {
            if (piSipMsg->AddHeader(ISipHeader::P_PREFERRED_IDENTITY,
                        objAssociatedUris.GetElementAt(i)) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    IMS_BOOL bTopmostSip = IMS_TRUE;
    IMS_BOOL bTelUriFound = IMS_FALSE;
    IMS_SINT32 nCount = 1;

    if ((nPreferredId == SipConfigV::PREFERRED_ID_TEL) ||
            (nPreferredId == SipConfigV::PREFERRED_ID_TEL_SIP))
    {
        bTopmostSip = IMS_FALSE;
    }

    if ((nPreferredId == SipConfigV::PREFERRED_ID_SIP_TEL) ||
            (nPreferredId == SipConfigV::PREFERRED_ID_TEL_SIP))
    {
        nCount += 1;
    }

    for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount(); ++i)
    {
        const AString& strId = objAssociatedUris.GetElementAt(i);

        if ((bTopmostSip && (strId.Contains("sip:") || strId.Contains("sips:"))) ||
                (!bTopmostSip && strId.Contains("tel:")))
        {
            if (piSipMsg->AddHeader(ISipHeader::P_PREFERRED_IDENTITY, strId) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                return IMS_FALSE;
            }

            if (!bTopmostSip)
            {
                bTelUriFound = IMS_TRUE;
            }
            break;
        }
    }

    if (nCount > 1)
    {
        for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount(); ++i)
        {
            const AString& strId = objAssociatedUris.GetElementAt(i);

            if ((bTopmostSip && strId.Contains("tel:")) ||
                    (!bTopmostSip && (strId.Contains("sip:") || strId.Contains("sips:"))))
            {
                if (piSipMsg->AddHeader(ISipHeader::P_PREFERRED_IDENTITY, strId) != IMS_SUCCESS)
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
        if (!bTopmostSip && !bTelUriFound)
        {
            for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount(); ++i)
            {
                const AString& strId = objAssociatedUris.GetElementAt(i);

                if (strId.Contains("sip:") || strId.Contains("sips:"))
                {
                    if (piSipMsg->AddHeader(ISipHeader::P_PREFERRED_IDENTITY, strId) != IMS_SUCCESS)
                    {
                        IMS_TRACE_E(0, "Adding P-Preferred-Identity failed", 0, 0, 0);
                        return IMS_FALSE;
                    }
                    break;
                }
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
void Service::SetRegBinding(IN IRegBinding* piRegBinding)
{
    if (m_piRegBinding != IMS_NULL)
    {
        m_piRegBinding->SetListener(IMS_NULL);
    }

    m_piRegBinding = piRegBinding;

    if (m_piRegBinding != IMS_NULL)
    {
        m_piRegBinding->SetListener(this);
    }
}

PRIVATE
void Service::UpdateAuthorizedUserIds()
{
    // Clear the previous public user identities
    for (IMS_UINT32 i = 0; i < m_objAuthorizedUserIds.GetSize(); ++i)
    {
        ISipHeader* piHeader = m_objAuthorizedUserIds.GetAt(i);

        if (piHeader != IMS_NULL)
        {
            piHeader->Destroy();
        }
    }

    m_objAuthorizedUserIds.Clear();

    const AStringArray& objAssociatedUris = GetAssociatedUris();

    for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount(); ++i)
    {
        ISipHeader* piHeader = SipParsingHelper::CreateHeader(
                ISipHeader::P_ASSOCIATED_URI, objAssociatedUris.GetElementAt(i));

        if (piHeader != IMS_NULL)
        {
            m_objAuthorizedUserIds.Append(piHeader);
        }
    }

    IMS_TRACE_D("Service :: AssociatedURIs (%d), AuthorizedUserIds (%d)",
            objAssociatedUris.GetCount(), m_objAuthorizedUserIds.GetSize(), 0);
}

PRIVATE
void Service::UpdateCallerCapabilityNPreference()
{
    const CoreServiceConfig* pServiceConfig = m_pAppConfig->GetCoreServiceConfigEx(m_strServiceId);

    m_pCallerCapability->Clear();

    // Updates the caller capabilities
    if (!m_pCallerCapability->Create(m_pAppConfig, pServiceConfig, GetSipConfigV()))
    {
        IMS_TRACE_E(0, "Creating the features from AppConfig & ServiceConfig failed", 0, 0, 0);
        return;
    }

    // Removes & updates the caller preference
    for (IMS_UINT32 i = 0; i < m_objAcceptContacts.GetSize(); ++i)
    {
        PreferenceHeader* pHeader = m_objAcceptContacts.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            delete pHeader;
        }
    }

    m_objAcceptContacts.Clear();

    if (!CallerPreference::CreateAcceptContactHeaders(
                m_pAppConfig, pServiceConfig, GetSipConfigV(), m_objAcceptContacts))
    {
        IMS_TRACE_E(0, "Creating Accept-Contact header failed", 0, 0, 0);
        return;
    }
}

PRIVATE
void Service::UpdateRegBindings()
{
    if (m_piRegBinding == IMS_NULL)
    {
        m_objCachedRegBinding.SetPortUc(Sip::PORT_UNSPECIFIED);
        m_objCachedRegBinding.SetPortUs(Sip::PORT_UNSPECIFIED);
        // RFC5626_FLOW_CONTROL
        m_objCachedRegBinding.SetPortFlowControl(Sip::PORT_UNSPECIFIED);
        // MULTI_REG_TRANSPORT
        m_objCachedRegBinding.SetTransportExt(Sip::TRANSPORT_EXT_ANY);
        m_objCachedRegBinding.SetIpAddress(IpAddress::NONE);
        m_objCachedRegBinding.SetContactAddress(SipAddress::ConstNull());
        m_objCachedRegBinding.SetContactAddressForOutgoingMessage(IMS_NULL);
        // IPSEC {
        m_objCachedRegBinding.SetSecurityClients(AStringArray::ConstNull());
        m_objCachedRegBinding.SetSecurityVerifys(AStringArray::ConstNull());
        // }
        // GRUU {
        m_objCachedRegBinding.SetPublicGruu(IMS_NULL);
        m_objCachedRegBinding.SetTemporaryGruu(IMS_NULL);
        // }
        m_objCachedRegBinding.SetAssociatedUris(AStringArray::ConstNull());
    }
    else
    {
        m_objCachedRegBinding.SetPortUc(m_piRegBinding->GetPortUc());
        m_objCachedRegBinding.SetPortUs(m_piRegBinding->GetPortUs());
        // RFC5626_FLOW_CONTROL
        m_objCachedRegBinding.SetPortFlowControl(m_piRegBinding->GetPortFlowControl());
        // MULTI_REG_TRANSPORT
        m_objCachedRegBinding.SetTransportExt(m_piRegBinding->GetTransportExt());
        m_objCachedRegBinding.SetIpAddress(m_piRegBinding->GetIpAddress());
        m_objCachedRegBinding.SetContactAddress(m_piRegBinding->GetContactAddress());
        m_objCachedRegBinding.SetContactAddressForOutgoingMessage(
                m_piRegBinding->GetContactAddressForOutgoingMessage());
        // IPSEC {
        m_objCachedRegBinding.SetSecurityClients(m_piRegBinding->GetSecurityClients());
        m_objCachedRegBinding.SetSecurityVerifys(m_piRegBinding->GetSecurityVerifys());
        // }
        // GRUU {
        m_objCachedRegBinding.SetPublicGruu(m_piRegBinding->GetPublicGruu());
        m_objCachedRegBinding.SetTemporaryGruu(m_piRegBinding->GetTemporaryGruu());
        // }
        m_objCachedRegBinding.SetAssociatedUris(m_piRegBinding->GetAssociatedUris());
    }
}
