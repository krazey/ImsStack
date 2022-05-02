/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100413  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "private/SipConfigV.h"
#include "TextParser.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "Sip.h"
#include "SipFeatures.h"
#include "SipDebug.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "SipConfigProxy.h"
#include "base/IMS.h"
#include "Service.h"
#include "util/DialogMethodManager.h"
#include "CallControlHelper.h"
#include "Capabilities.h"
#include "IOnNotificationListener.h"
#include "IOnReferenceListener.h"
#include "ImplicitNotifierState.h"
#include "ImplicitSubscriberState.h"
#include "Reference.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC GLOBAL
const IMS_CHAR Reference::EVENT_REFER[] = "refer";
PUBLIC GLOBAL
const IMS_CHAR Reference::MEDIA_TYPE[] = "message/sipfrag";



PUBLIC
Reference::NotifierState::NotifierState()
    : objSCCs(IMSList<ISIPClientConnection*>())
{
}

PUBLIC
Reference::NotifierState::~NotifierState()
{
    if (!objSCCs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSCCs.GetSize(); ++i)
        {
            ISIPClientConnection *piSCC = objSCCs.GetAt(i);

            if (piSCC != IMS_NULL)
            {
                piSCC->Close();
            }
        }

        objSCCs.Clear();
    }
}

/*

Remarks

*/
PUBLIC
void Reference::NotifierState::AddSCC(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    objSCCs.Append(piSCC);
}

/*

Remarks

*/
PUBLIC
void Reference::NotifierState::RemoveSCC(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objSCCs.GetSize(); ++i)
    {
        ISIPClientConnection *piTempSCC = objSCCs.GetAt(i);

        if (piSCC == piTempSCC)
        {
            objSCCs.RemoveAt(i);
            break;
        }
    }
}



PUBLIC
Reference::Reference(IN Service *pService_, IN CONST AString &strReferToURI_,
        IN CONST AString &strReferMethod_, IN CONST Replaces &objReplaces_,
        IN IMS_BOOL bImplicitRoutingRequired_ /* = IMS_FALSE */)
    : ServiceMethod(pService_)
    , nState(STATE_INITIATED)
    , strReferToURI(strReferToURI_)
    , objReferMethod(strReferMethod_)
    , pReplaces(IMS_NULL)
    , bFlag_ImplicitSubscription(IMS_FALSE)
    , pReferredMethod(IMS_NULL)
    , piListener(IMS_NULL)
    , pSubState(IMS_NULL)
    , bFlag_ReferenceInOtherDialog(IMS_FALSE)
    , bFlag_ImplicitRoutingRequired(bImplicitRoutingRequired_)
    , objNotifyMessages(IMSList<Message*>())
    , piNotificationListener(IMS_NULL)
    , pNotifierState(IMS_NULL)
{
    const AString &strSessionId
            = CallControlHelper::GetInstance()->GetSessionIdFromReplaces(&objReplaces_);

    if (!strSessionId.IsNULL())
    {
        pReplaces = new Replaces(objReplaces_);
    }
}

