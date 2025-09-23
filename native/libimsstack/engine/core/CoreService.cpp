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

#include "Capabilities.h"
#include "CoreService.h"
#include "IOnCoreServiceListener.h"
#include "IOnDirectCoreServiceListener.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImsCore.h"
#include "PageMessage.h"
#include "Publication.h"
#include "Reference.h"
#include "SessionEx.h"
#include "Sip.h"
#include "SipConnectionFactory.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "Subscription.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CoreService::CoreService(IN const AString& strAppId, IN const AString& strServiceId,
        IN const SipAddress* pImpu /*= IMS_NULL*/) :
        Service(ImsCore::CONNECTION_SCHEME, strAppId, strServiceId, pImpu),
        m_piCoreServiceListener(IMS_NULL),
        m_piDirectCoreServiceListener(IMS_NULL)
{
}

PUBLIC VIRTUAL CoreService::~CoreService()
{
    IMS_TRACE_D("Destructor :: CoreService (%s)", GetServiceId().GetStr(), 0, 0);
}

PUBLIC VIRTUAL void CoreService::Close()
{
    m_objReasonInfo.SetReasonType(IReasonInfo::REASON_TYPE_USER_ACTION);

    Service::Close();
}

PUBLIC VIRTUAL void CoreService::Abort()
{
    m_objReasonInfo.SetReasonType(IReasonInfo::REASON_TYPE_USER_ACTION);

    Service::Abort();
}

PUBLIC VIRTUAL void CoreService::HandleSessionInvitationReceived(IN Session* pSession)
{
    // The subclass MUST implement this method to handle an incoming INVITE request

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->OnCoreService_SessionInvitationReceived(
            this, DYNAMIC_CAST(SessionEx*, pSession));
}

PUBLIC VIRTUAL void CoreService::HandlePageMessageReceived(IN PageMessage* pPageMessage)
{
    // The subclass MUST implement this method to handle an incoming MESSAGE request

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->OnCoreService_PageMessageReceived(this, pPageMessage);
}

PUBLIC VIRTUAL void CoreService::HandleReferenceReceived(IN Reference* pReference)
{
    // The subclass MUST implement this method to handle an incoming REFER request

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->OnCoreService_ReferenceReceived(this, pReference);
}

PUBLIC VIRTUAL void CoreService::HandleCapabilityQueryReceived(IN Capabilities* pCapabilities)
{
    // The subclass MUST implement this method to handle an incoming OPTIONS request

    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    m_piCoreServiceListener->OnCoreService_CapabilityQueryReceived(this, pCapabilities);
}

PUBLIC
Capabilities* CoreService::CreateCapabilities(IN const AString& strFrom, IN const AString& strTo)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    Capabilities* pCapabilities = new Capabilities(this);

    if (pCapabilities == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pCapabilities->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pCapabilities;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Capabilities failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pCapabilities;
}

PUBLIC
PageMessage* CoreService::CreatePageMessage(IN const AString& strFrom, IN const AString& strTo)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    PageMessage* pPageMessage = new PageMessage(this);

    if (pPageMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PageMessage failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pPageMessage->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pPageMessage;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing PageMessage failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pPageMessage;
}

