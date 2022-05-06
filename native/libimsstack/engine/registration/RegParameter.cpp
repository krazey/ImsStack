/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Credential.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipClientConnection.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipSecurityHeader.h"
#include "SipConfigProxy.h"
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"
#include "base/SubscriberTracker.h"
#include "util/UserAgentHeader.h"
#include "RegParameter.h"

__IMS_TRACE_TAG_REG__;

class ExtraHeader : public RCObject
{
public:
    inline ExtraHeader() :
            RCObject(),
            piHeader(IMS_NULL)
    {
    }

    inline virtual ~ExtraHeader()
    {
        if (piHeader != IMS_NULL)
            piHeader->Destroy();
    }

    inline IMS_BOOL Create(IN CONST AString& strName, IN CONST AString& strValue)
    {
        piHeader = SipParsingHelper::CreateHeader(strName, strValue);

        if (piHeader == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    inline IMS_BOOL Equals(IN CONST ExtraHeader* pOther) const
    {
        if (pOther == IMS_NULL)
            return IMS_FALSE;

        if (piHeader == IMS_NULL)
            return IMS_FALSE;

        return piHeader->Equals(pOther->piHeader);
    }

public:
    ISipHeader* piHeader;
};

class ExtraHeaders
{
public:
    ExtraHeaders();
    ~ExtraHeaders();

public:
    IMS_BOOL AddHeader(IN CONST AString& strName, IN CONST AString& strValue);
    IMS_BOOL AddHeader(IN CONST AString& strName, IN CONST IMSList<AString>& objValues);
    void RemoveHeader(IN CONST AString& strName, IN CONST AString& strValue);
    void RemoveHeader(IN CONST AString& strName, IN CONST IMSList<AString>& objValues);
    const IMSList<ExtraHeader*>& GetHeaders() const;

private:
    IMSList<ExtraHeader*> objHeaders;
};

PUBLIC
ExtraHeaders::ExtraHeaders() {}

PUBLIC
ExtraHeaders::~ExtraHeaders()
{
    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        ExtraHeader* pHeader = objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
            pHeader->RemoveReference();
    }

    objHeaders.Clear();
}

PUBLIC
IMS_BOOL ExtraHeaders::AddHeader(IN CONST AString& strName, IN CONST AString& strValue)
{
    ExtraHeader* pNewHeader = new ExtraHeader();

    //---------------------------------------------------------------------------------------------

    if (pNewHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pNewHeader->Create(strName, strValue))
    {
        delete pNewHeader;
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        ExtraHeader* pHeader = objHeaders.GetAt(i);

        if (pHeader->Equals(pNewHeader))
        {
            pHeader->AddReference();

            delete pNewHeader;
            return IMS_TRUE;
        }
    }

    // If no match found...
    pNewHeader->AddReference();

    if (!objHeaders.Append(pNewHeader))
    {
        pNewHeader->RemoveReference();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ExtraHeaders::AddHeader(IN CONST AString& strName, IN CONST IMSList<AString>& objValues)
{
    //---------------------------------------------------------------------------------------------

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
void ExtraHeaders::RemoveHeader(IN CONST AString& strName, IN CONST AString& strValue)
{
    ExtraHeader* pNewHeader = new ExtraHeader();

    //---------------------------------------------------------------------------------------------

    if (pNewHeader == IMS_NULL)
    {
        return;
    }

    if (!pNewHeader->Create(strName, strValue))
    {
        delete pNewHeader;
        return;
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        ExtraHeader* pHeader = objHeaders.GetAt(i);

        if (pHeader->Equals(pNewHeader))
        {
            if (pHeader->IsShared())
                pHeader->RemoveReference();
            else
            {
                pHeader->RemoveReference();
                objHeaders.RemoveAt(i);
            }
            break;
        }
    }

    delete pNewHeader;
}

PUBLIC
void ExtraHeaders::RemoveHeader(IN CONST AString& strName, IN CONST IMSList<AString>& objValues)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objValues.GetSize(); ++i)
    {
        const AString& strValue = objValues.GetAt(i);

        RemoveHeader(strName, strValue);
    }
}

PUBLIC
const IMSList<ExtraHeader*>& ExtraHeaders::GetHeaders() const
{
    //---------------------------------------------------------------------------------------------

    return objHeaders;
}

PUBLIC
RegParameter::RegParameter(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        bPolicyForAuthenticationCredentials(IMS_TRUE),
        nTransportExt(Sip::TRANSPORT_EXT_ANY),
        nTransportExtForRegOnly(Sip::TRANSPORT_EXT_ANY),
        nPort(Sip::PORT_5060),
        nFlowControlOption(FLOW_CONTROL_BY_PROVISION),
        nPortFlowControl(Sip::PORT_UNSPECIFIED),
        strServingCSCF(AString::ConstNull()),
        pPreferredSecurityClient(IMS_NULL),
        pPreferredSecurityServer(IMS_NULL),
        pExtraHeaders(IMS_NULL),
        objBodyParts(IMSList<ISipMessageBodyPart*>()),
        bIsAuthRealmLenient(IMS_FALSE),
        pSIPTVs(IMS_NULL)
{
    pExtraHeaders = new ExtraHeaders();

    nPort = SipConfigProxy::GetPort(GetSlotId());

    IMS_TRACE_D("The default port_uc (%d) is selected", nPort, 0, 0);
}

PUBLIC VIRTUAL RegParameter::~RegParameter()
{
    RemoveAllMessageBodyParts();

    if (pExtraHeaders != IMS_NULL)
        delete pExtraHeaders;

    if (pPreferredSecurityServer != IMS_NULL)
        delete pPreferredSecurityServer;

    if (pPreferredSecurityClient != IMS_NULL)
        delete pPreferredSecurityClient;

    if (pSIPTVs != IMS_NULL)
        delete pSIPTVs;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 RegParameter::GetPort() const
{
    //---------------------------------------------------------------------------------------------

    return nPort;
}

/*

Remarks

*/
PUBLIC VIRTUAL const SipAddress& RegParameter::GetTopmostRouteAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objTopmostRouteAddress;
}

/*

Remarks

*/
PUBLIC VIRTUAL void RegParameter::SetSecurityVerifys(
        IN CONST IMSList<SipSecurityHeader>& objSecurityVerifys)
{
    //---------------------------------------------------------------------------------------------

    this->objSecurityVerifys = objSecurityVerifys;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT RegParameter::FormHeaders(
        IN_OUT ISipClientConnection*& piSCC, IN CONST RCPtr<RegStateTracker> pStateTracker)
{
    ISipMessage* piSIPMsg = piSCC->GetMessage();

    //---------------------------------------------------------------------------------------------

    // Re-write the Request-URI as the S-CSCF's address
    if (piSCC->SetRequestUri(strServingCSCF) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    // Sets the preloaded route set if present
    if (SipConfigProxy::IsRouteHeaderInRegRequired(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        for (IMS_SINT32 i = 0; i < objPreloadedRoutes.GetCount(); ++i)
        {
            if (piSIPMsg->AddHeader(ISipHeader::ROUTE, objPreloadedRoutes.GetElementAt(i)) !=
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
        if (!objPreloadedRoutes.IsEmpty())
        {
            piSCC->SetImplicitRouteHeader(objPreloadedRoutes.GetElementAt(0));
        }
        else
        {
            piSCC->SetImplicitRouteHeader(AString::ConstNull());
        }
    }

    // Add an additional registration headers from the CoreService configuration
    {
        const IMSList<ExtraHeader*>& objHeaders = pExtraHeaders->GetHeaders();

        for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
        {
            const ExtraHeader* pHeader = objHeaders.GetAt(i);

            if (pHeader->piHeader == IMS_NULL)
                continue;

            if (piSIPMsg->AddHeader(pHeader->piHeader->GetType(),
                        pHeader->piHeader->GetHeaderValue(),
                        pHeader->piHeader->GetName()) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting an extra headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }

        // Add the header values after checking whether or not Security-XXX headers exist
        if (IsSecurityAssociationRequired())
        {
            if (!SipConfigProxy::IsIpSecConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
            {
                // Do not add "sec-agree" option tag if IPSec is disabled.
            }
            else
            {
                piSIPMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_SEC_AGREE);
            }

            piSIPMsg->AddHeader(ISipHeader::REQUIRE, Sip::STR_SEC_AGREE);
            piSIPMsg->AddHeader(
                    ISipHeader::UNKNOWN, Sip::STR_SEC_AGREE, SipHeaderName::PROXY_REQUIRE);
        }
    }

    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        piSIPMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
    }

    if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        piSIPMsg->AddHeader(ISipHeader::SUPPORTED, "outbound");
    }

    if (!piSIPMsg->IsHeaderPresent(ISipHeader::ALLOW))
    {
        // Add an allowed/supported methods for this UA
        const AStringArray& objMethods =
                SipConfigProxy::GetRegAllowMethods(GetSlotId(), pStateTracker->GetSIPProfile());

        for (IMS_SINT32 i = 0; i < objMethods.GetCount(); ++i)
        {
            if (piSIPMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(i)) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Allow header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    // Add a User-Agent if configurable
    if (!piSIPMsg->IsHeaderPresent(ISipHeader::UNKNOWN, SipHeaderName::USER_AGENT))
    {
        if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
        {
            UserAgentHeader::SetHeader(SipHeaderName::USER_AGENT, pStateTracker->GetSIPProfile(),
                    AString::ConstNull(), pStateTracker->GetIPAddress(), GetSlotId(), piSIPMsg);
        }
    }

    // Sets an Authorization header; Check if this header SHOULD be set
    if (bPolicyForAuthenticationCredentials)
    {
        AString strAuthorization;

        strAuthorization.Sprintf("Digest "
                                 "username=\"%s\", "
                                 "realm=\"%s\", "
                                 "nonce=\"\", "
                                 "uri=\"%s\", "
                                 "response=\"\"",
                objCredential.GetUsername().GetStr(), objCredential.GetRealm().GetStr(),
                strServingCSCF.GetStr());

        if (SipConfigProxy::IsAuthenticationAlgorithmRequired(
                    GetSlotId(), pStateTracker->GetSIPProfile()))
        {
            strAuthorization.Append(", algorithm=");

            switch (objCredential.GetType())
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

        if (piSIPMsg->SetHeader(ISipHeader::AUTHORIZATION, strAuthorization) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Authorization header failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    // Sets the message body parts if present
    if (!objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);

            if (piBodyPart != IMS_NULL)
            {
                ISipMessageBodyPart* piNewBodyPart = piSIPMsg->CreateBodyPart();

                if (piNewBodyPart != IMS_NULL)
                {
                    piNewBodyPart->CopyFrom(piBodyPart);
                }
            }
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT RegParameter::FormRouteHeaders(
        IN_OUT ISipClientConnection*& piSCC, IN CONST RCPtr<RegStateTracker> pStateTracker)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!IsSecurityAssociationRequired())
    {
        IMS_TRACE_D("No security related headers", 0, 0, 0);
        return IMS_SUCCESS;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Removes all Route headers if present.
    IMS_SINT32 nHCount = piSIPMsg->GetHeaderCount(ISipHeader::ROUTE);

    while (nHCount > 0)
    {
        piSIPMsg->RemoveHeader(ISipHeader::ROUTE);
        --nHCount;
    }

    if (SipConfigProxy::IsRouteHeaderInRegRequired(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        for (IMS_SINT32 i = 0; i < objPreloadedRoutes.GetCount(); ++i)
        {
            const AString& strRoute = objPreloadedRoutes.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISipHeader::ROUTE, strRoute) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding a Route headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }
    else
    {
        // IMPLICIT_ROUTE
        if (!objPreloadedRoutes.IsEmpty())
        {
            piSCC->SetImplicitRouteHeader(objPreloadedRoutes.GetElementAt(0));
        }
        else
        {
            piSCC->SetImplicitRouteHeader(AString::ConstNull());
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT RegParameter::FormSecurityHeaders(IN_OUT ISipClientConnection*& piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!IsSecurityAssociationRequired())
    {
        IMS_TRACE_D("No security related headers", 0, 0, 0);
        return IMS_SUCCESS;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Removes all Security-Client/-Verify headers if present.
    IMS_SINT32 nHCount = piSIPMsg->GetHeaderCount(ISipHeader::SECURITY_CLIENT);

    while (nHCount > 0)
    {
        piSIPMsg->RemoveHeader(ISipHeader::SECURITY_CLIENT);
        --nHCount;
    }

    nHCount = piSIPMsg->GetHeaderCount(ISipHeader::SECURITY_VERIFY);

    while (nHCount > 0)
    {
        piSIPMsg->RemoveHeader(ISipHeader::SECURITY_VERIFY);
        --nHCount;
    }

    if (objNewSecurityClients.IsEmpty())
    {
        // If no new headers, set the Security-Client header using the current headers
        for (IMS_UINT32 i = 0; i < objSecurityClients.GetSize(); ++i)
        {
            const SipSecurityHeader& objSecurityHeader = objSecurityClients.GetAt(i);

            if (piSIPMsg->AddHeader(ISipHeader::SECURITY_CLIENT, objSecurityHeader.ToString()) !=
                    IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding a Security-Client headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    for (IMS_UINT32 i = 0; i < objNewSecurityClients.GetSize(); ++i)
    {
        const SipSecurityHeader& objSecurityHeader = objNewSecurityClients.GetAt(i);

        if (piSIPMsg->AddHeader(ISipHeader::SECURITY_CLIENT, objSecurityHeader.ToString()) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding a Security-Client headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    IMSList<SipSecurityHeader>* pSecurityHeaders = &objSecurityServers;

    if (!objSecurityVerifys.IsEmpty())
    {
        pSecurityHeaders = &objSecurityVerifys;
    }

    for (IMS_UINT32 i = 0; i < pSecurityHeaders->GetSize(); ++i)
    {
        const SipSecurityHeader& objSecurityHeader = pSecurityHeaders->GetAt(i);

        if (piSIPMsg->AddHeader(ISipHeader::SECURITY_VERIFY, objSecurityHeader.ToString()) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding a Security-Verify headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
Credential& RegParameter::GetCredential()
{
    //---------------------------------------------------------------------------------------------

    return objCredential;
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PUBLIC
IMS_SINT32 RegParameter::GetFlowControlOption() const
{
    //---------------------------------------------------------------------------------------------

    return nFlowControlOption;
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PUBLIC
IMS_SINT32 RegParameter::GetPortFlowControl() const
{
    //---------------------------------------------------------------------------------------------

    return nPortFlowControl;
}

/*

Remarks

*/
PUBLIC
const AStringArray& RegParameter::GetPreloadedRoutes() const
{
    //---------------------------------------------------------------------------------------------

    return objPreloadedRoutes;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegParameter::GetProtectedPortUC() const
{
    //---------------------------------------------------------------------------------------------

    if (objSecurityClients.IsEmpty())
    {
        return Sip::PORT_UNSPECIFIED;
    }

    if (pPreferredSecurityClient == IMS_NULL)
    {
        IMS_TRACE_D("The configured port_uc (%d) is selected", GetPort(), 0, 0);

        return GetPort();
    }

    IMS_TRACE_D(
            "The protected port_uc (%d) is selected", pPreferredSecurityClient->GetPortC(), 0, 0);

    return pPreferredSecurityClient->GetPortC();
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegParameter::GetProtectedPortUS() const
{
    //---------------------------------------------------------------------------------------------

    if (objSecurityClients.IsEmpty())
    {
        return Sip::PORT_UNSPECIFIED;
    }

    if (pPreferredSecurityClient == IMS_NULL)
    {
        IMS_TRACE_D("The configured port_us (%d) is selected", GetPort(), 0, 0);

        return GetPort();
    }

    IMS_TRACE_D(
            "The protected port_us (%d) is selected", pPreferredSecurityClient->GetPortS(), 0, 0);

    return pPreferredSecurityClient->GetPortS();
}

/*

Remarks

*/
PUBLIC
const IMSList<SipSecurityHeader>& RegParameter::GetSecurityClients() const
{
    //---------------------------------------------------------------------------------------------

    return objSecurityClients;
}

/*

Remarks

*/
PUBLIC
const IMSList<SipSecurityHeader>& RegParameter::GetSecurityVerifys() const
{
    //---------------------------------------------------------------------------------------------

    if (!objSecurityVerifys.IsEmpty())
    {
        return objSecurityVerifys;
    }

    return objSecurityServers;
}

/*

Remarks

*/
PUBLIC
const SipTimerValues* RegParameter::GetSIPTimerValues() const
{
    //---------------------------------------------------------------------------------------------

    return pSIPTVs;
}

/*

Remarks
 MULTI_REG_TRANSPORT
*/
PUBLIC
IMS_SINT32 RegParameter::GetTransportExt() const
{
    //---------------------------------------------------------------------------------------------

    return nTransportExt;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegParameter::GetTransportExtForRegOnly() const
{
    //---------------------------------------------------------------------------------------------

    return nTransportExtForRegOnly;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegParameter::IsAuthRealmLenient() const
{
    //---------------------------------------------------------------------------------------------

    return bIsAuthRealmLenient;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegParameter::IsSecurityAssociationPresent() const
{
    //---------------------------------------------------------------------------------------------

    return (!objSecurityServers.IsEmpty() || !objSecurityVerifys.IsEmpty());
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegParameter::IsSecurityAssociationRequired() const
{
    //---------------------------------------------------------------------------------------------

    return (!objSecurityClients.IsEmpty() || !objNewSecurityClients.IsEmpty());
}

/*

Remarks
 IMS_IPSEC_UDP_ENC
*/
PUBLIC
IMS_BOOL RegParameter::IsSecurityAssociationRequiredViaUDPEnc() const
{
    //---------------------------------------------------------------------------------------------

    return ((nTransportExt & Sip::TRANSPORT_EXT_IPSEC_UDP_ENC) != 0);
}

/*

Remarks

*/
PUBLIC
void RegParameter::RemovePreferredSecurityHeaders()
{
    //---------------------------------------------------------------------------------------------

    if (pPreferredSecurityServer != IMS_NULL)
    {
        delete pPreferredSecurityServer;
        pPreferredSecurityServer = IMS_NULL;
    }

    if (pPreferredSecurityClient != IMS_NULL)
    {
        delete pPreferredSecurityClient;
        pPreferredSecurityClient = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC
void RegParameter::RemoveSecurityServers()
{
    //---------------------------------------------------------------------------------------------

    objSecurityServers.Clear();
}

/*

Remarks

*/
PUBLIC
void RegParameter::Restore()
{
    //---------------------------------------------------------------------------------------------

    bPolicyForAuthenticationCredentials = IMS_TRUE;

    nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC);
    nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);

    objSecurityClients.Clear();
    objNewSecurityClients.Clear();
    objSecurityServers.Clear();
    objSecurityVerifys.Clear();

    objOldSecurityClients.Clear();
    objOldSecurityServers.Clear();
    objOldSecurityVerifys.Clear();

    if (pPreferredSecurityServer != IMS_NULL)
    {
        delete pPreferredSecurityServer;
        pPreferredSecurityServer = IMS_NULL;
    }

    if (pPreferredSecurityClient != IMS_NULL)
    {
        delete pPreferredSecurityClient;
        pPreferredSecurityClient = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC
void RegParameter::RestoreSecurityHeaders()
{
    //---------------------------------------------------------------------------------------------

    objSecurityClients = objOldSecurityClients;
    objSecurityServers = objOldSecurityServers;
    objSecurityVerifys = objOldSecurityVerifys;

    ChoosePreferredSecurityServer();
    ChoosePreferredSecurityClient();

    objOldSecurityClients.Clear();
    objOldSecurityServers.Clear();
    objOldSecurityVerifys.Clear();

    if ((pPreferredSecurityClient != IMS_NULL) || (pPreferredSecurityServer != IMS_NULL))
    {
        IMS_TRACE_D("RestoreSecurityHeaders :: client=%s, server=%s",
                (pPreferredSecurityClient != IMS_NULL)
                        ? pPreferredSecurityClient->ToString().GetStr()
                        : "(null)",
                (pPreferredSecurityServer != IMS_NULL)
                        ? pPreferredSecurityServer->ToString().GetStr()
                        : "(null)",
                0);
    }
}

/*

Remarks

*/
PUBLIC
void RegParameter::SetTransportExtForIPSec()
{
    //---------------------------------------------------------------------------------------------

    if (IsSecurityAssociationPresent())
    {
        nTransportExt |= Sip::TRANSPORT_EXT_IPSEC;

        if (!objSecurityVerifys.IsEmpty())
        {
            const SipSecurityHeader& objHeader = objSecurityVerifys.GetAt(0);

            if (objHeader.GetMode() == SipSecurityHeader::MODE_UDP_ENC_TUN)
            {
                nTransportExt |= Sip::TRANSPORT_EXT_IPSEC_UDP_ENC;
            }
            else
            {
                nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);
            }
        }
        else
        {
            nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);
        }
    }
    else
    {
        nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC);
        nTransportExt &= ~(Sip::TRANSPORT_EXT_IPSEC_UDP_ENC);
    }
}

/*

Remarks
 MULTI_SUBS
*/
PUBLIC
IMS_BOOL RegParameter::UpdateProfile(
        IN CONST SipAddress& objAOR, IN CONST AString& strSubsId /* = AString::ConstNull() */)
{
    const ImsSubscriberInfo* pSubsInfo = GetImsSubscriberInfo(GetSlotId(), objAOR, strSubsId);

    //---------------------------------------------------------------------------------------------

    if (pSubsInfo == IMS_NULL)
    {
        IMS_TRACE_E(0, "Subscriber (%s) info. is not found",
                SipDebug::GetUri1(objAOR.GetUri()).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // Set a S-CSCF's address
    AString strTmpCSCF = pSubsInfo->GetScscfAddress().MakeLower();

    if (strTmpCSCF.Contains("sip:") || strTmpCSCF.Contains("sips:"))
    {
        strServingCSCF = pSubsInfo->GetScscfAddress();
    }
    else
    {
        strServingCSCF.Sprintf("sip:%s", pSubsInfo->GetScscfAddress().GetStr());
    }

    // Set the credential information
    objCredential = pSubsInfo->GetCredential();

    // If the username field is empty, then sets it to the private user identity.
    if (objCredential.GetUsername().GetLength() == 0)
    {
        objCredential.SetUsername(pSubsInfo->GetPrivateUserId());
    }

    bIsAuthRealmLenient = pSubsInfo->IsAuthRealmLenient();

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegParameter::UpdateSecurityHeaders(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Clear new Security-Client headers if P-CSCF rejects the proposal.
        objNewSecurityClients.Clear();

        objOldSecurityClients.Clear();
        objOldSecurityServers.Clear();
        objOldSecurityVerifys.Clear();
        return IMS_TRUE;
    }

    if (!IsSecurityAssociationRequired())
    {
        // IPSec is not enabled if Security-Client is not present.
        // So, it should be ignored in this case.
        return IMS_TRUE;
    }

    // Updates the Security-Server headers
    IMSList<AString> objHeaders = piSIPMsg->GetHeaders(ISipHeader::SECURITY_SERVER);

    if (!objHeaders.IsEmpty())
    {
        objOldSecurityServers = objSecurityServers;
        objSecurityServers.Clear();
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

        objSecurityServers.Append(*pSecurityHeader);

        delete pSecurityHeader;
        piHeader->Destroy();
    }

    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // Updates the Security-Client headers if P-CSCF accepts the proposal
        if (!objNewSecurityClients.IsEmpty())
        {
            objOldSecurityClients = objSecurityClients;
            objSecurityClients = objNewSecurityClients;

            objOldSecurityVerifys = objSecurityVerifys;
        }

        ChoosePreferredSecurityServer();
        ChoosePreferredSecurityClient();
    }

    return IMS_TRUE;
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
void RegParameter::UpdateSIPProfile(IN SipProfile* pSIPProfile)
{
    //---------------------------------------------------------------------------------------------

    if (pSIPProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nProfilePort = pSIPProfile->GetPort();

    if ((nProfilePort != SipProfile::NOT_PROVISIONED) && (nProfilePort != nPort))
    {
        IMS_TRACE_D("Default port_uc :: %d >> %d", nPort, nProfilePort, 0);
        nPort = nProfilePort;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegParameter::AddExtraHeaders(IN CONST AStringArray& objHeaders)
{
    AString strName;
    AString strValue;
    IMSList<AString> objValues;

    //---------------------------------------------------------------------------------------------

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

        if (!pExtraHeaders->AddHeader(strName, objValues))
        {
            IMS_TRACE_E(0, "Adding the extra headers for registration failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegParameter::AddMessageBodyPart(IN ISipMessageBodyPart* piBodyPart)
{
    //---------------------------------------------------------------------------------------------

    if (piBodyPart == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return objBodyParts.Append(piBodyPart);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegParameter::AddPreloadedRoute(IN CONST AString& strRoute)
{
    //---------------------------------------------------------------------------------------------

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

    objPreloadedRoutes.AddElement(strRoute);

    // Topmost Route address
    if (objPreloadedRoutes.GetCount() == 1)
    {
        objTopmostRouteAddress = objAddress;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegParameter::AddPreloadedRoute(IN CONST AString& strHost,
        IN IMS_SINT32 nPort, IN CONST AString& strScheme /* = AString::ConstNull() */)
{
    SipAddress objAddress;

    //---------------------------------------------------------------------------------------------

    objAddress.SetScheme((strScheme.GetLength() == 0) ? Sip::STR_SIP : strScheme);
    objAddress.SetHost(strHost);
    objAddress.SetPort(nPort);
    objAddress.AddParameter("lr", AString::ConstNull());

    objPreloadedRoutes.AddElement(objAddress.ToString());

    // Topmost Route address
    if (objPreloadedRoutes.GetCount() == 1)
    {
        objTopmostRouteAddress = objAddress;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegParameter::AddSecurityClient(
        IN CONST SipSecurityHeader& objSecurityHeader)
{
    //---------------------------------------------------------------------------------------------

    return objNewSecurityClients.Append(objSecurityHeader);
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipSecurityHeader* RegParameter::GetPreferredSecurityClient() const
{
    //---------------------------------------------------------------------------------------------

    return pPreferredSecurityClient;
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipSecurityHeader* RegParameter::GetPreferredSecurityServer() const
{
    //---------------------------------------------------------------------------------------------

    return pPreferredSecurityServer;
}

/*

Remarks

*/
PRIVATE VIRTUAL const IMSList<SipSecurityHeader>& RegParameter::GetSecurityServers() const
{
    //---------------------------------------------------------------------------------------------

    return objSecurityServers;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegParameter::RemoveAllMessageBodyParts()
{
    //---------------------------------------------------------------------------------------------

    if (!objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);

            if (piBodyPart != IMS_NULL)
            {
                piBodyPart->Destroy();
            }
        }

        objBodyParts.Clear();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegParameter::RemoveAllPreloadedRoutes()
{
    //---------------------------------------------------------------------------------------------

    objPreloadedRoutes.RemoveAllElements();
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegParameter::RemoveExtraHeaders(IN CONST AStringArray& objHeaders)
{
    AString strName;
    AString strValue;
    IMSList<AString> objValues;

    //---------------------------------------------------------------------------------------------

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

        pExtraHeaders->RemoveHeader(strName, objValues);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegParameter::RemoveSecurityClients()
{
    //---------------------------------------------------------------------------------------------

    objNewSecurityClients.Clear();
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegParameter::SetAuthenticationCredentials(IN IMS_BOOL bPolicy)
{
    //---------------------------------------------------------------------------------------------

    bPolicyForAuthenticationCredentials = bPolicy;
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PRIVATE VIRTUAL void RegParameter::SetFlowControlOption(IN IMS_SINT32 nOption)
{
    //---------------------------------------------------------------------------------------------

    nFlowControlOption = nOption;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegParameter::SetPort(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    this->nPort = nPort;
}

/*

Remarks
 RFC5626_FLOW_CONTROL

*/
PRIVATE VIRTUAL void RegParameter::SetPortFlowControl(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    nPortFlowControl = nPort;
}

/*

Remarks

*/
PRIVATE
void RegParameter::SetSIPTimerValues(IN CONST SipTimerValues& objTVs)
{
    //---------------------------------------------------------------------------------------------

    if (pSIPTVs != IMS_NULL)
    {
        delete pSIPTVs;
        pSIPTVs = IMS_NULL;
    }

    pSIPTVs = new SipTimerValues(objTVs);
}

/*

Remarks
 MULTI_REG_TRANSPORT
*/
PRIVATE
void RegParameter::SetTransportExt(IN IMS_SINT32 nTransportExt)
{
    IMS_BOOL bIPSec = ((this->nTransportExt & Sip::TRANSPORT_EXT_IPSEC) != 0);
    IMS_BOOL bIPSecUDPEnc = ((this->nTransportExt & Sip::TRANSPORT_EXT_IPSEC_UDP_ENC) != 0);

    //---------------------------------------------------------------------------------------------

    this->nTransportExt = nTransportExt;

    if (bIPSec)
    {
        this->nTransportExt |= Sip::TRANSPORT_EXT_IPSEC;
    }

    if (bIPSecUDPEnc)
    {
        this->nTransportExt |= Sip::TRANSPORT_EXT_IPSEC_UDP_ENC;
    }
}

/*

Remarks

*/
PRIVATE
void RegParameter::SetTransportExtForRegOnly(IN IMS_SINT32 nTransportExt)
{
    //---------------------------------------------------------------------------------------------

    nTransportExtForRegOnly = nTransportExt;
}

/*

Remarks

*/
PRIVATE
void RegParameter::ChoosePreferredSecurityClient()
{
    //---------------------------------------------------------------------------------------------

    if (pPreferredSecurityClient != IMS_NULL)
    {
        delete pPreferredSecurityClient;
        pPreferredSecurityClient = IMS_NULL;
    }

    if (objSecurityClients.IsEmpty())
        return;

    if (objSecurityClients.GetSize() == 1)
    {
        pPreferredSecurityClient = new SipSecurityHeader(objSecurityClients.GetAt(0));

        return;
    }

    if (pPreferredSecurityServer == IMS_NULL)
    {
        pPreferredSecurityClient = new SipSecurityHeader(objSecurityClients.GetAt(0));

        IMS_TRACE_D("There is no preferred security-server, "
                    "so choose the topmost security-client",
                0, 0, 0);
        return;
    }

    SipSecurityHeader* pTmpPreferredHeader = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objSecurityClients.GetSize(); ++i)
    {
        const SipSecurityHeader& objClientHeader = objSecurityClients.GetAt(i);

        if (objClientHeader.GetMechanism() != pPreferredSecurityServer->GetMechanism())
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
        pPreferredSecurityClient = new SipSecurityHeader(*pTmpPreferredHeader);

        delete pTmpPreferredHeader;
    }
    else
    {
        IMS_TRACE_E(0, "No matched Security-Client header", 0, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE
void RegParameter::ChoosePreferredSecurityServer()
{
    //---------------------------------------------------------------------------------------------

    if (pPreferredSecurityServer != IMS_NULL)
    {
        delete pPreferredSecurityServer;
        pPreferredSecurityServer = IMS_NULL;
    }

    if (objSecurityServers.IsEmpty())
        return;

    if (objSecurityServers.GetSize() == 1)
    {
        pPreferredSecurityServer = new SipSecurityHeader(objSecurityServers.GetAt(0));

        return;
    }

    // Collect the mechanism from the Security-Clients
    SipSecurityHeader* pTmpPreferredHeader = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objSecurityServers.GetSize(); ++i)
    {
        IMS_BOOL bSupportedHeaderFound = IMS_FALSE;
        const SipSecurityHeader& objServerHeader = objSecurityServers.GetAt(i);

        for (IMS_UINT32 j = 0; j < objSecurityClients.GetSize(); ++j)
        {
            const SipSecurityHeader& objClientHeader = objSecurityClients.GetAt(j);

            if (objClientHeader.GetMechanism() == objServerHeader.GetMechanism())
            {
                bSupportedHeaderFound = IMS_TRUE;
                break;
            }
        }

        if (!bSupportedHeaderFound)
            continue;

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
        pPreferredSecurityServer = new SipSecurityHeader(*pTmpPreferredHeader);

        delete pTmpPreferredHeader;
    }
    else
    {
        IMS_TRACE_E(0, "No matched Security-Server header", 0, 0, 0);
    }
}

/*

Remarks
 MULTI_SUBS
*/
PRIVATE GLOBAL const ImsSubscriberInfo* RegParameter::GetImsSubscriberInfo(IN IMS_SINT32 nSlotId,
        IN CONST SipAddress& objAOR, IN CONST AString& strSubsId /* = AString::ConstNull() */)
{
    const SubscriberConfig* pSubscriberConfig = IMS_NULL;
    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    //---------------------------------------------------------------------------------------------

    if (strSubsId.GetLength() > 0)
    {
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strSubsId, nSlotId);
    }
    else
    {
        const AString& strId = SubscriberTracker::GetInstance()->GetSubscriberId(nSlotId, &objAOR);
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strId, nSlotId);
    }

    if (pSubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "SubscriberConfig is null; subsId=%s", strSubsId.GetStr(), 0, 0);
        return IMS_NULL;
    }

    IMS_SINT32 nCount = pSubscriberConfig->GetSubscriberCount();
    SipAddress objIMPU;

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        ImsSubscriberInfo* pSubsInfo = pSubscriberConfig->GetSubscriberInfoEx(i);

        if (pSubsInfo == IMS_NULL)
            continue;

        const AStringArray& objIMPUs = pSubsInfo->GetPublicUserIds();

        for (IMS_SINT32 j = 0; j < objIMPUs.GetCount(); ++j)
        {
            const AString& strIMPU = objIMPUs.GetElementAt(j);

            if (objIMPU.Create(strIMPU))
            {
                if (objIMPU.Equals(objAOR))
                {
                    return pSubsInfo;
                }
            }
        }
    }

    // not found
    return IMS_NULL;
}
