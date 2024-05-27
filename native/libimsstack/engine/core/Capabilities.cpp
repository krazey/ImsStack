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
#include "ImsLib.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "private/CapProperty.h"
#include "private/ConfigurationManager.h"
#include "private/MediaConfig.h"
#include "private/SipConfigV.h"

#include "Capabilities.h"
#include "IOnCapabilitiesListener.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImsCore.h"
#include "RemoteCapabilities.h"
#include "SdpSessionDescription.h"
#include "SdpMediaDescription.h"
#include "Service.h"
#include "ServiceContext.h"
#include "ServiceManager.h"
#include "ServiceProtocol.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/Ims.h"
#include "util/CallerCapability.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL const IMS_CHAR Capabilities::DEFAULT_MEDIA_TYPE[] = "application/sdp";

PUBLIC
Capabilities::Capabilities(IN Service* pService) :
        ServiceMethod(pService),
        m_nState(STATE_INACTIVE),
        m_objRemoteUserIdentities(ImsList<AString>()),
        m_piListener(IMS_NULL),
        m_pRemoteCapabilities(IMS_NULL)
{
}

PUBLIC VIRTUAL Capabilities::~Capabilities()
{
    if (m_pRemoteCapabilities != IMS_NULL)
    {
        delete m_pRemoteCapabilities;
        m_pRemoteCapabilities = IMS_NULL;
    }

    m_objRemoteUserIdentities.Clear();
}

PUBLIC
ImsList<AString> Capabilities::GetRemoteUserIdentities() const
{
    if (GetState() != STATE_ACTIVE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return ImsList<AString>();
    }

    return m_objRemoteUserIdentities;
}

/**
 * @brief Checks if the remote endpoint has the needed capabilities
 *        of the IMS application with the given argument.
 */
PUBLIC VIRTUAL IMS_BOOL Capabilities::HasCapabilities(IN const AString& strConnection) const
{
    if (GetState() != STATE_ACTIVE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FALSE;
    }

    if (strConnection.IsNULL())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    if (m_pRemoteCapabilities == IMS_NULL)
    {
        IMS_TRACE_E(0, "No remote capabilities", 0, 0, 0);

        Ims::SetLastError(ImsError::NOT_FOUND);
        return IMS_FALSE;
    }

    AString strAppId;
    AString strServiceId;

    // Parse the connection string and validate it.
    if (!ParseConnectionName(strConnection, strAppId, strServiceId))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    // Gets the configured application ids and checks if the app id exists or not
    AStringArray objAppIds = pConfigMngr->GetAppIds(GetSlotId());

    if (objAppIds.IsEmpty())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
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
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "AppId (%s) can't be found from the configuration manager",
                strAppId.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    const AppConfig* pAppConfig = pConfigMngr->GetAppConfig(strAppId, GetSlotId());

    // Check if the capabilities are compatible with the specified application.
    IMS_BOOL bSupported = m_pRemoteCapabilities->IsCompatible(pAppConfig, strServiceId);

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

PUBLIC
IMS_RESULT Capabilities::QueryCapabilities(IN IMS_SINT32 nFlags /*= FLAG_REQUEST_DEFAULT*/)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INACTIVE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // Create new connection and get SipMessage
    ISipDialog* piDialog = GetDialog();
    ISipClientConnection* piScc = IMS_NULL;

    if (piDialog == IMS_NULL)
    {
        piScc = CreateConnection(SipMethod(SipMethod::OPTIONS));
    }
    else
    {
        piScc = CreateConnection(piDialog, SipMethod(SipMethod::OPTIONS));
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating an OPTIONS SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

#if 0
    // Sets Expires header
    if (piSipMsg->SetHeader(ISipHeader::EXPIRES_SEC, "0") != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
        goto EXIT_QueryCapabilities;
    }

    // Sets Accept header
    if (piSipMsg->PrependHeader(ISipHeader::ACCEPT, DEFAULT_MEDIA_TYPE) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Accept header failed", 0, 0, 0);
        goto EXIT_QueryCapabilities;
    }
#endif

    // Sets Contact header with all the service's capability in this device.
    if (HasFlag(nFlags, FLAG_ADD_CONTACT_HEADER))
    {
        AString strContactHeader;
        IMS_BOOL bIsContactGruu = IMS_FALSE;

        if (CreateContactHeader(strContactHeader, bIsContactGruu))
        {
            if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
                goto EXIT_QueryCapabilities;
            }

            const AString strGruu(Sip::STR_GRUU);

            if (bIsContactGruu && !piSipMsg->IsOptionSupported(strGruu))
            {
                piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
            }
        }
    }

    // Set SDP
    if (HasFlag(nFlags, FLAG_ADD_SDP_BODY_PART))
    {
        // Create SDP for the response to OPTIONS request
        AString strSdp;

        if (!CreateSdp(strSdp, HasFlag(nFlags, FLAG_CHECK_MEDIA_CAPABILITIES), IMS_TRUE))
        {
            IMS_TRACE_E(0, "Creating SDP message for OPTIONS request failed", 0, 0, 0);
            goto EXIT_QueryCapabilities;
        }

        ISipMessageBodyPart* piSipBodyPart = piSipMsg->CreateSdpBodyPart();

        if (piSipBodyPart == IMS_NULL)
        {
            goto EXIT_QueryCapabilities;
        }

        piSipBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, DEFAULT_MEDIA_TYPE);
        piSipBodyPart->SetContent(strSdp);
    }

    // Send OPTIONS request to the network
    if (!SendNUpdateRequest(IMessage::CAPABILITIES_QUERY, piScc))
    {
        goto EXIT_QueryCapabilities;
    }

    SetState(STATE_PENDING);

    return IMS_SUCCESS;

