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
#include "Sip.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "SipParsingHelper.h"
#include "SipParameter.h"
#include "base/IMS.h"
#include "IMSCore.h"
#include "Capabilities.h"
#include "PageMessage.h"
#include "Publication.h"
#include "Reference.h"
#include "SessionEx.h"
#include "Subscription.h"
#include "SIPConnectionFactory.h"
#include "IOnCoreServiceListener.h"
#include "IOnDirectCoreServiceListener.h"
#include "CoreService.h"

__IMS_TRACE_TAG_IMS_CORE__;



/*

Remarks

*/
PUBLIC
CoreService::CoreService(IN CONST AString& strAppId_, IN CONST AString &strServiceId_,
        IN CONST SIPAddress *pIMPU_ /* = IMS_NULL */)
    : Service(IMSCore::CONNECTION_SCHEME, strAppId_, strServiceId_, pIMPU_)
    , piCoreServiceListener(IMS_NULL)
    , piDirectCoreServiceListener(IMS_NULL)
{
}

/*

Remarks

*/
PUBLIC VIRTUAL
CoreService::~CoreService()
{
    IMS_TRACE_D("Destructor :: CoreService (%s)", GetServiceId().GetStr(), 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void CoreService::Close()
{
    //---------------------------------------------------------------------------------------------

    objReasonInfo.SetReasonType(IReasonInfo::REASON_TYPE_USER_ACTION);

    Service::Close();
}

/*

Remarks

*/
PUBLIC VIRTUAL
void CoreService::Abort()
{
    //---------------------------------------------------------------------------------------------

    objReasonInfo.SetReasonType(IReasonInfo::REASON_TYPE_USER_ACTION);

    Service::Abort();
}

/*

Remarks

*/
PUBLIC VIRTUAL
void CoreService::HandleSessionInvitationReceived(IN Session *pSession)
{
    // The subclass MUST implement this method to handle an incoming INVITE request

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    piCoreServiceListener->OnCoreService_SessionInvitationReceived(this,
            DYNAMIC_CAST(SessionEx*, pSession));
}

/*

Remarks

*/
PUBLIC VIRTUAL
void CoreService::HandlePageMessageReceived(IN PageMessage *pPageMessage)
{
    // The subclass MUST implement this method to handle an incoming MESSAGE request

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    piCoreServiceListener->OnCoreService_PageMessageReceived(this, pPageMessage);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void CoreService::HandleReferenceReceived(IN Reference *pReference)
{
    // The subclass MUST implement this method to handle an incoming REFER request

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    piCoreServiceListener->OnCoreService_ReferenceReceived(this, pReference);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void CoreService::HandleCapabilityQueryReceived(IN Capabilities *pCapabilities)
{
    // The subclass MUST implement this method to handle an incoming OPTIONS request

    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_D("NO CORE SERVICE LISTENER", 0, 0, 0);
        return;
    }

    piCoreServiceListener->OnCoreService_CapabilityQueryReceived(this, pCapabilities);
}

/*

Remarks

*/
PUBLIC
Capabilities* CoreService::CreateCapabilities(IN CONST AString &strFrom, IN CONST AString &strTo)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    Capabilities *pCapabilities = new Capabilities(this);

    if (pCapabilities == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pCapabilities->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pCapabilities;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Capabilities failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pCapabilities;
}

/*

Remarks

*/
PUBLIC
PageMessage* CoreService::CreatePageMessage(IN CONST AString &strFrom, IN CONST AString &strTo)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    PageMessage *pPageMessage = new PageMessage(this);

    if (pPageMessage == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating PageMessage failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pPageMessage->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pPageMessage;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing PageMessage failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pPageMessage;
}

/*

Remarks

*/
PUBLIC
Publication* CoreService::CreatePublication(IN CONST AString &strFrom, IN CONST AString &strTo,
        IN CONST AString &strEvent)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_TRUE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    if (!IsEventPackageSupported(strEvent))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: Event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    Publication *pPublication = new Publication(this, strEvent);

    if (pPublication == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Publication failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pPublication->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pPublication;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Publication failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pPublication;
}

/*

Remarks

*/
PUBLIC
Reference* CoreService::CreateReference(IN CONST AString &strFrom, IN CONST AString &strTo,
        IN CONST AString &strReferTo, IN CONST AString &strReferMethod)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks Refer-To argument
    if (!ValidateReferTo(strReferTo, strReferMethod))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    Reference *pReference = new Reference(this, strReferTo, strReferMethod, Replaces());

    if (pReference == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pReference->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pReference;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Reference failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pReference;
}

/*

Remarks

*/
PUBLIC
Session* CoreService::CreateSession(IN CONST AString &strFrom, IN CONST AString &strTo)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    Session *pSession = new Session(this);

    if (pSession == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSession->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pSession;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pSession;
}

/*

Remarks

*/
PUBLIC
SessionEx* CoreService::CreateSessionEx(IN CONST AString &strFrom, IN CONST AString &strTo)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_FALSE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    SessionEx *pSession = new SessionEx(this);

    if (pSession == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SessionImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSession->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pSession;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pSession;
}

/*

Remarks

*/
PUBLIC
Subscription* CoreService::CreateSubscription(IN CONST AString &strFrom, IN CONST AString &strTo,
        IN CONST AString &strEvent)
{
    //---------------------------------------------------------------------------------------------

    // Checks if the current service is already opened
    if (!IsImsConnected())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);

        IMS_TRACE_E(0, "Service MUST be opened", 0, 0, 0);
        return IMS_NULL;
    }

    // Validates From/To arguments
    if (!ValidateFromAndTo(strFrom, strTo, IMS_TRUE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // Checks an event package from the application configuration
    if (!IsEventPackageSupported(strEvent))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: Event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    Subscription *pSubscription = new Subscription(this, strEvent);

    if (pSubscription == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Subscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSubscription->InitMethod(strFrom, strTo, GetAuthorizedUserId()))
    {
        delete pSubscription;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Subscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pSubscription;
}

/*

Remarks

*/
PUBLIC
AString CoreService::GetLocalUserId() const
{
    //---------------------------------------------------------------------------------------------

    return GetAuthorizedUserId().ToString();
}

/*

Remarks

*/
PUBLIC
void CoreService::SetListener(IN IOnCoreServiceListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piCoreServiceListener = piListener;
}

/*

Remarks

*/
PUBLIC
ISIPConnectionFactory* CoreService::CreateSIPConnectionFactory()
{
    //---------------------------------------------------------------------------------------------

    return new SIPConnectionFactory(this);
}

/*

Remarks

*/
PUBLIC
void CoreService::SetDirectListener(IN IOnDirectCoreServiceListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piDirectCoreServiceListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void CoreService::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    // Error code: LOSS_OF_NETWORK__NO_COMMUNICATION_WITH_PROXY
    (void) nErrorCode;

    // Notify the service closed
    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService - LISTENER IS NOT REGISTERED", 0, 0, 0);
        return;
    }

    // TODO:: define a new reason type
    objReasonInfo.SetReasonType(IReasonInfo::REASON_TYPE_NONE);

    piCoreServiceListener->OnCoreService_ServiceClosed(this, &objReasonInfo);
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL CoreService::ServerConnection_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    // If the application does not set the listener, then sends 480 response to the request.
    if (piCoreServiceListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO CORE SERVICE LISTENER", 0, 0, 0);

        if (CheckAndHandleDirectSIPRequest(piSSC) == RESULT_DIRECT_TXN_HANDLED)
        {
            return IMS_TRUE;
        }

        SendResponse(piSSC, SIPStatusCode::SC_480);
        return IMS_FALSE;
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();
    AString strRemoteUserId;

    // Gets the remote user identity;
    // If P-Asserted-Identity is present, then gets this header field value.
    // Otherwise, gets From header field value.
    if (piSIPMsg->IsHeaderPresent(ISIPHeader::P_ASSERTED_IDENTITY))
        strRemoteUserId = piSIPMsg->GetHeader(ISIPHeader::P_ASSERTED_IDENTITY);
    else
        strRemoteUserId = piSIPMsg->GetHeader(ISIPHeader::FROM);

    const SIPMethod &objMethod = piSIPMsg->GetMethod();
    Method *pMethod = IMS_NULL;

    IMS_TRACE_I("CoreService (%s) :: %s request received ...",
        GetServiceId().GetStr(), objMethod.ToString().GetStr(), 0);

    // Checks if the direct listener is registered or not...
    // And if direct listener is present, pass the transaction to that listener.
    if (CheckAndHandleDirectSIPRequest(piSSC) == RESULT_DIRECT_TXN_HANDLED)
    {
        IMS_TRACE_D("The owner of direct listener handles the message", 0, 0, 0);
        return IMS_TRUE;
    }

    switch (objMethod.ToInt())
    {
        case SIPMethod::INVITE:
        {
            pMethod = new SessionEx(this);

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating Session failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        break;

        case SIPMethod::CANCEL:
        {
            // Get a session information from the CANCEL transaction
            // NotifyCancelRequest(piSSC);
            return IMS_TRUE;
        }
        break;

        case SIPMethod::MESSAGE:
        {
            pMethod = new PageMessage(this);

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating PageMessage failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        break;

        case SIPMethod::OPTIONS:
        {
            pMethod = new Capabilities(this);

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        break;

        case SIPMethod::REFER:
        {
            if (piSIPMsg->GetHeaderCount(ISIPHeader::REFER_TO) != 1)
            {
                SendResponse(piSSC, SIPStatusCode::SC_400, AString("Mandatory Header Missing"));
                return IMS_FALSE;
            }

            AString strReferTo = piSIPMsg->GetHeader(ISIPHeader::REFER_TO);

            if (strReferTo.IsNULL() || strReferTo.IsEmpty())
            {
                SendResponse(piSSC, SIPStatusCode::SC_400, AString("Invalid Header Field"));
                return IMS_FALSE;
            }

            ISIPHeader *piHeader = SIPParsingHelper::CreateHeader(
                                        ISIPHeader::REFER_TO, strReferTo);

            if (piHeader == IMS_NULL)
            {
                SendResponse(piSSC, SIPStatusCode::SC_400, AString("Invalid Header Field"));
                return IMS_FALSE;
            }

            const SIPAddress *pAddress = piHeader->GetSIPAddress();

            if (pAddress == IMS_NULL)
            {
                piHeader->Destroy();
                SendResponse(piSSC, SIPStatusCode::SC_400, AString("Invalid Header Field"));
                return IMS_FALSE;
            }

            const SIPParameter *pMethodP = pAddress->GetParameter(SIP::STR_METHOD);

            if ((pMethodP != IMS_NULL)
                && (pMethodP->GetValue().IsNULL() || pMethodP->GetValue().IsEmpty()))
            {
                piHeader->Destroy();
                SendResponse(piSSC, SIPStatusCode::SC_400, AString("Mandatory Parameter Missing"));
                return IMS_FALSE;
            }

            const ISIPHeader* piReplaces = pAddress->GetHeader(ISIPHeader::REPLACES);
            Replaces objReplaces;

            if (piReplaces != IMS_NULL)
            {
                objReplaces.Create(piReplaces->ToString());
            }

            if (pMethodP == IMS_NULL)
            {
                IMS_TRACE_D("Refer-To :: method parameter does not exist", 0, 0, 0);

                SIPMethod objMethod(SIPMethod::INVITE);
                pMethod = new Reference(this,
                                pAddress->GetURI(), objMethod.ToString(), objReplaces);
            }
            else
            {
                pMethod = new Reference(this,
                                pAddress->GetURI(), pMethodP->GetValue(), objReplaces);
            }

            piHeader->Destroy();

            if (pMethod == IMS_NULL)
            {
                IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        break;

        default:
            IMS_TRACE_I("Method (%s) is not supported by this service (%s)",
                objMethod.ToString().GetStr(), GetServiceId().GetStr(), 0);

            // Methods which are handled inside of a dialog
            if (objMethod.Equals(SIPMethod::BYE)
                || objMethod.Equals(SIPMethod::INFO)
                || objMethod.Equals(SIPMethod::UPDATE)
                || objMethod.Equals(SIPMethod::PRACK)
                || objMethod.Equals(SIPMethod::NOTIFY))
            {
                SendResponse(piSSC, SIPStatusCode::SC_481);
            }
            else
            {
                // SUBSCRIBE / REGISTER / PUBLISH / ACK (?)
                // 405 Method Not Allowed ???
                if (!objMethod.Equals(SIPMethod::ACK))
                {
                    SendResponse(piSSC, SIPStatusCode::SC_405);
                }
            }

            piSSC->Close();
            return IMS_TRUE;
    }

    if (!pMethod->InitMethod(AString::ConstNull(),
            strRemoteUserId, GetAuthorizedUserId(), IMS_FALSE))
    {
        pMethod->Destroy();

        IMS_TRACE_E(0, "Initializing Method failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Handle the request in detailed procedure
    if (!pMethod->ServerConnection_NotifyRequest(piSSC))
    {
        pMethod->Destroy();

        IMS_TRACE_E(0, "Handling an incoming SIP request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_SINT32 CoreService::CheckAndHandleDirectSIPRequest(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (piDirectCoreServiceListener == IMS_NULL)
    {
        return RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    SIPConnectionFactory *pSCF = new SIPConnectionFactory(this, piSSC);

    IMS_SINT32 nResult
            = piDirectCoreServiceListener->OnDirectCoreService_TransactionReceived(this, pSCF);

    if (nResult != RESULT_DIRECT_TXN_HANDLED)
    {
        if (pSCF != IMS_NULL)
        {
            delete pSCF;
        }
    }

    return nResult;
}