PUBLIC
Publication* CoreService::CreatePublication(
        IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_TRUE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    if (!IsEventPackageSupported(strEvent))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: Event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    Publication* pPublication = new Publication(this, strEvent);

    if (pPublication == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Publication failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pPublication->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pPublication;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Publication failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pPublication;
}

PUBLIC
Reference* CoreService::CreateReference(IN const AString& strFrom, IN const AString& strTo,
        IN const AString& strReferTo, IN const AString& strReferMethod)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks Refer-To argument
    if (!ValidateReferTo(strReferTo, strReferMethod))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    Reference* pReference = new Reference(this, strReferTo, strReferMethod, Replaces());

    if (pReference == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pReference->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pReference;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Reference failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pReference;
}

PUBLIC
Session* CoreService::CreateSession(IN const AString& strFrom, IN const AString& strTo)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    Session* pSession = new Session(this);

    if (pSession == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSession->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pSession;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pSession;
}

PUBLIC
SessionEx* CoreService::CreateSessionEx(IN const AString& strFrom, IN const AString& strTo)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    SessionEx* pSession = new SessionEx(this);

    if (pSession == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SessionImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSession->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pSession;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pSession;
}

PUBLIC
Subscription* CoreService::CreateSubscription(
        IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent)
{
    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_TRUE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks an event package from the application configuration
    if (!IsEventPackageSupported(strEvent))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: Event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    Subscription* pSubscription = new Subscription(this, strEvent);

    if (pSubscription == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Subscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSubscription->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pSubscription;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Subscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pSubscription;
}

PUBLIC
ISipConnectionFactory* CoreService::CreateSipConnectionFactory()
{
    return new SipConnectionFactory(this);
}

PRIVATE VIRTUAL void CoreService::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    // Error code: LOSS_OF_NETWORK__NO_COMMUNICATION_WITH_PROXY
    (void)nErrorCode;

    // Notify the service closed
    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService - LISTENER IS NOT REGISTERED", 0, 0, 0);
        return;
    }

    // TODO:: define a new reason type
    m_objReasonInfo.SetReasonType(IReasonInfo::REASON_TYPE_NONE);

    m_piCoreServiceListener->OnCoreService_ServiceClosed(this, &m_objReasonInfo);
}