EXIT_QueryCapabilities:
    piScc->Close();
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT Capabilities::Accept(IN IMS_SINT32 nFlags /*= FLAG_RESPONSE_DEFAULT*/)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PENDING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::CAPABILITIES_QUERY);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Send a 200 OK to OPTIONS request
    if (CreateResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
    }

    // Now, the variable, piSIPMsg will be used for OPTIONS response message
    ISipMessage* piSipMsg = piSsc->GetMessage();

    if (HasFlag(nFlags, FLAG_ADD_SDP_BODY_PART))
    {
        // Sets Accept header
        if (piSipMsg->PrependHeader(ISipHeader::ACCEPT, DEFAULT_MEDIA_TYPE) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Accept header failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    // Sets Contact header with all the service's capability in this device.
    if (HasFlag(nFlags, FLAG_ADD_CONTACT_HEADER))
    {
        AString strContactHeader;
        IMS_BOOL bIsContactGruu = IMS_FALSE;

        if (CreateContactHeader(strContactHeader, bIsContactGruu,
                    HasFlag(nFlags, FLAG_ADD_ALL_REGISTERED_FEATURES_IN_CONTACT_HEADER)))
        {
            if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
                return IMS_FAILURE;
            }

            const AString strGruu(Sip::STR_GRUU);

            if (bIsContactGruu && !piSipMsg->IsOptionSupported(strGruu))
            {
                piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
            }
        }
    }

    // Create SDP for the response to OPTIONS request
    if (HasFlag(nFlags, FLAG_ADD_SDP_BODY_PART))
    {
        AString strSdp;

        if (!CreateSdp(strSdp, HasFlag(nFlags, FLAG_CHECK_MEDIA_CAPABILITIES)))
        {
            IMS_TRACE_E(0, "Creating SDP message for OPTIONS response failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        ISipMessageBodyPart* piSipBodyPart = piSipMsg->CreateSdpBodyPart();

        if (piSipBodyPart == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        piSipBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, DEFAULT_MEDIA_TYPE);
        piSipBodyPart->SetContent(strSdp);
    }

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSsc))
    {
        IMS_TRACE_E(0, "Accepting Capabilities failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetState(STATE_ACTIVE);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT Capabilities::Reject(
        IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter /*= 0*/)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PENDING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::CAPABILITIES_QUERY);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Send a failure final response to OPTIONS request
    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
    }

    // Now, the variable, piSIPMsg will be used for OPTIONS response message
    ISipMessage* piSipMsg = piSsc->GetMessage();

    if (nRetryAfter > 0)
    {
        AString strRetryAfter;

        strRetryAfter.SetNumber(nRetryAfter);

        piSipMsg->AddHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfter);
    }

    // Accept headers
    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    // Sets Contact header with all the service's capability in this device.
    AString strContactHeader;
    IMS_BOOL bIsContactGruu = IMS_FALSE;

    if (CreateContactHeader(strContactHeader, bIsContactGruu, IMS_FALSE))
    {
        if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        const AString strGruu(Sip::STR_GRUU);

        if (bIsContactGruu && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSsc))
    {
        IMS_TRACE_E(0, "Rejecting Capabilities failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetState(STATE_INACTIVE);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    return IMS_SUCCESS;
}

/**
 * @brief This method handles the incoming OPTIONS request within a dialog.
 *
 * This OPTIONS request received within a dialog generates a 200 (OK) response
 * that is identical to one constructed outside a dialog and does not have
 * any impact on the dialog.
 */
PUBLIC GLOBAL IMS_RESULT Capabilities::HandleOptionsRequestWithinDialog(
        IN Service* pService, IN const Method* pOwnerMethod, IN ISipServerConnection* piSsc)
{
    if ((pService == IMS_NULL) || (piSsc == IMS_NULL))
    {
        return IMS_FAILURE;
    }

    Capabilities* pCapabilities = new Capabilities(pService);

    if (pCapabilities == IMS_NULL)
    {
        pService->SendResponse(piSsc, SipStatusCode::SC_488);
        piSsc->Close();

        IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (!pCapabilities->InitMethod(pOwnerMethod, IMS_FALSE))
    {
        pService->SendResponse(piSsc, SipStatusCode::SC_488);
        piSsc->Close();
        delete pCapabilities;

        IMS_TRACE_E(0, "Initializing Capabilities failed", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (!pCapabilities->ServerConnection_NotifyRequest(piSsc))
    {
        delete pCapabilities;

        IMS_TRACE_E(0, "Handling Capabilities failed", 0, 0, 0);
        return IMS_SUCCESS;
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_BOOL Capabilities::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_CAPABILITY_QUERY_RECEIVED:
            GetService()->HandleCapabilityQueryReceived(this);
            return IMS_TRUE;
        case AMSG_CAPABILITY_QUERY_DELIVERED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnCapabilities_QueryDelivered(this);
            }
            return IMS_TRUE;
        case AMSG_CAPABILITY_QUERY_DELIVERY_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnCapabilities_QueryDeliveryFailed(this);
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL IMS_BOOL Capabilities::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Clear the connection to preserve the SIP connection
    ClearConnection(IMessage::CAPABILITIES_QUERY);

    if (!SendNUpdateRequestEx(IMessage::CAPABILITIES_QUERY, piScc, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(IMessage::CAPABILITIES_QUERY, piScc);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL Capabilities::NotifySipRequest(IN ISipServerConnection* piSsc)
{
    IMS_TRACE_I("Capabilities - OPTIONS REQUEST RECEIVED ...", 0, 0, 0);

    if (!UpdateRequestOnReceived(IMessage::CAPABILITIES_QUERY, piSsc))
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
    AString strSdp;
    ISipMessage* piSipMsg;
    ISipMessageBodyPart* piSipBodyPart;
    IMS_BOOL bIsContactGruu = IMS_FALSE;

    // Send a 200 OK to OPTIONS request
    if (CreateResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to OPTIONS request failed", 0, 0, 0);
        goto EXIT_NotifySipRequest;
    }

    // Now, the variable, piSIPMsg will be used for OPTIONS response message
    piSipMsg = piSsc->GetMessage();

    // Accept headers
    // Accept-Encoding headers
    // Accept-Language headers
    // Supported headers

    // Sets Contact header with all the service's capability in this device.

    if (CreateContactHeader(strContactHeader, bIsContactGruu))
    {
        if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
            goto EXIT_NotifySipRequest;
        }

        const AString strGruu(Sip::STR_GRUU);

        if (bIsContactGruu && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    // Create SDP for the response to OPTIONS request
    if (!CreateSdp(strSdp))
    {
        IMS_TRACE_E(0, "Creating SDP message for OPTIONS response failed", 0, 0, 0);
        goto EXIT_NotifySipRequest;
    }

    piSipBodyPart = piSipMsg->CreateSdpBodyPart();

    if (piSipBodyPart == IMS_NULL)
    {
        goto EXIT_NotifySipRequest;
    }

    piSipBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, DEFAULT_MEDIA_TYPE);
    piSipBodyPart->SetContent(strSdp);

    if (!SendNUpdateResponse(IMessage::CAPABILITIES_QUERY, piSsc))
    {
        IMS_TRACE_E(0, "Accepting Capabilities by engine failed", 0, 0, 0);
        goto EXIT_NotifySipRequest;
    }

    SetState(STATE_ACTIVE);

EXIT_NotifySipRequest:

    IMS_TRACE_I("INCOMING OPTIONS REQUEST HAS BEEN HANDLED BY ENGINE", 0, 0, 0);

    // Destroy SIP transaction
    CloseConnection(IMessage::CAPABILITIES_QUERY);

    Destroy();

    return IMS_TRUE;
}

PRIVATE VIRTUAL void Capabilities::NotifySipResponse(IN ISipClientConnection* piScc)
{
    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    // Add the response message received
    UpdateResponseOnReceived(IMessage::CAPABILITIES_QUERY, piScc);

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
        if (RespondToChallenge(piScc))
        {
            return;
        }
        // }
    }

    // Check the status code
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        HandleCapabilities(piScc);

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

PRIVATE VIRTUAL void Capabilities::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::OPTIONS))
    {
        piSc->Close();
        return;
    }

    CloseConnection(IMessage::CAPABILITIES_QUERY);

    SetState(STATE_INACTIVE);
    PostMessage(AMSG_CAPABILITY_QUERY_DELIVERY_FAILED, 0, 0);
}

PRIVATE
IMS_BOOL Capabilities::CreateContactHeader(OUT AString& strContactHeader,
        OUT IMS_BOOL& bIsContactGruu, IN IMS_BOOL bIncludeAllFeatures /*= IMS_TRUE*/) const
{
    CallerCapability objCc(0);

    if (bIncludeAllFeatures)
    {
        ImsList<Service*> objServices =
                ServiceContext::GetInstance()->GetServiceManager()->GetServices(GetSlotId());

        // Collects the feature parameters for Contact header
        for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
        {
            const Service* pService = objServices.GetAt(i);
            const CallerCapability* pCc = pService->GetCallerCapability();

            if (pCc != IMS_NULL)
            {
                objCc.AddFeatures(pCc);
            }
        }
    }

    bIsContactGruu = IMS_FALSE;

    AString strContact(AString::ConstNull());
    Service* pService = GetService();
    IMS_BOOL bGruuSupported =
            SipConfigProxy::IsGruuConfigured(GetSlotId(), pService->GetSipProfile());

    if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pService->GetSipProfile()))
    {
        SipAddress objContact;
        // 4 Consider the Privacy information (temp-gruu)
        const SipAddress* pPubGruu = bGruuSupported ? pService->GetPublicGruu() : IMS_NULL;

        if (pPubGruu != IMS_NULL)
        {
            bIsContactGruu = IMS_TRUE;
            objContact = *pPubGruu;
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
        const SipAddress* pPubGruu = bGruuSupported ? pService->GetPublicGruu() : IMS_NULL;

        if (pPubGruu != IMS_NULL)
        {
            bIsContactGruu = IMS_TRUE;
            strContact = pPubGruu->ToString();
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

    if (!objCc.IsEmpty())
    {
        strContactHeader.Append(TextParser::CHAR_SEMICOLON);
        strContactHeader.Append(objCc.ToString());
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL Capabilities::CreateSdp(OUT AString& strSdp,
        IN IMS_BOOL bCheckMediaCapability /*= IMS_TRUE*/,
        IN IMS_BOOL bRequest /*= IMS_FALSE*/) const
{
    IpAddress objLocalAddress = GetService()->GetIpAddress();

    if (objLocalAddress.Equals(IpAddress::NONE))
    {
        objLocalAddress = IpAddress::LOOPBACK;
    }

    SdpSessionDescription objSessionDesc;

    // Create a session-level mandatory descriptions
    if (!objSessionDesc.CreateMandatoryLines(GetUserAor()->GetUri(), objLocalAddress))
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
    SdpDescription objSessionSdpFields;
    SdpDescription objFramedSdpFields;
    SdpDescription objAudioSdpFields;
    SdpDescription objVideoSdpFields;
    ImsList<Service*> objServices =
            ServiceContext::GetInstance()->GetServiceManager()->GetServices(GetSlotId());

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service* pService = objServices.GetAt(i);
        AppConfig* pAppConfig = pService->GetAppConfig();

        // Collects SDP fields from IMS registry for session-level properties
        CollectSdpFieldsFromRegistry(
                pAppConfig, bRequest, CapProperty::SECTOR_SESSION, objSessionSdpFields);

        if (!bCheckMediaCapability || pAppConfig->IsStreamMediaAudioSupported())
        {
            bStreamAudioSupported = IMS_TRUE;

            // Collects SDP fields from IMS registry for media-level properties
            // (StreamMedia - audio)
            CollectSdpFieldsFromRegistry(
                    pAppConfig, bRequest, CapProperty::SECTOR_STREAM_AUDIO, objAudioSdpFields);
        }

        if (!bCheckMediaCapability || pAppConfig->IsStreamMediaVideoSupported())
        {
            bStreamVideoSupported = IMS_TRUE;

            // Collects SDP fields from IMS registry for media-level properties
            // (StreamMedia - video)
            CollectSdpFieldsFromRegistry(
                    pAppConfig, bRequest, CapProperty::SECTOR_STREAM_VIDEO, objVideoSdpFields);
        }

        if (!bCheckMediaCapability || pAppConfig->IsFramedMediaSupported())
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
            CollectSdpFieldsFromRegistry(
                    pAppConfig, bRequest, CapProperty::SECTOR_FRAMED, objFramedSdpFields);
        }
    }

    //// Copy the collected SDP fields to the SdpSessionDescription
    CopySdpFields(objSessionSdpFields, objSessionDesc);

    // Form a SdpDescription to the raw SDP format
    strSdp.Append(objSessionDesc.Encode());

    const MediaConfig* pMediaConfig =
            ConfigurationManager::GetInstance()->GetMediaConfig(GetSlotId());

    if (pMediaConfig == IMS_NULL)
    {
        // No media configuration.
        return IMS_TRUE;
    }

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
        CopySdpFields(objAudioSdpFields, objStreamAudioDesc);

        // Form a SdpDescription to the raw SDP format
        strSdp.Append(objStreamAudioDesc.Encode());
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
        CopySdpFields(objVideoSdpFields, objStreamVideoDesc);

        // Form a SdpDescription to the raw SDP format
        strSdp.Append(objStreamVideoDesc.Encode());
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
        CopySdpFields(objFramedSdpFields, objFramedMediaDesc);

        // Form a SdpDescription to the raw SDP format
        strSdp.Append(objFramedMediaDesc.Encode());
    }

    return IMS_TRUE;
}

PRIVATE
void Capabilities::HandleCapabilities(IN ISipClientConnection* piScc)
{
    ISipMessage* piSipMsg = piScc->GetMessage();

    // Gets the Contact headers
    ImsList<AString> objContacts = piSipMsg->GetHeaders(ISipHeader::CONTACT_ANY);

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
                    m_objRemoteUserIdentities.Append(pAddress->ToString());
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
    ImsList<AString> objTokens;
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

    if (m_pRemoteCapabilities != IMS_NULL)
    {
        delete m_pRemoteCapabilities;
        m_pRemoteCapabilities = IMS_NULL;
    }

    m_pRemoteCapabilities = new RemoteCapabilities();

    if (m_pRemoteCapabilities == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating RemoteCapabilities failed", 0, 0, 0);
        return;
    }

    if (!m_pRemoteCapabilities->Create(objTokens))
    {
        IMS_TRACE_E(0, "Creating a remote capabilities failed", 0, 0, 0);
        return;
    }
}

PRIVATE
IMS_BOOL Capabilities::ParseConnectionName(
        IN const AString& strConnection, OUT AString& strAppId, OUT AString& strServiceId) const
{
    AString strServiceType;
    AString strParams;
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
    ImsList<AString> objTokens = strParams.Split(TextParser::CHAR_SEMICOLON);

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

PRIVATE
void Capabilities::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Capabilities :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE GLOBAL void Capabilities::CollectSdpFieldsFromRegistry(IN const AppConfig* pAppConfig,
        IN IMS_BOOL bRequest, IN IMS_SINT32 nSector, IN_OUT SdpDescription& objSdpDesc)
{
    AStringArray objSdpLines;

    // Collects session-level/media-level SDP lines
    objSdpLines =
            pAppConfig->GetCapabilitySdps(nSector, CapProperty::MESSAGE_TYPE_REQUEST_RESPONSE);
    SetSdpFields(objSdpLines, objSdpDesc);

    if (bRequest)
    {
        // Clear the previous elements
        objSdpLines.RemoveAllElements();

        // Gets SDP fields for outgoing OPTIONS request
        objSdpLines = pAppConfig->GetCapabilitySdps(nSector, CapProperty::MESSAGE_TYPE_REQUEST);
        SetSdpFields(objSdpLines, objSdpDesc);
    }
    else
    {
        // Clear the previous elements
        objSdpLines.RemoveAllElements();

        // Gets SDP fields for outgoing OPTIONS response
        objSdpLines = pAppConfig->GetCapabilitySdps(nSector, CapProperty::MESSAGE_TYPE_RESPONSE);
        SetSdpFields(objSdpLines, objSdpDesc);
    }
}

PRIVATE GLOBAL void Capabilities::CopySdpFields(
        IN const SdpDescription& objSdpDesc, IN_OUT SdpDescription& objConcreteSdpDesc)
{
    if (objSdpDesc.Contains(Sdp::TYPE_B))
    {
        const ImsList<SdpBandwidth>& objBandwidths = objSdpDesc.GetBandwidths();

        for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBandwidths.GetAt(i);

            objConcreteSdpDesc.AddBandwidth(objBandwidth);
        }
    }

    if (objSdpDesc.Contains(Sdp::TYPE_A))
    {
        const ImsList<SdpAttribute>& objAttributes = objSdpDesc.GetAttributes();

        for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttr = objAttributes.GetAt(i);

            objConcreteSdpDesc.AddAttribute(objAttr);
        }
    }
}

PRIVATE GLOBAL void Capabilities::SetSdpFields(
        IN const AStringArray& objSdpLines, IN_OUT SdpDescription& objSdpDesc)
{
    SdpDescription objTempDesc;

    if (!objTempDesc.Decode(objSdpLines))
    {
        IMS_TRACE_E(0, "Decoding SDP fields from Cap property failed", 0, 0, 0);
        return;
    }

    if (objTempDesc.Contains(Sdp::TYPE_B))
    {
        const ImsList<SdpBandwidth>& objBandwidths = objTempDesc.GetBandwidths();

        for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBandwidths.GetAt(i);

            if (!objSdpDesc.Contains(objBandwidth))
            {
                objSdpDesc.AddBandwidth(objBandwidth);
            }
        }
    }

    if (objTempDesc.Contains(Sdp::TYPE_A))
    {
        const ImsList<SdpAttribute>& objAttributes = objTempDesc.GetAttributes();

        for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttr = objAttributes.GetAt(i);

            if (!objSdpDesc.Contains(objAttr))
            {
                objSdpDesc.AddAttribute(objAttr);
            }
        }
    }
}

PRIVATE GLOBAL const IMS_CHAR* Capabilities::StateToString(IN IMS_SINT32 nState)
{
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
