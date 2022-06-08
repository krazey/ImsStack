/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091201  toastops@                 Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IMSLib.h"
#include "private/ConfigurationManager.h"
#include "private/SipConfigV.h"
#include "private/MediaConfig.h"
#include "private/CapProperty.h"
#include "SdpSessionDescription.h"
#include "SdpMediaDescription.h"
#include "ServiceProtocol.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "Sip.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipConfigProxy.h"
#include "base/IMS.h"
#include "IMSCore.h"
#include "ServiceManager.h"
#include "Service.h"
#include "util/CallerCapability.h"
#include "IOnCapabilitiesListener.h"
#include "RemoteCapabilities.h"
#include "Capabilities.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL const IMS_CHAR Capabilities::DEFAULT_MEDIA_TYPE[] = "application/sdp";

PUBLIC
Capabilities::Capabilities(IN Service* pService_) :
        ServiceMethod(pService_),
        nState(STATE_INACTIVE),
        objRemoteUserIdentities(IMSList<AString>()),
        piListener(IMS_NULL),
        pRemoteCapabilities(IMS_NULL)
{
}

PUBLIC VIRTUAL Capabilities::~Capabilities()
{
    //---------------------------------------------------------------------------------------------

    if (pRemoteCapabilities != IMS_NULL)
    {
        delete pRemoteCapabilities;
        pRemoteCapabilities = IMS_NULL;
    }

    objRemoteUserIdentities.Clear();
}

/*

Remarks

*/
PUBLIC VIRTUAL void Capabilities::Destroy()
{
    //---------------------------------------------------------------------------------------------

    ServiceMethod::Destroy();
}

/*

Remarks

*/
PUBLIC
IMSList<AString> Capabilities::GetRemoteUserIdentities() const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMSList<AString>();
    }

    return objRemoteUserIdentities;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 Capabilities::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Checks if the remote endpoint has the needed capabilities
of the IMS application with the given argument.

*/
PUBLIC VIRTUAL IMS_BOOL Capabilities::HasCapabilities(IN CONST AString& strConnection) const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FALSE;
    }

    if (strConnection.IsNULL())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    if (pRemoteCapabilities == IMS_NULL)
    {
        IMS_TRACE_E(0, "No remote capabilities", 0, 0, 0);

        IMS::SetLastError(IMSError::NOT_FOUND);
        return IMS_FALSE;
    }

    AString strAppId;
    AString strServiceId;

    // Parse the connection string and validate it.
    if (!ParseConnectionName(strConnection, strAppId, strServiceId))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    // Gets the configured application ids and checks if the app id exists or not
    AStringArray objAppIds = pConfigMngr->GetAppIds(GetSlotId());

    if (objAppIds.IsEmpty())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    IMS_BOOL bFound = IMS_FALSE;

    for (IMS_SINT32 i = 0; i < objAppIds.GetCount(); ++i)
    {
        if (strAppId.EqualsIgnoreCase(objAppIds.GetElementAt(i)))
        {
            bFound = IMS_TRUE;
            break;
        }
    }

    if (!bFound)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "AppId (%s) can't be found from the configuration manager",
                strAppId.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    const AppConfig* pAppConfig = pConfigMngr->GetAppConfig(strAppId, GetSlotId());

    // Check if the capabilities are compatible with the specified application.
    IMS_BOOL bSupported = pRemoteCapabilities->IsCompatible(pAppConfig, strServiceId);

    if (bSupported)
    {
        IMS_TRACE_I("APP (%s, %s) IS COMPATIBLE", strAppId.GetStr(), strServiceId.GetStr(), 0);
    }
    else
    {
        IMS_TRACE_I("APP (%s, %s) IS NOT COMPATIBLE", strAppId.GetStr(), strServiceId.GetStr(), 0);
    }

    return bSupported;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Capabilities::QueryCapabilities(IN IMS_BOOL bSDPInRequest,
        IN IMS_BOOL bContactInRequest /* = IMS_TRUE */, IN IMS_BOOL bCheckSupport /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INACTIVE)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // Create new connection and get SipMessage
    ISipDialog* piDialog = GetDialog();
    ISipClientConnection* piSCC = IMS_NULL;

    if (piDialog == IMS_NULL)
    {
        piSCC = CreateConnection(SipMethod(SipMethod::OPTIONS));
    }
    else
    {
        piSCC = CreateConnection(piDialog, SipMethod(SipMethod::OPTIONS));
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating an OPTIONS SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

#if 0
    // Sets Expires header
    if (piSIPMsg->SetHeader(ISipHeader::EXPIRES_SEC, "0") != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
        goto EXIT_QueryCapabilities;
    }

    // Sets Accept header
    if (piSIPMsg->PrependHeader(ISipHeader::ACCEPT, DEFAULT_MEDIA_TYPE) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Accept header failed", 0, 0, 0);
        goto EXIT_QueryCapabilities;
    }
#endif

    // Sets Contact header with all the service's capability in this device.
    if (bContactInRequest)
    {
        AString strContactHeader;
        IMS_BOOL bIsContactGRUU = IMS_FALSE;

        if (CreateContactHeader(strContactHeader, bIsContactGRUU))
        {
            if (piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
                goto EXIT_QueryCapabilities;
            }

            const AString strGRUU(Sip::STR_GRUU);

            if (bIsContactGRUU && !piSIPMsg->IsOptionSupported(strGRUU))
            {
                piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
            }
        }
    }

    // Set SDP
    if (bSDPInRequest)
    {
        // Create SDP for the response to OPTIONS request
        AString strSDP;

        if (!CreateSDP(strSDP, bCheckSupport, IMS_TRUE))
        {
            IMS_TRACE_E(0, "Creating SDP message for OPTIONS request failed", 0, 0, 0);
            goto EXIT_QueryCapabilities;
        }

        ISipMessageBodyPart* piSIPBodyPart = piSIPMsg->CreateSdpBodyPart();

        if (piSIPBodyPart == IMS_NULL)
        {
            goto EXIT_QueryCapabilities;
        }

        piSIPBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, DEFAULT_MEDIA_TYPE);
        piSIPBodyPart->SetContent(strSDP);
    }

    // Send OPTIONS request to the network
    if (!SendNUpdateRequest(IMessage::CAPABILITIES_QUERY, piSCC))
    {
        goto EXIT_QueryCapabilities;
    }

    SetState(STATE_PENDING);

    return IMS_SUCCESS;

