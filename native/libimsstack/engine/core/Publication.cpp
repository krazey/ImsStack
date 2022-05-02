/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100423  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipStatusCode.h"
#include "base/IMS.h"
#include "base/IRefreshListener.h"
#include "Service.h"
#include "IOnPublicationListener.h"
#include "PubState.h"
#include "PublicationRefreshHelper.h"
#include "Publication.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
Publication::Publication(IN Service *pService, IN CONST AString &strEvent_)
    : ServiceMethod(pService)
    , nState(STATE_INACTIVE)
    , strEvent(strEvent_)
    , piListener(IMS_NULL)
    , pPubState(IMS_NULL)
    , piRefreshListener(IMS_NULL)
    , pRefreshHelper(IMS_NULL)
{
}

PUBLIC VIRTUAL
Publication::~Publication()
{
    //---------------------------------------------------------------------------------------------

    if (pRefreshHelper != IMS_NULL)
    {
        delete pRefreshHelper;
        pRefreshHelper = IMS_NULL;
    }

    if (pPubState != IMS_NULL)
    {
        delete pPubState;
        pPubState = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Publication::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pRefreshHelper != IMS_NULL)
    {
        if (pRefreshHelper->IsRequestPending())
        {
            pRefreshHelper->AbortConnection();
        }
    }

    ServiceMethod::Destroy();
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PUBLIC VIRTUAL
void Publication::SetMessageMediator(IN IMessageMediator *piMediator)
{
    //---------------------------------------------------------------------------------------------

    Method::SetMessageMediator(piMediator);

    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->SetMessageMediator(piMediator);
    }
}

