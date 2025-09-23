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

#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"

#include "ISipClientConnection.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "RegParameter.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/SubscriberTracker.h"
#include "util/UserAgentHeader.h"

__IMS_TRACE_TAG_REG__;

class ExtraHeader : public RcObject
{
public:
    inline ExtraHeader() :
            RcObject(),
            m_piHeader(IMS_NULL)
    {
    }

    inline ~ExtraHeader() override
    {
        if (m_piHeader != IMS_NULL)
        {
            m_piHeader->Destroy();
        }
    }

    inline IMS_BOOL Create(IN const AString& strName, IN const AString& strValue)
    {
        m_piHeader = SipParsingHelper::CreateHeader(strName, strValue);

        if (m_piHeader == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    inline IMS_BOOL Equals(IN const ExtraHeader* pOther) const
    {
        if (pOther == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (m_piHeader == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return m_piHeader->Equals(pOther->m_piHeader);
    }

public:
    ISipHeader* m_piHeader;
};

class ExtraHeaders
{
public:
    ExtraHeaders();
    ~ExtraHeaders();

public:
    IMS_BOOL AddHeader(IN const AString& strName, IN const AString& strValue);
    IMS_BOOL AddHeader(IN const AString& strName, IN const ImsList<AString>& objValues);
    void RemoveHeader(IN const AString& strName, IN const AString& strValue);
    void RemoveHeader(IN const AString& strName, IN const ImsList<AString>& objValues);
    inline const ImsList<ExtraHeader*>& GetHeaders() const { return m_objHeaders; }

private:
    ImsList<ExtraHeader*> m_objHeaders;
};

PUBLIC
ExtraHeaders::ExtraHeaders() {}

PUBLIC
ExtraHeaders::~ExtraHeaders()
{
    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        ExtraHeader* pHeader = m_objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            pHeader->RemoveReference();
        }
    }

    m_objHeaders.Clear();
}

PUBLIC
IMS_BOOL ExtraHeaders::AddHeader(IN const AString& strName, IN const AString& strValue)
{
    ExtraHeader* pNewHeader = new ExtraHeader();

    if (pNewHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pNewHeader->Create(strName, strValue))
    {
        delete pNewHeader;
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        ExtraHeader* pHeader = m_objHeaders.GetAt(i);

        if (pHeader->Equals(pNewHeader))
        {
            pHeader->AddReference();

            delete pNewHeader;
            return IMS_TRUE;
        }
    }

    // If no match found...
    pNewHeader->AddReference();

    if (!m_objHeaders.Append(pNewHeader))
    {
        pNewHeader->RemoveReference();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ExtraHeaders::AddHeader(IN const AString& strName, IN const ImsList<AString>& objValues)
{
    for (IMS_UINT32 i = 0; i < objValues.GetSize(); ++i)
    {
        const AString& strValue = objValues.GetAt(i);

        if (!AddHeader(strName, strValue))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void ExtraHeaders::RemoveHeader(IN const AString& strName, IN const AString& strValue)
{
    ExtraHeader* pNewHeader = new ExtraHeader();

    if (pNewHeader == IMS_NULL)
    {
        return;
    }

    if (!pNewHeader->Create(strName, strValue))
    {
        delete pNewHeader;
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        ExtraHeader* pHeader = m_objHeaders.GetAt(i);

        if (pHeader->Equals(pNewHeader))
        {
            if (pHeader->IsShared())
            {
                pHeader->RemoveReference();
            }
            else
            {
                pHeader->RemoveReference();
                m_objHeaders.RemoveAt(i);
            }
            break;
        }
    }

    delete pNewHeader;
}

PUBLIC
void ExtraHeaders::RemoveHeader(IN const AString& strName, IN const ImsList<AString>& objValues)
{
    for (IMS_UINT32 i = 0; i < objValues.GetSize(); ++i)
    {
        const AString& strValue = objValues.GetAt(i);

        RemoveHeader(strName, strValue);
    }
}

PUBLIC
RegParameter::RegParameter(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_bPolicyForAuthenticationCredentials(IMS_TRUE),
        m_nTransportExt(Sip::TRANSPORT_EXT_ANY),
        m_nTransportExtForRegOnly(Sip::TRANSPORT_EXT_ANY),
        m_nPort(Sip::PORT_5060),
        m_nFlowControlOption(FLOW_CONTROL_BY_PROVISION),
        m_nPortFlowControl(Sip::PORT_UNSPECIFIED),
        m_strServingCscf(AString::ConstNull()),
        m_pPreferredSecurityClient(IMS_NULL),
        m_pPreferredSecurityServer(IMS_NULL),
        m_pExtraHeaders(IMS_NULL),
        m_objBodyParts(ImsList<ISipMessageBodyPart*>()),
        m_bIsAuthRealmLenient(IMS_FALSE),
        m_pSipTimerValues(IMS_NULL)
{
    m_pExtraHeaders = new ExtraHeaders();
    m_nPort = SipConfigProxy::GetPort(GetSlotId());

    IMS_TRACE_D("The default port_uc (%d) is selected", m_nPort, 0, 0);
}

PUBLIC VIRTUAL RegParameter::~RegParameter()
{
    RemoveAllMessageBodyParts();

    if (m_pExtraHeaders != IMS_NULL)
    {
        delete m_pExtraHeaders;
    }

    if (m_pPreferredSecurityServer != IMS_NULL)
    {
        delete m_pPreferredSecurityServer;
    }

    if (m_pPreferredSecurityClient != IMS_NULL)
    {
        delete m_pPreferredSecurityClient;
    }

    if (m_pSipTimerValues != IMS_NULL)
    {
        delete m_pSipTimerValues;
    }
}

PUBLIC
IMS_RESULT RegParameter::FormHeaders(
        IN_OUT ISipClientConnection*& piScc, IN const RcPtr<RegStateTracker>& pStateTracker)
{
    ISipMessage* piSipMsg = piScc->GetMessage();

    // Re-write the Request-URI as the S-CSCF's address
    if (piScc->SetRequestUri(m_strServingCscf) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    // Sets the preloaded route set if present
    if (SipConfigProxy::IsRouteHeaderInRegRequired(GetSlotId(), pStateTracker->GetSipProfile()))
    {
        for (IMS_SINT32 i = 0; i < m_objPreloadedRoutes.GetCount(); ++i)
        {
            if (piSipMsg->AddHeader(ISipHeader::ROUTE, m_objPreloadedRoutes.GetElementAt(i)) !=
                    IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Route header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }
    else
    {
        // IMPLICIT_ROUTE
        if (!m_objPreloadedRoutes.IsEmpty())
        {
            piScc->SetImplicitRouteHeader(m_objPreloadedRoutes.GetElementAt(0));
        }
        else
        {
            piScc->SetImplicitRouteHeader(AString::ConstNull());
        }
    }

    // Add an additional registration headers from the CoreService configuration
    const ImsList<ExtraHeader*>& objHeaders = m_pExtraHeaders->GetHeaders();

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const ExtraHeader* pHeader = objHeaders.GetAt(i);

        if (pHeader->m_piHeader == IMS_NULL)
        {
            continue;
        }

        if (piSipMsg->AddHeader(pHeader->m_piHeader->GetType(),
                    pHeader->m_piHeader->GetHeaderValue(),
                    pHeader->m_piHeader->GetName()) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting an extra headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    // Add the header values after checking whether or not Security-XXX headers exist
    if (IsSecurityAssociationRequired())
    {
        if (!SipConfigProxy::IsIpSecConfigured(GetSlotId(), pStateTracker->GetSipProfile()))
        {
            // Do not add "sec-agree" option tag if IPSec is disabled.
        }
        else
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_SEC_AGREE);
        }

        piSipMsg->AddHeader(ISipHeader::REQUIRE, Sip::STR_SEC_AGREE);
        piSipMsg->AddHeader(ISipHeader::PROXY_REQUIRE, Sip::STR_SEC_AGREE);
    }

    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), pStateTracker->GetSipProfile()))
    {
        piSipMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
    }

    if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId()))
    {
        piSipMsg->AddHeader(ISipHeader::SUPPORTED, "outbound");
    }

    if (!piSipMsg->IsHeaderPresent(ISipHeader::ALLOW))
    {
        // Add an allowed/supported methods for this UA
        const AStringArray& objMethods =
                SipConfigProxy::GetRegAllowMethods(GetSlotId(), pStateTracker->GetSipProfile());

        for (IMS_SINT32 i = 0; i < objMethods.GetCount(); ++i)
        {
            if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(i)) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Allow header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    // Add a User-Agent if configurable
    if (!piSipMsg->IsHeaderPresent(ISipHeader::USER_AGENT))
    {
        if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), pStateTracker->GetSipProfile()))
        {
            UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, pStateTracker->GetSipProfile(),
                    AString::ConstNull(), pStateTracker->GetIpAddress(), GetSlotId(), piSipMsg);
        }
    }

    // Sets an Authorization header; Check if this header SHOULD be set
    if (m_bPolicyForAuthenticationCredentials)
    {
        AString strAuthorization;

        strAuthorization.Sprintf("Digest "
                                 "username=\"%s\", "
                                 "realm=\"%s\", "
                                 "nonce=\"\", "
                                 "uri=\"%s\", "
                                 "response=\"\"",
                m_objCredential.GetUsername().GetStr(), m_objCredential.GetRealm().GetStr(),
                m_strServingCscf.GetStr());

        if (SipConfigProxy::IsAuthenticationAlgorithmRequired(
                    GetSlotId(), pStateTracker->GetSipProfile()))
        {
            strAuthorization.Append(", algorithm=");

            switch (m_objCredential.GetType())
            {
                case Credential::TYPE_AKAv1_MD5:
                    strAuthorization.Append(Credential::STR_AKAv1_MD5);
                    break;
                case Credential::TYPE_AKAv2_MD5:
                    strAuthorization.Append(Credential::STR_AKAv2_MD5);
                    break;
                default:
                    strAuthorization.Append(Credential::STR_MD5);
                    break;
            }
        }

        if (piSipMsg->SetHeader(ISipHeader::AUTHORIZATION, strAuthorization) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Authorization header failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    // Sets the message body parts if present
    if (!m_objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
        {
            const ISipMessageBodyPart* piBodyPart = m_objBodyParts.GetAt(i);

            if (piBodyPart != IMS_NULL)
            {
                ISipMessageBodyPart* piNewBodyPart = piSipMsg->CreateBodyPart();

                if (piNewBodyPart != IMS_NULL)
                {
                    piNewBodyPart->CopyFrom(piBodyPart);
                }
            }
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT RegParameter::FormRouteHeaders(
        IN_OUT ISipClientConnection*& piScc, IN const RcPtr<RegStateTracker>& pStateTracker)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!IsSecurityAssociationRequired())
    {
        IMS_TRACE_D("No security related headers", 0, 0, 0);
        return IMS_SUCCESS;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Removes all Route headers if present.
    IMS_SINT32 nHCount = piSipMsg->GetHeaderCount(ISipHeader::ROUTE);

    while (nHCount > 0)
    {
        piSipMsg->RemoveHeader(ISipHeader::ROUTE);
        --nHCount;
    }

    if (SipConfigProxy::IsRouteHeaderInRegRequired(GetSlotId(), pStateTracker->GetSipProfile()))
    {
        for (IMS_SINT32 i = 0; i < m_objPreloadedRoutes.GetCount(); ++i)
        {
            const AString& strRoute = m_objPreloadedRoutes.GetElementAt(i);

            if (piSipMsg->AddHeader(ISipHeader::ROUTE, strRoute) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding a Route headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }
    else
    {
        // IMPLICIT_ROUTE
        if (!m_objPreloadedRoutes.IsEmpty())
        {
            piScc->SetImplicitRouteHeader(m_objPreloadedRoutes.GetElementAt(0));
        }
        else
        {
            piScc->SetImplicitRouteHeader(AString::ConstNull());
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT RegParameter::FormSecurityHeaders(IN const ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!IsSecurityAssociationRequired())
    {
        IMS_TRACE_D("No security related headers", 0, 0, 0);
        return IMS_SUCCESS;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Removes all Security-Client/-Verify headers if present.
    IMS_SINT32 nHCount = piSipMsg->GetHeaderCount(ISipHeader::SECURITY_CLIENT);

    while (nHCount > 0)
    {
        piSipMsg->RemoveHeader(ISipHeader::SECURITY_CLIENT);
        --nHCount;
    }

    nHCount = piSipMsg->GetHeaderCount(ISipHeader::SECURITY_VERIFY);

    while (nHCount > 0)
    {
        piSipMsg->RemoveHeader(ISipHeader::SECURITY_VERIFY);
        --nHCount;
    }

    if (m_objNewSecurityClients.IsEmpty())
    {
        // If no new headers, set the Security-Client header using the current headers
        for (IMS_UINT32 i = 0; i < m_objSecurityClients.GetSize(); ++i)
        {
            const SipSecurityHeader& objSecurityHeader = m_objSecurityClients.GetAt(i);

            if (piSipMsg->AddHeader(ISipHeader::SECURITY_CLIENT, objSecurityHeader.ToString()) !=
                    IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding a Security-Client headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    for (IMS_UINT32 i = 0; i < m_objNewSecurityClients.GetSize(); ++i)
    {
        const SipSecurityHeader& objSecurityHeader = m_objNewSecurityClients.GetAt(i);

        if (piSipMsg->AddHeader(ISipHeader::SECURITY_CLIENT, objSecurityHeader.ToString()) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding a Security-Client headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    ImsList<SipSecurityHeader>* pSecurityHeaders = &m_objSecurityServers;

    if (!m_objSecurityVerifys.IsEmpty())
    {
        pSecurityHeaders = &m_objSecurityVerifys;
    }

    for (IMS_UINT32 i = 0; i < pSecurityHeaders->GetSize(); ++i)
    {
        const SipSecurityHeader& objSecurityHeader = pSecurityHeaders->GetAt(i);

        if (piSipMsg->AddHeader(ISipHeader::SECURITY_VERIFY, objSecurityHeader.ToString()) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding a Security-Verify headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_SINT32 RegParameter::GetProtectedPortUc() const
{
    if (m_objSecurityClients.IsEmpty())
    {
        return Sip::PORT_UNSPECIFIED;
    }

    if (m_pPreferredSecurityClient == IMS_NULL)
    {
        IMS_TRACE_D("The configured port_uc (%d) is selected", GetPort(), 0, 0);

        return GetPort();
    }

    IMS_TRACE_D(
            "The protected port_uc (%d) is selected", m_pPreferredSecurityClient->GetPortC(), 0, 0);

    return m_pPreferredSecurityClient->GetPortC();
}

PUBLIC
IMS_SINT32 RegParameter::GetProtectedPortUs() const
{
    if (m_objSecurityClients.IsEmpty())
    {
        return Sip::PORT_UNSPECIFIED;
    }

    if (m_pPreferredSecurityClient == IMS_NULL)
    {
        IMS_TRACE_D("The configured port_us (%d) is selected", GetPort(), 0, 0);

        return GetPort();
    }

    IMS_TRACE_D(
            "The protected port_us (%d) is selected", m_pPreferredSecurityClient->GetPortS(), 0, 0);

    return m_pPreferredSecurityClient->GetPortS();
}

PUBLIC
const ImsList<SipSecurityHeader>& RegParameter::GetSecurityVerifys() const
{
    if (!m_objSecurityVerifys.IsEmpty())
    {
        return m_objSecurityVerifys;
    }

    return m_objSecurityServers;
}

PUBLIC
void RegParameter::RemovePreferredSecurityHeaders()
{
    if (m_pPreferredSecurityServer != IMS_NULL)
    {
        delete m_pPreferredSecurityServer;
        m_pPreferredSecurityServer = IMS_NULL;
    }

    if (m_pPreferredSecurityClient != IMS_NULL)
    {
        delete m_pPreferredSecurityClient;
        m_pPreferredSecurityClient = IMS_NULL;
    }
}

PUBLIC
void RegParameter::Restore()
{
    m_bPolicyForAuthenticationCredentials = IMS_TRUE;

    m_nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC);
    m_nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);

    m_objSecurityClients.Clear();
    m_objNewSecurityClients.Clear();
    m_objSecurityServers.Clear();
    m_objSecurityVerifys.Clear();

    m_objOldSecurityClients.Clear();
    m_objOldSecurityServers.Clear();
    m_objOldSecurityVerifys.Clear();

    if (m_pPreferredSecurityServer != IMS_NULL)
    {
        delete m_pPreferredSecurityServer;
        m_pPreferredSecurityServer = IMS_NULL;
    }

    if (m_pPreferredSecurityClient != IMS_NULL)
    {
        delete m_pPreferredSecurityClient;
        m_pPreferredSecurityClient = IMS_NULL;
    }
}

PUBLIC
void RegParameter::RestoreSecurityHeaders()
{
    m_objSecurityClients = m_objOldSecurityClients;
    m_objSecurityServers = m_objOldSecurityServers;
    m_objSecurityVerifys = m_objOldSecurityVerifys;

    ChoosePreferredSecurityServer();
    ChoosePreferredSecurityClient();

    m_objOldSecurityClients.Clear();
    m_objOldSecurityServers.Clear();
    m_objOldSecurityVerifys.Clear();

    if ((m_pPreferredSecurityClient != IMS_NULL) || (m_pPreferredSecurityServer != IMS_NULL))
    {
        IMS_TRACE_D("RestoreSecurityHeaders :: client=%s, server=%s",
                (m_pPreferredSecurityClient != IMS_NULL)
                        ? m_pPreferredSecurityClient->ToString().GetStr()
                        : "(null)",
                (m_pPreferredSecurityServer != IMS_NULL)
                        ? m_pPreferredSecurityServer->ToString().GetStr()
                        : "(null)",
                0);
    }
}

PUBLIC
void RegParameter::SetTransportExtForIpSec()
{
    if (IsSecurityAssociationPresent())
    {
        m_nTransportExt |= Sip::TRANSPORT_EXT_IPSEC;

        if (!m_objSecurityVerifys.IsEmpty())
        {
            const SipSecurityHeader& objHeader = m_objSecurityVerifys.GetAt(0);

            if (objHeader.GetMode() == SipSecurityHeader::MODE_UDP_ENC_TUN)
            {
                m_nTransportExt |= Sip::TRANSPORT_EXT_IPSEC_UDP_ENC;
            }
            else
            {
                m_nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);
            }
        }
        else
        {
            m_nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);
        }
    }
    else
    {
        m_nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC);
        m_nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);
    }
}

PUBLIC
IMS_BOOL RegParameter::UpdateProfile(
        IN const SipAddress& objAor, IN const AString& strSubsId /*= AString::ConstNull()*/)
{
    const ImsSubscriberInfo* pSubsInfo = GetImsSubscriberInfo(GetSlotId(), objAor, strSubsId);

    if (pSubsInfo == IMS_NULL)
    {
        IMS_TRACE_E(0, "Subscriber (%s) info. is not found",
                SipDebug::GetUri1(objAor.GetUri()).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // Set a S-CSCF's address
    AString strTmpCscf = pSubsInfo->GetScscfAddress().MakeLower();

    if (strTmpCscf.Contains("sip:") || strTmpCscf.Contains("sips:"))
    {
        m_strServingCscf = pSubsInfo->GetScscfAddress();
    }
    else
    {
        m_strServingCscf.Sprintf("sip:%s", pSubsInfo->GetScscfAddress().GetStr());
    }

    // Set the credential information
    m_objCredential = pSubsInfo->GetCredential();

    // If the username field is empty, then sets it to the private user identity.
    if (m_objCredential.GetUsername().GetLength() == 0)
    {
        m_objCredential.SetUsername(pSubsInfo->GetPrivateUserId());
    }

    m_bIsAuthRealmLenient = pSubsInfo->IsAuthRealmLenient();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL RegParameter::UpdateSecurityHeaders(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Clear new Security-Client headers if P-CSCF rejects the proposal.
        m_objNewSecurityClients.Clear();

        m_objOldSecurityClients.Clear();
        m_objOldSecurityServers.Clear();
        m_objOldSecurityVerifys.Clear();
        return IMS_TRUE;
    }

    if (!IsSecurityAssociationRequired())
    {
        // IPSec is not enabled if Security-Client is not present.
        // So, it should be ignored in this case.
        return IMS_TRUE;
    }

    // Updates the Security-Server headers
    ImsList<AString> objHeaders = piSipMsg->GetHeaders(ISipHeader::SECURITY_SERVER);

    if (!objHeaders.IsEmpty())
    {
        m_objOldSecurityServers = m_objSecurityServers;
        m_objSecurityServers.Clear();
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const AString& strHeader = objHeaders.GetAt(i);
        ISipHeader* piHeader =
                SipParsingHelper::CreateHeader(ISipHeader::SECURITY_SERVER, strHeader);

        if (piHeader == IMS_NULL)
        {
            IMS_TRACE_E(0, "Parsing Security-Server failed", 0, 0, 0);
            continue;
        }

        SipSecurityHeader* pSecurityHeader = SipSecurityHeader::FromSipHeader(piHeader);

        if (pSecurityHeader == IMS_NULL)
        {
            piHeader->Destroy();
            continue;
        }

        m_objSecurityServers.Append(*pSecurityHeader);

        delete pSecurityHeader;
        piHeader->Destroy();
    }

    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // Updates the Security-Client headers if P-CSCF accepts the proposal
        if (!m_objNewSecurityClients.IsEmpty())
        {
            m_objOldSecurityClients = m_objSecurityClients;
            m_objSecurityClients = m_objNewSecurityClients;

            m_objOldSecurityVerifys = m_objSecurityVerifys;
        }

        ChoosePreferredSecurityServer();
        ChoosePreferredSecurityClient();
    }

    return IMS_TRUE;
}

PUBLIC
void RegParameter::UpdateSipProfile(IN const SipProfile* pProfile)
{
    if (pProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nProfilePort = pProfile->GetPort();

    if ((nProfilePort != SipProfile::NOT_PROVISIONED) && (nProfilePort != m_nPort))
    {
        IMS_TRACE_D("Default port_uc :: %d >> %d", m_nPort, nProfilePort, 0);
        m_nPort = nProfilePort;
    }
}

PRIVATE VIRTUAL IMS_BOOL RegParameter::AddExtraHeaders(IN const AStringArray& objHeaders)
{
    AString strName;
    AString strValue;
    ImsList<AString> objValues;

    for (IMS_SINT32 i = 0; i < objHeaders.GetCount(); ++i)
    {
        const AString& strHeader = objHeaders.GetElementAt(i);
        IMS_SINT32 nIndex = strHeader.GetIndexOf(TextParser::CHAR_COLON);

        if (nIndex == AString::NPOS)
        {
            IMS_TRACE_E(0, "Malformed header (%s)", strHeader.GetStr(), 0, 0);
            continue;
        }

        strName = strHeader.GetSubStr(0, nIndex);
        strName = strName.Trim();

        strValue = strHeader.GetSubStr(nIndex + 1);

        objValues = strValue.Split(TextParser::CHAR_COMMA);

        if (!m_pExtraHeaders->AddHeader(strName, objValues))
        {
            IMS_TRACE_E(0, "Adding the extra headers for registration failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL RegParameter::AddMessageBodyPart(IN ISipMessageBodyPart* piBodyPart)
{
    if (piBodyPart == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_objBodyParts.Append(piBodyPart);
}

PRIVATE VIRTUAL IMS_BOOL RegParameter::AddPreloadedRoute(IN const AString& strRoute)
{
    if (strRoute.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    SipAddress objAddress(strRoute);

    if (objAddress.IsSchemeTel())
    {
        IMS_TRACE_E(0, "tel URI(%s) is not allowed in the preloaded route set",
                SipDebug::GetCharA1(strRoute.GetStr(), 9), 0, 0);
        return IMS_FALSE;
    }

    if (objAddress.GetScheme().GetLength() == 0)
    {
        IMS_TRACE_E(0, "URI scheme is not specified; route(%s)",
                SipDebug::GetCharA1(strRoute.GetStr(), 5), 0, 0);
        return IMS_FALSE;
    }

    m_objPreloadedRoutes.AddElement(strRoute);

    // Topmost Route address
    if (m_objPreloadedRoutes.GetCount() == 1)
    {
        m_objTopmostRouteAddress = objAddress;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL RegParameter::AddPreloadedRoute(IN const AString& strHost,
        IN IMS_SINT32 nPort, IN const AString& strScheme /*= AString::ConstNull()*/)
{
    SipAddress objAddress;

    objAddress.SetScheme((strScheme.GetLength() == 0) ? Sip::STR_SIP : strScheme);
    objAddress.SetHost(strHost);
    objAddress.SetPort(nPort);
    objAddress.AddParameter("lr", AString::ConstNull());

    m_objPreloadedRoutes.AddElement(objAddress.ToString());

    // Topmost Route address
    if (m_objPreloadedRoutes.GetCount() == 1)
    {
        m_objTopmostRouteAddress = objAddress;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegParameter::RemoveAllMessageBodyParts()
{
    if (!m_objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
        {
            ISipMessageBodyPart* piBodyPart = m_objBodyParts.GetAt(i);

            if (piBodyPart != IMS_NULL)
            {
                piBodyPart->Destroy();
            }
        }

        m_objBodyParts.Clear();
    }
}

PRIVATE VIRTUAL void RegParameter::RemoveExtraHeaders(IN const AStringArray& objHeaders)
{
    AString strName;
    AString strValue;
    ImsList<AString> objValues;

    for (IMS_SINT32 i = 0; i < objHeaders.GetCount(); ++i)
    {
        const AString& strHeader = objHeaders.GetElementAt(i);
        IMS_SINT32 nIndex = strHeader.GetIndexOf(TextParser::CHAR_COLON);

        if (nIndex == AString::NPOS)
        {
            IMS_TRACE_E(0, "Malformed header (%s)", strHeader.GetStr(), 0, 0);
            continue;
        }

        strName = strHeader.GetSubStr(0, nIndex);
        strName = strName.Trim();

        strValue = strHeader.GetSubStr(nIndex + 1);

        objValues = strValue.Split(TextParser::CHAR_COMMA);

        m_pExtraHeaders->RemoveHeader(strName, objValues);
    }
}

PRIVATE
void RegParameter::SetSipTimerValues(IN const SipTimerValues& objTimerValues)
{
    if (m_pSipTimerValues != IMS_NULL)
    {
        delete m_pSipTimerValues;
        m_pSipTimerValues = IMS_NULL;
    }

    m_pSipTimerValues = new SipTimerValues(objTimerValues);
}

PRIVATE
void RegParameter::SetTransportExt(IN IMS_SINT32 nTransportExt)
{
    IMS_BOOL bIpSec = ((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC) != 0);
    IMS_BOOL bIpSecUdpEnc = ((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC_UDP_ENC) != 0);

    m_nTransportExt = nTransportExt;

    if (bIpSec)
    {
        m_nTransportExt |= Sip::TRANSPORT_EXT_IPSEC;
    }

    if (bIpSecUdpEnc)
    {
        m_nTransportExt |= Sip::TRANSPORT_EXT_IPSEC_UDP_ENC;
    }
}

PRIVATE
void RegParameter::ChoosePreferredSecurityClient()
{
    if (m_pPreferredSecurityClient != IMS_NULL)
    {
        delete m_pPreferredSecurityClient;
        m_pPreferredSecurityClient = IMS_NULL;
    }

    if (m_objSecurityClients.IsEmpty())
    {
        return;
    }

    if (m_objSecurityClients.GetSize() == 1)
    {
        m_pPreferredSecurityClient = new SipSecurityHeader(m_objSecurityClients.GetAt(0));

        return;
    }

    if (m_pPreferredSecurityServer == IMS_NULL)
    {
        m_pPreferredSecurityClient = new SipSecurityHeader(m_objSecurityClients.GetAt(0));

        IMS_TRACE_D("There is no preferred security-server, "
                    "so choose the topmost security-client",
                0, 0, 0);
        return;
    }

    SipSecurityHeader* pTmpPreferredHeader = IMS_NULL;

    for (IMS_UINT32 i = 0; i < m_objSecurityClients.GetSize(); ++i)
    {
        const SipSecurityHeader& objClientHeader = m_objSecurityClients.GetAt(i);

        if (objClientHeader.GetMechanism() != m_pPreferredSecurityServer->GetMechanism())
        {
            continue;
        }

        if (pTmpPreferredHeader == IMS_NULL)
        {
            pTmpPreferredHeader = new SipSecurityHeader(objClientHeader);
        }
        else
        {
            // Compares the 'q' values
            const AString& strNewPreference = objClientHeader.GetPreference();
            const AString& strOldPreference = pTmpPreferredHeader->GetPreference();

            if (AString::Compare(strNewPreference.GetStr(), strOldPreference.GetStr()) > 0)
            {
                // Change the preferred security header
                delete pTmpPreferredHeader;

                pTmpPreferredHeader = new SipSecurityHeader(objClientHeader);
            }
        }
    }

    if (pTmpPreferredHeader != IMS_NULL)
    {
        m_pPreferredSecurityClient = new SipSecurityHeader(*pTmpPreferredHeader);

        delete pTmpPreferredHeader;
    }
    else
    {
        IMS_TRACE_E(0, "No matched Security-Client header", 0, 0, 0);
    }
}

PRIVATE
void RegParameter::ChoosePreferredSecurityServer()
{
    if (m_pPreferredSecurityServer != IMS_NULL)
    {
        delete m_pPreferredSecurityServer;
        m_pPreferredSecurityServer = IMS_NULL;
    }

    if (m_objSecurityServers.IsEmpty())
    {
        return;
    }

    if (m_objSecurityServers.GetSize() == 1)
    {
        m_pPreferredSecurityServer = new SipSecurityHeader(m_objSecurityServers.GetAt(0));

        return;
    }

    // Collect the mechanism from the Security-Clients
    SipSecurityHeader* pTmpPreferredHeader = IMS_NULL;

    for (IMS_UINT32 i = 0; i < m_objSecurityServers.GetSize(); ++i)
    {
        IMS_BOOL bSupportedHeaderFound = IMS_FALSE;
        const SipSecurityHeader& objServerHeader = m_objSecurityServers.GetAt(i);

        for (IMS_UINT32 j = 0; j < m_objSecurityClients.GetSize(); ++j)
        {
            const SipSecurityHeader& objClientHeader = m_objSecurityClients.GetAt(j);

            if (objClientHeader.GetMechanism() == objServerHeader.GetMechanism())
            {
                bSupportedHeaderFound = IMS_TRUE;
                break;
            }
        }

        if (!bSupportedHeaderFound)
        {
            continue;
        }

        if (pTmpPreferredHeader == IMS_NULL)
        {
            pTmpPreferredHeader = new SipSecurityHeader(objServerHeader);
        }
        else
        {
            // Compares the 'q' values
            const AString& strNewPreference = objServerHeader.GetPreference();
            const AString& strOldPreference = pTmpPreferredHeader->GetPreference();

            if (AString::Compare(strNewPreference.GetStr(), strOldPreference.GetStr()) > 0)
            {
                // Change the preferred security header
                delete pTmpPreferredHeader;

                pTmpPreferredHeader = new SipSecurityHeader(objServerHeader);
            }
        }
    }

    if (pTmpPreferredHeader != IMS_NULL)
    {
        m_pPreferredSecurityServer = new SipSecurityHeader(*pTmpPreferredHeader);

        delete pTmpPreferredHeader;
    }
    else
    {
        IMS_TRACE_E(0, "No matched Security-Server header", 0, 0, 0);
    }
}

PRIVATE GLOBAL const ImsSubscriberInfo* RegParameter::GetImsSubscriberInfo(IN IMS_SINT32 nSlotId,
        IN const SipAddress& objAor, IN const AString& strSubsId /*= AString::ConstNull()*/)
{
    const SubscriberConfig* pSubscriberConfig = IMS_NULL;
    const ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    if (strSubsId.GetLength() > 0)
    {
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strSubsId, nSlotId);
    }
    else
    {
        const AString& strId = SubscriberTracker::GetInstance()->GetSubscriberId(nSlotId, &objAor);
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strId, nSlotId);
    }

    if (pSubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "SubscriberConfig is null; subsId=%s", strSubsId.GetStr(), 0, 0);
        return IMS_NULL;
    }

    IMS_SINT32 nCount = pSubscriberConfig->GetSubscriberCount();
    SipAddress objImpu;

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        ImsSubscriberInfo* pSubsInfo = pSubscriberConfig->GetSubscriberInfoEx(i);

        if (pSubsInfo == IMS_NULL)
        {
            continue;
        }

        const AStringArray& objImpus = pSubsInfo->GetPublicUserIds();

        for (IMS_SINT32 j = 0; j < objImpus.GetCount(); ++j)
        {
            const AString& strImpu = objImpus.GetElementAt(j);

            if (objImpu.Create(strImpu))
            {
                if (objImpu.Equals(objAor))
                {
                    return pSubsInfo;
                }
            }
        }
    }

    // not found
    return IMS_NULL;
}