EXIT_QueryCapabilities:
    piSCC->Close();
    return IMS_FAILURE;
}

PUBLIC
IMS_RESULT Capabilities::QueryCapabilitiesEx()
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INACTIVE)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // Create new connection and get SipMessage
    ISipDialog* piDialog = GetDialog();
    ISipClientConnection* piSCC = IMS_NULL;

    if (piDialog == IMS_NULL)
    {
        piSCC = CreateConnection(SipMethod(SipMethod::OPTIONS));
    }
    else
    {
        piSCC = CreateConnection(piDialog, SipMethod(SipMethod::OPTIONS));
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating an OPTIONS SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Send OPTIONS request to the network
    if (!SendNUpdateRequest(IMessage::CAPABILITIES_QUERY, piSCC))
    {
        goto EXIT_QueryCapabilities;
    }

    SetState(STATE_PENDING);

    return IMS_SUCCESS;

EXIT_QueryCapabilities:
    piSCC->Close();
    return IMS_FAILURE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void Capabilities::SetListener(IN IOnCapabilitiesListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT Capabilities::Accept(
        IN IMS_BOOL bFeatureInContact /* = IMS_TRUE */, IN IMS_BOOL bCheckSupport /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PENDING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSSC = GetServerConnection(IMessage::CAPABILITIES_QUERY);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Send a 200 OK to OPTIONS request
    if (CreateResponse(piSSC, SipStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
    }

    // Now, the variable, piSIPMsg will be used for OPTIONS response message
    ISipMessage* piSIPMsg = piSSC->GetMessage();

    // Sets Accept header
    if (piSIPMsg->PrependHeader(ISipHeader::ACCEPT, DEFAULT_MEDIA_TYPE) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Accept header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    // Sets Contact header with all the service's capability in this device.
    AString strContactHeader;
    IMS_BOOL bIsContactGRUU = IMS_FALSE;

    if (CreateContactHeader(strContactHeader, bIsContactGRUU, bFeatureInContact))
    {
        if (piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        const AString strGRUU(Sip::STR_GRUU);

        if (bIsContactGRUU && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    // Create SDP for the response to OPTIONS request
    AString strSDP;

    if (!CreateSDP(strSDP, bCheckSupport))
    {
        IMS_TRACE_E(0, "Creating SDP message for OPTIONS response failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessageBodyPart* piSIPBodyPart = piSIPMsg->CreateSdpBodyPart();

    if (piSIPBodyPart == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    piSIPBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, DEFAULT_MEDIA_TYPE);
    piSIPBodyPart->SetContent(strSDP);

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSSC))
    {
        IMS_TRACE_E(0, "Accepting Capabilities failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetState(STATE_ACTIVE);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT Capabilities::AcceptEx()
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PENDING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSSC = GetServerConnection(IMessage::CAPABILITIES_QUERY);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Send a 200 OK to OPTIONS request
    if (CreateResponse(piSSC, SipStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
    }

    // Accept header
    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSSC))
    {
        IMS_TRACE_E(0, "Accepting Capabilities failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetState(STATE_ACTIVE);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT Capabilities::Reject(
        IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PENDING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSSC = GetServerConnection(IMessage::CAPABILITIES_QUERY);

    if (piSSC == IMS_NULL)
        return IMS_FAILURE;

    // Send a failure final response to OPTIONS request
    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
    }

    // Now, the variable, piSIPMsg will be used for OPTIONS response message
    ISipMessage* piSIPMsg = piSSC->GetMessage();

    if (nRetryAfter > 0)
    {
        AString strRetryAfter;

        strRetryAfter.SetNumber(nRetryAfter);

        piSIPMsg->AddHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfter);
    }

    // Accept headers
    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    // Sets Contact header with all the service's capability in this device.
    AString strContactHeader;
    IMS_BOOL bIsContactGRUU = IMS_FALSE;

    if (CreateContactHeader(strContactHeader, bIsContactGRUU, IMS_FALSE))
    {
        if (piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        const AString strGRUU(Sip::STR_GRUU);

        if (bIsContactGRUU && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSSC))
    {
        IMS_TRACE_E(0, "Rejecting Capabilities failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetState(STATE_INACTIVE);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    return IMS_SUCCESS;
}

/*
 This method handles the incoming OPTIONS request within a dialog.
This OPTIONS request received within a dialog generates a 200 (OK) response
that is identical to one constructed outside a dialog and does not have
any impact on the dialog.

Remarks

*/
PUBLIC GLOBAL IMS_RESULT Capabilities::HandleOPTIONSRequestWithinDialog(
        IN Service* pService, IN CONST Method* pOwnerMethod, IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    if ((pService == IMS_NULL) || (piSSC == IMS_NULL))
    {
        return IMS_FAILURE;
    }

    Capabilities* pCapabilities = new Capabilities(pService);

    if (pCapabilities == IMS_NULL)
    {
        pService->SendResponse(piSSC, SipStatusCode::SC_488);
        piSSC->Close();

        IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (!pCapabilities->InitMethod(pOwnerMethod, IMS_FALSE))
    {
        pService->SendResponse(piSSC, SipStatusCode::SC_488);
        piSSC->Close();
        delete pCapabilities;

        IMS_TRACE_E(0, "Initializing Capabilities failed", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (!pCapabilities->ServerConnection_NotifyRequest(piSSC))
    {
        delete pCapabilities;

        IMS_TRACE_E(0, "Handling Capabilities failed", 0, 0, 0);
        return IMS_SUCCESS;
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Capabilities::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_CAPABILITY_QUERY_RECEIVED:
            GetService()->HandleCapabilityQueryReceived(this);
            return IMS_TRUE;

        case AMSG_CAPABILITY_QUERY_DELIVERED:
            if (piListener != IMS_NULL)
            {
                piListener->OnCapabilities_QueryDelivered(this);
            }
            return IMS_TRUE;

        case AMSG_CAPABILITY_QUERY_DELIVERY_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->OnCapabilities_QueryDeliveryFailed(this);
            }
            return IMS_TRUE;

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PRIVATE VIRTUAL IMS_BOOL Capabilities::SendRequestToChallenge(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Clear the connection to preserve the SIP connection
    ClearConnection(IMessage::CAPABILITIES_QUERY);

    if (!SendNUpdateRequestEx(IMessage::CAPABILITIES_QUERY, piSCC, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(IMessage::CAPABILITIES_QUERY, piSCC);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Capabilities::NotifySIPRequest(IN ISipServerConnection* piSSC)
{
    ISipMessage* piSIPMsg = piSSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Capabilities - OPTIONS REQUEST RECEIVED ...", 0, 0, 0);

    if (!UpdateRequestOnReceived(IMessage::CAPABILITIES_QUERY, piSSC))
    {
        IMS_TRACE_E(0, "Updating SIP message failed", 0, 0, 0);
    }

    // Check if the response needs to be handled by application.
    const SipConfigV* pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->IsCapabilitiesRespByApp())
        {
            IMS_TRACE_I("INCOMING OPTIONS REQUEST WILL BE HANDLED BY APPLICATION", 0, 0, 0);

            PostMessage(AMSG_CAPABILITY_QUERY_RECEIVED, 0, 0);

            SetState(STATE_PENDING);

            return IMS_TRUE;
        }
    }

    AString strContactHeader;
    AString strSDP;
    ISipMessageBodyPart* piSIPBodyPart;
    IMS_BOOL bIsContactGRUU = IMS_FALSE;

    // Send a 200 OK to OPTIONS request
    if (CreateResponse(piSSC, SipStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
        goto EXIT_NotifySIPRequest;
    }

    // Now, the variable, piSIPMsg will be used for OPTIONS response message
    piSIPMsg = piSSC->GetMessage();

    // Accept headers
    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    // Sets Contact header with all the service's capability in this device.

    if (CreateContactHeader(strContactHeader, bIsContactGRUU))
    {
        if (piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            goto EXIT_NotifySIPRequest;
        }

        const AString strGRUU(Sip::STR_GRUU);

        if (bIsContactGRUU && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    // Create SDP for the response to OPTIONS request
    if (!CreateSDP(strSDP))
    {
        IMS_TRACE_E(0, "Creating SDP message for OPTIONS response failed", 0, 0, 0);
        goto EXIT_NotifySIPRequest;
    }

    piSIPBodyPart = piSIPMsg->CreateSdpBodyPart();

    if (piSIPBodyPart == IMS_NULL)
    {
        goto EXIT_NotifySIPRequest;
    }

    piSIPBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, DEFAULT_MEDIA_TYPE);
    piSIPBodyPart->SetContent(strSDP);

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSSC))
    {
        IMS_TRACE_E(0, "Accepting Capabilities by engine failed", 0, 0, 0);
        goto EXIT_NotifySIPRequest;
    }

    SetState(STATE_ACTIVE);

EXIT_NotifySIPRequest:

    IMS_TRACE_I("INCOMING OPTIONS REQUEST HAS BEEN HANDLED BY ENGINE", 0, 0, 0);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    Destroy();

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Capabilities::NotifySIPResponse(IN ISipClientConnection* piSCC)
{
    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    // Add the response message received
    UpdateResponseOnReceived(IMessage::CAPABILITIES_QUERY, piSCC);

    // Handle the response to OPTIONS request ...
    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return;
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // AUTH_SIP_DIGEST {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return;
        }
        // }
    }

    // Check the status code
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        HandleCapabilities(piSCC);

        CloseConnection(IMessage::CAPABILITIES_QUERY);

        SetState(STATE_ACTIVE);
        PostMessage(AMSG_CAPABILITY_QUERY_DELIVERED, 0, 0);
    }
    else
    {
        CloseConnection(IMessage::CAPABILITIES_QUERY);

        SetState(STATE_INACTIVE);
        PostMessage(AMSG_CAPABILITY_QUERY_DELIVERY_FAILED, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Capabilities::NotifySIPError(
        IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    const SipMethod& objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::OPTIONS))
    {
        piSC->Close();
        return;
    }

    CloseConnection(IMessage::CAPABILITIES_QUERY);

    SetState(STATE_INACTIVE);
    PostMessage(AMSG_CAPABILITY_QUERY_DELIVERY_FAILED, 0, 0);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Capabilities::CreateContactHeader(OUT AString& strContactHeader,
        OUT IMS_BOOL& bIsContactGRUU, IN IMS_BOOL bWithFeature /* = IMS_TRUE */) const
{
    IMSList<Service*> objServices = ServiceManager::GetInstance()->GetServices(GetSlotId());
    CallerCapability objCC(0);

    //---------------------------------------------------------------------------------------------

    if (bWithFeature)
    {
        // Collects the feature parameters for Contact header
        for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
        {
            const Service* pService = objServices.GetAt(i);
            const CallerCapability* pCC = pService->GetCallerCapability();

            if (pCC != IMS_NULL)
            {
                objCC.AddFeatures(pCC);
            }
        }
    }

    bIsContactGRUU = IMS_FALSE;

    AString strContact(AString::ConstNull());
    Service* pService = GetService();
    IMS_BOOL bGRUUSupported =
            SipConfigProxy::IsGruuConfigured(GetSlotId(), GetService()->GetSipProfile());

    if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), GetService()->GetSipProfile()))
    {
        SipAddress objContact;
        // 4 Consider the Privacy information (temp-gruu)
        const SipAddress* pPubGRUU = bGRUUSupported ? pService->GetPublicGruu() : IMS_NULL;

        if (pPubGRUU != IMS_NULL)
        {
            bIsContactGRUU = IMS_TRUE;
            objContact = *pPubGRUU;
        }
        else
        {
            const SipAddress* pContact = pService->GetContactAddressForOutgoingMessage();

            objContact = (pContact != IMS_NULL) ? *pContact : pService->GetContactAddress();
        }

        if (SipConfigProxy::IsMultipleRegConfigured(
                    pService->GetSlotId(), pService->GetSipProfile()))
        {
            objContact.AddParameter(Sip::STR_OB, AString::ConstNull());
        }

        strContact = objContact.ToString();
    }
    else
    {
        // 4 Consider the Privacy information (temp-gruu)
        const SipAddress* pPubGRUU = bGRUUSupported ? pService->GetPublicGruu() : IMS_NULL;

        if (pPubGRUU != IMS_NULL)
        {
            bIsContactGRUU = IMS_TRUE;
            strContact = pPubGRUU->ToString();
        }
        else
        {
            const SipAddress* pContact = pService->GetContactAddressForOutgoingMessage();

            strContact = (pContact != IMS_NULL) ? pContact->ToString()
                                                : pService->GetContactAddress().ToString();
        }
    }

    //// Constructs the Contact header with contact-feature parameters
    strContactHeader.Append(strContact);

    if (!objCC.IsEmpty())
    {
        strContactHeader.Append(TextParser::CHAR_SEMICOLON);
        strContactHeader.Append(objCC.ToString());
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Capabilities::CreateSDP(OUT AString& strSDP, IN IMS_BOOL bCheckSupport /* = IMS_TRUE */,
        IN IMS_BOOL bRequest /* = IMS_FALSE */) const
{
    IPAddress objLocalAddress = GetService()->GetIpAddress();

    //---------------------------------------------------------------------------------------------

    if (objLocalAddress.Equals(IPAddress::NONE))
    {
        objLocalAddress = IPAddress::LOOPBACK;
    }

    SdpSessionDescription objSessionDesc;

    // Create a session-level mandatory descriptions
    if (!objSessionDesc.CreateMandatoryLines(GetUserAOR()->GetUri(), objLocalAddress))
    {
        IMS_TRACE_E(0, "Creating a session descriptor failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bStreamAudioSupported = IMS_FALSE;
    IMS_BOOL bStreamVideoSupported = IMS_FALSE;
    IMS_BOOL bFramedMediaSupported = IMS_FALSE;
    IMS_BOOL bFramedMediaMaxSizePresent = IMS_FALSE;
    AStringArray objAcceptTypes;
    IMS_UINT32 nMaxSize = 0;
    SdpDescription objSessionSDPFields;
    SdpDescription objFramedSDPFields;
    SdpDescription objAudioSDPFields;
    SdpDescription objVideoSDPFields;
    IMSList<Service*> objServices = ServiceManager::GetInstance()->GetServices(GetSlotId());

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service* pService = objServices.GetAt(i);
        AppConfig* pAppConfig = pService->GetAppConfig();

        // Collects SDP fields from IMS registry for session-level properties
        CollectSDPFieldsFromRegistry(
                pAppConfig, bRequest, CapProperty::SECTOR_SESSION, objSessionSDPFields);

        if ((!bCheckSupport) || (bCheckSupport && pAppConfig->IsStreamMediaAudioSupported()))
        {
            bStreamAudioSupported = IMS_TRUE;

            // Collects SDP fields from IMS registry for media-level properties
            // (StreamMedia - audio)
            CollectSDPFieldsFromRegistry(
                    pAppConfig, bRequest, CapProperty::SECTOR_STREAM_AUDIO, objAudioSDPFields);
        }

        if ((!bCheckSupport) || (bCheckSupport && pAppConfig->IsStreamMediaVideoSupported()))
        {
            bStreamVideoSupported = IMS_TRUE;

            // Collects SDP fields from IMS registry for media-level properties
            // (StreamMedia - video)
            CollectSDPFieldsFromRegistry(
                    pAppConfig, bRequest, CapProperty::SECTOR_STREAM_VIDEO, objVideoSDPFields);
        }

        if ((!bCheckSupport) || (bCheckSupport && pAppConfig->IsFramedMediaSupported()))
        {
            bFramedMediaSupported = IMS_TRUE;

            const AStringArray& objMimeTypes = pAppConfig->GetFramedMediaMimeTypes();

            for (IMS_SINT32 j = 0; j < objMimeTypes.GetCount(); ++j)
            {
                const AString& strElement = objMimeTypes.GetElementAt(j);

                if (!objAcceptTypes.Contains(strElement, IMS_FALSE))
                {
                    objAcceptTypes.AddElement(strElement);
                }
            }

            if (pAppConfig->IsFramedMediaMaxSizePresent())
            {
                bFramedMediaMaxSizePresent = IMS_TRUE;

                if (nMaxSize == 0)
                {
                    nMaxSize = pAppConfig->GetFramedMediaMaxSize();
                }
                else
                {
                    nMaxSize = IMS_MIN(nMaxSize, pAppConfig->GetFramedMediaMaxSize());
                }
            }

            // Collects SDP fields from IMS registry for media-level properties (FramedMedia)
            CollectSDPFieldsFromRegistry(
                    pAppConfig, bRequest, CapProperty::SECTOR_FRAMED, objFramedSDPFields);
        }
    }

    //// Copy the collected SDP fields to the SdpSessionDescription
    CopySDPFields(objSessionSDPFields, objSessionDesc);

    // Form a SdpDescription to the raw SDP format
    strSDP.Append(objSessionDesc.Encode());

    const MediaConfig* pMediaConfig =
            ConfigurationManager::GetInstance()->GetMediaConfig(GetSlotId());

    // Form a SDP in case of StreamAudio
    if (bStreamAudioSupported)
    {
        SdpMediaDescription objStreamAudioDesc;
        const AStringArray& objAudioCap =
                pMediaConfig->GetMediaCapabilities(IMediaConfig::STREAM_AUDIO);

        if (!objStreamAudioDesc.Decode(objAudioCap))
        {
            IMS_TRACE_E(0, "Decoding a StreamMedia(Audio) description failed", 0, 0, 0);
            return IMS_FALSE;
        }

        //// Copy the collected SDP fields to the SdpMediaDescription for StreamMedia (audio)
        CopySDPFields(objAudioSDPFields, objStreamAudioDesc);

        // Form a SdpDescription to the raw SDP format
        strSDP.Append(objStreamAudioDesc.Encode());
    }

    // Form a SDP in case of StreamVideo
    if (bStreamVideoSupported)
    {
        SdpMediaDescription objStreamVideoDesc;
        const AStringArray& objVideoCap =
                pMediaConfig->GetMediaCapabilities(IMediaConfig::STREAM_VIDEO);

        if (!objStreamVideoDesc.Decode(objVideoCap))
        {
            IMS_TRACE_E(0, "Decoding a StreamMedia(Video) description failed", 0, 0, 0);
            return IMS_FALSE;
        }

        //// Copy the collected SDP fields to the SdpMediaDescription for StreamMedia (video)
        CopySDPFields(objVideoSDPFields, objStreamVideoDesc);

        // Form a SdpDescription to the raw SDP format
        strSDP.Append(objStreamVideoDesc.Encode());
    }

    // Form a SDP in case of Framed
    if (bFramedMediaSupported)
    {
        SdpMediaDescription objFramedMediaDesc;
        const AStringArray& objFramedCap = pMediaConfig->GetMediaCapabilities(IMediaConfig::FRAMED);

        if (!objFramedMediaDesc.Decode(objFramedCap))
        {
            IMS_TRACE_E(0, "Decoding a FramedMedia description failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Add the attributes - accept-types
        if (!objAcceptTypes.IsEmpty())
        {
            AString strAcceptTypes = objAcceptTypes.GetElementAt(0);

            for (IMS_SINT32 i = 1; i < objAcceptTypes.GetCount(); ++i)
            {
                strAcceptTypes.Append(TextParser::CHAR_SP);
                strAcceptTypes.Append(objAcceptTypes.GetElementAt(i));
            }

            SdpAttribute objAttr;

            objAttr.SetValue(SdpAttribute::ACCEPT_TYPES, strAcceptTypes);
            objFramedMediaDesc.AddAttribute(objAttr);
        }

        // Add the attribute ("max-size") if present
        if (bFramedMediaMaxSizePresent)
        {
            SdpAttribute objAttr;
            AString strMaxSize;

            strMaxSize.SetNumber(nMaxSize);

            objAttr.SetValue(SdpAttribute::MAX_SIZE, strMaxSize);
            objFramedMediaDesc.AddAttribute(objAttr);
        }

        //// Copy the collected SDP fields to the SdpMediaDescription for FramedMedia
        CopySDPFields(objFramedSDPFields, objFramedMediaDesc);

        // Form a SdpDescription to the raw SDP format
        strSDP.Append(objFramedMediaDesc.Encode());
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void Capabilities::HandleCapabilities(IN ISipClientConnection* piSCC)
{
    ISipMessage* piSIPMsg = piSCC->GetMessage();

    //---------------------------------------------------------------------------------------------

    // Gets the Contact headers
    IMSList<AString> objContacts = piSIPMsg->GetHeaders(ISipHeader::CONTACT_ANY);

    if (objContacts.IsEmpty())
    {
        IMS_TRACE_D("Capabilities :: no contacts", 0, 0, 0);
        return;
    }

    // Collects the remote user identities from Contact headers in SIP message
    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        const AString& strContact = objContacts.GetAt(i);
        ISipHeader* piContact =
                SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, strContact);

        if (piContact != IMS_NULL)
        {
            const SipAddress* pAddress = piContact->GetSipAddress();

            if (pAddress != IMS_NULL)
            {
                const AString& strScheme = pAddress->GetScheme();

                // Collects all SIP & TEL URIs ...
                if (strScheme.EqualsIgnoreCase(Sip::STR_SIP) ||
                        strScheme.EqualsIgnoreCase(Sip::STR_SIPS) ||
                        strScheme.EqualsIgnoreCase(Sip::STR_TEL))
                {
                    objRemoteUserIdentities.Append(pAddress->ToString());
                }
            }

            piContact->Destroy();
        }
    }

    // Sets the remote capabilities
    // If a multiple contact headers exist, then evaluate all the contacts.
    // If q-value is present, choose a contact header according to the 'q' value.
    // If q-value is not present, choose the topmost contact header with "SIP" URI scheme.
    const AString& strContactHeader = objContacts.GetAt(0);
    IMSList<AString> objTokens;
    IMS_SINT32 nStartIndex = strContactHeader.GetIndexOf(TextParser::CHAR_RAQUOT);

    if (nStartIndex == AString::NPOS)
    {
        objTokens = strContactHeader.Split(TextParser::CHAR_SEMICOLON);

        // Delete the first element (header value - Contact URI; it's not header parameter)
        objTokens.RemoveAt(0);
    }
    else
    {
        nStartIndex = strContactHeader.GetIndexOf(TextParser::CHAR_SEMICOLON, nStartIndex);

        if (nStartIndex != AString::NPOS)
        {
            objTokens =
                    strContactHeader.GetSubStr(nStartIndex + 1).Split(TextParser::CHAR_SEMICOLON);
        }
    }

    if (pRemoteCapabilities != IMS_NULL)
    {
        delete pRemoteCapabilities;
        pRemoteCapabilities = IMS_NULL;
    }

    pRemoteCapabilities = new RemoteCapabilities();

    if (pRemoteCapabilities == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating RemoteCapabilties failed", 0, 0, 0);
        return;
    }

    if (!pRemoteCapabilities->Create(objTokens))
    {
        IMS_TRACE_E(0, "Creating a remote capabilities failed", 0, 0, 0);
        return;
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Capabilities::ParseConnectionName(
        IN CONST AString& strConnection, OUT AString& strAppId, OUT AString& strServiceId) const
{
    AString strServiceType;
    AString strParams;

    //---------------------------------------------------------------------------------------------

    Protocol::ParseName(strConnection, strServiceType, strAppId, strParams);

    if (!strServiceType.EqualsIgnoreCase(GetService()->GetScheme()))
    {
        IMS_TRACE_E(0, "Illegal URI scheme - Connection (%s), Scheme (%s)", strConnection.GetStr(),
                strServiceType.GetStr(), 0);
        return IMS_FALSE;
    }

    if (!strAppId.StartsWith("//"))
    {
        IMS_TRACE_E(0, "Connection string is malformed after URI scheme - Connection (%s)",
                strConnection.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    strAppId = strAppId.GetSubStr(2);

    // If 'serviceId' parameter is present, extract the parameter value from the connection string
    IMSList<AString> objTokens = strParams.Split(TextParser::CHAR_SEMICOLON);

    if (objTokens.IsEmpty())
    {
        strServiceId = AString::ConstEmpty();
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
        {
            const AString& strToken = objTokens.GetAt(i);
            IMS_SINT32 nStartIndex = strToken.GetIndexOf(TextParser::CHAR_EQUAL);

            if (nStartIndex != AString::NPOS)
            {
                AString strName = strToken.GetSubStr(0, nStartIndex);

                if (strName.EqualsIgnoreCase(ServiceProtocol::CONNECTION_PARAM_SERVICE_ID))
                {
                    strServiceId = strToken.GetSubStr(nStartIndex + 1);
                    break;
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
void Capabilities::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Capabilities :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE GLOBAL void Capabilities::CollectSDPFieldsFromRegistry(IN CONST AppConfig* pAppConfig,
        IN IMS_BOOL bRequest, IN IMS_SINT32 nSector, IN_OUT SdpDescription& objSDPDesc)
{
    AStringArray objSDPLines;

    //---------------------------------------------------------------------------------------------

    // Collects session-level/media-level SDP lines

    objSDPLines =
            pAppConfig->GetCapabilitySDPs(nSector, CapProperty::MESSAGE_TYPE_REQUEST_RESPONSE);
    SetSDPFields(objSDPLines, objSDPDesc);

    if (bRequest)
    {
        // Clear the previous elements
        objSDPLines.RemoveAllElements();

        // Gets SDP fields for outgoing OPTIONS request
        objSDPLines = pAppConfig->GetCapabilitySDPs(nSector, CapProperty::MESSAGE_TYPE_REQUEST);
        SetSDPFields(objSDPLines, objSDPDesc);
    }
    else
    {
        // Clear the previous elements
        objSDPLines.RemoveAllElements();

        // Gets SDP fields for outgoing OPTIONS response
        objSDPLines = pAppConfig->GetCapabilitySDPs(nSector, CapProperty::MESSAGE_TYPE_RESPONSE);
        SetSDPFields(objSDPLines, objSDPDesc);
    }
}

/*

Remarks

*/
PRIVATE GLOBAL void Capabilities::CopySDPFields(
        IN CONST SdpDescription& objSDPDesc, IN_OUT SdpDescription& objConcreteSDPDesc)
{
    //---------------------------------------------------------------------------------------------

    if (objSDPDesc.Contains(Sdp::TYPE_B))
    {
        const IMSList<SdpBandwidth>& objBandwidths = objSDPDesc.GetBandwidths();

        for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBandwidths.GetAt(i);

            objConcreteSDPDesc.AddBandwidth(objBandwidth);
        }
    }

    if (objSDPDesc.Contains(Sdp::TYPE_A))
    {
        const IMSList<SdpAttribute>& objAttributes = objSDPDesc.GetAttributes();

        for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttr = objAttributes.GetAt(i);

            objConcreteSDPDesc.AddAttribute(objAttr);
        }
    }
}

/*

Remarks

*/
PRIVATE GLOBAL void Capabilities::SetSDPFields(
        IN CONST AStringArray& objSDPLines, IN_OUT SdpDescription& objSDPDesc)
{
    SdpDescription objTempDesc;

    //---------------------------------------------------------------------------------------------

    if (!objTempDesc.Decode(objSDPLines))
    {
        IMS_TRACE_E(0, "Decoding SDP fields from Cap property failed", 0, 0, 0);
        return;
    }

    if (objTempDesc.Contains(Sdp::TYPE_B))
    {
        const IMSList<SdpBandwidth>& objBandwidths = objTempDesc.GetBandwidths();

        for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBandwidths.GetAt(i);

            if (!objSDPDesc.Contains(objBandwidth))
            {
                objSDPDesc.AddBandwidth(objBandwidth);
            }
        }
    }

    if (objTempDesc.Contains(Sdp::TYPE_A))
    {
        const IMSList<SdpAttribute>& objAttributes = objTempDesc.GetAttributes();

        for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttr = objAttributes.GetAt(i);

            // Attribute value is compared. Is it OK?????
            if (!objSDPDesc.Contains(objAttr))
            {
                objSDPDesc.AddAttribute(objAttr);
            }
        }
    }
}

PRIVATE GLOBAL const IMS_CHAR* Capabilities::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
        case STATE_INACTIVE:
            return "STATE_INACTIVE";
        case STATE_PENDING:
            return "STATE_PENDING";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        default:
            return "__INVALID__";
    }
}