PUBLIC VIRTUAL
Reference::~Reference()
{
    //---------------------------------------------------------------------------------------------

    CleanupOnDestroy();

    if (pReplaces != IMS_NULL)
    {
        delete pReplaces;
    }

    while (!objNotifyMessages.IsEmpty())
    {
        Message *pMessage = objNotifyMessages.GetAt(0);

        if (pMessage != IMS_NULL)
            delete pMessage;

        objNotifyMessages.RemoveAt(0);
    }

    if (pSubState != IMS_NULL)
    {
        delete pSubState;
    }

    if (pReferredMethod != IMS_NULL)
    {
        pReferredMethod->SetReferredMessageListener(IMS_NULL);
    }

    if (pNotifierState != IMS_NULL)
    {
        delete pNotifierState;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Reference::Destroy()
{
    //---------------------------------------------------------------------------------------------

    CleanupOnDestroy();
    ServiceMethod::Destroy();
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::Accept()
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PROCEEDING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SetState(STATE_REFERRING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::ConnectReferMethod(IN Method *pReferMethod)
{
    //---------------------------------------------------------------------------------------------

    if (pReferMethod == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_REFERRING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    pReferredMethod = pReferMethod;

    if (!pReferredMethod->SetReferredMessageListener(this))
    {
        IMS_TRACE_E(0, "Not implemented method", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
const SIPMethod& Reference::GetReferMethod() const
{
    //---------------------------------------------------------------------------------------------

    return objReferMethod;
}

/*

Remarks

*/
PUBLIC
const AString& Reference::GetReferToUserId() const
{
    //---------------------------------------------------------------------------------------------

    return strReferToURI;
}

/*

Remarks

*/
PUBLIC
const AString& Reference::GetReplaces() const
{
    //---------------------------------------------------------------------------------------------

    if (pReplaces == IMS_NULL)
    {
        return AString::ConstNull();
    }

    IMS_TRACE_D("___ Replaces: %s ___",
            SIPDebug::GetStr1(pReplaces->ToString(IMS_FALSE), 8, '@').GetStr(), 0, 0);

    return CallControlHelper::GetInstance()->GetSessionIdFromReplaces(pReplaces);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 Reference::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::Refer(IN IMS_BOOL bImplicitSubscription)
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        IMS_TRACE_E(0, "To send the reference request, the state MUST be an INITIATED state",
                0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISIPDialog *piDialog = GetDialog();
    SIPMethod objMethod(SIPMethod::REFER);
    ISIPClientConnection *piSCC = IMS_NULL;

    if (piDialog == IMS_NULL)
    {
        piSCC = CreateConnection(objMethod);
    }
    else
    {
        piSCC = CreateConnectionL(piDialog, objMethod);
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    if (SIPFeatures::IsEventHeaderApplicableForRefer(GetSlotId()))
    {
        // Event header
        piSIPMsg->SetHeader(ISIPHeader::EVENT, EVENT_REFER);
    }

    // Refer-To header
    SIPAddress objReferToURI;

    if (!objReferToURI.Create(strReferToURI))
    {
        IMS_TRACE_E(0, "Parsing Refer-To (URI) failed", 0, 0, 0);
        IMS::SetLastError(IMSError::PARSING_ERROR);
        return IMS_FAILURE;
    }

    if (!objReferMethod.Equals(SIPMethod::INVALID))
    {
        if (objReferToURI.SetParameter(SIP::STR_METHOD,
                objReferMethod.ToString().GetStr()) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting a 'method' parameter failed", 0, 0, 0);
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    if (pReplaces != IMS_NULL)
    {
        objReferToURI.SetHeader(ISIPHeader::REPLACES, pReplaces->ToString(IMS_TRUE));
    }

    piSIPMsg->SetHeader(ISIPHeader::REFER_TO, objReferToURI.ToString());

    if (SIPFeatures::IsReferSubHeaderSupported(GetSlotId()))
    {
        // Supported header : "norefersub"
        piSIPMsg->AddHeader(ISIPHeader::SUPPORTED, "norefersub");

        // Refer-Sub header : "true" / "false"
        if (bImplicitSubscription)
        {
            piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
                    TextParser::STR_SMALL_TRUE, SIPHeaderName::REFER_SUB);
        }
        else
        {
            piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
                    TextParser::STR_SMALL_FALSE, SIPHeaderName::REFER_SUB);
        }
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::REFERENCE_REFER, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    pSubState->SetOperation(SubState::OPERATION_CREATE);

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    // Store the flag to indicate if the implicit subscription is requested or not
    bFlag_ImplicitSubscription = bImplicitSubscription;

    // Update the dialog state for REFER-created dialog (to handle early NOTIFY request)
    if (bFlag_ReferenceInOtherDialog)
    {
        CheckNCreateDialog(piSCC, IMS_TRUE);
    }

    SetState(STATE_PROCEEDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::Reject()
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PROCEEDING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // Send a NOTIFY request with 'terminated' sub-state
    AString strRejectRequest("SIP/2.0 603 Declined");

    DoNotification(SubState::SUB_STATE_TERMINATED, strRejectRequest);

    SetState(STATE_TERMINATED);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Reference::SetListener(IN IOnReferenceListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::SetReplaces(IN CONST AString &strSessionId)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_INITIATED)
    {
        IMS_TRACE_E(0, "To set the Replaces header, the state MUST be an INITIATED state",
                0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if (strSessionId.IsNULL() || strSessionId.IsEmpty())
    {
        IMS_TRACE_E(0, "Session id is null or empty string", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (pReplaces != IMS_NULL)
    {
        delete pReplaces;
        pReplaces = IMS_NULL;
    }

    Replaces *pTmpReplaces
            = CallControlHelper::GetInstance()->GetReplacesFromSessionId(strSessionId);

    if (pTmpReplaces == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find a Replaces from sessionId (%s)", strSessionId.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    pReplaces = new Replaces(*pTmpReplaces);

    if (pReplaces == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::AcceptEx(IN IMS_SINT32 nStatusCode /* = 202 */,
        IN IMS_BOOL b100Trying /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISIPServerConnection *piSSC = GetServerConnection(IMessage::REFERENCE_REFER);

    if (piSSC == IMS_NULL)
        return IMS_FAILURE;

    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to REFER request failed", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    // Send a response to REFER request immediately...
    if (!SendNUpdateResponse(IMessage::REFERENCE_REFER, piSSC))
    {
        IMS_TRACE_E(0, "Sending the response to REFER request failed ...", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    // Check & create a dialog
    CheckNCreateDialog(piSSC, bFlag_ReferenceInOtherDialog);

    SetState(STATE_REFERRING);

    CloseConnection(IMessage::REFERENCE_REFER);

    // Send a NOTIFY request with 'active' sub-state
    if (b100Trying)
    {
        DoNotification(SubState::SUB_STATE_ACTIVE, AString("SIP/2.0 100 Trying"));
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::ReferEx(IN IMS_BOOL bImplicitSubscription,
        IN CONST AString &strHeadersForReferTo /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        IMS_TRACE_E(0, "To send the reference request, the state MUST be an INITIATED state",
                0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISIPDialog *piDialog = GetDialog();
    SIPMethod objMethod(SIPMethod::REFER);
    ISIPClientConnection *piSCC = IMS_NULL;

    if (piDialog == IMS_NULL)
    {
        piSCC = CreateConnection(objMethod);
    }
    else
    {
        piSCC = CreateConnectionL(piDialog, objMethod);
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    if (SIPFeatures::IsEventHeaderApplicableForRefer(GetSlotId()))
    {
        // Event header
        piSIPMsg->SetHeader(ISIPHeader::EVENT, EVENT_REFER);
    }

    // Refer-To header
    SIPAddress objReferToURI;

    if (!objReferToURI.Create(strReferToURI))
    {
        IMS_TRACE_E(0, "Parsing Refer-To (URI) failed", 0, 0, 0);
        IMS::SetLastError(IMSError::PARSING_ERROR);
        return IMS_FAILURE;
    }

    if (!objReferMethod.Equals(SIPMethod::INVALID))
    {
        if (objReferToURI.SetParameter(SIP::STR_METHOD,
                objReferMethod.ToString().GetStr()) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting a 'method' parameter failed", 0, 0, 0);
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    if (pReplaces != IMS_NULL)
    {
        objReferToURI.SetHeader(ISIPHeader::REPLACES, pReplaces->ToString(IMS_TRUE));
    }

    // Sets the additional header parameters
    if (strHeadersForReferTo.GetLength() > 0)
    {
        if (objReferToURI.SetHeaders(strHeadersForReferTo, IMS_FALSE) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting the additional header parameters (%s) failed",
                    strHeadersForReferTo.GetStr(), 0, 0);
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    piSIPMsg->SetHeader(ISIPHeader::REFER_TO, objReferToURI.ToString());

    if (SIPFeatures::IsReferSubHeaderSupported(GetSlotId()))
    {
        // Supported header : "norefersub"
        piSIPMsg->AddHeader(ISIPHeader::SUPPORTED, "norefersub");

        // Refer-Sub header : "true" / "false"
        if (bImplicitSubscription)
        {
            piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
                    TextParser::STR_SMALL_TRUE, SIPHeaderName::REFER_SUB);
        }
        else
        {
            piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
                    TextParser::STR_SMALL_FALSE, SIPHeaderName::REFER_SUB);
        }
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::REFERENCE_REFER, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    pSubState->SetOperation(SubState::OPERATION_CREATE);

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    // Store the flag to indicate if the implicit subscription is requested or not
    bFlag_ImplicitSubscription = bImplicitSubscription;

    // Update the dialog state for REFER-created dialog (to handle early NOTIFY request)
    if (bFlag_ReferenceInOtherDialog)
    {
        CheckNCreateDialog(piSCC, IMS_TRUE);
    }

    SetState(STATE_PROCEEDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::RejectEx(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPStatusCode::IsFinalFailure(nStatusCode))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISIPServerConnection *piSSC = GetServerConnection(IMessage::REFERENCE_REFER);

    if (piSSC == IMS_NULL)
        return IMS_FAILURE;

    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to REFER request failed", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    // Send a response to REFER request immediately...
    if (!SendNUpdateResponse(IMessage::REFERENCE_REFER, piSSC))
    {
        IMS_TRACE_E(0, "Sending the response to REFER request failed ...", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    SetState(STATE_TERMINATED);

    CloseConnection(IMessage::REFERENCE_REFER);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Reference::SendNotification(IN IMS_SINT32 nSubState, IN CONST ByteArray &objContent,
        IN IMS_SINT32 nReason /*= ISubscriptionState::REASON_NONE*/,
        IN IMS_SINT32 nExpires /*= (-1)*/)
{
    if (GetState() != STATE_REFERRING)
    {
        IMS_TRACE_E(0, "Notification can't be sent in %s", StateToString(GetState()), 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    nSubState = SubState::GetSubStateFromSubscriptionState(nSubState);

    if (nSubState == SubState::SUB_STATE_INIT)
    {
        IMS_TRACE_E(0, "Invalid sub-state", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    nReason = SubState::GetReasonFromSubscriptionState(nReason);

    return DoNotification(nSubState, objContent, nReason, nExpires);
}

/*

Remarks

*/
PUBLIC
void Reference::SetNotificationListener(IN IOnNotificationListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piNotificationListener = piListener;
}

/*

Remarks

*/
PUBLIC
void Reference::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    //---------------------------------------------------------------------------------------------

    bFlag_ImplicitRoutingRequired = bFlag;

    // FIXME: If the routing address needs to be provisioned by the application,
    // please add a second argument for it.
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Reference::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
    case AMSG_REFERENCE_DELIVERED:
        if (piListener != IMS_NULL)
        {
            piListener->OnReference_Delivered(this);
        }
        return IMS_TRUE;

    case AMSG_REFERENCE_DELIVERY_FAILED:
        if (piListener != IMS_NULL)
        {
            piListener->OnReference_DeliveryFailed(this);
        }
        return IMS_TRUE;

    case AMSG_REFERENCE_NOTIFY:
        if (!objNotifyMessages.IsEmpty())
        {
            Message *pMessage = objNotifyMessages.GetAt(0);

            if (piListener != IMS_NULL)
            {
                piListener->OnReference_NotifyReceived(this, pMessage);
            }

            objNotifyMessages.RemoveAt(0);
            delete pMessage;
        }
        return IMS_TRUE;

    case AMSG_REFERENCE_TERMINATED:
        if (piListener != IMS_NULL)
        {
            piListener->OnReference_Terminated(this);
        }
        return IMS_TRUE;

    case AMSG_NOTIFICATION_DELIVERED:
        if (piNotificationListener != IMS_NULL)
        {
            piNotificationListener->OnNotification_Delivered(this);
        }
        return IMS_TRUE;

    case AMSG_NOTIFICATION_DELIVERY_FAILED:
        if (piNotificationListener != IMS_NULL)
        {
            piNotificationListener->OnNotification_DeliveryFailed(this,
                    LONG_TO_SINT(objMSG.nLparam));
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
PROTECTED VIRTUAL
IMS_BOOL Reference::SendRequestToChallenge(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Clear the connection to preserve the SIP connection
    ClearConnection(IMessage::REFERENCE_REFER);

    if (!SendNUpdateRequestEx(IMessage::REFERENCE_REFER, piSCC, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(IMessage::REFERENCE_REFER, piSCC);
        return IMS_FALSE;
    }

    // Update the subscription state
    if (!pSubState->UpdateState(piSCC->GetMessage()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Reference::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    (void) nErrorCode;

    if (GetState() == STATE_PROCEEDING)
    {
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);
    }
    else
    {
        // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
#endif
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Reference::InitInstance()
{
    //---------------------------------------------------------------------------------------------

    if (pSubState == IMS_NULL)
    {
        if (IsMobileOriginated())
        {
            pSubState = new ImplicitSubscriberState();
        }
        else
        {
            pSubState = new ImplicitNotifierState();
            pNotifierState = new NotifierState();
        }

        if (pSubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscription state (MO: %s) failed",
                TextParser::BooleanToString(IsMobileOriginated()), 0, 0);
            return IMS_FALSE;
        }

        if (!pSubState->CreateEventPackage(EVENT_REFER))
        {
            IMS_TRACE_E(0, "Creating an event package for an event (refer) failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    GetService()->RegisterMethod(this);

    if (GetDialog() != IMS_NULL)
    {
        bFlag_ReferenceInOtherDialog = IMS_TRUE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Reference::NotifySIPRequest(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Reference - REFER REQUEST RECEIVED ...", 0, 0, 0);

    if (!UpdateRequestOnReceived(IMessage::REFERENCE_REFER, piSSC))
    {
        IMS_TRACE_E(0, "Updating SIP message failed", 0, 0, 0);
    }

    // Check if the response needs to be handled by application.
    const SipConfigV *pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->IsReferenceRespByApp())
        {
            IMS_TRACE_I("INCOMING REFER REQUEST WILL BE HANDLED BY APPLICATION", 0, 0, 0);

            // Notify the information which the Reference is received
            if (!bFlag_ReferenceInOtherDialog)
            {
                GetService()->HandleReferenceReceived(this);
            }

            return IMS_TRUE;
        }
    }

    // Send a 202 ACCEPTED to REFER request
    if (CreateResponse(piSSC, SIPStatusCode::SC_202) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to REFER request failed", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FALSE;
    }

    // Send a response to REFER request immediately...
    if (!SendNUpdateResponse(IMessage::REFERENCE_REFER, piSSC))
    {
        IMS_TRACE_E(0, "Sending the response to REFER request failed ...", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FALSE;
    }

    // Check & create a dialog
    CheckNCreateDialog(piSSC, bFlag_ReferenceInOtherDialog);

    SetState(STATE_PROCEEDING);

    CloseConnection(IMessage::REFERENCE_REFER);

    // Send a NOTIFY request with 'active' sub-state
    DoNotification(SubState::SUB_STATE_ACTIVE, AString("SIP/2.0 100 Trying"));

    if (!bFlag_ReferenceInOtherDialog)
    {
        GetService()->HandleReferenceReceived(this);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Reference::NotifySIPResponse(IN ISIPClientConnection *piSCC)
{
    const SIPMethod &objMethod = piSCC->GetMethod();

    //---------------------------------------------------------------------------------------------

    // REFER-Issuer behavior
    if (objMethod.Equals(SIPMethod::REFER))
    {
        if (!pSubState->UpdateState(piSCC->GetMessage()))
        {
            CloseConnection(IMessage::REFERENCE_REFER);
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);

            IMS_TRACE_E(0, "Updating an implicit subscription failed", 0, 0, 0);
            return;
        }

        // Add the response message received
        if (!UpdateResponseOnReceived(IMessage::REFERENCE_REFER, piSCC))
        {
            CloseConnection(IMessage::REFERENCE_REFER);
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);

            IMS_TRACE_E(0, "Storing Message failed", 0, 0, 0);
            return;
        }

        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        // AUTH_SIP_DIGEST {
        // Handle 401/407 response
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
        // }

        // Check the status code
        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            CheckNCreateDialog(piSCC, bFlag_ReferenceInOtherDialog);

            CloseConnection(IMessage::REFERENCE_REFER);

            SetState(STATE_REFERRING);
            PostMessage(AMSG_REFERENCE_DELIVERED, 0, 0);
        }
        else
        {
            CloseConnection(IMessage::REFERENCE_REFER);

            SetState(STATE_TERMINATED);

            if (bFlag_ImplicitSubscription)
                PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);
            else
                PostMessage(AMSG_REFERENCE_DELIVERED, 0, 0);
        }
    }
    // REFER-Recipient behavior
    else if (objMethod.Equals(SIPMethod::NOTIFY))
    {
        // Update the subscription state for NOTIFY response
        pSubState->UpdateState(piSCC->GetMessage());

        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            PostMessage(AMSG_NOTIFICATION_DELIVERED, 0, 0);
        }
        else
        {
            PostMessage(AMSG_NOTIFICATION_DELIVERY_FAILED, 0, nStatusCode);
        }

        if (pNotifierState != IMS_NULL)
        {
            pNotifierState->RemoveSCC(piSCC);
        }

        piSCC->Close();

        if (pSubState->GetState() == SubState::STATE_TERMINATED)
        {
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
        }
    }
    else
    {
        IMS_TRACE_E(0, "___ NOT HANDLED METHOD (%s) ___", objMethod.ToString().GetStr(), 0, 0);

        piSCC->Close();
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Reference::NotifySIPError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
        IN CONST AString &strMessage)
{
    const SIPMethod &objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void) nCode;
    (void) strMessage;

    //1 : Needs to be handled according to the SIP method (REFER/NOTIFY) ?

    if (objMethod.Equals(SIPMethod::REFER))
    {
        CloseConnection(IMessage::REFERENCE_REFER);
    }
    else if (objMethod.Equals(SIPMethod::NOTIFY))
    {
        IMS_TRACE_E(0, "NOTIFY transaction will be closed", 0, 0, 0);

        PostMessage(AMSG_NOTIFICATION_DELIVERY_FAILED, 0, 0);

        if (pNotifierState != IMS_NULL)
        {
            pNotifierState->RemoveSCC(DYNAMIC_CAST(ISIPClientConnection*, piSC));
        }

        piSC->Close();
    }

    if (GetState() == STATE_PROCEEDING)
    {
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);
    }
    else
    {
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Reference::Dialog_Compare(IN ISIPServerConnection *piSSC) const
{

    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        return IMS_FALSE;
    }

    // Filters some method which does not handle in the reference (implicit subscription)
    if (!piSSC->GetMethod().Equals(SIPMethod::NOTIFY))
    {
        return IMS_FALSE;
    }

    ISIPDialog *piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        // In case of an early NOTIFY received ...
        if (GetState() == STATE_PROCEEDING)
        {
            ISIPClientConnection *piSCC = GetClientConnection(IMessage::REFERENCE_REFER);

            IMS_TRACE_I("Checks if the early NOTIFY is received or not ...", 0, 0, 0);

            if (piSCC != IMS_NULL)
            {
                piDialog = piSCC->GetDialog();
            }
        }
    }

    if (piDialog == IMS_NULL)
    {
        IMS_TRACE_D("No dialog exists", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piDialog->IsSameDialog(piSSC))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Reference::Dialog_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    const SIPMethod& objMethod = piSSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    if (!objMethod.Equals(SIPMethod::NOTIFY))
    {
        // Handling of OPTIONS request
        if (objMethod.Equals(SIPMethod::OPTIONS))
        {
            if (Capabilities::HandleOPTIONSRequestWithinDialog(
                    GetService(), this, piSSC) != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        GetService()->SendResponse(piSSC, SIPStatusCode::SC_405);
        piSSC->Close();

        IMS_TRACE_E(0, "___ UNHANDLED SIP METHOD (%s) ___",
                piSSC->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (SIPConfigProxy::IsRequestUriValidationRequiredInMidDialog(
            GetSlotId(), GetService()->GetSIPProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString &strRequestURI = piSSC->GetRequestURI();
        SIPAddress objRequestURI(strRequestURI);

        if (!GetService()->ValidateRequestURI(objRequestURI, piSSC->GetDialog(), IMS_TRUE))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SIPDebug::GetUri1(strRequestURI).GetStr(), 0, 0);

            GetService()->SendResponse(piSSC, SIPStatusCode::SC_404);
            piSSC->Close();
            return IMS_FALSE;
        }
    }

    // Update the dialog info.
    if (GetState() == STATE_PROCEEDING)
    {
        CheckNCreateDialog(piSSC, bFlag_ReferenceInOtherDialog);
    }

    // Update the subscription state for NOTIFY request
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        piSSC->Close();
        return IMS_FALSE;
    }

    Message *pMessage
            = Message::CreateReceivedMessage(GetService()->GetAppConfig(), piSSC->GetMessage());

    if (pMessage == IMS_NULL)
    {
        // Internal error ... ???
        if (GetService()->SendResponse(piSSC, SIPStatusCode::SC_500))
        {
            pSubState->UpdateState(piSSC->GetMessage());
        }

        piSSC->Close();
        return IMS_FALSE;
    }

    // Send a 2xx response to NOTIFY request immediately
    if (GetService()->SendResponse(piSSC, SIPStatusCode::SC_200) == IMS_FALSE)
    {
        delete pMessage;
        piSSC->Close();

        IMS_TRACE_E(0, "Creating & sending the response to NOTIFY request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update the subscription state for NOTIFY response
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        delete pMessage;
        piSSC->Close();

        return IMS_FALSE;
    }

    if (!objNotifyMessages.Append(pMessage))
    {
        delete pMessage;

        IMS_TRACE_E(0, "Queueing NOTIFY message failed", 0, 0, 0);
    }

    piSSC->Close();

    PostMessage(AMSG_REFERENCE_NOTIFY, 0, 0);

    if (pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Reference::ReferredMessage_NotifyOnActive(IN ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    //1 In this moment, only includes the start line

    if (piSIPMsg == IMS_NULL)
    {
        // Nothing to do...
        return;
    }

    //4 ACK ?
    if (!objReferMethod.Equals(SIPMethod::INVALID))
    {
        if (!objReferMethod.Equals(piSIPMsg->GetMethod()))
        {
            IMS_TRACE_E(0, "Not matched method (Refer-To: %s, Message: %s)",
                    objReferMethod.ToString().GetStr(),
                    piSIPMsg->GetMethod().ToString().GetStr(), 0);
            return;
        }
    }
    else
    {
        // As a default, INVITE method
        if (!piSIPMsg->GetMethod().Equals(SIPMethod::INVITE))
        {
            IMS_TRACE_E(0, "Not matched method (Refer-To: empty, Message: %s)",
                    piSIPMsg->GetMethod().ToString().GetStr(), 0, 0);
            return;
        }
    }

    ByteArray objContent = piSIPMsg->ToByteArray(ISIPMessage::OPT_START_LINE);

    if (objContent.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Creating a message/sipfrag failed", 0, 0, 0);
        return;
    }

    DoNotification(SubState::SUB_STATE_ACTIVE, objContent);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Reference::ReferredMessage_NotifyOnTerminated(
        IN IMS_SINT32 nReasonCode /* = SubState::REASON_NONE */,
        IN ISIPMessage *piSIPMsg /* = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    //1 In this moment, only includes the start line

    ByteArray objContent;

    if (piSIPMsg != IMS_NULL)
    {
        if (!objReferMethod.Equals(piSIPMsg->GetMethod()))
        {
            IMS_TRACE_E(0, "Not matched method (Refer-To: %s, Message: %s)",
                    objReferMethod.ToString().GetStr(),
                    piSIPMsg->GetMethod().ToString().GetStr(), 0);
            return;
        }

        objContent = piSIPMsg->ToByteArray(ISIPMessage::OPT_START_LINE);
    }

    DoNotification(SubState::SUB_STATE_TERMINATED, objContent, nReasonCode);

    if (pReferredMethod != IMS_NULL)
    {
        pReferredMethod->SetReferredMessageListener(IMS_NULL);
        pReferredMethod = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE
void Reference::CleanupOnDestroy()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_TERMINATED)
    {
        ISIPDialog *piDialog = GetDialog();

        if (piDialog != IMS_NULL)
        {
            piDialog->TerminateDialogUsage();
        }

        SetState(STATE_TERMINATED);
    }

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    GetService()->DeregisterMethod(this);
}

/*

Remarks

*/
PRIVATE
ISIPClientConnection* Reference::CreateConnectionL(IN ISIPDialog *piDialog,
        IN CONST SIPMethod &objMethod)
{
    ISIPClientConnection *piSCC = CreateConnection(piDialog, objMethod);

    //---------------------------------------------------------------------------------------------

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    if (bFlag_ImplicitRoutingRequired && (piSCC != IMS_NULL))
    {
        const AStringArray &objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piSCC->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }

    return piSCC;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Reference::DoNotification(IN IMS_SINT32 nSubState, IN CONST ByteArray &objContent,
        IN IMS_SINT32 nReasonCode /* = SubState::REASON_NONE */,
        IN IMS_SINT32 nExpires /* = (-1) */)
{
    ISIPDialog *piDialog = GetDialog();

    //---------------------------------------------------------------------------------------------

    if (piDialog == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    SIPMethod objMethod(SIPMethod::NOTIFY);
    ISIPClientConnection *piSCC = GetService()->CreateConnection(piDialog, objMethod);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    piSCC->SetListener(this);
    piSCC->SetErrorListener(this);

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    if (bFlag_ImplicitRoutingRequired)
    {
        const AStringArray &objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piSCC->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Event header
    piSIPMsg->SetHeader(ISIPHeader::EVENT, EVENT_REFER);

    // Subscription-State header
    AString strSubState;

    if (nSubState == SubState::SUB_STATE_ACTIVE)
    {
        strSubState = SIP::STR_ACTIVE;

        // 'expires' parameter ???
    }
    else if (nSubState == SubState::SUB_STATE_PENDING)
    {
        strSubState = SIP::STR_PENDING;
    }
    else if (nSubState == SubState::SUB_STATE_TERMINATED)
    {
        strSubState = SIP::STR_TERMINATED;

        if ((nReasonCode > SubState::REASON_NONE)
                && (nReasonCode < SubState::REASON_MAX))
        {
            strSubState += TextParser::CHAR_SEMICOLON;
            strSubState += SubState::STR_REASON;
            strSubState += TextParser::CHAR_EQUAL;

            switch (nReasonCode)
            {
            case SubState::REASON_DEACTIVATED:
                strSubState += SubState::STR_REASON_DEACTIVATED;
                break;
            case SubState::REASON_PROBATION:
                strSubState += SubState::STR_REASON_PROBATION;
                break;
            case SubState::REASON_REJECTED:
                strSubState += SubState::STR_REASON_REJECTED;
                break;
            case SubState::REASON_TIMEOUT:
                strSubState += SubState::STR_REASON_TIMEOUT;
                break;
            case SubState::REASON_GIVEUP:
                strSubState += SubState::STR_REASON_GIVEUP;
                break;
            case SubState::REASON_NORESOURCE:
                strSubState += SubState::STR_REASON_NORESOURCE;
                break;
            default:
                break;
            }
        }
    }

    if (nExpires >= 0)
    {
        AString strExpires;
        strExpires.SetNumber(nExpires);

        strSubState += TextParser::CHAR_SEMICOLON;
        strSubState += SIP::STR_EXPIRES;
        strSubState += TextParser::CHAR_EQUAL;
        strSubState += strExpires;
    }

    piSIPMsg->SetHeader(ISIPHeader::SUBSCRIPTION_STATE, strSubState);

    // Set contents (message/sipfrag)
    if (!objContent.IsNULL())
    {
        piSIPMsg->SetHeader(ISIPHeader::CONTENT_TYPE, MEDIA_TYPE);

        ISIPMessageBodyPart *piSIPBodyPart = piSIPMsg->CreateBodyPart();

        if (piSIPBodyPart == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a new SIP message body part failed", 0, 0, 0);
            piSCC->Close();
            return IMS_FAILURE;
        }

        piSIPBodyPart->SetContent(objContent);
    }

    // SIP_MESSAGE_MEDIATOR
    (void) AdjustMessage(piSCC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

    // Try to send a NOTIFY request to the network
    if (piSCC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a NOTIFY request through the implicit subscription failed",
                0, 0, 0);
        piSCC->Close();
        return IMS_FAILURE;
    }

    pSubState->UpdateState(piSCC->GetMessage());

    if (pNotifierState != IMS_NULL)
    {
        pNotifierState->AddSCC(piSCC);
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
void Reference::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Reference :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* Reference::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
    case STATE_INITIATED:
        return "STATE_INITIATED";
    case STATE_PROCEEDING:
        return "STATE_PROCEEDING";
    case STATE_REFERRING:
        return "STATE_REFERRING";
    case STATE_TERMINATED:
        return "STATE_TERMINATED";
    default:
        return "__INVALID__";
    }
}