PRIVATE VIRTUAL IMS_BOOL CoreService::ServerConnection_NotifyRequest(IN ISipServerConnection* piSsc)
{
    // If the application does not set the listener, then sends 480 response to the request.
    if (m_piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        if (CheckAndHandleDirectSipRequest(piSsc) == RESULT_DIRECT_TXN_HANDLED)
        {
            return IMS_TRUE;
        }

        SendResponse(piSsc, SipStatusCode::SC_480);
        return IMS_FALSE;
    }

    const ISipMessage* piSipMsg = piSsc->GetMessage();
    AString strRemoteUserId;

    // Gets the remote user identity;
    // If P-Asserted-Identity is present, then gets this header field value.
    // Otherwise, gets From header field value.
    if (piSipMsg->IsHeaderPresent(ISipHeader::P_ASSERTED_IDENTITY))
    {
        strRemoteUserId = piSipMsg->GetHeader(ISipHeader::P_ASSERTED_IDENTITY);
    }
    else
    {
        strRemoteUserId = piSipMsg->GetHeader(ISipHeader::FROM);
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();
    Method* pMethod = IMS_NULL;

    IMS_TRACE_I("CoreService (%s) :: %s request received ...", GetServiceId().GetStr(),
            objMethod.ToString().GetStr(), 0);

    // Checks if the direct listener is registered or not...
    // And if direct listener is present, pass the transaction to that listener.
    if (CheckAndHandleDirectSipRequest(piSsc) == RESULT_DIRECT_TXN_HANDLED)
    {
        IMS_TRACE_D("The owner of direct listener handles the message", 0, 0, 0);
        return IMS_TRUE;
    }

    switch (objMethod.ToInt())
    {
        case SipMethod::INVITE:
        {
            pMethod = new SessionEx(this);

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating Session failed", 0, 0, 0);
                return IMS_FALSE;
            }
            break;
        }
        case SipMethod::CANCEL:
        {
            // Get a session information from the CANCEL transaction
            // NotifyCancelRequest(piSsc);
            return IMS_TRUE;
        }
        case SipMethod::MESSAGE:
        {
            pMethod = new PageMessage(this);

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating PageMessage failed", 0, 0, 0);
                return IMS_FALSE;
            }
            break;
        }
        case SipMethod::OPTIONS:
        {
            pMethod = new Capabilities(this);

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
                return IMS_FALSE;
            }
            break;
        }
        case SipMethod::REFER:
        {
            if (piSipMsg->GetHeaderCount(ISipHeader::REFER_TO) != 1)
            {
                SendResponse(piSsc, SipStatusCode::SC_400, AString("Mandatory Header Missing"));
                return IMS_FALSE;
            }

            AString strReferTo = piSipMsg->GetHeader(ISipHeader::REFER_TO);

            if (strReferTo.IsNULL() || strReferTo.IsEmpty())
            {
                SendResponse(piSsc, SipStatusCode::SC_400, AString("Invalid Header Field"));
                return IMS_FALSE;
            }

            ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::REFER_TO, strReferTo);

            if (piHeader == IMS_NULL)
            {
                SendResponse(piSsc, SipStatusCode::SC_400, AString("Invalid Header Field"));
                return IMS_FALSE;
            }

            const SipAddress* pAddress = piHeader->GetSipAddress();

            if (pAddress == IMS_NULL)
            {
                piHeader->Destroy();
                SendResponse(piSsc, SipStatusCode::SC_400, AString("Invalid Header Field"));
                return IMS_FALSE;
            }

            const SipParameter* pMethodP = pAddress->GetParameter(Sip::STR_METHOD);

            if ((pMethodP != IMS_NULL) &&
                    (pMethodP->GetValue().IsNULL() || pMethodP->GetValue().IsEmpty()))
            {
                piHeader->Destroy();
                SendResponse(piSsc, SipStatusCode::SC_400, AString("Mandatory Parameter Missing"));
                return IMS_FALSE;
            }

            const ISipHeader* piReplaces = pAddress->GetHeader(ISipHeader::REPLACES);
            Replaces objReplaces;

            if (piReplaces != IMS_NULL)
            {
                objReplaces.Create(piReplaces->ToString());
            }

            if (pMethodP == IMS_NULL)
            {
                IMS_TRACE_D("Refer-To :: method parameter does not exist", 0, 0, 0);

                SipMethod objReferToMethod(SipMethod::INVITE);
                pMethod = new Reference(
                        this, pAddress->GetUri(), objReferToMethod.ToString(), objReplaces);
            }
            else
            {
                pMethod =
                        new Reference(this, pAddress->GetUri(), pMethodP->GetValue(), objReplaces);
            }

            piHeader->Destroy();

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
                return IMS_FALSE;
            }
            break;
        }
        default:
            IMS_TRACE_I("Method (%s) is not supported by this service (%s)",
                    objMethod.ToString().GetStr(), GetServiceId().GetStr(), 0);

            // Methods which are handled inside of a dialog
            if (objMethod.Equals(SipMethod::BYE) || objMethod.Equals(SipMethod::INFO) ||
                    objMethod.Equals(SipMethod::UPDATE) || objMethod.Equals(SipMethod::PRACK) ||
                    objMethod.Equals(SipMethod::NOTIFY))
            {
                SendResponse(piSsc, SipStatusCode::SC_481);
            }
            else
            {
                // SUBSCRIBE / REGISTER / PUBLISH / ACK (?)
                // 405 Method Not Allowed ???
                if (!objMethod.Equals(SipMethod::ACK))
                {
                    SendResponse(piSsc, SipStatusCode::SC_405);
                }
            }

            piSsc->Close();
            return IMS_TRUE;
    }

    if (!pMethod->InitMethod(
                AString::ConstNull(), strRemoteUserId, GetAuthorizedUserId(), IMS_FALSE))
    {
        pMethod->Destroy();

        IMS_TRACE_E(0, "Initializing Method failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Handle the request in detailed procedure
    if (!pMethod->ServerConnection_NotifyRequest(piSsc))
    {
        pMethod->Destroy();

        IMS_TRACE_E(0, "Handling an incoming SIP request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 CoreService::CheckAndHandleDirectSipRequest(IN ISipServerConnection* piSsc)
{
    if (m_piDirectCoreServiceListener == IMS_NULL)
    {
        return RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    SipConnectionFactory* pScf = new SipConnectionFactory(this, piSsc);

    IMS_SINT32 nResult =
            m_piDirectCoreServiceListener->OnDirectCoreService_TransactionReceived(this, pScf);

    if (nResult != RESULT_DIRECT_TXN_HANDLED)
    {
        if (pScf != IMS_NULL)
        {
            delete pScf;
        }
    }

    return nResult;
}