/*

Remarks

*/
PUBLIC
const AString& Publication::GetEvent() const
{
    //---------------------------------------------------------------------------------------------

    return strEvent;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 Publication::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT Publication::Publish(IN CONST ByteArray &objState, IN CONST AString &strContentType)
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() == STATE_PENDING)
    {
        IMS_TRACE_E(0, "To publish an event state, the state MUST be an INACTIVE or ACTIVE state",
                0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if ((objState.IsNULL() && !strContentType.IsNULL())
            || (!objState.IsNULL() && strContentType.IsNULL()))
    {
        IMS_TRACE_E(0, "One of the arguments is NULL", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!strContentType.IsNULL() && !TextParser::IsValidMediaType(strContentType))
    {
        IMS_TRACE_E(0, "Invalid content type (%s)", strContentType.GetStr(), 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // if the state is in ACTIVE and refresh is started by the publication,
    // keep the request and after refresh is completed, try to send a PUBLISH request...
    if (pPubState->GetOperation() == PubState::OPERATION_REFRESH)
    {
        //1 : save all the information ? try to send later .... ???
        //SetState(STATE_PENDING);

        //IMS::SetLastError(IMSError::NO_ERROR);
        //return IMS_FAILURE;
    }

    ISIPClientConnection *piSCC = CreateConnection(SIPMethod(SIPMethod::PUBLISH));

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Event header
    piSIPMsg->SetHeader(ISIPHeader::EVENT, GetEvent());

    // Accept, Allow-Events, Expires
    IMS_BOOL bDefaultExpiresRequired = IMS_FALSE;

    // Basically, Expires header will be automatically added according to the event package,
    // but IMS engine will not add the EXpires header even though it's not present and it will
    // be controlled by the enabler.
    if (bDefaultExpiresRequired && !piSIPMsg->IsHeaderPresent(ISIPHeader::EXPIRES_ANY))
    {
        // Expires header ?
        AString strExpires;

        if (pPubState->GetEventPackage()->GetDuration() != (-1))
        {
            strExpires.Sprintf("%d", pPubState->GetEventPackage()->GetDuration());
        }
        else
        {
            strExpires.Sprintf("%d", pPubState->GetEventPackage()->GetDefaultDuration());
        }

        piSIPMsg->SetHeader(ISIPHeader::EXPIRES_ANY, strExpires);
    }

    // OPERATION_MODIFY
    if (nState == STATE_ACTIVE)
    {
        // SIP-If-Match header
        if (!piSIPMsg->IsHeaderPresent(ISIPHeader::SIP_IF_MATCH))
        {
            if (!pPubState->GetEntityTag().IsNULL())
            {
                piSIPMsg->SetHeader(ISIPHeader::SIP_IF_MATCH, pPubState->GetEntityTag());
            }
        }
    }

    // Set an additional message body part at the last position
    if (!objState.IsNULL() && !strContentType.IsNULL())
    {
        ISIPMessageBodyPart *piBodyPart = piSIPMsg->CreateBodyPart();

        if (piBodyPart == IMS_NULL)
        {
            piSCC->Close();
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        // Set headers
        piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE, strContentType);

        // Set content
        piBodyPart->SetContent(objState);
    }

    // Try to send a PUBLISH request to the network
    if (!SendNUpdateRequest(IMessage::PUBLICATION_PUBLISH, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    if (GetState() == STATE_INACTIVE)
        pPubState->SetOperation(PubState::OPERATION_CREATE);
    else
        pPubState->SetOperation(PubState::OPERATION_MODIFY);

    // Update the publication state
    if (!pPubState->UpdateState(piSIPMsg))
    {
        pPubState->SetOperation(PubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Publication::SetListener(IN IOnPublicationListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT Publication::Unpublish()
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "To unpublish an event state, the state MUST be an ACTIVE state", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // If the state is in ACTIVE and refresh is started by the publication,
    // keep the request and after refresh is completed, try to send a PUBLISH request...
    if (pPubState->GetOperation() == PubState::OPERATION_REFRESH)
    {
        //4 : save all the information ? try to send later .... ???
        //nPendingOperation = PubState::OPERATION_REMOVE;

        //SetState(STATE_PENDING);

        //IMS::SetLastError(IMSError::NO_ERROR);
        //return IMS_FAILURE;
    }

    ISIPClientConnection *piSCC = CreateConnection(SIPMethod(SIPMethod::PUBLISH));

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Event header
    piSIPMsg->SetHeader(ISIPHeader::EVENT, GetEvent());

    // Expires header (overwrites the header field if present)
    piSIPMsg->SetHeader(ISIPHeader::EXPIRES_ANY, "0");

    // SIP-If-Match header
    if (!pPubState->GetEntityTag().IsNULL())
    {
        piSIPMsg->SetHeader(ISIPHeader::SIP_IF_MATCH, pPubState->GetEntityTag());
    }

    // Try to send a PUBLISH request to the network
    if (!SendNUpdateRequest(IMessage::PUBLICATION_UNPUBLISH, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    pPubState->SetOperation(PubState::OPERATION_REMOVE);

    // Update the publication state
    if (!pPubState->UpdateState(piSIPMsg))
    {
        pPubState->SetOperation(PubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Publication::SetRefreshListener(IN IRefreshListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piRefreshListener = piListener;
}

/*

Remarks

*/
PUBLIC
void Publication::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_INACTIVE)
    {
        IMS_TRACE_E(0, "Refresh policy can't be applied in the state (%s)",
                StateToString(GetState()), 0, 0);
        return;
    }

    if (pRefreshHelper == IMS_NULL)
    {
        return;
    }

    switch (nPolicy)
    {
    case REFRESH_POLICY_NO_REFRESH:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_NO_REFRESH,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    case REFRESH_POLICY_SPEC:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_SPEC,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    case REFRESH_POLICY_REMAIN_TIME:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_REMAIN_TIME,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    case REFRESH_POLICY_RATIO:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_RATIO,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    default:
        IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
        break;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL Publication::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
    case AMSG_PUBLICATION_DELIVERED:
        if (piListener != IMS_NULL)
        {
            piListener->OnPublication_Delivered(this);
        }
        return IMS_TRUE;

    case AMSG_PUBLICATION_DELIVERY_FAILED:
        if (piListener != IMS_NULL)
        {
            piListener->OnPublication_DeliveryFailed(this);
        }
        return IMS_TRUE;

    case AMSG_PUBLICATION_TERMINATED:
        if (piListener != IMS_NULL)
        {
            piListener->OnPublication_Terminated(this);
        }
        return IMS_TRUE;

        //[2012/11/5]hyunho.shin : add refresh handler
    case AMSG_PUBLICATION_REFRESH_STARTED:
        if (piListener != IMS_NULL)
        {
            piListener->OnPublication_RefreshStarted(this);
        }
        return IMS_TRUE;

    case AMSG_PUBLICATION_REFRESH_COMPLETED:
        if (piListener != IMS_NULL)
        {
            piListener->OnPublication_RefreshCompleted(this);
        }
        return IMS_TRUE;
        //[2012/11/5]hyunho.shin : end

    default:
        break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PRIVATE VIRTUAL
IMS_BOOL Publication::SendRequestToChallenge(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pPubState->GetOperation() == PubState::OPERATION_REFRESH)
    {
        if (!Method::SendRequestToChallenge(piSCC))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        IMS_SINT32 nServiceMethod = IMessage::PUBLICATION_PUBLISH;

        if (pPubState->GetOperation() == PubState::OPERATION_REMOVE)
        {
            nServiceMethod = IMessage::PUBLICATION_UNPUBLISH;
        }

        // Clear the connection to preserve the SIP connection
        ClearConnection(nServiceMethod);

        if (!SendNUpdateRequestEx(nServiceMethod, piSCC, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(nServiceMethod, piSCC);
            return IMS_FALSE;
        }
    }

    // Update the publication state
    if (!pPubState->UpdateState(piSCC->GetMessage()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Publication::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    (void) nErrorCode;

    SetState(STATE_INACTIVE);
    PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);

    pPubState->Clear();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL Publication::InitInstance()
{
    //---------------------------------------------------------------------------------------------

    if (pPubState == IMS_NULL)
    {
        pPubState = new PubState();

        if (pPubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a publication state failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!pPubState->CreateEventPackage(strEvent))
        {
            IMS_TRACE_E(0, "Creating an event package for an event (%s) failed",
                    strEvent.GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    if (pRefreshHelper == IMS_NULL)
    {
        pRefreshHelper = new PublicationRefreshHelper(this, pPubState);

        if (pRefreshHelper == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a publication refresh helper failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Publication::NotifySIPResponse(IN ISIPClientConnection *piSCC)
{
    ISIPMessage *piSIPMsg = piSCC->GetMessage();
    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("The response is received in the %s", StateToString(GetState()), 0, 0);

    if (!objMethod.Equals(SIPMethod::PUBLISH))
    {
        piSCC->Close();
        return;
    }

    // Update the publication state
    if (!pPubState->UpdateState(piSIPMsg))
    {
        PostMessage(AMSG_PUBLICATION_DELIVERY_FAILED, 0, 0);

        CloseConnection();

        if (pPubState->GetOperation() == PubState::OPERATION_CREATE)
        {
            pPubState->Clear();
        }
        return;
    }

    // Handle the response according to the SIP method.
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    ReceiveResponse(piSCC);

    // Handle the response to PUBLISH request ...
    if ( SIPStatusCode::Is1XX(nStatusCode) )
    {
        // Do nothing ...
        return;
    }
    else if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
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

    // Start or re-start a publication refresh timer
    pRefreshHelper->UpdateOnMessageReceived(piSCC);

    if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        if (pPubState->GetOperation() == PubState::OPERATION_REMOVE)
        {
            SetState(STATE_INACTIVE);
            PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
        }
        else
        {
            SetState(STATE_ACTIVE);
            PostMessage(AMSG_PUBLICATION_DELIVERED, 0, 0);
        }
    }
    else
    {
        if (GetState() != STATE_ACTIVE)
        {
            SetState(STATE_INACTIVE);
        }

        PostMessage(AMSG_PUBLICATION_DELIVERY_FAILED, 0, 0);
    }

    CloseConnection();

    if (GetState() == STATE_ACTIVE)
    {
        if (pPubState->IsTerminated())
        {
            SetState(STATE_INACTIVE);
            PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
        }
    }

    // Initialize all the EPA's state at this moment,
    // because the application MAY use this subscription to add/modify/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        pPubState->Clear();
    }

    //4 check "nPendingOperation" member field
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Publication::NotifySIPError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
        IN CONST AString &strMessage)
{
    const SIPMethod &objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void) nCode;
    (void) strMessage;

    if (!objMethod.Equals(SIPMethod::PUBLISH))
    {
        piSC->Close();
        return;
    }

    IMS_SINT32 nOperation = pPubState->GetOperation();

    if ((nOperation != PubState::NO_OPERATION)
        && (nOperation != PubState::OPERATION_REFRESH))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_PUBLICATION_DELIVERY_FAILED, 0, 0);
    }

    CloseConnection();

    // Initialize all the EPA's state at this moment,
    // because the application MAY use this publication to add/modify/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        pPubState->Clear();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Publication::Refreshable_RefreshCompleted(IN ISIPClientConnection *piSCC,
        IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ PUBLICATION REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyCompleted(piSCC);
    }

    // In case, the publication refresh request is successfully done.
    if (nCode == 0)
    {
        // for handling refresh error
        ReceiveResponse(piSCC);

        if (!pPubState->UpdateState(piSCC->GetMessage()))
        {
            // Set the operation
            pPubState->SetOperation(PubState::NO_OPERATION);

            if (pPubState->IsTerminated() && !pRefreshHelper->IsTimerActive())
            {
                SetState(STATE_INACTIVE);
                PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
            }

            return;
        }

        // AUTH_SIP_DIGEST {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if ((nStatusCode == SIPStatusCode::SC_401)
                || (nStatusCode == SIPStatusCode::SC_407))
        {
            // In case of other method except for REGISTER,
            // the UE only supports the authentication algorithm, MD5
            if (RespondToChallenge(piSCC))
            {
                return;
            }
        }

        ResetChallengeCount(piSCC);
        // }
        // sending refresh started message
        PostMessage(AMSG_PUBLICATION_REFRESH_COMPLETED, 0, 0);
    }
    // The subscription refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        pPubState->UpdateStateOnTxnTimerExpired();

        Refreshable_RefreshTerminated();
    }

    // Set the operation
    pPubState->SetOperation(PubState::NO_OPERATION);

    if (pPubState->IsTerminated() && !pRefreshHelper->IsTimerActive())
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL Publication::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ PUBLICATION REFRESH STARTED ... State(%d)", nState, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
    }

    if (bDoImplicitRefresh && (nState == STATE_ACTIVE))
    {
        // Set the operation
        pPubState->SetOperation(PubState::OPERATION_REFRESH);

        // Send a refresh request : PUBLISH
        ISIPClientConnection *piSCC = CreateConnection(SIPMethod(SIPMethod::PUBLISH));

        if (piSCC == IMS_NULL)
        {
            pPubState->SetOperation(PubState::NO_OPERATION);

            IMS_TRACE_E(0, "Creating a new SIP connection for a publication refresh failed",
                    0, 0, 0);
            return IMS_FALSE;
        }

        ISIPMessage *piSIPMsg = piSCC->GetMessage();

        if (!pPubState->SetHeaders(piSIPMsg))
        {
            pPubState->SetOperation(PubState::NO_OPERATION);
            piSCC->Close();

            IMS_TRACE_E(0, "Setting SIP headers to refresh PUBLISH request failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (pRefreshHelper->SendRefreshRequest(piSCC) != IMS_SUCCESS)
        {
            pPubState->SetOperation(PubState::NO_OPERATION);
            piSCC->Close();

            IMS_TRACE_E(0, "Sending a publication refresh request failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Update the publication state
        pPubState->UpdateState(piSIPMsg);

        //[2012/11/5]hyunho.shin : sending refresh started message
        PostMessage(AMSG_PUBLICATION_REFRESH_STARTED, 0, 0);
        //[2012/11/5]hyunho.shin : end

        return IMS_TRUE;
    }

    return (bDoImplicitRefresh == IMS_TRUE) ? IMS_FALSE : IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Publication::Refreshable_RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("_____ PUBLICATION REFRESH TERMINATED ...", 0, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyTerminated();
    }

    if (GetState() == STATE_ACTIVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
    }

    if (GetState() == STATE_INACTIVE)
    {
        pPubState->Clear();
    }
}

/*

Remarks

*/
PRIVATE
void Publication::CloseConnection()
{
    //---------------------------------------------------------------------------------------------

    switch (pPubState->GetOperation())
    {
    case PubState::OPERATION_CREATE:
    case PubState::OPERATION_MODIFY:
        ServiceMethod::CloseConnection(IMessage::PUBLICATION_PUBLISH);
        break;

    case PubState::OPERATION_REMOVE:
        ServiceMethod::CloseConnection(IMessage::PUBLICATION_UNPUBLISH);
        break;

    default:
        // Do nothing ...
        break;
    }

    pPubState->SetOperation(PubState::NO_OPERATION);
}

/*

Remarks

*/
PRIVATE
void Publication::ReceiveResponse(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    switch (pPubState->GetOperation())
    {
    case PubState::OPERATION_CREATE:
    case PubState::OPERATION_MODIFY:
    // hansung.cho -2013/10/23- for handling refresh error
    case PubState::OPERATION_REFRESH:
    // hansung.cho -2013/10/23- for handling refresh error
        UpdateResponseOnReceived(IMessage::PUBLICATION_PUBLISH, piSCC);
        break;

    case PubState::OPERATION_REMOVE:
        UpdateResponseOnReceived(IMessage::PUBLICATION_UNPUBLISH, piSCC);
        break;

    default:
        // Do nothing ...
        break;
    }
}

/*

Remarks

*/
PRIVATE
void Publication::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Publication :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* Publication::StateToString(IN IMS_SINT32 nState)
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
