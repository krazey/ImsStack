/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IMSLib.h"
#include "private/SipConfigV.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "ISipAckPackage.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipHeaderName.h"
#include "SipHeaderUtils.h"
#include "SipParsingHelper.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "SipParameter.h"
#include "SipConfigProxy.h"
#include "SipTimerValuesHelper.h"

#include "base/IMS.h"
#include "base/IRefreshListener.h"
#include "util/OperatorFeatureResolver.h"
#include "util/UserAgentHeader.h"
#include "Service.h"
#include "util/MethodManager.h"
// CALLER_PREFERENCE_MANAGER
#include "util/CallerPreferenceManager.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"
#include "SDPOAState.h"
#include "Capabilities.h"
#include "Reference.h"
#include "Subscription.h"
#include "CallControlHelper.h"
#include "Replaces.h"
#include "SessionRefreshHelper.h"
#include "SessionDescriptor.h"
#include "RetransmissionHelper.h"
#include "media/MediaFactory.h"
#include "IOnSessionListener.h"
#include "offeranswer/SdpProfile.h"
#include "Session.h"

#define __IMS_SEND_ACK_IN_TERMINATING_STATE__

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
Session::Session(IN Service *pService_)
    : ServiceMethod(pService_)
    , nState(STATE_CREATED)
    , bFlag_Alerting(IMS_FALSE)
    , bFlag_SDPInInitialINVITE(IMS_FALSE)
    , bFlag_TerminatePending(IMS_FALSE)
    , bFlag_UpdateRequestor(IMS_FALSE)
    , bFlag_SDPNonRPRAllowed(IMS_FALSE)
    , bFlag_100TryingNotification(IMS_FALSE)
    , bFlag_TerminateMethodBYE(IMS_FALSE)
    , bFlag_SessionUpdateNotificationInProgress(IMS_FALSE)
    , bFlag_ImplicitRoutingRequired(IMS_FALSE)
    , bFlag_AckWithSDPInProgress(IMS_FALSE)
    , nConfigValue(CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE)
    , nCompletedListenerCalls(0)
    , nTerminationReason(TERMINATION_REASON_UNKNOWN)
    , strTerminationReasonFromApp(AString::ConstNull())
    , pOAState(IMS_NULL)
    , pSessionDescriptor(IMS_NULL)
    , piSessionListener(IMS_NULL)
    , piRefreshListener(IMS_NULL)
    , pRefreshHelper(IMS_NULL)
    , pRetransmissionHelper(IMS_NULL)
    , piReferredMessageListener(IMS_NULL)
#ifdef __IMS_SESSION_RETRY_TASK__
    , pRetransmissionTask(IMS_NULL)
#endif
    , piAckPackage(IMS_NULL)
    , strSessionIdForCallControl(AString::ConstNull())
    , piSCC_BYE(IMS_NULL)
    , objPreviousCallerPreference(IMSList<AString>())
    , pForkedSessions(IMS_NULL)
    , pVirtualEarlySession(IMS_NULL)
{
}

PUBLIC VIRTUAL
Session::~Session()
{
    //---------------------------------------------------------------------------------------------

    CleanupOnDestroy();

    if (pRefreshHelper != IMS_NULL)
    {
        delete pRefreshHelper;
        pRefreshHelper = IMS_NULL;
    }

    if (pRetransmissionHelper != IMS_NULL)
    {
        delete pRetransmissionHelper;
        pRetransmissionHelper = IMS_NULL;
    }

#ifdef __IMS_SESSION_RETRY_TASK__
    if (pRetransmissionTask != IMS_NULL)
    {
        delete pRetransmissionTask;
        pRetransmissionTask = IMS_NULL;
    }
#endif

    if (piAckPackage != IMS_NULL)
    {
        piAckPackage->Destroy();
        piAckPackage = IMS_NULL;
    }

    if (pSessionDescriptor != IMS_NULL)
    {
        delete pSessionDescriptor;
        pSessionDescriptor = IMS_NULL;
    }

    if (!objMedias.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            Media *pMedia = objMedias.GetAt(i);

            MediaFactory::DestroyMedia(pMedia);
        }

        objMedias.Clear();
    }

    if (pOAState != IMS_NULL)
    {
        delete pOAState;
        pOAState = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Session::Destroy()
{
    //---------------------------------------------------------------------------------------------

    CleanupOnDestroy();
    ServiceMethod::Destroy();
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PUBLIC VIRTUAL
void Session::SetMessageMediator(IN IMessageMediator *piMediator)
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
IMS_RESULT Session::Accept()
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_NEGOTIATING)
            && (nState != STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To accept a session, the state MUST be a NEGOTIATING or RENEGOTIATING; " \
                "(%s)", StateToString(nState), 0, 0);
        return IMS_FAILURE;
    }

    if (HasPendingPRAck())
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "UAS MUST delay sending 2XX until the provisional response " \
                "is acknowledged", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPServerConnection *piSSC = IMS_NULL;

    // Create an answer if offer received

    if (nState == STATE_NEGOTIATING)
        piSSC = GetServerConnection(IMessage::SESSION_START);
    else
        piSSC = GetServerConnection(IMessage::SESSION_UPDATE);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSSC, SIPStatusCode::SC_200) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    // Session-Expires
    // Require
    if (pRefreshHelper != IMS_NULL)
    {
        if (!SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
        {
            if ((nState == STATE_RENEGOTIATING) && piSSC->GetMethod().Equals(SIPMethod::INVITE))
            {
                if (!pRefreshHelper->AddSpecificHeaderWithoutParameterChange(piSSC))
                {
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
            else
            {
                if (!pRefreshHelper->AddSpecificHeader(piSSC))
                {
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            if (!pRefreshHelper->AddSpecificHeader(piSSC))
            {
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    if (nState == STATE_NEGOTIATING)
    {
        CheckNSetSDPBodyPart(piSIPMsg);

        if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
        {
            CloseConnection(IMessage::SESSION_START);

            // Throw exception: INVALID_OPERATION
            SetState(STATE_TERMINATED);
            CleanupMedia();

            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            return IMS_FAILURE;
        }

        // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
        // When INVITE_TXN_HANDLING_CORRECTION is disabled:
        // CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());

        // Update the media information
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());

        // Case) Initial offer is sent by 200 OK
        if (GetOfferAnswerState() == SDPOAState::STATE_OFFER_SENT)
        {
            UpdateMedia(Media::SESSION_START);
        }
        else
        {
            UpdateMedia(Media::SESSION_STARTED);

            // 'Replaces' header handling ...
            // RACE_CONDITION: ACK & re-INVITE in MT
            AddSessionToCallControlHelperIfNotPresent();
        }

        SetState(STATE_ESTABLISHING);
    }
    else
    {
        // If re-INVITE is received with SDP, it will be set by the SDP offer/answer context
        // If no SDP in the re-INVITE, generate the SDP from the current view (capabilities)
        if (piSIPMsg->GetMethod().Equals(SIPMethod::INVITE)
                && (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
        {
            if ((pOAState != IMS_NULL) && (pOAState->IsOfferProgress()))
            {
                // Offer change will be sent in 200 OK to re-INVITE request
                CheckNSetSDPBodyPart(piSIPMsg);
            }
            else
            {
                SetSDPBodyPartFromCurrentView(piSIPMsg);
            }
        }
        else
        {
            CheckNSetSDPBodyPart(piSIPMsg);
        }

        // For in-dialog INVITE request... (when ACK waiting timer expired)
        if (piSIPMsg->GetMethod().Equals(SIPMethod::INVITE))
        {
            piSSC->SetErrorListener(this);
        }

        if (!SendNUpdateResponse(IMessage::SESSION_UPDATE, piSSC))
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            // Throw exception: INVALID_OPERATION
            SetState(STATE_ESTABLISHED);
            CleanupMedia();

            // CANCEL for re-INVITE
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
        // When INVITE_TXN_HANDLING_CORRECTION is disabled:
        // CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        // Update the media information
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());

        // Case) Offer is sent by 200 OK
        if (GetOfferAnswerState() == SDPOAState::STATE_OFFER_CHANGE_SENT)
            UpdateMedia(Media::SESSION_UPDATE);
        else
            UpdateMedia(Media::SESSION_UPDATED);

        if (piSSC->GetMethod().Equals(SIPMethod::INVITE))
            SetState(STATE_REESTABLISHING);
        else
            SetState(STATE_ESTABLISHED);
    }

    // Create a dialog after sending 2xx without 1xx response with a tag
    CheckNCreateDialog(piSSC);

    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->UpdateOnMessageSent(piSSC);
    }

    Stop2xxRetransmission();

    if (piSSC->GetMethod().Equals(SIPMethod::INVITE))
    {
        Start2xxRetransmission();

        // CALLER_PREFERENCE_MANAGER
        if (nState == STATE_NEGOTIATING)
        {
            ISIPDialog *piDialog = GetDialog();

            if (piDialog != IMS_NULL)
            {
                CallerPreferenceManager::GetInstance()->UpdateDialogId(GetName(),
                        piDialog->GetDialogID());
            }
        }
    }

    IMS_TRACE_D("Session is accepted", 0, 0, 0);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
Capabilities* Session::CreateCapabilities()
{
    //---------------------------------------------------------------------------------------------

    if (!IsMidDialogTransactionCreatable())
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Capabilities can't be created in the state(%s)",
                StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    // If the Capabilities can not be created, throw exception
    Capabilities *pCapabilities = new Capabilities(GetService());

    if (pCapabilities == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pCapabilities->InitMethod(this))
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
Media* Session::CreateMedia(IN CONST AString &strType, IN IMS_SINT32 nDirection,
        IN IMS_SINT32 nCountOfDescriptor /* = 0 */, IN IMS_BOOL bIMSExtension /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SDPOAState is null", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_NULL;
    }

    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED)
            && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSDPOAState = GetOfferAnswerState();

        if ((nSDPOAState != SDPOAState::STATE_IDLE)
                && (nSDPOAState != SDPOAState::STATE_ESTABLISHED))
        {
            IMS_TRACE_E(0, "To create a media, the state MUST be an INITIALIZED or ESTABLISHED; " \
                    "(%s, %d)", StateToString(nState), nSDPOAState, 0);
            IMS::SetLastError(IMSError::ILLEGAL_STATE);
            return IMS_NULL;
        }
    }

    if ((nDirection != Media::DIRECTION_INACTIVE)
            && (nDirection != Media::DIRECTION_RECEIVE)
            && (nDirection != Media::DIRECTION_SEND)
            && (nDirection != Media::DIRECTION_SEND_RECEIVE))
    {
        if (bIMSExtension && (nDirection == Media::DIRECTION_NONE))
        {
            // Do not specify the SDP direction in the media-level description
        }
        else
        {
            IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Direction argument is invalid", 0, 0, 0);
            return IMS_NULL;
        }
    }

    if (objMedias.IsEmpty())
    {
        if (!CheckNCreateSessionDescriptor())
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Can't create an initial SDP offer", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Create a new SDP media parameter

    Media *pMedia = MediaFactory::CreateOutgoingMedia(
            strType, nDirection, GetService(), pOAState, nCountOfDescriptor);

    if (pMedia == IMS_NULL)
    {
        // Convert the media error code to the IMS error code

        IMS_TRACE_E(0, "Creating Media (%s) failed", strType.GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!AddMedia(pMedia))
    {
        MediaFactory::DestroyMedia(pMedia);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pMedia;
}

/*

Remarks

*/
PUBLIC
Reference* Session::CreateReference(IN CONST AString &strReferTo, IN CONST AString &strReferMethod)
{
    //---------------------------------------------------------------------------------------------

    if (!IsMidDialogTransactionCreatable())
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Reference can't be created in the state(%s)",
                StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    if (!Service::ValidateReferTo(strReferTo, strReferMethod))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // If the Reference can not be created, throw exception
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    Reference *pReference = new Reference(GetService(),
            strReferTo, strReferMethod, Replaces(), bFlag_ImplicitRoutingRequired);

    if (pReference == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pReference->InitMethod(this))
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
Subscription* Session::CreateSubscription(IN CONST AString &strEvent)
{
    //---------------------------------------------------------------------------------------------

    if (!IsMidDialogTransactionCreatable())
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Subscription can't be created in the state(%s)",
                StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    // Checks an event package from the application configuration
    if (!GetService()->IsEventPackageSupported(strEvent))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    // If the subscriptino can not be created, throw exception
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    Subscription *pSubscription = new Subscription(
            GetService(), strEvent, bFlag_ImplicitRoutingRequired);

    if (pSubscription == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Subscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSubscription->InitMethod(this))
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
ISIPClientConnection* Session::CreateTransaction(IN CONST SIPMethod &objMethod)
{
    //---------------------------------------------------------------------------------------------

    switch (objMethod.ToInt())
    {
    case SIPMethod::ACK:
    case SIPMethod::BYE:
    case SIPMethod::CANCEL:
    case SIPMethod::INVITE:
    case SIPMethod::OPTIONS:
    case SIPMethod::PRACK:
    case SIPMethod::REFER:
    case SIPMethod::UPDATE:
        IMS_TRACE_E(0, "Method (%s) is not allowed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;

    default:
        break;
    }

    ISIPDialog *piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't create a mid-call transaction; Dialog is null", 0, 0, 0);
        return IMS_NULL;
    }

    return GetService()->CreateConnection(piDialog, objMethod);
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 Session::GetConfiguration() const
{
    //---------------------------------------------------------------------------------------------

    return nConfigValue;
}

/*

Remarks

*/
PUBLIC
const ISIPHeader* Session::GetContactHeader() const
{
    ISIPDialog *piDialog = GetDialog();

    //---------------------------------------------------------------------------------------------

    if (piDialog != IMS_NULL)
    {
        IMS_SINT32 nDState = piDialog->GetState();

        if ((nDState == ISIPDialog::STATE_EARLY)
                || (nDState == ISIPDialog::STATE_CONFIRMED))
        {
            return piDialog->GetContactHeader();
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
const Replaces* Session::GetReplaces() const
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_NEGOTIATING)
            && (nState != STATE_ESTABLISHING)
            && (nState != STATE_ESTABLISHED)
            && (nState != STATE_RENEGOTIATING)
            && (nState != STATE_REESTABLISHING))
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    if (GetSessionId().GetLength() <= 0)
    {
        IMS_TRACE_E(0, "Session id is invalid", 0, 0, 0);
        return IMS_NULL;
    }

    return CallControlHelper::GetInstance()->GetReplacesFromSessionId(GetSessionId());
}

/*

Remarks

*/
PUBLIC
const AString& Session::GetSessionId() const
{
    //---------------------------------------------------------------------------------------------

    return strSessionIdForCallControl;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 Session::GetTerminationReason() const
{
    //---------------------------------------------------------------------------------------------

    // Checks if session already notifies the terminated event.
    if ((nCompletedListenerCalls & LISTENER_CALL_TERMINATED) != 0)
    {
        return nTerminationReason;
    }

    if (GetState() != STATE_TERMINATED)
    {
        IMS_TRACE_E(0, "The termination reason can't be retrieved in the %s",
                StateToString(GetState()), 0, 0);
        return TERMINATION_REASON_INVALID;
    }

    return nTerminationReason;
}

/*

Remarks

*/
PUBLIC
const IMSList<Media*>& Session::GetMedia() const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_TERMINATED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To get a media, the state MUST not be a TERMINATED; (%s)",
                StateToString(GetState()), 0, 0);
        return objMedias;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return objMedias;
}

/*

Remarks

*/
PUBLIC
SessionDescriptor* Session::GetSessionDescriptor()
{
    //---------------------------------------------------------------------------------------------

    if (!CheckNCreateSessionDescriptor())
    {
        return IMS_NULL;
    }

    return pSessionDescriptor;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 Session::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Session::HasPendingUpdate() const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_ESTABLISHED)
    {
        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const Media *pMedia = objMedias.GetAt(i);

            if ((pMedia->GetState() == Media::STATE_INACTIVE)
                    || ((pMedia->GetState() == Media::STATE_ACTIVE)
                        && (pMedia->GetUpdateState() != Media::UPDATE_UNCHANGED)))
            {
                IMS_TRACE_D("Session has at least a pending update for the media", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    IMS_TRACE_D("Session has no pending update for the media", 0, 0, 0);

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Session::IsFinalResponseReceivedForInitialInviteRequest() const
{
    //---------------------------------------------------------------------------------------------

    if (pForkedSessions.IsNull())
    {
        return IsInviteFinalResponseReceived(IMessage::SESSION_START);
    }

    const IMSList<Method*>& objMethods = pForkedSessions->GetMethods();

    if (objMethods.IsEmpty())
    {
        return IsInviteFinalResponseReceived(IMessage::SESSION_START);
    }

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); i++)
    {
        const Session* pSession = DYNAMIC_CAST(const Session*, objMethods.GetAt(i));

        if (pSession == IMS_NULL)
        {
            continue;
        }

        if (pSession->IsInviteFinalResponseReceived(IMessage::SESSION_START))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Session::IsReliableProvResponseSupported() const
{
    Message *pMessage = GetPreviousRequest(IMessage::SESSION_START);

    //---------------------------------------------------------------------------------------------

    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISIPMessage *piSIPMsg = pMessage->GetMessage();

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piSIPMsg->IsOptionSupported(SIP::STR_100REL);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Session::IsSDPNegotiationAllowedForNonRPR() const
{
    return bFlag_SDPNonRPRAllowed;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::Reject()
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_NEGOTIATING)
            && (nState != STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To reject a session, the state MUST be a NEGOTIATING or RENEGOTIATING; " \
                "(%s)", StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    ISIPServerConnection *piSSC = IMS_NULL;

    // Create an answer if offer received

    if (nState == STATE_NEGOTIATING)
        piSSC = GetServerConnection(IMessage::SESSION_START);
    else
        piSSC = GetServerConnection(IMessage::SESSION_UPDATE);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSSC, SIPStatusCode::SC_486) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    if (nState == STATE_NEGOTIATING)
    {
        if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
        {
            CloseConnection(IMessage::SESSION_START);

            IMS::SetLastError(IMSError::GENERAL_ERROR);
            SetState(STATE_TERMINATED);
            CleanupMedia();
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        UpdateMedia(Media::SESSION_START_FAILED);

        CloseConnection(IMessage::SESSION_START);

        SetState(STATE_TERMINATED);

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
    }
    else
    {
        if (!SendNUpdateResponse(IMessage::SESSION_UPDATE, piSSC))
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            IMS::SetLastError(IMSError::GENERAL_ERROR);
            SetState(STATE_ESTABLISHED);
            CleanupMedia();

            // CANCEL for re-INVITE
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        // CANCEL for re-INVITE
        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        UpdateMedia(Media::SESSION_UPDATE_FAILED);
        RestoreEx();

        if (SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
        {
            pRefreshHelper->UpdateOnMessageSent(piSSC);
        }

        CloseConnection(IMessage::SESSION_UPDATE);

        SetState(STATE_ESTABLISHED);

        PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
    }

    IMS_TRACE_D("Session :: Reject ()", 0, 0, 0);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::Reject(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_NEGOTIATING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To reject a session, the state MUST be a NEGOTIATING; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    if ((nStatusCode != STATUSCODE_433_ANONYMITY_DISALLOWED)
            && (nStatusCode != STATUSCODE_480_TEMPORARILY_UNAVAILABLE)
            && (nStatusCode != STATUSCODE_486_BUSY_HERE)
            && (nStatusCode != STATUSCODE_488_NOT_ACCEPTABLE_HERE)
            && (nStatusCode != STATUSCODE_600_BUSY_EVERYWHERE)
            && (nStatusCode != STATUSCODE_603_DECLINE))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Unsupported status code (%d) to reject a session", nStatusCode, 0, 0);
        return IMS_FAILURE;
    }

    // Send a reject response

    // Create an answer if offer received

    ISIPServerConnection *piSSC = GetServerConnection(IMessage::SESSION_START);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    ISIPMessage* piSIPMsg = piSSC->GetMessage();
    SetSDPBodyPartFromRefusedView(piSIPMsg);

    if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
    {
        CloseConnection(IMessage::SESSION_START);

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        SetState(STATE_TERMINATED);
        CleanupMedia();
        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

        return IMS_FAILURE;
    }

    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC->GetMessage());
    UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
    UpdateMedia(Media::SESSION_START_FAILED);

    CloseConnection(IMessage::SESSION_START);
    SetState(STATE_TERMINATED);

    IMS_TRACE_D("Session :: Reject (%d)", nStatusCode, 0, 0);

    PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::RejectEx(IN IMS_SINT32 nStatusCode,
        IN CONST AString &strReasonPhrase /* = AString::ConstNull() */)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_NEGOTIATING)
        && (nState != STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To reject a session, the state MUST be a NEGOTIATING or RENEGOTIATING; " \
                "(%s)", StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    ISIPServerConnection *piSSC = IMS_NULL;

    // Create an answer if offer received

    if (nState == STATE_NEGOTIATING)
        piSSC = GetServerConnection(IMessage::SESSION_START);
    else
        piSSC = GetServerConnection(IMessage::SESSION_UPDATE);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSSC, nStatusCode, strReasonPhrase) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    ISIPMessage* piSIPMsg = piSSC->GetMessage();
    SetSDPBodyPartFromRefusedView(piSIPMsg);

    if (nState == STATE_NEGOTIATING)
    {
        if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
        {
            CloseConnection(IMessage::SESSION_START);

            IMS::SetLastError(IMSError::GENERAL_ERROR);
            SetState(STATE_TERMINATED);
            CleanupMedia();
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        UpdateMedia(Media::SESSION_START_FAILED);

        CloseConnection(IMessage::SESSION_START);

        SetState(STATE_TERMINATED);

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
    }
    else
    {
        if (!SendNUpdateResponse(IMessage::SESSION_UPDATE, piSSC))
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            IMS::SetLastError(IMSError::GENERAL_ERROR);
            SetState(STATE_ESTABLISHED);
            CleanupMedia();

            // CANCEL for re-INVITE
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        // CANCEL for re-INVITE
        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        UpdateMedia(Media::SESSION_UPDATE_FAILED);
        RestoreEx();

        if (SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
        {
            pRefreshHelper->UpdateOnMessageSent(piSSC);
        }

        CloseConnection(IMessage::SESSION_UPDATE);

        SetState(STATE_ESTABLISHED);

        PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
    }

    IMS_TRACE_D("Session :: RejectEx (%d)", nStatusCode, 0, 0);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::RejectWithDiversion(IN CONST AString &strAlternativeUserAddress)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_NEGOTIATING)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To reject a session, the state MUST be a NEGOTIATING; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    ISIPHeader *piHeader = SIPParsingHelper::CreateHeader(
            ISIPHeader::CONTACT_NORMAL, strAlternativeUserAddress);

    if (piHeader == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "No contact headers in 3xx response", 0, 0, 0);
        return IMS_FAILURE;
    }

    piHeader->Destroy();

    // Send "302 - Moved Temporarily" response with the given Contact address

    // Create an answer if offer received

    ISIPServerConnection *piSSC = GetServerConnection(IMessage::SESSION_START);

    if (piSSC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSSC, SIPStatusCode::SC_302) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    // Set Contact header
    piSIPMsg->SetHeader(ISIPHeader::CONTACT_ANY, strAlternativeUserAddress);

    if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
    {
        CloseConnection(IMessage::SESSION_START);

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        SetState(STATE_TERMINATED);
        CleanupMedia();
        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

        return IMS_FAILURE;
    }

    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC->GetMessage());
    UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
    UpdateMedia(Media::SESSION_START_FAILED);

    CloseConnection(IMessage::SESSION_START);
    SetState(STATE_TERMINATED);

    PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::RemoveMedia(IN Media *pMedia)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_INITIATED)
            && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSDPOAState = GetOfferAnswerState();

        if ((nSDPOAState != SDPOAState::STATE_IDLE)
                && (nSDPOAState != SDPOAState::STATE_ESTABLISHED))
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);

            IMS_TRACE_E(0, "To remove a media, the state MUST be a INITIATED or ESTABLISHED; " \
                    "(%s, %d)", StateToString(nState), nSDPOAState, 0);
            return IMS_FAILURE;
        }
    }

    if (pMedia == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_UINT32 i;

    for (i = 0; i < objMedias.GetSize(); ++i)
    {
        const Media *pExistingMedia = objMedias.GetAt(i);

        if (pExistingMedia->Equals(pMedia))
        {
            break;
        }
    }

    if (i >= objMedias.GetSize())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "No matched media (%p)", pMedia, 0, 0);
        return IMS_FAILURE;
    }

    IMS_BOOL bMidSyncRequired = (i < (objMedias.GetSize() - 1));
    Media *pMatchedMedia = objMedias.GetAt(i);

    pMatchedMedia->RemoveMedia();

    if ((pMatchedMedia->GetState() == Media::STATE_INACTIVE)
            || (pMatchedMedia->GetState() == Media::STATE_DELETED))
    {
        objMedias.RemoveAt(i);
    }

    if (bMidSyncRequired)
    {
        for (IMS_UINT32 j = i; j < objMedias.GetSize(); j++)
        {
            Media *pMedia = objMedias.GetAt(j);
            pMedia->SetMid(j);
        }
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::RemoveMedia(IN IMS_UINT32 nIndex)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_INITIATED)
            && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSDPOAState = GetOfferAnswerState();

        if ((nSDPOAState != SDPOAState::STATE_IDLE)
                && (nSDPOAState != SDPOAState::STATE_ESTABLISHED))
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);

            IMS_TRACE_E(0, "To remove a media, the state MUST be a INITIATED or ESTABLISHED; " \
                    "(%s, %d)", StateToString(nState), nSDPOAState, 0);
            return IMS_FAILURE;
        }
    }

    if (nIndex >= objMedias.GetSize())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid index (%d) in the total size (%d)",
                nIndex, objMedias.GetSize(), 0);
        return IMS_FAILURE;
    }

    IMS_BOOL bMidSyncRequired = (nIndex < (objMedias.GetSize() - 1));
    Media *pMedia = objMedias.GetAt(nIndex);

    if (pMedia == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pMedia->RemoveMedia();

    if ((pMedia->GetState() == Media::STATE_INACTIVE)
        || (pMedia->GetState() == Media::STATE_DELETED))
    {
        objMedias.RemoveAt(nIndex);
    }

    if (bMidSyncRequired)
    {
        for (IMS_UINT32 j = nIndex; j < objMedias.GetSize(); j++)
        {
            Media *pMedia = objMedias.GetAt(j);
            pMedia->SetMid(j);
        }
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::Restore()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ESTABLISHED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To restore a media, the state MUST be a ESTABLISHED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    // Restore the proposal view
    RestoreOfferAnswerState();

    // Remove the local capabilities that are not negotiated ?????
    RestoreEx();

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::SendAck()
{
    IMS_SINT32 nCallState = GetCallState();

    //---------------------------------------------------------------------------------------------

    if ((nCallState != CallState::STATE_INVITE_2XX_RECEIVED)
            && (nCallState != CallState::STATE_REINVITE_2XX_RECEIVED))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Invalid call state (%d) to send ACK", nCallState, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nServiceMethod;

    if (nCallState == CallState::STATE_INVITE_2XX_RECEIVED)
        nServiceMethod = IMessage::SESSION_START;
    else
        nServiceMethod = IMessage::SESSION_UPDATE;

    ISIPClientConnection *piSCC = GetClientConnection(nServiceMethod);

    if (piSCC == IMS_NULL)
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (SendRequestToACK(piSCC, nServiceMethod) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending ACK request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    CloseConnection(nServiceMethod);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
        IN CONST AString &strReason /* = AString::ConstNull() */, IN IMS_SINT32 nFlags /* =  0 */)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPStatusCode::IsProvisional(nStatusCode))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid provisional response code (%d, %s)",
                nStatusCode, strReason.GetStr(), 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = GetState();

    if ((bFlag_UpdateRequestor)
            || (IsMobileOriginated() && (nState == STATE_NEGOTIATING)))
    {
        IMS::SetLastError(IMSError::INVALID_OPERATION);

        IMS_TRACE_E(0, "Session is the mobile-terminated session", 0, 0, 0);
        return IMS_FAILURE;
    }

    if ((nState != STATE_NEGOTIATING)
            && (nState != STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        IMS_TRACE_E(0, "To send a provisional response, the state MUST be a NEGOTIATING " \
                "or RENEGOTIATING; (%s)", StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    // Check 180 ???
    IMS_SINT32 nServiceMethod;

    if (nState == STATE_NEGOTIATING)
    {
        nServiceMethod = IMessage::SESSION_START;
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
    }

    Message *pMessage = GetPreviousRequest(nServiceMethod);

    if (pMessage == IMS_NULL)
    {
        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = pMessage->GetMessage();

    // Do nothing if the handling method is not INVITE
    if (!piSIPMsg->GetMethod().Equals(SIPMethod::INVITE))
    {
        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    ISIPServerConnection *piSSC = GetServerConnection(nServiceMethod);

    if (piSSC == IMS_NULL)
    {
        IMS::SetLastError(IMSError::INVALID_OPERATION);

        IMS_TRACE_E(0, "SIP server connection is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ PR :: SENDING %d \"%s\" ___", nStatusCode,
            strReason.IsNULL() ? SIPStatusCode::GetReasonPhrase(nStatusCode) : strReason.GetStr(),
            0);

    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // REMOVE_RECORD_ROUTE_HEADERS
    if ((nFlags & FLAG_REMOVE_RECORD_ROUTES) != 0)
    {
        RemoveRecordRouteHeaders(piSSC->GetMessage());
    }

    if (!SendNUpdateResponse(nServiceMethod, piSSC))
    {
        IMS_TRACE_E(0, "Sending a response failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC->GetMessage());

    CheckNCreateDialog(piSSC);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Session::Set100TryingNotification(IN IMS_BOOL b100TryingNotification)
{
    //---------------------------------------------------------------------------------------------

    bFlag_100TryingNotification = b100TryingNotification;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::SetCallerPreference(IN CONST IMSList<AString> &objCallerPreference)
{
    //---------------------------------------------------------------------------------------------

    // CALLER_PREFERENCE_MANAGER
    objPreviousCallerPreference.Clear();
    objPreviousCallerPreference = objCallerPreference;

    CallerPreferenceManager::GetInstance()->UpdateAcceptContacts(GetName(), objCallerPreference);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Session::SetConfiguration(IN IMS_SINT32 nConfigValue)
{
    //---------------------------------------------------------------------------------------------

    this->nConfigValue = nConfigValue;
}

/*

Remarks
 CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST

*/
PUBLIC
IMS_RESULT Session::SetContactParameter(IN CONST AString &strParameter,
        IN IMS_SINT32 nOperation /* = 0 (0: ADD, 1: REMOVE) */)
{
    ISIPDialog *piDialog = GetDialog();

    //---------------------------------------------------------------------------------------------

    // FIXME: needs to check the dialog's state?

    if (piDialog == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piDialog->SetContactParameter(strParameter, nOperation);
}

/*

Remarks

*/
PUBLIC
void Session::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    //---------------------------------------------------------------------------------------------

    bFlag_ImplicitRoutingRequired = bFlag;

    // FIXME: If the routing address needs to be provisioned by the application,
    // please add a second argument for it.
}

/*

Remarks

*/
PUBLIC
void Session::SetReasonForCallTermination(IN IMS_SINT32 nReason)
{
    //---------------------------------------------------------------------------------------------

    strTerminationReasonFromApp.Sprintf("%x", nReason);
}

/*

Remarks

*/
PUBLIC
void Session::SetRefreshListener(IN IRefreshListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piRefreshListener = piListener;
}

/*

Remarks

*/
PUBLIC
void Session::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

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
PUBLIC
void Session::SetListener(IN IOnSessionListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piSessionListener = piListener;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::Start()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_INITIATED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To start a session, the state MUST be an INITIATED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    //1 Its checking is moved to the enabler implementation
#if 0
    if (objMedias.IsEmpty())
    {
        IMS_TRACE_E(0, "No media in a session", 0, 0, 0);
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }
#endif

    // Check if the created media is initialized or not
    if (!IsMediaInitializationDone())
    {
        IMS_TRACE_E(0, "Media is not ready", 0, 0, 0);
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Send INVITE request
    if (SendRequestToINVITE() != IMS_SUCCESS)
    {
        SetState(STATE_TERMINATED);
        CleanupMedia();

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

        return IMS_FAILURE;
    }

    SetState(STATE_NEGOTIATING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::Terminate()
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState == STATE_TERMINATING)
            || (nState == STATE_TERMINATED))
    {
        IMS::SetLastError(IMSError::NO_ERROR);

        IMS_TRACE_D("Session is already terminating or terminated in Terminate()", 0, 0, 0);
        return IMS_SUCCESS;
    }

    // Cease the 2xx retransmission
    Stop2xxRetransmission();

    // 'Replaces' header handling ...
    RemoveSessionFromCallControlHelper();

    RemovePreviousMessage(IMessage::SESSION_TERMINATE);
    CleanupMedia();

    SetTerminationReason(TERMINATION_REASON_USER_ACTION);

    switch (nState)
    {
    case STATE_CREATED:
    case STATE_INITIATED:
        SetState(STATE_TERMINATED);
        IMS::SetLastError(IMSError::NO_ERROR);

        IMS_TRACE_D("Session is terminated in the state (%s)", StateToString(GetState()), 0, 0);
        return IMS_SUCCESS;

    case STATE_NEGOTIATING:
        TerminateOnNegotiating();
        break;

    case STATE_ESTABLISHING:
        TerminateOnEstablishing();
        break;

    case STATE_ESTABLISHED:
        // Checks if the session refresh is ongoing...
        if (pRefreshHelper != IMS_NULL)
        {
            if (pRefreshHelper->IsRequestPending())
            {
                IMS_TRACE_D("Stopping the session refresh on Terminate()", 0, 0, 0);

                pRefreshHelper->AbortConnection();
            }
        }

        SetState(STATE_TERMINATING);

        if (SendRequestToBYE() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending BYE request on ESTABLISHED failed", 0, 0, 0);
            return IMS_FAILURE;
        }
        break;

    case STATE_RENEGOTIATING:
        TerminateOnReNegotiating();
        break;

    case STATE_REESTABLISHING:
        TerminateOnReEstablishing();
        break;

    case STATE_TERMINATING:
    case STATE_TERMINATED:
        break;

    default:
        break;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::TerminateEx(IN IMS_BOOL bTerminateMethodBYE /* = IMS_FALSE */)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("TerminateEx()", 0, 0, 0);

    if ((nState == STATE_TERMINATING)
            || (nState == STATE_TERMINATED))
    {
        IMS::SetLastError(IMSError::NO_ERROR);

        IMS_TRACE_D("Session is already terminating or terminated in Terminate()", 0, 0, 0);
        return IMS_SUCCESS;
    }

    bFlag_TerminateMethodBYE = bTerminateMethodBYE;

    // Cease the 2xx retransmission
    Stop2xxRetransmission();

    IMS_SINT32 nCallState = GetCallState();

    if (!bFlag_TerminateMethodBYE
            && bFlag_UpdateRequestor
            && (nState == STATE_RENEGOTIATING)
                && ((nCallState == CallState::STATE_REINVITE_SENT)
                    || (nCallState == CallState::STATE_REINVITE_1XX_RECEIVED)))
    {
        // CASE :: CANCEL for re-INVITE request...
    }
    else
    {
        // 'Replaces' header handling ...
        RemoveSessionFromCallControlHelper();

        RemovePreviousMessage(IMessage::SESSION_TERMINATE);
        CleanupMedia();

        SetTerminationReason(TERMINATION_REASON_USER_ACTION);
    }

    switch (nState)
    {
    case STATE_CREATED:
    case STATE_INITIATED:
        SetState(STATE_TERMINATED);
        IMS::SetLastError(IMSError::NO_ERROR);

        IMS_TRACE_D("Session is terminated in the state (%s)", StateToString(GetState()), 0, 0);
        return IMS_SUCCESS;

    case STATE_NEGOTIATING:
        TerminateOnNegotiating();
        break;

    case STATE_ESTABLISHING:
        TerminateOnEstablishing();
        break;

    case STATE_ESTABLISHED:
        // Checks if the session refresh is ongoing...
        if (pRefreshHelper != IMS_NULL)
        {
            if (pRefreshHelper->IsRequestPending())
            {
                IMS_TRACE_D("Stopping the session refresh on Terminate()", 0, 0, 0);

                pRefreshHelper->AbortConnection();
            }
        }

        SetState(STATE_TERMINATING);

        if (SendRequestToBYE() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending BYE request on ESTABLISHED failed", 0, 0, 0);
            return IMS_FAILURE;
        }
        break;

    case STATE_RENEGOTIATING:
        if (bFlag_TerminateMethodBYE)
        {
            TerminateOnReNegotiating();
            break;
        }

        if (bFlag_UpdateRequestor)
        {
            if (nCallState == CallState::STATE_REINVITE_SENT)
            {
                IMS_TRACE_I("_____ No 1xx response to re-INVITE received _____", 0, 0, 0);

                // Set the flag and wait & send CANCEL when receiving 1xx response
                // bFlag_TerminatePending = IMS_TRUE;

                // Abort the ongoing transaction
                CloseConnection(IMessage::SESSION_UPDATE);

                SetState(STATE_ESTABLISHED);
                RestoreOfferAnswerState();
                RestoreEx();

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
                break;
            }
            else if (nCallState == CallState::STATE_REINVITE_1XX_RECEIVED)
            {
                // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
                // RemovePreviousMessage(IMessage::SESSION_CANCEL);

                // Send CANCEL
                if (SendRequestToCANCEL() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                    IMS_TRACE_E(0, "Sending CANCEL request failed", 0, 0, 0);
                    break;
                }
            }
            else
            {
                TerminateOnReNegotiating();
            }
        }
        else
        {
            TerminateOnReNegotiating();
        }
        break;

    case STATE_REESTABLISHING:
        TerminateOnReEstablishing();
        break;

    case STATE_TERMINATING:
    case STATE_TERMINATED:
        break;

    default:
        break;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::Update()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ESTABLISHED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To update a session, the state MUST be an ESTABLISHED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    if (!HasPendingUpdate())
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        IMS_TRACE_E(0, "There are no updates to be made to the session", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Checks if the session refresh is ongoing...
    if (SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            GetSlotId(), GetService()->GetSIPProfile()))
    {
        if ((pRefreshHelper != IMS_NULL) && (pRefreshHelper->IsRequestPending()))
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);
            IMS_TRACE_E(0, "Session refresh is ongoing", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (objMedias.IsEmpty() || !IsMediaInitializationDone())
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        IMS_TRACE_E(0, "There is no media or all the medias are not ready to update the session",
                0, 0, 0);
        return IMS_FAILURE;
    }

    // ACK_RETRANSMISSION_TO_2XX
    RemoveStrayAcks();

    SIPMethod objMethod = SelectUpdateMethod();

    if (objMethod.Equals(SIPMethod::INVITE))
    {
        if (SendRequestToINVITE() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        if (SendRequestToUPDATE() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }

    bFlag_UpdateRequestor = IMS_TRUE;
    SetState(STATE_RENEGOTIATING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Session::UpdateEx(IN IMS_SINT32 nMethod /* = SIPMethod::INVALID */,
        IN IMS_BOOL bSessionRefresh /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    // CASE: session refresh
    if (bSessionRefresh)
    {
        return SendRequestForRefresh(nMethod);
    }

    // re-INVITE is selected as the default
    if (nMethod == SIPMethod::INVALID)
    {
        nMethod = SIPMethod::INVITE;
    }

    if ((nMethod != SIPMethod::INVITE) && (nMethod != SIPMethod::UPDATE))
    {
        IMS_TRACE_E(0, "To update a session, the method MUST be an INVITE or UPDATE; (%d)",
                nMethod, 0, 0);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_ESTABLISHED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To update a session, the state MUST be an ESTABLISHED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    // Checks if the session refresh is ongoing...
    if (SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            GetSlotId(), GetService()->GetSIPProfile()))
    {
        if ((pRefreshHelper != IMS_NULL) && (pRefreshHelper->IsRequestPending()))
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);
            IMS_TRACE_E(0, "Session refresh is ongoing", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (objMedias.IsEmpty() || !IsMediaInitializationDone())
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        IMS_TRACE_E(0, "There is no media or all the medias are not ready to update the session",
                0, 0, 0);
        return IMS_FAILURE;
    }

    if (nMethod == SIPMethod::INVITE)
    {
        if (SendRequestToINVITE() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        if (SendRequestToUPDATE() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }

    bFlag_UpdateRequestor = IMS_TRUE;
    SetState(STATE_RENEGOTIATING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
/*

Remarks

*/
PUBLIC
IMS_RESULT Session::CreateFailureSdp()
{
    if (pOAState == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    pOAState->CreateRefusedView();

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Session::DestroyFailureSdp()
{
    if (pOAState == IMS_NULL)
    {
        return;
    }

    pOAState->DestroyRefusedView();
}

/*

Remarks

*/
PUBLIC
ISessionParameter* Session::GetFailureSdp() const
{
    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pOAState->GetRefusedView();
}
// }

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
    case AMSG_SESSION_INVITATION_RECEIVED:
        GetService()->HandleSessionInvitationReceived(this);
        return IMS_TRUE;

    case AMSG_SESSION_ALERTING:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_Alerting(this);
        }
        return IMS_TRUE;

    case AMSG_SESSION_REFERENCE_RECEIVED:
        {
            Reference *pReference = reinterpret_cast<Reference*>(objMSG.nLparam);

            if (pReference == IMS_NULL)
            {
                return IMS_TRUE;
            }

            if (piSessionListener != IMS_NULL)
            {
                piSessionListener->OnSession_ReferenceReceived(this, pReference);
            }
            else
            {
                //4 Find a proper response code
                pReference->Reject();
                pReference->Destroy();
            }
        }
        return IMS_TRUE;

    case AMSG_SESSION_STARTED:
        if (piSessionListener != IMS_NULL)
        {
            if (CheckNSetListenerCall(LISTENER_CALL_STARTED))
            {
                piSessionListener->OnSession_Started(this);
            }
        }

        // ACK_WITH_SDP_IN_PROGRESS
        if (bFlag_AckWithSDPInProgress)
        {
            bFlag_AckWithSDPInProgress = IMS_FALSE;
        }
        return IMS_TRUE;

    case AMSG_SESSION_START_FAILED:
        if (piSessionListener != IMS_NULL)
        {
            if (CheckNSetListenerCall(LISTENER_CALL_START_FAILED))
            {
                piSessionListener->OnSession_StartFailed(this);
            }
        }
        return IMS_TRUE;

    case AMSG_SESSION_TERMINATED:
        // 'Replaces' header handling ...
        RemoveSessionFromCallControlHelper();

        if (piSessionListener != IMS_NULL)
        {
            if (CheckNSetListenerCall(LISTENER_CALL_TERMINATED))
            {
                piSessionListener->OnSession_Terminated(this);
            }
        }
        return IMS_TRUE;

    case AMSG_SESSION_UPDATED:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_Updated(this);
        }

        // RACE_CONDITION : SESSION_UPDATE
        SetSessionUpdateNotificationState(IMS_FALSE);

        // ACK_WITH_SDP_IN_PROGRESS
        if (bFlag_AckWithSDPInProgress)
        {
            bFlag_AckWithSDPInProgress = IMS_FALSE;
        }
        return IMS_TRUE;

    case AMSG_SESSION_UPDATE_FAILED:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_UpdateFailed(this);
        }

        // RACE_CONDITION : SESSION_UPDATE
        SetSessionUpdateNotificationState(IMS_FALSE);
        return IMS_TRUE;

    case AMSG_SESSION_UPDATE_RECEIVED:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_UpdateReceived(this);
        }
        return IMS_TRUE;

    case AMSG_SESSION_CANCEL_DELIVERED:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_CancelDelivered(this);
        }
        return IMS_TRUE;

    case AMSG_SESSION_CANCEL_DELIVERY_FAILED:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_CancelDeliveryFailed(this);
        }
        return IMS_TRUE;

    case AMSG_SESSION_FORKED_RESPONSE_RECEIVED:
        {
            Session *pForkedSession = reinterpret_cast<Session*>(objMSG.nLparam);

            if (pForkedSession == IMS_NULL)
            {
                return IMS_TRUE;
            }

            // RACE_CONDITION (MO CANCEL & forked INVITE response)
            IMS_BOOL bTerminated = (piSessionListener == IMS_NULL) ? IMS_TRUE : IMS_FALSE;
            IMS_SINT32 nState = GetState();

            if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
            {
                IMS_TRACE_D("Forked INVITE response is received in %s state; so, it's ignored.",
                        StateToString(nState), 0, 0);

                bTerminated = IMS_TRUE;
            }

            if (bTerminated)
            {
                IMS_SINT32 nStatusCode = 0;
                IMessage *piMessage
                        = pForkedSession->GetPreviousResponse(IMessage::SESSION_START);

                if (piMessage != IMS_NULL)
                {
                    nStatusCode = piMessage->GetStatusCode();

                    if (SIPStatusCode::IsFinalSuccess(nStatusCode))
                    {
                        pForkedSession->SendAck();

                        pForkedSession->SetTerminationReason(TERMINATION_REASON_USER_ACTION);
                        pForkedSession->SendRequestToBYEInternal();
                    }
                }

                pForkedSession->Destroy();

                if ((nState == STATE_TERMINATING)
                        && SIPStatusCode::IsFinal(nStatusCode)
                        && (piSessionListener != IMS_NULL))
                {
                    SetState(STATE_TERMINATED);
                    piSessionListener->OnSession_Terminated(this);
                }
                return IMS_TRUE;
            }

            if (!piSessionListener->OnSession_ForkedResponseReceived(this, pForkedSession))
            {
                pForkedSession->Destroy();
            }
        }
        return IMS_TRUE;

    case AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED:
        if (piSessionListener != IMS_NULL)
        {
            piSessionListener->OnSession_ProvisionalResponseReceived(this, objMSG.nLparam);
        }
        return IMS_TRUE;

    case AMSG_SESSION_TRANSACTION_RECEIVED:
        {
            ISIPServerConnection *piSSC = reinterpret_cast<ISIPServerConnection*>(objMSG.nLparam);

            if (piSSC == IMS_NULL)
            {
                return IMS_TRUE;
            }

            if (piSessionListener != IMS_NULL)
            {
                if (!piSessionListener->OnSession_TransactionReceived(this, piSSC))
                {
                    GetService()->SendResponse(piSSC, SIPStatusCode::SC_480);
                    piSSC->Close();
                }
            }
            else
            {
                //4 Find a proper response code
                GetService()->SendResponse(piSSC, SIPStatusCode::SC_480);
                piSSC->Close();
            }
        }
        return IMS_TRUE;

    case AMSG_SESSION_DELAYED_DIALOG_TRANSACTION_RECEIVED:
        {
            // ACK_WITH_SDP_IN_PROGRESS
            ISIPServerConnection *piSSC = reinterpret_cast<ISIPServerConnection*>(objMSG.nLparam);

            if (piSSC != IMS_NULL)
            {
                const SIPMethod &objMethod = piSSC->GetMethod();

                if (bFlag_AckWithSDPInProgress
                        && (objMethod.Equals(SIPMethod::INVITE)
                            || objMethod.Equals(SIPMethod::UPDATE)))
                {
                    // To avoid an infinite loop, clear the flag
                    bFlag_AckWithSDPInProgress = IMS_FALSE;
                    IMS_TRACE_D("CriticialError :: AckWithSDPInProgress is cleared", 0, 0, 0);
                }

                Dialog_NotifyRequest(piSSC);
            }
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
PROTECTED VIRTUAL
void Session::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    (void) nErrorCode;

    // If the error code is SERVICE_CLOSING, do something ...
    IMS_TRACE_D("Session :: Exception_NotifyError() ... Error (%d) on %s",
            nErrorCode, StateToString(nState), 0);

    if (nState == STATE_TERMINATED)
        return;

    if ((nState == STATE_NEGOTIATING)
            || (nState == STATE_ESTABLISHING) /* ACK wait timeout */)
    {
        CloseConnection(IMessage::SESSION_START);
        CleanupMedia();

        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        if (!bFlag_TerminatePending)
        {
            // To send a BYE by enabler
            if (IsMobileOriginated()
                    && (nState == STATE_NEGOTIATING))
            {
                // Keeps the current state for MO session
            }
            else
            {
                SetState(STATE_ESTABLISHED);
            }

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            return;
        }
    }
    else if (((nState == STATE_RENEGOTIATING)
            || (nState == STATE_REESTABLISHING) /* ACK wait timeout */)
            && !IsConfigurationSet(CONFIG_IGNORE_DEREG_ON_SESSION_UPDATE))
    {
        CloseConnection(IMessage::SESSION_UPDATE);
        RestoreOfferAnswerState();
        RestoreEx();

        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        if (!bFlag_TerminatePending)
        {
            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            return;
        }
    }

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    if (bFlag_TerminatePending || (nState == STATE_TERMINATING))
    {
        if (nErrorCode == IMSError::SERVICE_CLOSED)
        {
            SetTerminationReason(TERMINATION_REASON_SERVICE_CLOSED);
        }

        SetState(STATE_TERMINATED);
        CleanupMedia();

        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::InitInstance()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_CREATED)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FALSE;
    }

    // Instantiate SDP offer/answer object
    IMS_BOOL bSDPVersionCheck = IMS_TRUE;
    const SipConfigV *pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        bFlag_SDPNonRPRAllowed = pSipConfigV->IsSessionSDPNonRPRAllowed();
        bSDPVersionCheck = pSipConfigV->IsSessionSDPVersionCheckSupported();

        // Flag to check if the 100 Trying response should be notified or not
        bFlag_100TryingNotification = pSipConfigV->Is100TryingNotificationRequired();
    }

    pOAState = new SDPOAState(bSDPVersionCheck, IMS_TRUE);

    if (pOAState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_FALSE;
    }

    // Create a session refresh helper
    pRefreshHelper = new SessionRefreshHelper(GetService(), this);

    if (pRefreshHelper == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_FALSE;
    }

    // Session : MO
    if (IsMobileOriginated())
    {
        SetState(STATE_INITIATED);
    }
    // Session : MT
    else
    {
        CancellableMethodManager::GetInstance()->AddMethod(GetName(), this);
    }

    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);

    // CALLER_PREFERENCE_MANAGER
    CallerPreferenceManager::GetInstance()->CreatePreferenceWrapper(GetName(),
            AString::ConstNull());

    GetService()->RegisterMethod(this);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::NotifySIPRequest(IN ISIPServerConnection *piSSC)
{
    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("___>>> INCOMING SESSION RECEIVED <<<___", 0, 0, 0);

    // Update the call state
    UpdateCallStateOnMessageReceived(piSIPMsg);

    // Negotiate session timer info.
    switch (pRefreshHelper->UpdateOnMessageReceived(piSSC))
    {
    case SessionRefreshHelper::RESULT_ERROR:
    case SessionRefreshHelper::RESULT_REJECT_500:
        GetService()->SendResponse(piSSC, SIPStatusCode::SC_500);
        return IMS_FALSE;

    case SessionRefreshHelper::RESULT_REJECT_422:
        CreateResponse(piSSC, SIPStatusCode::SC_422);
        pRefreshHelper->AddSpecificHeader(piSSC);
        piSSC->Send();
        return IMS_FALSE;

    default:
        break;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSIPMsg);

    if (HandleRequestToINVITE(piSSC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Handling INVITE request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::NotifySIPForkedResponse(IN ISIPClientConnection *piSCC,
        IN ISIPClientConnection *piForkedSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC != GetClientConnection(IMessage::SESSION_START))
    {
        IMS_TRACE_E(0, "No client connection matches for a forked response", 0, 0, 0);

        piForkedSCC->Close();
        return IMS_FALSE;
    }

    if (!piForkedSCC->GetMethod().Equals(SIPMethod::INVITE))
    {
        IMS_TRACE_E(0, "SIP forked response is non-INVITE, so the session ignores it", 0, 0, 0);

        piForkedSCC->Close();
        return IMS_FALSE;
    }

    // Create a session from the SIP forked response
    Session *pSession = CreateSession();

    if (pSession == IMS_NULL)
    {
        piForkedSCC->Close();
        return IMS_FALSE;
    }

    // Management of forked sessions
    if (pForkedSessions.IsNull())
    {
        pForkedSessions = new MethodManager();

        // First forked session detected
        pForkedSessions->AddMethod(this);
        pForkedSessions->AddMethod(pSession);
    }
    else
    {
        // New forked session
        pForkedSessions->AddMethod(pSession);
    }

    pSession->pForkedSessions = pForkedSessions;

    // Set a configuration as the original configuration
    pSession->nConfigValue = nConfigValue;

    // Update the dialog info. enforcelly
    pSession->CheckNCreateDialog(piForkedSCC, IMS_TRUE);

    if (bFlag_SDPInInitialINVITE)
    {
        // INVITE request has been sent with SDP, so creates a media from the SDP.
        pSession->bFlag_SDPInInitialINVITE = IMS_TRUE;
        pSession->CheckNCreateSessionDescriptor();

        // Update the proposed view from this session capabilities
        const SessionParameter *pCapabilities = pOAState->GetCapabilities();
        SessionParameter *pProposalView = pSession->pOAState->GetProposalView();

        (*pProposalView) = (*pCapabilities);

        // Create all the medias from the proposal view
        if (!pSession->CreateMediaFromSDP())
        {
            pSession->Destroy();
            piForkedSCC->Close();

            IMS_TRACE_E(0, "Creating the media from SDP failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if (!pSession->UpdateRequestOnSent(IMessage::SESSION_START, piForkedSCC))
    {
        pSession->Destroy();
        piForkedSCC->Close();

        IMS_TRACE_E(0, "Updating request on sent failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Set listener for the forked client connection
    piForkedSCC->SetListener(pSession);
    piForkedSCC->SetErrorListener(pSession);

    // Update the call state
    pSession->UpdateCallStateOnMessageSent(piForkedSCC->GetMessage());
    // Update the Offer/Answer state
    pSession->UpdateOfferAnswerStateOnMessageSent(piForkedSCC->GetMessage());
    // Update the media state
    pSession->UpdateMedia(Media::SESSION_START);

    pSession->SetState(STATE_NEGOTIATING);

    PostMessage(AMSG_SESSION_FORKED_RESPONSE_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(pSession));

    pSession->HandleAllSIPResponse(piForkedSCC);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Session::NotifySIPResponse(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ Response received at the state, %s", StateToString(GetState()), 0, 0);

    if (GetState() == STATE_TERMINATED)
    {
        IMS_TRACE_E(0, "Response message will not be handled in the TERMINATED state", 0, 0, 0);

        if (piSCC->GetMethod().Equals(SIPMethod::INVITE)
            && (piSCC->GetStatusCode() >= SIPStatusCode::SC_200))
        {
            RestoreOfferAnswerState();

            SendRequestToACK(piSCC, IMessage::SERVICEMETHOD_INVALID);
        }
        return;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Update the call state
    UpdateCallStateOnMessageReceived(piSIPMsg);

    // Handle the response according to the SIP method.
    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    // Update a session refresh timer
    if (pRefreshHelper->UpdateOnMessageReceived(piSCC)
            != SessionRefreshHelper::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Updating a session refresh timer failed", 0, 0, 0);
    }

    // Update the Offer/Answer state
    if (objMethod.Equals(SIPMethod::PRACK)
            && (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
    {
        // no-op :: ignore PRACK response if SDP OA state is in ESTABLISHED
        IMS_TRACE_D("Session :: PRACK response doesn't participate in SDP OAE", 0, 0, 0);
    }
    else
    {
        if (IsConfigurationSet(CONFIG_SUPPORT_EARLY_SESSION_MODEL)
                && (GetState() == STATE_NEGOTIATING)
                && (piSIPMsg->GetStatusCode() == SIPStatusCode::SC_183)
                && OperatorFeatureResolver::IsMessageForEarlySessionModel(piSIPMsg))
        {
            if (pVirtualEarlySession.IsNull())
            {
                pVirtualEarlySession = new VirtualSession(GetService(), GetUserAOR());
            }

            pVirtualEarlySession->Notify18xResponse(piSIPMsg);
        }
        else
        {
            if ((GetCallState() == CallState::STATE_INVITE_2XX_RECEIVED)
                    || (piSIPMsg->GetSDPBodyPart() != IMS_NULL)
                    || (piSIPMsg->GetStatusCode() == SIPStatusCode::SC_180))
            {
                // KT
                // 183 (to-tag1) -> 183 (early-session, to-tag2) -> 200-INVITE (to-tag2)
                // 183 (early-session, to-tag1) -> 183 (forking, to-tag2) -> 200-INVITE (to-tag3)
                pVirtualEarlySession = IMS_NULL;
            }

            UpdateOfferAnswerStateOnMessageReceived(piSIPMsg);
        }
    }

    switch (objMethod.ToInt())
    {
    // ACK request will be sent inside of HandleResponseToINVITE() method.
    case SIPMethod::INVITE:
        HandleResponseToINVITE(piSCC);
        break;

    case SIPMethod::BYE:
        //3 check if the same connection exists or not
        HandleResponseToBYE(piSCC);
        break;

    case SIPMethod::CANCEL:
        HandleResponseToCANCEL(piSCC);
        break;

    case SIPMethod::UPDATE:
        if (piSCC != GetClientConnection(IMessage::SESSION_UPDATE))
        {
            // Do nothing ...
            break;
        }

        HandleResponseToUPDATE(piSCC);
        break;

    case SIPMethod::PRACK:
        break;

    default:
        IMS_TRACE_E(0, "Not handled method (%s)", objMethod.ToString().GetStr(), 0, 0);
        break;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Session::NotifySIPError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
        IN CONST AString &strMessage)
{
    const SIPMethod &objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void) nCode;
    (void) strMessage;

    switch (objMethod.ToInt())
    {
    case SIPMethod::INVITE:
        {
            IMS_SINT32 nOldState = GetState();

            if ((nOldState == STATE_NEGOTIATING)
                || (nOldState == STATE_ESTABLISHING) /* ACK wait timeout */)
            {
                CloseConnection(IMessage::SESSION_START);

                CleanupMedia();

                // Cease the 2xx retransmission
                Stop2xxRetransmission();

                if (bFlag_TerminatePending)
                {
                    bFlag_TerminatePending = IMS_FALSE;

                    if (nOldState == STATE_NEGOTIATING)
                    {
                        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                    }
                    else
                    {
                        // ACK wait timer expired ...
                        if (SendRequestToBYE() != IMS_SUCCESS)
                        {
                            SetState(STATE_TERMINATED);

                            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                            break;
                        }

                        SetState(STATE_TERMINATING);
                    }
                }
                else
                {
                    // To send a BYE by enabler
                    if (IsMobileOriginated()
                            && (nOldState == STATE_NEGOTIATING))
                    {
                        // Keeps the current state for MO session
                    }
                    else
                    {
                        SetState(STATE_ESTABLISHED);
                    }

                    // It contains the incoming INVITE case when the error is occurred
                    // in the SIP transport layer
                    PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
                }
            }
            else if ((nOldState == STATE_RENEGOTIATING)
                || (nOldState == STATE_REESTABLISHING) /* ACK wait timeout */)
            {
                // CALLER_PREFERENCE_MANAGER
                if (nOldState == STATE_RENEGOTIATING)
                {
                    IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

                    if (piMessage != IMS_NULL)
                    {
                        UpdateCallerPreference(piMessage->GetMessage(), 0);
                    }
                }

                CloseConnection(IMessage::SESSION_UPDATE);

                // Cease the 2xx retransmission
                Stop2xxRetransmission();

                RestoreOfferAnswerState();
                RestoreEx();

                if (bFlag_TerminatePending)
                {
                    bFlag_TerminatePending = IMS_FALSE;

                    if (SendRequestToBYE() != IMS_SUCCESS)
                    {
                        SetState(STATE_TERMINATED);

                        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                        break;
                    }

                    SetState(STATE_TERMINATING);
                }
                else
                {
                    SetState(STATE_ESTABLISHED);

                    // RACE_CONDITION : SESSION_UPDATE
                    if (nOldState == STATE_RENEGOTIATING)
                    {
                        SetSessionUpdateNotificationState(IMS_TRUE);
                    }

                    // It contains the incoming INVITE case when the error is occurred
                    // in the SIP transport layer
                    PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
                }
            }
        }
        break;

    case SIPMethod::BYE:
        if (piSC == piSCC_BYE)
        {
            if (piSCC_BYE != IMS_NULL)
            {
                piSCC_BYE->Close();
                piSCC_BYE = IMS_NULL;
            }

            if (GetState() != STATE_TERMINATED)
            {
                SetState(STATE_TERMINATED);
                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }
            break;
        }

        CloseConnection(IMessage::SESSION_TERMINATE);

        if (GetState() != STATE_TERMINATED)
        {
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        break;

    case SIPMethod::CANCEL:
        // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
        {
            IMS_SINT32 nMethodForCancel = IMessage::SESSION_TERMINATE;
            IMS_SINT32 nState = GetState();

            if (nState >= STATE_ESTABLISHED)
            {
                if ((nState != STATE_TERMINATING) && (nState != STATE_TERMINATED))
                {
                    nMethodForCancel = IMessage::SESSION_CANCEL;
                }
                else
                {
                    IMessage *piRequest = GetPreviousRequest(IMessage::SESSION_CANCEL);

                    if (piRequest != IMS_NULL)
                    {
                        IMessage *piResponse = GetPreviousResponse(IMessage::SESSION_CANCEL);

                        if (piResponse == IMS_NULL)
                        {
                            nMethodForCancel = IMessage::SESSION_CANCEL;
                        }
                        else if (!SIPStatusCode::IsFinal(piResponse->GetStatusCode()))
                        {
                            nMethodForCancel = IMessage::SESSION_CANCEL;
                        }
                    }
                }
            }

            CloseConnection(nMethodForCancel);

            if (nState == STATE_RENEGOTIATING)
            {
                // Notifies the application that CANCEL (for re-INVITE) is failed
                if (nMethodForCancel == IMessage::SESSION_CANCEL)
                {
                    PostMessage(AMSG_SESSION_CANCEL_DELIVERY_FAILED, 0, 0);
                }
            }
            else if (nState != STATE_TERMINATED)
            {
                SetState(STATE_TERMINATED);
                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }
        }
        break;

    case SIPMethod::UPDATE:
        if (GetState() != STATE_RENEGOTIATING)
        {
            // It MAY be handled by the SessionEx...
            break;
        }

        CloseConnection(IMessage::SESSION_UPDATE);

        RestoreOfferAnswerState();
        RestoreEx();

        SetState(STATE_ESTABLISHED);
        // RACE_CONDITION : SESSION_UPDATE
        SetSessionUpdateNotificationState(IMS_TRUE);

        PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        break;

    case SIPMethod::ACK:
    case SIPMethod::PRACK:
        // If the transaction timer of PRACK is expired before the call is established,
        // it SHOULD be notified to the application and the application SHOULD terminate the call.
        // (RFC 3261 recommendation)
        break;

    default:
        IMS_TRACE_E(0, "Not handled method (%s)", objMethod.ToString().GetStr(), 0, 0);

        piSC->Close();
        break;
    }
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PROTECTED VIRTUAL
IMS_BOOL Session::SendRequestToChallenge(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // For session refresh case
    if (piSCC == pRefreshHelper->GetConnection())
    {
        if (pRefreshHelper->SendRefreshRequest(piSCC) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSCC->GetMessage());

        return IMS_TRUE;
    }

    IMS_SINT32 nServiceMethod = IMessage::SESSION_START;

    if (GetState() == STATE_NEGOTIATING)
        nServiceMethod = IMessage::SESSION_START;
    else
        nServiceMethod = IMessage::SESSION_UPDATE;

    // Try to send an INVITE request to the network

    // Clear the connection to preserve the SIP connection
    ClearConnection(nServiceMethod);

    if (!SendNUpdateRequestEx(nServiceMethod, piSCC, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(nServiceMethod, piSCC);
        return IMS_FALSE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSCC->GetMessage());

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());

    // Update the media state also.
    if (GetState() == STATE_NEGOTIATING)
        UpdateMedia(Media::SESSION_START);
    else
        UpdateMedia(Media::SESSION_UPDATE);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::SetReferredMessageListener(IN IReferredMessageListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piReferredMessageListener = piListener;

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
const AString& Session::GetConnectionAddress() const
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return AString::ConstNull();
    }

    SdpSessionParameter *pSessionParam = IMS_NULL;

    pOAState->GetSessionCurrentView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        pOAState->GetSessionProposalView(pSessionParam);
    }

    if (pSessionParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "No current or proposal view exists", 0, 0, 0);
        return AString::ConstNull();
    }

    return pSessionParam->GetConnectionAddress();
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_SINT32 Session::GetSessionState() const
{
    //---------------------------------------------------------------------------------------------

    if (nState == STATE_CREATED)
        return SESSION_STATE_CREATED;
    else if (nState == STATE_INITIATED)
        return SESSION_STATE_INITIATED;
    else if (nState == STATE_NEGOTIATING)
        return SESSION_STATE_NEGOTIATING;
    else if (nState == STATE_ESTABLISHING)
        return SESSION_STATE_ESTABLISHING;
    else if (nState == STATE_ESTABLISHED)
        return SESSION_STATE_ESTABLISHED;
    else if (nState == STATE_RENEGOTIATING)
        return SESSION_STATE_RENEGOTIATING;
    else if (nState == STATE_REESTABLISHING)
        return SESSION_STATE_REESTABLISHING;
    else if (nState == STATE_TERMINATING)
        return SESSION_STATE_TERMINATING;
    else if (nState == STATE_TERMINATED)
        return SESSION_STATE_TERMINATED;
    else
        return (-1);
}

/*

Remarks

*/
PROTECTED VIRTUAL
SdpSessionParameter* Session::GetSessionParameter() const
{

    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter *pSessionParam = IMS_NULL;

    if (nState == STATE_ESTABLISHED)
    {
        if (pOAState->GetSessionCurrentView(pSessionParam) != ISDPOAState::RESULT_SUCCESS)
        {
            return IMS_NULL;
        }
    }
    else
    {
        if (pOAState->GetSessionProposalView(pSessionParam) != ISDPOAState::RESULT_SUCCESS)
        {
            return IMS_NULL;
        }
    }

    return pSessionParam;
}

/*

Remarks

*/
PROTECTED VIRTUAL
const AString& Session::GetPeerConnectionAddress() const
{

    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return AString::ConstNull();
    }

    SdpSessionParameter *pSessionParam = IMS_NULL;

    pOAState->GetSessionPeerView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Getting a peer view failed", 0, 0, 0);
        return AString::ConstNull();
    }

    return pSessionParam->GetConnectionAddress();
}

/*

Remarks

*/
PROTECTED VIRTUAL
SdpSessionParameter* Session::GetPeerSessionParameter() const
{

    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter *pSessionParam = IMS_NULL;

    pOAState->GetSessionPeerView(pSessionParam);

    return pSessionParam;
}

/*

Remarks

*/
PROTECTED VIRTUAL
SdpSessionParameter* Session::GetProposalSessionParameter()
{

    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter *pSessionParam = IMS_NULL;

    pOAState->GetSessionProposalView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        // Checks and create a proposal view if it does not exist
        if (!pOAState->IsOfferProgress())
        {
            IMS_SINT32 nResult = pOAState->CreateProposalView();

            if ((nResult != ISDPOAState::RESULT_SUCCESS)
                    && (nResult != ISDPOAState::RESULT_ALREADY_EXIST))
            {
                return IMS_NULL;
            }

            pOAState->GetSessionProposalView(pSessionParam);
        }
    }

    return pSessionParam;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::Cancellable_Compare(IN ISIPServerConnection *piSSC_CANCEL) const
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("CANCEL comparing ... Session (%d), Call (%d)", GetState(), GetCallState(), 0);

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!GetService()->IsImsConnected())
    {
        IMS_TRACE_E(0, "There is no registration", 0, 0, 0);
        return IMS_FALSE;
    }
#endif

    ISIPServerConnection *piSSC_INVITE = IMS_NULL;
    IMS_SINT32 nState = GetState();

    if (nState == STATE_NEGOTIATING)
    {
        if (IsMobileOriginated())
        {
            IMS_TRACE_D("CANCEL is ignored ... (not mobile-terminated session)", 0, 0, 0);
            return IMS_FALSE;
        }

        piSSC_INVITE = GetServerConnection(IMessage::SESSION_START);
    }
    else if (nState == STATE_RENEGOTIATING)
    {
        if (bFlag_UpdateRequestor)
        {
            IMS_TRACE_D("CANCEL is ignored ... (not update receiver)", 0, 0, 0);
            return IMS_FALSE;
        }

        piSSC_INVITE = GetServerConnection(IMessage::SESSION_UPDATE);
    }
    // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
    else
    {
        // When INVITE_TXN_HANDLING_CORRECTION is enabled:
        if (nState == STATE_ESTABLISHING)
        {
            if (IsMobileOriginated())
            {
                IMS_TRACE_D("CANCEL is ignored ... (not mobile-terminated session) " \
                        "in ESTABLISHING state", 0, 0, 0);
                return IMS_FALSE;
            }

            piSSC_INVITE = GetServerConnection(IMessage::SESSION_START);
        }
    }

    if (piSSC_INVITE == IMS_NULL)
    {
        IMS_TRACE_D("No INVITE server transaction in %s", StateToString(nState), 0, 0);
        return IMS_FALSE;
    }

    return piSSC_CANCEL->IsSameTransaction(piSSC_INVITE);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::Cancellable_NotifyRequest(IN ISIPServerConnection *piSSC_CANCEL)
{
    ISIPMessage *piSIPMsg = piSSC_CANCEL->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (!piSIPMsg->GetMethod().Equals(SIPMethod::CANCEL))
    {
        return IMS_FALSE;
    }

    // Checks if Request-URI is matched or not
    const AString &strRequestURI = piSSC_CANCEL->GetRequestURI();
    SIPAddress objRequestURI(strRequestURI);

    if (!GetService()->ValidateRequestURI(objRequestURI))
    {
        IMS_BOOL bRURIMatched = IMS_FALSE;
        IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_START);
        ISIPMessage *piSIPMsg = (piMessage != IMS_NULL) ? piMessage->GetMessage() : IMS_NULL;

        if (piSIPMsg != IMS_NULL)
        {
            SIPAddress objOrigRURI(piSIPMsg->GetRequestURI());
            bRURIMatched = objRequestURI.Equals(objOrigRURI);
        }

        if (!bRURIMatched)
        {
            IMS_TRACE_D("Request-URI (%s) in a CANCEL request is not matched",
                    SIPDebug::GetUri1(strRequestURI).GetStr(), 0, 0);

            (void) CreateResponse(piSSC_CANCEL, SIPStatusCode::SC_404);
            (void) piSSC_CANCEL->Send();
            piSSC_CANCEL->Close();
            return IMS_TRUE;
        }
    }

    IMS_SINT32 nCallState = GetCallState();

    IMS_TRACE_I("CANCEL request received ... State : Session (%d), Call (%d)",
            nState, nCallState, 0);

    if ((nCallState != CallState::STATE_INVITE_RECEIVED)
            && (nCallState != CallState::STATE_INVITE_1XX_SENT)
            && (nCallState != CallState::STATE_REINVITE_RECEIVED)
            && (nCallState != CallState::STATE_REINVITE_1XX_SENT))
    {
        IMS_TRACE_D("Ignore the CANCEL request and maintain the session state ...", 0, 0, 0);

        // If it supports draft
        // "Correct transaction handling for 200 responses to SIP INVITE request",
        // then send 200 OK to CANCEL.
        // Then, UAC will send BYE request...
        // When INVITE_TXN_HANDLING_CORRECTION is enabled:
        (void) CreateResponse(piSSC_CANCEL, SIPStatusCode::SC_200);
        (void) piSSC_CANCEL->Send();
        piSSC_CANCEL->Close();

        // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
        // LGU+ requirement - After 200 OK sent & CANCEL received, terminate the session.
        // 140501, The below requirement is removed by LGU+.
        #if 0
        {
            IMS_SINT32 nState = GetState();

            if (nState == STATE_ESTABLISHING)
            {
                IMS_TRACE_D("Session will be terminated by CANCEL after user accepts...",
                        0, 0, 0);

                // Cease the 2xx retransmission
                Stop2xxRetransmission();

                CleanupMedia();

                SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

                SendRequestToBYEInternal();

                SetState(STATE_TERMINATED);
                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }
        }
        #endif

        return IMS_TRUE;
    }

    // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
    IMS_SINT32 nMethodForCancel = IMessage::SESSION_TERMINATE;

    if (GetState() >= STATE_ESTABLISHED)
    {
        nMethodForCancel = IMessage::SESSION_CANCEL;
    }

    if (!UpdateRequestOnReceived(nMethodForCancel, piSSC_CANCEL))
    {
        IMS_TRACE_E(0, "Updating SIP message failed", 0, 0, 0);
    }

    HandleRequestToCANCEL(piSSC_CANCEL);

    CloseConnection(nMethodForCancel);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::Dialog_Compare(IN ISIPServerConnection *piSSC) const
{
    // Filters some method which does not handle in the session

    //---------------------------------------------------------------------------------------------

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!GetService()->IsImsConnected())
    {
        return IMS_FALSE;
    }
#endif

    ISIPDialog *piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SIPMethod &objMethod = piSSC->GetMethod();

    if (objMethod.Equals(SIPMethod::REFER))
    {
        // If the server transaction has the same dialog identifier with a dialog of this session,
        // then it will be handled by this session.
        ISIPDialog *piReferDialog = piSSC->GetDialog();

        if (piReferDialog == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strDialogId = piDialog->GetDialogID();
        AString strReferDialogId = piReferDialog->GetDialogID();

        if (strDialogId.Equals(strReferDialogId))
        {
            IMS_TRACE_D("Session :: Dialog (%s), Refer's Dialog (%s)",
                    SIPDebug::GetCharA1(strDialogId.GetStr(), 8, '@'),
                    SIPDebug::GetCharA2(strReferDialogId.GetStr(), 8, '@'), 0);
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }
    else if (objMethod.Equals(SIPMethod::OPTIONS))
    {
        //1  In this moment, OPTIONS request will not be handled by Session
        //return IMS_FALSE;
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
IMS_BOOL Session::Dialog_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    ISIPMessage *piSIPMsg = piSSC->GetMessage();
    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Dialog_NotifyRequest - %s request received...",
            objMethod.ToString().GetStr(), 0, 0);

    // FIX_TIMING_ISSUE: ACK_WITH_SDP_IN_PROGRESS
    if (bFlag_AckWithSDPInProgress
            && (objMethod.Equals(SIPMethod::INVITE) || objMethod.Equals(SIPMethod::UPDATE))
            && (piSIPMsg->GetSDPBodyPart() != IMS_NULL))
    {
        IMS_TRACE_I("Processing of incoming request will be delayed", 0, 0, 0);
        PostMessage(AMSG_SESSION_DELAYED_DIALOG_TRANSACTION_RECEIVED,
                0, reinterpret_cast<IMS_UINTP>(piSSC));
        return IMS_TRUE;
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

            // PATCH_SIP_DIALOG_TERMINATED_STATE
            if (objMethod.Equals(SIPMethod::BYE))
            {
                UpdateRequestOnReceived(IMessage::SESSION_TERMINATE, piSSC);
            }

            if (!objMethod.Equals(SIPMethod::ACK))
            {
                GetService()->SendResponse(piSSC, SIPStatusCode::SC_404);
            }

            // PATCH_SIP_DIALOG_TERMINATED_STATE
            // The dialog state will be transited to "TERMINATED" state,
            // so notify the application that this session is terminated...
            if (piSSC->GetMethod().Equals(SIPMethod::BYE))
            {
                UpdateResponseOnSent(IMessage::SESSION_TERMINATE, piSSC);

                bFlag_TerminatePending = IMS_FALSE;

                // Cease the 2xx retransmission
                Stop2xxRetransmission();
                CleanupMedia();

                // Clear the previous SIP connection
                ClearConnection(IMessage::SESSION_TERMINATE);

                SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }

            piSSC->Close();

            return IMS_FALSE;
        }
    }

    // Update the call state
    UpdateCallStateOnMessageReceived(piSIPMsg);

    // Update a session refresh timer
    if (objMethod.Equals(SIPMethod::INVITE)
            && !SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
    {
        IMS_TRACE_D("re-INVITE request will not participate in the session refresh", 0, 0, 0);

        pRefreshHelper->UpdateTimerOptionOnRequestReceived(piSSC);
    }
    else
    {
        switch (pRefreshHelper->UpdateOnMessageReceived(piSSC))
        {
        case SessionRefreshHelper::RESULT_ERROR:
            IMS_TRACE_E(0, "Updating a session refresh timer failed", 0, 0, 0);
            break;
        case SessionRefreshHelper::RESULT_REJECT_422:
            IMS_TRACE_D("SessionTimer :: REJECT_422", 0, 0, 0);
            break;
        case SessionRefreshHelper::RESULT_REJECT_500:
            IMS_TRACE_D("SessionTimer :: REJECT_500", 0, 0, 0);
            break;
        default:
            break;
        }
    }

    switch (objMethod.ToInt())
    {
    case SIPMethod::INVITE:
        // ACK_RETRANSMISSION_TO_2XX
        RemoveStrayAcks();

        if (HandleRequestToINVITEWithinDialog(piSSC) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
        break;

    case SIPMethod::ACK:
        HandleRequestToACK(piSSC);

        // When ACK request is received, the server connection has been created newly...
        // So, close the server connection directly.
        piSSC->Close();
        break;

    case SIPMethod::BYE:
        if (HandleRequestToBYE(piSSC) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
        break;

    case SIPMethod::INFO:
        if (piSessionListener == IMS_NULL)
        {
            IMS_TRACE_D("No session listener", 0, 0, 0);

            //4 Find a proper response code
            GetService()->CreateResponse(piSSC, SIPStatusCode::SC_480);
            piSSC->Send();
            piSSC->Close();
            break;
        }

        PostMessage(AMSG_SESSION_TRANSACTION_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSSC));
        break;

    case SIPMethod::REFER:
        if (HandleRequestToREFER(piSSC) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
        break;

    case SIPMethod::OPTIONS:
        if (Capabilities::HandleOPTIONSRequestWithinDialog(
                GetService(), this, piSSC) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
        break;

    case SIPMethod::UPDATE:
        // ACK_RETRANSMISSION_TO_2XX
        RemoveStrayAcks();

        if (HandleRequestToUPDATE(piSSC) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
        break;

    case SIPMethod::PRACK:
        // PRACK method will be handled by SessionEx class.
        break;

    default:
        // Unknown method received
        if (piSessionListener == IMS_NULL)
        {
            IMS_TRACE_D("No session listener", 0, 0, 0);

            //4 Find a proper response code
            GetService()->CreateResponse(piSSC, SIPStatusCode::SC_480);
            piSSC->Send();
            piSSC->Close();
            break;
        }

        PostMessage(AMSG_SESSION_TRANSACTION_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSSC));
        break;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Session::Refreshable_RefreshCompleted(IN ISIPClientConnection *piSCC,
        IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ SESSION REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyCompleted(piSCC);
    }

    // In case, the session refresh request is successfully done.
    if (nCode == 0)
    {
        const SIPMethod &objMethod = piSCC->GetMethod();
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        // Update the call state
        UpdateCallStateOnMessageReceived(piSCC->GetMessage());

        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (objMethod.Equals(SIPMethod::INVITE))
            {
                if (SendRequestToACK(piSCC, IMessage::SESSION_UPDATE) != IMS_SUCCESS)
                {
                    // Clean up the session resources
                    //SetState(STATE_TERMINATED);
                    //CleanupMedia();
                    //PostEvent(AMSG_SESSION_TERMINATED, IMS_NULL);
                    return;
                }
            }

            IMS_TRACE_I("___ SESSION REFRESH IS SUCCESSFULLY DONE", 0, 0, 0);
        }
        else if ((nStatusCode == SIPStatusCode::SC_408)
            || (nStatusCode == SIPStatusCode::SC_481))
        {
            CleanupMedia();

            if (GetState() != STATE_ESTABLISHED)
            {
                IMS_TRACE_D("BYE can't be sent in the state (%s)",
                        StateToString(GetState()), 0, 0);
                return;
            }

            if (nStatusCode == SIPStatusCode::SC_408)
                SetTerminationReason(TERMINATION_REASON_REFRESH_408);
            else
                SetTerminationReason(TERMINATION_REASON_REFRESH_481);

            // Send BYE request
            if (SendRequestToBYEInternal() != IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                return;
            }

            SetState(STATE_TERMINATING);
        }

        // AUTH_SIP_DIGEST {
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
    }
    // The session refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        CleanupMedia();

        if (GetState() != STATE_ESTABLISHED)
        {
            IMS_TRACE_D("BYE can't be sent in the state (%s)", StateToString(GetState()), 0, 0);
            return;
        }

        SetTerminationReason(TERMINATION_REASON_REFRESH_TXN_TIMEOUT);

        // Send a BYE ???
        if (SendRequestToBYEInternal() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ SESSION REFRESH STARTED ... State(%d)", nState, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
    }

    IMS_BOOL bRefreshable = IMS_FALSE;

    if (nState == STATE_ESTABLISHED)
    {
        // Send a refresh request : UPDATE or re-INVITE
        bRefreshable = IMS_TRUE;
    }
    else
    {
        if (!SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
        {
            if ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING))
            {
                // Send a refresh request : UPDATE
                bRefreshable = IMS_TRUE;
            }
        }
    }

    if (bRefreshable)
    {
        IMS_RESULT nRefreshResult = IMS_SUCCESS;

        if (bDoImplicitRefresh)
        {
            nRefreshResult = SendRequestForRefresh();
        }

        return (nRefreshResult == IMS_SUCCESS) ? IMS_TRUE : IMS_FALSE;
    }

    return (bDoImplicitRefresh == IMS_TRUE) ? IMS_FALSE : IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Session::Refreshable_RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("___ SESSION REFRESH TERMINATED ...", 0, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyTerminated();
    }

    if (GetState() == STATE_ESTABLISHED)
    {
        CleanupMedia();

        SetTerminationReason(TERMINATION_REASON_REFRESH_TIMEOUT);

        // Send a BYE ???
        if (SendRequestToBYEInternal() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_RESULT Session::RetransmissionHelper_NotifyStatus(IN IMS_SINT32 nStatus)
{
    //---------------------------------------------------------------------------------------------

    if (pRetransmissionHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "Stray notification received", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = GetState();

    switch (nStatus)
    {
    case RetransmissionHelper::NOTIFICATION_INTERNAL_ERROR:
        IMS_TRACE_D("RetransmissionHelper :: INTERNAL_ERROR", 0, 0, 0);

        if (nState == STATE_ESTABLISHING)
        {
            CleanupMedia();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                // To send a BYE by enabler
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            }
        }
        else if (nState == STATE_REESTABLISHING)
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            RestoreOfferAnswerState();
            RestoreEx();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
        }
        break;

    case RetransmissionHelper::NOTIFICATION_RETRANSMIT:
        IMS_TRACE_D("RetransmissionHelper :: RETRANSMIT", 0, 0, 0);

        {
            ISIPServerConnection *piINVITE = IMS_NULL;

            if (nState == STATE_ESTABLISHING)
            {
                piINVITE = GetServerConnection(IMessage::SESSION_START);
            }
            else
            {
                piINVITE = GetServerConnection(IMessage::SESSION_UPDATE);
            }

            if (piINVITE == IMS_NULL)
                return IMS_FAILURE;

            if (piINVITE->Send() != IMS_SUCCESS)
                return IMS_FAILURE;
        }
        break;

    case RetransmissionHelper::NOTIFICATION_TIMER_EXPIRED:
        IMS_TRACE_D("RetransmissionHelper :: TIMER_EXPIRED", 0, 0, 0);

        if (nState == STATE_ESTABLISHING)
        {
            CleanupMedia();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                // To send a BYE by enabler
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            }
        }
        else if (nState == STATE_REESTABLISHING)
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            RestoreOfferAnswerState();
            RestoreEx();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
        }
        break;

    default:
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

#ifdef __IMS_SESSION_RETRY_TASK__
/*

Remarks

*/
PROTECTED VIRTUAL
IMS_RESULT Session::ExecuteCmd()
{
    ISIPServerConnection *piINVITE = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("RetransmissionTask :: RETRANSMIT", 0, 0, 0);

    if (GetState() == STATE_ESTABLISHING)
    {
        piINVITE = GetServerConnection(IMessage::SESSION_START);
    }
    else
    {
        piINVITE = GetServerConnection(IMessage::SESSION_UPDATE);
    }

    if (piINVITE == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (piINVITE->Send() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    pRefreshHelper->UpdateOnMessageSent(piINVITE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Session::RetryTaskHelper_OnCompleted(IN RetryTaskHelper *pTaskHelper, IN RetryCmd *pCmd,
        IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    (void) pCmd;

    if (pRetransmissionTask == IMS_NULL)
    {
        IMS_TRACE_E(0, "Stray notification received", 0, 0, 0);
        return;
    }

    if (pRetransmissionTask != pTaskHelper)
    {
        IMS_TRACE_E(0, "Not matched retry task received", 0, 0, 0);
        return;
    }

    IMS_SINT32 nState = GetState();

    switch (nCode)
    {
    case RetryTaskHelper::RESULT_OK:
        break;

    case RetryTaskHelper::RESULT_NOK_INTERNAL_OPERATION:
        IMS_TRACE_D("RetransmissionTask :: INTERNAL_ERROR", 0, 0, 0);

        if (nState == STATE_ESTABLISHING)
        {
            CleanupMedia();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                // To send a BYE by enabler
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            }
        }
        else if (nState == STATE_REESTABLISHING)
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            RestoreOfferAnswerState();
            RestoreEx();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
        }

        pTaskHelper->Terminate();
        break;

    case RetryTaskHelper::RESULT_NOK_TIMER_EXPIRED:
        IMS_TRACE_D("RetransmissionTask :: TIMER_EXPIRED", 0, 0, 0);

        if (nState == STATE_ESTABLISHING)
        {
            CleanupMedia();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                // To send a BYE by enabler
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            }
        }
        else if (nState == STATE_REESTABLISHING)
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            RestoreOfferAnswerState();
            RestoreEx();

            if (bFlag_TerminatePending)
            {
                bFlag_TerminatePending = IMS_FALSE;

                // ACK wait timer expired ...
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }
                else
                {
                    SetState(STATE_TERMINATING);
                }
            }
            else
            {
                SetState(STATE_ESTABLISHED);

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
        }

        pTaskHelper->Terminate();
        break;

    default:
        break;
    }
}
#endif

/*

Remarks

*/
PROTECTED VIRTUAL
Session* Session::CreateSession()
{
    Session *pSession = new Session(GetService());

    //---------------------------------------------------------------------------------------------

    if (pSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSession->InitMethod(this, IsMobileOriginated()))
    {
        delete pSession;

        IMS_TRACE_E(0, "Initializing Session failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pSession;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_RESULT Session::HandleProvisionalResponse(IN ISIPClientConnection *piSCC)
{
    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    if (nStatusCode == SIPStatusCode::SC_100)
    {
        if (bFlag_100TryingNotification)
        {
            // INDEX_FOR_PROVISIONAL_RESPONSE_MESSAGE
            IMSList<Message*> objResponses;

            if ((GetState() == STATE_ESTABLISHING) || (GetState() == STATE_NEGOTIATING))
            {
                objResponses = GetPreviousResponses(IMessage::SESSION_START);
            }
            else
            {
                objResponses = GetPreviousResponses(IMessage::SESSION_UPDATE);
            }

            IMS_TRACE_D("100 Trying is received; It will be handled by the application...",
                    0, 0, 0);
            PostMessage(AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED, 0, objResponses.GetSize()-1);
        }
        else
        {
            IMS_TRACE_D("100 Trying is received; Ignore it...", 0, 0, 0);
        }

        return IMS_SUCCESS;
    }

    if (bFlag_SDPNonRPRAllowed || IsConfigurationSet(CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR))
    {
        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSCC->GetMessage());

        if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
                || (nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
                || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
                || (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND))
        {
            // Update the media state
            UpdateMedia(Media::SESSION_EARLY_UPDATE);
        }
        else
        {
            // This can only happen if the other end sends response to an INVITE/re-INVITE
            // with unsupported media descriptors.
            // This should not happen and if it does there is no easy way for us to signal that
            // we do not accept the response.
            // We simply ignore the response and do not send an ACK which should cause the
            // other side to time out.
        }
    }

    // INDEX_FOR_PROVISIONAL_RESPONSE_MESSAGE
    IMSList<Message*> objResponses;

    if ((GetState() == STATE_ESTABLISHING) || (GetState() == STATE_NEGOTIATING))
    {
        objResponses = GetPreviousResponses(IMessage::SESSION_START);
    }
    else
    {
        objResponses = GetPreviousResponses(IMessage::SESSION_UPDATE);
    }

    PostMessage(AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED, 0, objResponses.GetSize() - 1);

    if (nStatusCode == SIPStatusCode::SC_180)
    {
        /*
        if (nState == STATE_TERMINATING)
        {
            // Do something ; process the pending session termination procedure
            // Do CANCEL procedure.
        }*/
        NotifyAlerting();
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_RESULT Session::HandleRequestToUPDATE(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    //4 Check if we have sent a session refresh request
    IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

    if (piMessage != IMS_NULL)
    {
        if (piMessage->GetMethod().Equals(SIPMethod::INVITE))
        {
            IMessage *piResponse = GetPreviousResponse(IMessage::SESSION_UPDATE);

            // 140818, CONDITION_ACK_WAITING_STATE
            // It needs to properly handle UPDATE request(session refresh) before receiving ACK.
            if ((piResponse == IMS_NULL)
                    || ((piResponse != IMS_NULL)
                        && (piResponse->GetStatusCode() < SIPStatusCode::SC_200))
                    || (GetState() == STATE_REESTABLISHING))
            {
                //4 check SDP presentity
                IMS_TRACE_D("re-INVITE transaction is in progress...; do implicitly answer " \
                        "to the UPDATE request", 0, 0, 0);

                if (SendResponseToRefreshUPDATE(piSSC) != IMS_SUCCESS)
                {
                    piSSC->Close();
                    return IMS_FAILURE;
                }

                piSSC->Close();
                return IMS_SUCCESS;
            }
        }
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSSC->GetMessage());

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSSC->GetMessage());

    if ((nOAResult == SdpOfferAnswer::RESULT_FAILURE)
            || (nOAResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (GetService()->SendResponse(piSSC, SIPStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
        }

        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        RestoreOfferAnswerState();

        piSSC->Close();

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (GetService()->CreateResponse(piSSC, SIPStatusCode::SC_606) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);

            RestoreOfferAnswerState();
            return IMS_FAILURE;
        }

        AString strWarning;

        if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSIPProfile()))
        {
            AString strUAString = SIPConfigProxy::GetUaString(
                    GetSlotId(), GetService()->GetSIPProfile());

            strUAString = strUAString.Replace(" ", "");

            if (strUAString.GetLength() != 0)
                strWarning.Sprintf("304 %s \"Media Type Not Available\"", strUAString.GetStr());
            else
                strWarning = "304 \"Media Type Not Available\"";
        }
        else
        {
            strWarning = "304 \"Media Type Not Available\"";
        }

        (void) piSSC->GetMessage()->SetHeader(ISIPHeader::WARNING, strWarning);

        // SIP_MESSAGE_MEDIATOR
        (void) AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSSC->Send() != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
        }

        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        RestoreOfferAnswerState();

        piSSC->Close();

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        IMS_TRACE_D("QoS precondition is required ...", 0, 0, 0);
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
    {
        IMS_TRACE_D("NO MEDIA STATE CHANGED", 0, 0, 0);
    }

    // RACE_CONDITION : SESSION_UPDATE
    if (IsSessionUpdateNotificationInProgress())
    {
        CopyPreviousMessage(IMessage::SESSION_UPDATE, IMessage::SESSION_STALE_UPDATE);
    }

    UpdateRequestOnReceived(IMessage::SESSION_UPDATE, piSSC);

    // Update the media state
    UpdateMedia(Media::SESSION_UPDATE);

    SetState(STATE_RENEGOTIATING);

    PostMessage(AMSG_SESSION_UPDATE_RECEIVED, 0, 0);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_RESULT Session::HandleResponseToUPDATE(IN ISIPClientConnection *piSCC)
{
    IMS_SINT32 nStatusCode = piSCC->GetMessage()->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    UpdateResponseOnReceived(IMessage::SESSION_UPDATE, piSCC);

    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        return IMS_SUCCESS;
    }

    // AUTH_SIP_DIGEST {
    // Handle 401/407 response
    if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return IMS_SUCCESS;
        }
    }
    // }

    bFlag_UpdateRequestor = IMS_FALSE;

    if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        // CALLER_PREFERENCE_MANAGER
        IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if (piMessage != IMS_NULL)
        {
            UpdateCallerPreference(piMessage->GetMessage(), nStatusCode);
        }

        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSCC->GetMessage());

        if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
                || (nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
                || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
        {
            // Update the media state
            UpdateMedia(Media::SESSION_UPDATED);

            if (GetState() == STATE_RENEGOTIATING)
            {
                SetState(STATE_ESTABLISHED);
                // RACE_CONDITION : SESSION_UPDATE
                SetSessionUpdateNotificationState(IMS_TRUE);

                PostMessage(AMSG_SESSION_UPDATED, 0, 0);
            }
        }
        else
        {
            // This can only happen if the other end sends response to an INVITE/re-INVITE
            // with unsupported media descriptors.
            // This should not happen and if it does there is no easy way for us to signal that
            // we do not accept the response.
            // We simply ignore the response and do not send an ACK which should cause the
            // other side to time out.

            // Update the media state
            UpdateMedia(Media::SESSION_UPDATE_FAILED);

            RestoreOfferAnswerState();
            RestoreEx();

            if (GetState() == STATE_RENEGOTIATING)
            {
                SetState(STATE_ESTABLISHED);
                // RACE_CONDITION : SESSION_UPDATE
                SetSessionUpdateNotificationState(IMS_TRUE);

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
        }
    }
    else
    {
        // Update the media state
        UpdateMedia(Media::SESSION_UPDATE_FAILED);

        if (GetState() == STATE_RENEGOTIATING)
        {
            RestoreOfferAnswerState();
            RestoreEx();

            SetState(STATE_ESTABLISHED);
            // RACE_CONDITION : SESSION_UPDATE
            SetSessionUpdateNotificationState(IMS_TRUE);

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }
    }

    CloseConnection(IMessage::SESSION_UPDATE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::HasPendingPRAck() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Session::IsEarlyUpdateInProgress() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::AddRefreshSpecificHeaders(IN ISIPConnection *piSC)
{
    //---------------------------------------------------------------------------------------------

    if (pRefreshHelper == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return pRefreshHelper->AddSpecificHeader(piSC);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::CheckNSetListenerCall(IN IMS_SINT32 nListenerCall)
{
    if ((nCompletedListenerCalls & nListenerCall) != nListenerCall)
    {
        nCompletedListenerCalls |= nListenerCall;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::CheckNCreateSessionDescriptor()
{
    //---------------------------------------------------------------------------------------------

    // Case 1) Initial offer from the local user agent
    if (GetState() == STATE_INITIATED)
    {
        if (pSessionDescriptor != IMS_NULL)
        {
            IMS_TRACE_D("SessionDescriptor already exists", 0, 0, 0);
            return IMS_TRUE;
        }

        // Create a media capabilities for this service & session
        const SIPAddress* pUserAor = GetUserAOR();
        const SIPAddress::UserInfoPart* pUserInfo = pUserAor->GetUserInfoPart();
        const AString& strUserId = (pUserInfo != IMS_NULL)
                ? pUserInfo->GetUser()
                : pUserAor->IsSchemeTEL() ? pUserAor->GetHost() : pUserAor->GetUser();

        if (!pOAState->CreateCapabilities(GetService(), strUserId))
        {
            IMS_TRACE_E(0, "Creating SDP capabilities failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Create a new offer with only SDP session parameter
        // The media parameters are created when CreateMedia() method invoked
        if (!pOAState->InitiateOffer(SDPOAState::OFFER_NEW))
        {
            IMS_TRACE_E(0, "Initiating SDP offer failed", 0, 0, 0);
            return IMS_FALSE;
        }

        pSessionDescriptor = new SessionDescriptor(this);

        if (pSessionDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SessionDescriptor failed", 0, 0, 0);
            return IMS_FALSE;
        }

        IMS_TRACE_D("___ SessionDescriptor is created in INITIATED state ___", 0, 0, 0);
    }
    // Case 2) Initial offer from the peer user agent
    else
    {
        if (pSessionDescriptor != IMS_NULL)
        {
            IMS_TRACE_D("SessionDescriptor already exists", 0, 0, 0);
            return IMS_TRUE;
        }

        IMS_SINT32 nOAState = GetOfferAnswerState();

        // IDLE :: incoming INVITE w/o SDP
        // OFFER_RECEIVED :: incoming INVITE w/ SDP
        if ((nOAState != SDPOAState::STATE_IDLE)
                && (nOAState != SDPOAState::STATE_OFFER_RECEIVED))
        {
            IMS_TRACE_E(0, "__ SessionDescriptor can't be created in offer/answer state (%d) __",
                    nOAState, 0, 0);
            return IMS_FALSE;
        }

        // Create a media capabilities for this service & session
        const SIPAddress* pUserAor = GetUserAOR();
        const SIPAddress::UserInfoPart* pUserInfo = pUserAor->GetUserInfoPart();
        const AString& strUserId = (pUserInfo != IMS_NULL)
                ? pUserInfo->GetUser()
                : pUserAor->IsSchemeTEL() ? pUserAor->GetHost() : pUserAor->GetUser();

        if (!pOAState->CreateCapabilities(GetService(), strUserId))
        {
            IMS_TRACE_E(0, "Creating SDP capabilities failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Create a new offer with only SDP session parameter
        // The media parameters are created when CreateMedia() method invoked
        if (nOAState == SDPOAState::STATE_IDLE)
        {
            if (!pOAState->InitiateOffer(SDPOAState::OFFER_NEW))
            {
                IMS_TRACE_E(0, "Initiating SDP offer (MT) failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        pSessionDescriptor = new SessionDescriptor(this);

        if (pSessionDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SessionDescriptor failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (nOAState == SDPOAState::STATE_IDLE)
        {
            IMS_TRACE_D("__ SessionDescriptor is created in IDLE state __", 0, 0, 0);
        }
        else
        {
            IMS_TRACE_D("__ SessionDescriptor is created in OFFER_RECEIVED state __", 0, 0, 0);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::CheckNSetSDPBodyPart(IN_OUT ISIPMessage *&piSIPMsg)
{

    //---------------------------------------------------------------------------------------------

    if (objMedias.IsEmpty())
    {
        IMS_TRACE_D("There is no media", 0, 0, 0);
        return IMS_TRUE;
    }

    // SDP message to be set
    AString strSDP;

    if (!pOAState->GetSDP(strSDP))
    {
        IMS_TRACE_D("There is no SDP message body", 0, 0, 0);
        return IMS_TRUE;
    }

    ISIPMessageBodyPart *piBodyPart = piSIPMsg->CreateSDPBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSDP(strSDP);

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE, SIP::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSDP);

    // Set the Content-Length header
    AString strCLEN;
    strCLEN.SetNumber(objSDP.GetLength());

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_UNKNOWN,
            strCLEN, SIPHeaderName::CONTENT_LENGTH);

    IMS_TRACE_D("SDP is formed by SDP offer/answer context", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::CheckNTerminateSession(IN ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (!bFlag_TerminatePending)
    {
        IMS_TRACE_D("No pending terminate()", 0, 0, 0);
        return IMS_FALSE;
    }

    // Reset the flag
    bFlag_TerminatePending = IMS_FALSE;

    if (IsMobileOriginated() && (GetState() == STATE_NEGOTIATING))
    {
        // Check the SIP status code to terminate the session
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (SIPStatusCode::Is1XX(nStatusCode))
        {
            CleanupMedia();

            if (bFlag_TerminateMethodBYE)
            {
                // Send BYE
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                    return IMS_TRUE;
                }
            }
            else
            {
                // Send CANCEL
                if (SendRequestToCANCEL() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                    return IMS_TRUE;
                }
            }

            SetState(STATE_TERMINATING);
        }
        else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
#ifdef __IMS_SEND_ACK_IN_TERMINATING_STATE__
            // Send ACK
            IMS_SINT32 nState = GetState();
            IMS_SINT32 nServiceMethod = IMessage::SERVICEMETHOD_INVALID;

            if (nState == STATE_NEGOTIATING)
                nServiceMethod = IMessage::SESSION_START;
            else if (nState == STATE_RENEGOTIATING)
                nServiceMethod = IMessage::SESSION_UPDATE;

            ISIPClientConnection *piSCC = GetClientConnection(nServiceMethod);

            if ((piSCC != IMS_NULL)
                && (piSIPMsg->GetMethod().Equals(SIPMethod::INVITE)))
            {
                // Check & create a session descriptor when an initial offer received
                CheckNCreateSessionDescriptor();

                IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSIPMsg);

                if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
                        || (nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
                        || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
                        || (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND))
                {
                    if (pOAState->IsSessionChanged())
                    {
                        if ((GetOfferAnswerState() == SDPOAState::STATE_IDLE)
                                || (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
                        {
                            pOAState->CompleteExchange();
                        }
                    }
                }
                else
                {
                    RestoreOfferAnswerState();
                }

                SendRequestToACK(piSCC, nServiceMethod);
            }
#endif // __IMS_SEND_ACK_IN_TERMINATING_STATE__

            CleanupMedia();

            // Send BYE
            if (SendRequestToBYE() != IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                return IMS_TRUE;
            }

            SetState(STATE_TERMINATING);
        }
        else
        {
            CleanupMedia();

            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
    }
    else
    {
        // ACK request is received, now we can terminate the session
        if (SendRequestToBYE() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            return IMS_TRUE;
        }

        SetState(STATE_TERMINATING);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
ISIPClientConnection* Session::CreateConnectionL(IN ISIPDialog *piDialog,
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
PROTECTED
IMS_SINT32 Session::GetCallState() const
{
    //---------------------------------------------------------------------------------------------

    return objCallState.GetState();
}

/*

Remarks

*/
PROTECTED
IMS_SINT32 Session::GetOfferAnswerState() const
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return SDPOAState::STATE_IDLE;
    }

    return pOAState->GetState();
}

/*

Remarks

*/
PROTECTED
SessionRefreshHelper* Session::GetRefreshHelper() const
{
    //---------------------------------------------------------------------------------------------

    return pRefreshHelper;
}

/*

Remarks

*/
PROTECTED
IMS_SINT32 Session::HandleSDPOfferAnswer(IN ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return SdpOfferAnswer::RESULT_NOT_CHANGED;
    }

    IMS_SINT32 nOAResult = pOAState->HandleOfferAnswer(piSIPMsg);

    // If incoming SDP offer contains "qos" attributes and precondition is not supported,
    // then the corresponding SDP attributes will be removed.
    if (!SdpProfile::GetInstance()->IsAttributePreconditionSupported(GetSlotId()))
    {
        IMS_SINT32 nOAState = pOAState->GetState();

        if (((nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
                || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
             && ((nOAState == SDPOAState::STATE_OFFER_RECEIVED)
                || (nOAState == SDPOAState::STATE_OFFER_CHANGE_RECEIVED)))
        {
            SessionParameter *pSessionParam = pOAState->GetProposalView();

            if (pSessionParam != IMS_NULL)
            {
                const IMSList<SdpMediaParameter*> &objMediaParams
                        = pSessionParam->GetMediaParameters();

                for (IMS_UINT32 i = 0; i < objMediaParams.GetSize(); i++)
                {
                    SdpMediaParameter *pMediaParam = objMediaParams.GetAt(i);

                    if (pMediaParam != IMS_NULL)
                    {
                        IMS_TRACE_I("SDP attributes(qos:%s) are removed",
                                pMediaParam->GetMedia().GetTypeEx().GetStr(), 0, 0);

                        pMediaParam->RemovePrecondition(SdpAttribute::CURR);
                        pMediaParam->RemovePrecondition(SdpAttribute::DES);
                        pMediaParam->RemovePrecondition(SdpAttribute::CONF);
                    }
                }
            }
        }
    }

    return nOAResult;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::IsInviteFinalResponseReceived(IN IMS_SINT32 nServiceMethod) const
{
    Message* pMessage = GetPreviousResponse(nServiceMethod);
    ISIPMessage* piSIPMsg = (pMessage != IMS_NULL) ? pMessage->GetMessage() : IMS_NULL;

    //---------------------------------------------------------------------------------------------

    return (piSIPMsg != IMS_NULL)
            && piSIPMsg->GetMethod().Equals(SIPMethod::INVITE)
            && (piSIPMsg->GetStatusCode() >= SIPStatusCode::SC_200);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::IsMidDialogTransactionCreatable() const
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState == STATE_ESTABLISHED)
            || (nState == STATE_RENEGOTIATING)
            || (nState == STATE_REESTABLISHING))
    {
        return IMS_TRUE;
    }
    else if ((nState == STATE_ESTABLISHING)
            && !IsMobileOriginated())
    {
        IMS_SINT32 nCallState = GetCallState();

        if ((nCallState == CallState::STATE_INVITE_2XX_SENT)
                || (nCallState == CallState::STATE_ESTABLISHED))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::IsTerminatePending() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_TerminatePending;
}

/*

Remarks

*/
PROTECTED
void Session::NotifyAlerting()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_NEGOTIATING)
    {
        if (!bFlag_Alerting)
        {
            bFlag_Alerting = IMS_TRUE;

            IMS_TRACE_D("Session :: Alerting ...", 0, 0, 0);

            PostMessage(AMSG_SESSION_ALERTING, 0, 0);
        }
    }
}

/*

Remarks

*/
PROTECTED
void Session::RestoreEx()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        Media *pMedia = objMedias.GetAt(i);

        if (pMedia->GetState() == Media::STATE_INACTIVE)
        {
            // Delete the media from the list ?????
        }

        //4 Add the code to restore the medias
        pMedia->RestoreMedia();
    }
}

/*

Remarks

*/
PROTECTED
IMS_RESULT Session::SendResponseToRefreshUPDATE(IN ISIPServerConnection *piSSC)
{
    if (!CreateResponse(piSSC, SIPStatusCode::SC_200))
    {
        IMS_TRACE_E(0, "Creating a response failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Session-Expires
    // Require
    if (pRefreshHelper != IMS_NULL)
    {
        if (!pRefreshHelper->AddSpecificHeader(piSSC))
        {
            IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    // SIP_MESSAGE_MEDIATOR
    (void) AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

    if (piSSC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a response to UPDATE request (session refresh) failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->UpdateOnMessageSent(piSSC);
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::SetSDPBodyPartFromCurrentView(IN_OUT ISIPMessage *&piSIPMsg)
{

    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SDPOAState is null", 0, 0, 0);
        return IMS_FALSE;
    }

    SessionParameter *pCurrentView = pOAState->GetCurrentView();

    if (pCurrentView == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no current view", 0, 0, 0);
        return IMS_FALSE;
    }

    ISIPMessageBodyPart *piBodyPart = piSIPMsg->CreateSDPBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSDP(pCurrentView->ToSDP());

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE, SIP::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSDP);

    // Set the Content-Length header
    AString strCLEN;
    strCLEN.SetNumber(objSDP.GetLength());

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_UNKNOWN,
            strCLEN, SIPHeaderName::CONTENT_LENGTH);

    IMS_TRACE_D("SDP is formed by the current view", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks
 REFUSE_SDP_OFFER_ANSWER_EXCHANGE
*/
PROTECTED
IMS_BOOL Session::SetSDPBodyPartFromRefusedView(IN_OUT ISIPMessage *&piSIPMsg)
{
    SessionParameter* pRefusedView = (pOAState != IMS_NULL) ?\
            pOAState->GetRefusedView() : IMS_NULL;

    if (pRefusedView == IMS_NULL)
    {
        // No operations if not present
        return IMS_TRUE;
    }

    const SIPMethod& objMethod = piSIPMsg->GetMethod();

    if (!objMethod.Equals(SIPMethod::INVITE)
            && !objMethod.Equals(SIPMethod::BYE)
            && !objMethod.Equals(SIPMethod::CANCEL))
    {
        IMS_TRACE_D("Refused view is not allowed; method=%s",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    ISIPMessageBodyPart *piBodyPart = piSIPMsg->CreateSDPBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSDP(pRefusedView->ToSDP());

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE, SIP::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSDP);

    // Set the Content-Length header
    AString strCLEN;
    strCLEN.SetNumber(objSDP.GetLength());

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_UNKNOWN,
            strCLEN, SIPHeaderName::CONTENT_LENGTH);

    IMS_TRACE_I("SDP is formed by the refused view", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
void Session::SetTerminationReason(IN IMS_SINT32 nReason)
{
    //---------------------------------------------------------------------------------------------

    if (nTerminationReason != TERMINATION_REASON_UNKNOWN)
        return;

    nTerminationReason = nReason;

    switch (nTerminationReason)
    {
    case TERMINATION_REASON_USER_ACTION:
        IMS_TRACE_D("Session is terminated by user action", 0, 0, 0);
        break;
    case TERMINATION_REASON_REMOTE_ACTION:
        IMS_TRACE_D("Session is terminated by remote action", 0, 0, 0);
        break;
    case TERMINATION_REASON_REFRESH_408:
        IMS_TRACE_D("Session is terminated by 408 response of session refresh", 0, 0, 0);
        break;
    case TERMINATION_REASON_REFRESH_481:
        IMS_TRACE_D("Session is terminated by 481 response of session refresh", 0, 0, 0);
        break;
    case TERMINATION_REASON_REFRESH_TXN_TIMEOUT:
        IMS_TRACE_D("Session is terminated by txn timeout of session refresh", 0, 0, 0);
        break;
    case TERMINATION_REASON_REFRESH_TIMEOUT:
        IMS_TRACE_D("Session is terminated by timer expiration of session refresh", 0, 0, 0);
        break;
    case TERMINATION_REASON_SERVICE_CLOSED:
        IMS_TRACE_D("Session is terminated by the de-registration", 0, 0, 0);
        break;
    default:
        break;
    }
}

/*

Remarks

*/
PROTECTED
void Session::UpdateCallStateOnMessageReceived(IN CONST ISIPMessage *piSIPMsg,
        IN IMS_SINT32 nMode /* = CallState::MODE_RECEIVED */)
{
    //---------------------------------------------------------------------------------------------

    objCallState.UpdateState(piSIPMsg, nMode);
}

/*

Remarks

*/
PROTECTED
void Session::UpdateCallStateOnMessageSent(IN CONST ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    objCallState.UpdateState(piSIPMsg, CallState::MODE_SENT);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::UpdateMedia(IN IMS_SINT32 nTrigger)
{
    IMS_BOOL bResult = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    if (!pOAState->IsSessionChanged())
    {
        IMS_TRACE_D("UpdateMedia :: No session changed", 0, 0, 0);
        return IMS_TRUE;
    }

    switch (GetOfferAnswerState())
    {
    case SDPOAState::STATE_OFFER_SENT:
    case SDPOAState::STATE_OFFER_CHANGE_SENT:
        bResult = UpdateMediaOnOfferSent(nTrigger);
        break;

    case SDPOAState::STATE_ESTABLISHED:
        if (pOAState->GetMode() != SDPOAState::MODE_IDLE)
        {
            if (pOAState->GetMode() == SDPOAState::MODE_OFFERER)
                bResult = UpdateMediaOnAnswerReceived(nTrigger);
            else
                bResult = UpdateMediaOnAnswerSent(nTrigger);
        }
        else
        {
            IMS_TRACE_D("UpdateMedia :: SDP Offer/Answer is IDLE", 0, 0, 0);
        }
        break;

    case SDPOAState::STATE_OFFER_RECEIVED:
    case SDPOAState::STATE_OFFER_CHANGE_RECEIVED:
        bResult = UpdateMediaOnOfferReceived(nTrigger);
        break;

    default:
        IMS_TRACE_D("UpdateMedia :: NOT HANDLED", 0, 0, 0);
        return IMS_FALSE;
    }

    // Move the proposed view to the current view
    if ((GetOfferAnswerState() == SDPOAState::STATE_IDLE)
            || (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
    {
        pOAState->CompleteExchange();
    }

    return bResult;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::RestoreOfferAnswerState()
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pOAState->RestoreState();
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::UpdateOfferAnswerStateOnMessageReceived(IN CONST ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = GetState();
    IMS_BOOL bIsCallEstablished = IMS_FALSE;

    //RFC 6337: Section 3.1.1
    //After the UAC has received the answer in a reliable provisional response to the INVITE,
    //[RFC3261] requires that any SDP in subsequent responses be ignored.
    if (IsConfigurationSet(CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE))
    {
       if ((piSIPMsg->GetType() == ISIPMessage::TYPE_RESPONSE)
                && piSIPMsg->GetMethod().Equals(SIPMethod::INVITE))
       {
           if ((nState == STATE_NEGOTIATING)
                    && (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
           {
               IMS_TRACE_D("UpdateOfferAnswerStateOnMessageReceived() :: "
                        "Ignore the SDP in subsequent RPR as offer-answer is completed", 0, 0, 0);

               pOAState->UpdateStateOnTransactionCompleted(
                        piSIPMsg, SDPOAState::MESSAGE_RECEIVED);
               return IMS_TRUE;
           }
       }
    }

    if ((nState == STATE_ESTABLISHED)
            || (nState == STATE_RENEGOTIATING)
            || (nState == STATE_REESTABLISHING))
    {
        bIsCallEstablished = IMS_TRUE;
    }

    return pOAState->UpdateState(piSIPMsg,
            SDPOAState::MESSAGE_RECEIVED, bIsCallEstablished, (bFlag_SDPNonRPRAllowed
                || IsConfigurationSet(CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR)));
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Session::UpdateOfferAnswerStateOnMessageSent(IN CONST ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (pOAState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = GetState();
    IMS_BOOL bIsCallEstablished = IMS_FALSE;

    if ((nState == STATE_ESTABLISHED)
            || (nState == STATE_RENEGOTIATING)
            || (nState == STATE_REESTABLISHING))
    {
        bIsCallEstablished = IMS_TRUE;
    }

    return pOAState->UpdateState(piSIPMsg,
            SDPOAState::MESSAGE_SENT, bIsCallEstablished, (bFlag_SDPNonRPRAllowed
                || IsConfigurationSet(CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR)));
}

/*

Remarks
 CALLER_PREFERENCE_MANAGER
*/
PROTECTED
void Session::UpdateCallerPreference(IN CONST ISIPMessage *piPreviousSIPMsg,
        IN IMS_SINT32 nStatusCode /* = 200 */)
{
    //---------------------------------------------------------------------------------------------

    if (piPreviousSIPMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISIPMessage is null", 0, 0, 0);
        return;
    }

    const SIPMethod &objMethod = piPreviousSIPMsg->GetMethod();

    if (objMethod.Equals(SIPMethod::INVITE))
    {
        IMS_SINT32 nState = GetState();

        if (nState == STATE_NEGOTIATING)
        {
            ISIPDialog *piDialog = GetDialog();

            if ((piDialog != IMS_NULL)
                    && ((piDialog->GetState() == ISIPDialog::STATE_EARLY)
                        || (piDialog->GetState() == ISIPDialog::STATE_CONFIRMED)))
            {
                CallerPreferenceManager::GetInstance()->UpdateDialogId(GetName(),
                        piDialog->GetDialogID());
                CallerPreferenceManager::GetInstance()->UpdateAcceptContacts(
                        GetName(), piPreviousSIPMsg->GetHeaders(ISIPHeader::ACCEPT_CONTACT));
            }
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            if (SIPStatusCode::IsProvisional(nStatusCode)
                    || SIPStatusCode::IsFinalSuccess(nStatusCode))
            {
                CallerPreferenceManager::GetInstance()->UpdateAcceptContacts(
                        GetName(), piPreviousSIPMsg->GetHeaders(ISIPHeader::ACCEPT_CONTACT));
            }
            else
            {
                // If the re-INVITE has been failed, restore the caller preference
                // to the previous one
                CallerPreferenceManager::GetInstance()->UpdateAcceptContacts(
                        GetName(), objPreviousCallerPreference);
            }
        }
    }
    else if (objMethod.Equals(SIPMethod::UPDATE))
    {
        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            CallerPreferenceManager::GetInstance()->UpdateAcceptContacts(
                    GetName(), piPreviousSIPMsg->GetHeaders(ISIPHeader::ACCEPT_CONTACT));
        }
    }
    else
    {
        IMS_TRACE_D("The method(%s) can't modify the caller preference",
                objMethod.ToString().GetStr(), 0, 0);
    }
}

/*

Remarks
 REMOVE_RECORD_ROUTE_HEADERS
*/
PROTECTED GLOBAL
void Session::RemoveRecordRouteHeaders(IN ISIPMessage * piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("Remove Record-Route headers in {%s, %d}",
            piSIPMsg->GetMethod().ToString().GetStr(), piSIPMsg->GetStatusCode(), 0);

    IMS_SINT32 nHCount = piSIPMsg->GetHeaderCount(ISIPHeader::RECORD_ROUTE);

    while (nHCount > 0)
    {
        piSIPMsg->RemoveHeader(ISIPHeader::RECORD_ROUTE);
        --nHCount;
    }
}

/*

Remarks

*/
PRIVATE
void Session::AddSessionToCallControlHelper()
{
    // If the sessionId is already present, please remove the session from the CallControlHelper
    if (strSessionIdForCallControl.GetLength() > 0)
    {
        RemoveSessionFromCallControlHelper();
    }

    strSessionIdForCallControl = CallControlHelper::CreateSessionId();

    // Create a Replaces header info...
    Replaces* pReplaces = CallControlHelper::CreateReplaces(IsMobileOriginated(), GetDialog());

    CallControlHelper::GetInstance()->AddSession(strSessionIdForCallControl, pReplaces);

    IMS_TRACE_D("CallControlHelper :: AddSession (%s)",
            strSessionIdForCallControl.GetStr(), 0, 0);
}

/*

Remarks

*/
PRIVATE
void Session::RemoveSessionFromCallControlHelper()
{
    if (strSessionIdForCallControl.GetLength() <= 0)
    {
        IMS_TRACE_D("No session id for the 3rd-party call control", 0, 0, 0);
        return;
    }

    CallControlHelper::GetInstance()->RemoveSession(strSessionIdForCallControl);

    IMS_TRACE_D("CallControlHelper :: RemoveSession (%s), Count (%d)",
            strSessionIdForCallControl.GetStr(),
            CallControlHelper::GetInstance()->GetSessionCount(), 0);

    strSessionIdForCallControl = AString::ConstNull();
}

/*

Remarks

*/
PRIVATE
void Session::CleanupOnDestroy()
{
    //---------------------------------------------------------------------------------------------

    pVirtualEarlySession = IMS_NULL;

    // Close the SIP client connection for an internal BYE
    if (piSCC_BYE != IMS_NULL)
    {
        piSCC_BYE->Close();
        piSCC_BYE = IMS_NULL;
    }

    // Checks if the session refresh is ongoing...
    if (pRefreshHelper != IMS_NULL)
    {
        if (pRefreshHelper->IsRequestPending())
        {
            IMS_TRACE_D("Stopping the session refresh on CleanupOnDestroy()", 0, 0, 0);

            pRefreshHelper->AbortConnection();
        }
    }

    // Management of forked sessions
    if (!pForkedSessions.IsNull())
    {
        pForkedSessions->RemoveMethod(this);
        pForkedSessions = IMS_NULL;
    }

    // Guard code for Cancellable & Dialog
    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    // CALLER_PREFERENCE_MANAGER
    CallerPreferenceManager::GetInstance()->DestroyPreferenceWrapper(GetName());

    // Stop 2xx retransmission if it is running...
    Stop2xxRetransmission();

    // 'Replaces' header handling ...
    RemoveSessionFromCallControlHelper();

    // Clean up the resources
    GetService()->DeregisterMethod(this);
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleRequestToACK(IN ISIPServerConnection *piSSC)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    // UAS_REINVITE_RECEIVED_BEFORE_ACK
    // It's an optimization for the race condition handling between ACK and re-INVITE.
    if ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING))
    {
        IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if ((piMessage != IMS_NULL)
                && piMessage->GetMethod().Equals(SIPMethod::INVITE)
                && (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            // Don't process ACK request anymore if it doesn't match
            IMS_UINT32 nCSeqINVITE = piMessage->GetMessage()->GetCSeqNumber();
            IMS_UINT32 nCSeqACK = piSSC->GetMessage()->GetCSeqNumber();

            if (nCSeqINVITE != nCSeqACK)
            {
                IMS_TRACE_D("Stray ACK is received; CSeq-INVITE=%d, CSeq-ACK=%d",
                        nCSeqINVITE, nCSeqACK, 0);
                return IMS_SUCCESS;
            }
        }
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
    // When INVITE_TXN_HANDLING_CORRECTION is enabled:
    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    UpdateRequestOnReceived(IMessage::SESSION_ACK, piSSC);
    // ACK transaction will be closed by the caller
    ClearConnection(IMessage::SESSION_ACK);

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSIPMsg);

    // Check the termination pending & do the termination procedure
    if (CheckNTerminateSession(piSIPMsg))
    {
        return IMS_SUCCESS;
    }

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSIPMsg);

    if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
            || (nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
            || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
    {
        UpdateMedia(Media::SESSION_UPDATED);
    }
    else
    {
        UpdateMedia(Media::SESSION_UPDATE_FAILED);
    }

    if ((nState == STATE_ESTABLISHING)
        || (nState == STATE_REESTABLISHING))
    {
        // ACK_WITH_SDP_IN_PROGRESS
        if (piSIPMsg->GetSDPBodyPart() != IMS_NULL)
        {
            bFlag_AckWithSDPInProgress = IMS_TRUE;
        }

        // Notify the session establishment to MEDIA

        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        // Handle the ACK according to the state
        if (nState == STATE_ESTABLISHING)
        {
            // 'Replaces' header handling ...
            // RACE_CONDITION: ACK & re-INVITE in MT
            if (nOAResult != SdpOfferAnswer::RESULT_NOT_CHANGED)
            {
                AddSessionToCallControlHelper();
            }
            else
            {
                AddSessionToCallControlHelperIfNotPresent();
            }

            SetState(STATE_ESTABLISHED);

            PostMessage(AMSG_SESSION_STARTED, 0, 0);

            CloseConnection(IMessage::SESSION_START);
        }
        else
        {
            SetState(STATE_ESTABLISHED);

            PostMessage(AMSG_SESSION_UPDATED, 0, 0);

            CloseConnection(IMessage::SESSION_UPDATE);
        }
    }
    // RACE_CONDITION : UPDATE (w/ SDP) received before ACK received
    else if (nState == STATE_RENEGOTIATING)
    {
        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        IMS_BOOL bInitialSession = IMS_TRUE;
        IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if ((piMessage != IMS_NULL)
                && piMessage->GetMethod().Equals(SIPMethod::INVITE)
                && (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            bInitialSession = IMS_FALSE;
        }
        else if ((nCompletedListenerCalls & LISTENER_CALL_STARTED) != 0)
        {
            bInitialSession = IMS_FALSE;
        }

        // Handle the ACK according to the state
        if (bInitialSession)
        {
            // 'Replaces' header handling ...
            // RACE_CONDITION: ACK & re-INVITE in MT
            AddSessionToCallControlHelperIfNotPresent();

            PostMessage(AMSG_SESSION_STARTED, 0, 0);

            CloseConnection(IMessage::SESSION_START);
        }
        else
        {
            PostMessage(AMSG_SESSION_UPDATED, 0, 0);

            CloseConnection(IMessage::SESSION_UPDATE);
        }
    }
    else if (nState == STATE_TERMINATING)
    {
        // What to do ???
        // Clean up the reference related informations
        SetState(STATE_TERMINATED);

        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleRequestToBYE(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    UpdateRequestOnReceived(IMessage::SESSION_TERMINATE, piSSC);

    if (IMS::GetLastError() == IMSError::ALREADY_EXISTS)
    {
        GetService()->SendResponse(piSSC, SIPStatusCode::SC_200);
        piSSC->Close();

        return IMS_SUCCESS;
    }

    IMS_SINT32 nOldState = GetState();

    // RACE CONDITION : reset the terminate request flag
    if (bFlag_TerminatePending)
    {
        bFlag_TerminatePending = IMS_FALSE;

        IMS_TRACE_D("The pending terminate request will be reset", 0, 0, 0);
    }

    // Cease the 2xx retransmission
    Stop2xxRetransmission();

    SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

    SetState(STATE_TERMINATED);

    if (CreateResponse(piSSC, SIPStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to BYE request failed", 0, 0, 0);
        goto EXIT_HandleByeRequest;
    }

    if (!SendNUpdateResponse(IMessage::SESSION_TERMINATE, piSSC))
    {
        goto EXIT_HandleByeRequest;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC->GetMessage());

EXIT_HandleByeRequest:

    CleanupMedia();

    if (nOldState != STATE_TERMINATED)
    {
        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }

    CloseConnection(IMessage::SESSION_TERMINATE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleRequestToCANCEL(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    // RACE CONDITION : reset the terminate request flag
    if (bFlag_TerminatePending)
    {
        bFlag_TerminatePending = IMS_FALSE;

        IMS_TRACE_D("The pending terminate request will be reset", 0, 0, 0);
    }

    // Send a 200 OK to CANCEL request
    if (CreateResponse(piSSC, SIPStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to CANCEL request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
    IMS_SINT32 nMethodForCancel = IMessage::SESSION_TERMINATE;

    if (GetState() >= STATE_ESTABLISHED)
    {
        nMethodForCancel = IMessage::SESSION_CANCEL;
    }

    if (!SendNUpdateResponse(nMethodForCancel, piSSC))
    {
        return IMS_FAILURE;
    }

    // Send a failure final response (487) to an INVITE request
    ISIPServerConnection *piSSC_INVITE = IMS_NULL;
    IMS_SINT32 nCallState = GetCallState();
    IMS_SINT32 nServiceMethod = IMessage::SERVICEMETHOD_INVALID;

    if ((nCallState == CallState::STATE_INVITE_1XX_SENT)
            || (nCallState == CallState::STATE_INVITE_RECEIVED))
    {
        nServiceMethod = IMessage::SESSION_START;
    }
    else if ((nCallState == CallState::STATE_REINVITE_1XX_SENT)
            || (nCallState == CallState::STATE_REINVITE_RECEIVED))
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
    }

    piSSC_INVITE = GetServerConnection(nServiceMethod);

    if (piSSC_INVITE == IMS_NULL)
    {
        IMS_TRACE_E(0, "INVITE transaction does not exist", 0, 0, 0);

        SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

        SetState(STATE_TERMINATED);
        CleanupMedia();
        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = GetState();

    if (CreateResponse(piSSC_INVITE, SIPStatusCode::SC_487) == IMS_FALSE)
    {
        if (nState == STATE_NEGOTIATING)
        {
            SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

            SetState(STATE_TERMINATED);
            CleanupMedia();
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            RestoreOfferAnswerState();
            RestoreEx();

            SetState(STATE_ESTABLISHED);
            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }

        return IMS_FAILURE;
    }

    if (!SendNUpdateResponse(nServiceMethod, piSSC_INVITE))
    {
        if (nState == STATE_NEGOTIATING)
        {
            SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

            SetState(STATE_TERMINATED);
            CleanupMedia();
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            RestoreOfferAnswerState();
            RestoreEx();

            SetState(STATE_ESTABLISHED);
            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }

        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC_INVITE->GetMessage());

    if (nState == STATE_NEGOTIATING)
    {
        SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

        SetState(STATE_TERMINATED);
        CleanupMedia();
        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }
    else if (nState == STATE_RENEGOTIATING)
    {
        RestoreOfferAnswerState();
        RestoreEx();

        SetState(STATE_ESTABLISHED);
        PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleRequestToINVITE(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    UpdateRequestOnReceived(IMessage::SESSION_START, piSSC);

    // Restore the media state if it needs

    // Check & create a session descriptor when an initial offer received
    if (GetOfferAnswerState() != SDPOAState::STATE_IDLE)
    {
        // Incoming INVITE w/ SDP
        CheckNCreateSessionDescriptor();
    }

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSSC->GetMessage());

    if ((nOAResult == SdpOfferAnswer::RESULT_FAILURE)
            || (nOAResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (CreateResponse(piSSC, SIPStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (CreateResponse(piSSC, SIPStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        AString strWarning("304 \"Media Type Not Available\"");
        (void) piSSC->GetMessage()->SetHeader(ISIPHeader::WARNING, strWarning);

        if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        IMS_TRACE_D("QoS precondition is required ...", 0, 0, 0);
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
    {
        IMS_TRACE_D("NO MEDIA STATE CHANGED", 0, 0, 0);
    }

    // Update the media state
    UpdateMedia(Media::SESSION_START);

    // According to the offer/answer, the failure final response can be sent.

    SetState(STATE_NEGOTIATING);

    PostMessage(AMSG_SESSION_INVITATION_RECEIVED, 0, 0);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleRequestToINVITEWithinDialog(IN ISIPServerConnection *piSSC)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState == STATE_TERMINATING)
            || (nState == STATE_TERMINATED))
    {
        IMS_TRACE_D("re-INVITE is received in TERMINATING or TERMINATED state (%d)",
                nState, 0, 0);

        GetService()->SendResponse(piSSC, SIPStatusCode::SC_488);
        piSSC->Close();
        return IMS_SUCCESS;
    }

    IMS_SINT32 nStatusCode = SIPStatusCode::SC_INVALID;

    if (nState == STATE_NEGOTIATING)
    {
        nStatusCode = SIPStatusCode::SC_491;
    }
    // Checks the message validity:
    // If re-INVITE is sent, we needs to reject the message with 491 Request Pending response.
    else if ((nState == STATE_RENEGOTIATING) || (pOAState->IsOfferProgress()))
    {
        nStatusCode = SIPStatusCode::SC_491;
    }
    // Offer is sent by 200 OK, and waits for ACK w/ SDP answer
    else if ((nState == STATE_REESTABLISHING)
            && (GetOfferAnswerState() == SDPOAState::STATE_OFFER_CHANGE_SENT))
    {
        ISIPMessage *piSIPMsg = piSSC->GetMessage();

        if ((piSIPMsg != IMS_NULL) && (piSIPMsg->GetSDPBodyPart() != IMS_NULL))
        {
            nStatusCode = SIPStatusCode::SC_500;
        }
    }

    if (nStatusCode != SIPStatusCode::SC_INVALID)
    {
        IMS_TRACE_I("Rejecting re-INVITE with %d ...", nStatusCode, 0, 0);

        if (GetService()->CreateResponse(piSSC, nStatusCode))
        {
            // Sets Retry-After header field
            AString strRAHdr;

            strRAHdr.SetNumber(SIPHeaderUtil::GenerateRetryAfterSeconds(10));

            piSSC->GetMessage()->SetHeader(ISIPHeader::RETRY_AFTER_SEC, strRAHdr);

            piSSC->Send();

            // Update the call state
            UpdateCallStateOnMessageSent(piSSC->GetMessage());
        }

        piSSC->Close();

        return IMS_SUCCESS;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSSC->GetMessage());

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSSC->GetMessage());

    if ((nOAResult == SdpOfferAnswer::RESULT_FAILURE)
            || (nOAResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (GetService()->SendResponse(piSSC, SIPStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        RestoreOfferAnswerState();

        piSSC->Close();

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        // Original status code: 606, KDDI: 488
        IMS_SINT32 nStatusCode = SIPStatusCode::SC_488;

        IMS_TRACE_I("Rejecting SDP Offer/Answer with %d (Not Acceptable) ...", nStatusCode, 0, 0);

        if (GetService()->CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);

            RestoreOfferAnswerState();
            return IMS_FAILURE;
        }

        AString strWarning;

        if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSIPProfile()))
        {
            AString strUAString = SIPConfigProxy::GetUaString(
                    GetSlotId(), GetService()->GetSIPProfile());

            strUAString = strUAString.Replace(" ", "");

            if (strUAString.GetLength() != 0)
                strWarning.Sprintf("304 %s \"Media Type Not Available\"", strUAString.GetStr());
            else
                strWarning = "304 \"Media Type Not Available\"";
        }
        else
        {
            strWarning = "304 \"Media Type Not Available\"";
        }

        (void) piSSC->GetMessage()->SetHeader(ISIPHeader::WARNING, strWarning);

        // SIP_MESSAGE_MEDIATOR
        (void) AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSSC->Send() != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSSC->GetMessage());
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        RestoreOfferAnswerState();

        piSSC->Close();

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        IMS_TRACE_D("QoS precondition is required ...", 0, 0, 0);
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
    {
        IMS_TRACE_D("NO MEDIA STATE CHANGED", 0, 0, 0);
    }

    // RACE_CONDITION : SESSION_UPDATE
    if (IsSessionUpdateNotificationInProgress())
    {
        CopyPreviousMessage(IMessage::SESSION_UPDATE, IMessage::SESSION_STALE_UPDATE);
    }

    // UAS_REINVITE_RECEIVED_BEFORE_ACK
    // If re-INVITE is received before receiving ACK to the previous 200 OK,
    // then UAS stops the 2xx retransmission and abort the previous INVITE transaction.
    // Even though ACK waiting timer is expired, it will be ignored in J180 layer.
    if (nState == STATE_ESTABLISHING)
    {
        IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_START);

        if ((piMessage != IMS_NULL)
                && (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            Stop2xxRetransmission();
            CloseConnection(IMessage::SESSION_START);
        }
    }
    else if (nState == STATE_REESTABLISHING)
    {
        IMessage *piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if ((piMessage != IMS_NULL)
                && piMessage->GetMethod().Equals(SIPMethod::INVITE)
                && (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            Stop2xxRetransmission();
            CloseConnection(IMessage::SESSION_UPDATE);
        }
    }

    UpdateRequestOnReceived(IMessage::SESSION_UPDATE, piSSC);

    // Update the media state
    UpdateMedia(Media::SESSION_UPDATE);

    //3 PRACK / UPDATE

    // According to the offer/answer, the failure final response can be sent.

#if 0
    // Send 180 Ringing
    IMS_DEBUG_("Sending 180 \"Ringing\" ...");

    if (CreateResponse(piSSC, SIPStatusCode::SC_180) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    if (!SendNUpdateResponse(IMessage::SESSION_START, piSSC))
    {
        IMS_TRACE_E(0, "Sending a response failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    CheckNCreateDialog(piSSC);
#endif

    SetState(STATE_RENEGOTIATING);

    PostMessage(AMSG_SESSION_UPDATE_RECEIVED, 0, 0);

    // To cancel re-INVITE request when it is received in the mid-dialog
    CancellableMethodManager::GetInstance()->AddMethod(GetName(), this);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleRequestToREFER(IN ISIPServerConnection *piSSC)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState == STATE_TERMINATING)
            || (nState == STATE_TERMINATED))
    {
        IMS_TRACE_D("REFER is received in TERMINATING or TERMINATED state (%d)", nState, 0, 0);

        GetService()->SendResponse(piSSC, SIPStatusCode::SC_488);
        piSSC->Close();
        return IMS_SUCCESS;
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    if (piSIPMsg->GetHeaderCount(ISIPHeader::REFER_TO) != 1)
    {
        GetService()->SendResponse(piSSC,
                SIPStatusCode::SC_400, AString("Mandatory Header Missing"));
        piSSC->Close();
        return IMS_FAILURE;
    }

    AString strReferTo = piSIPMsg->GetHeader(ISIPHeader::REFER_TO);
    SIPAddress objReferTo;

    if (!objReferTo.Create(strReferTo))
    {
        GetService()->SendResponse(piSSC,
                SIPStatusCode::SC_400, AString("Invalid Header Field"));
        piSSC->Close();
        return IMS_FAILURE;
    }

    const SIPParameter *pMethodP = objReferTo.GetParameter(SIP::STR_METHOD);

    if (pMethodP == IMS_NULL)
    {
        GetService()->SendResponse(piSSC,
                SIPStatusCode::SC_400, AString("Mandatory Parameter Missing"));
        piSSC->Close();
        return IMS_FAILURE;
    }

    const ISIPHeader* piReplaces = objReferTo.GetHeader(ISIPHeader::REPLACES);
    Replaces objReplaces;

    if (piReplaces != IMS_NULL)
    {
        objReplaces.Create(piReplaces->ToString());
    }

    Reference *pReference = new Reference(GetService(),
                                objReferTo.GetURI(), pMethodP->GetValue(), objReplaces);

    if (pReference == IMS_NULL)
    {
        piSSC->Close();

        IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pReference->InitMethod(this, IMS_FALSE))
    {
        piSSC->Close();
        delete pReference;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Reference failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pReference->ServerConnection_NotifyRequest(piSSC))
    {
        delete pReference;
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Handling Reference failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    PostMessage(AMSG_SESSION_REFERENCE_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(pReference));

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleResponseToBYE(IN ISIPClientConnection *piSCC)
{
    IMS_SINT32 nOldState = GetState();
    IMS_SINT32 nStatusCode = piSCC->GetMessage()->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Session - Got a %d response to BYE request", nStatusCode, 0, 0);

    if (piSCC == piSCC_BYE)
    {
        if (piSCC_BYE != IMS_NULL)
        {
            piSCC_BYE->Close();
            piSCC_BYE = IMS_NULL;
        }

        SetState(STATE_TERMINATED);

        if (nOldState != STATE_TERMINATED)
        {
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }

        return IMS_SUCCESS;
    }

    UpdateResponseOnReceived(IMessage::SESSION_TERMINATE, piSCC);

    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        return IMS_SUCCESS;
    }
    else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        SetState(STATE_TERMINATED);
    }
    else
    {
        IMS_TRACE_D("%d response received, so WHAT TO DO ?; Just terminates", nStatusCode, 0, 0);

        SetState(STATE_TERMINATED);
    }

    if (nOldState != STATE_TERMINATED)
    {
        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }

    CloseConnection(IMessage::SESSION_TERMINATE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleResponseToCANCEL(IN ISIPClientConnection *piSCC)
{
    IMS_SINT32 nStatusCode = piSCC->GetMessage()->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Session - Got a %d response to CANCEL request", nStatusCode, 0, 0);

    // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
    IMS_SINT32 nMethodForCancel = IMessage::SESSION_TERMINATE;
    IMS_SINT32 nState = GetState();

    if (nState >= STATE_ESTABLISHED)
    {
        if ((nState != STATE_TERMINATING) && (nState != STATE_TERMINATED))
        {
            nMethodForCancel = IMessage::SESSION_CANCEL;
        }
        else
        {
            IMessage *piRequest = GetPreviousRequest(IMessage::SESSION_CANCEL);

            if (piRequest != IMS_NULL)
            {
                IMessage *piResponse = GetPreviousResponse(IMessage::SESSION_CANCEL);

                if (piResponse == IMS_NULL)
                {
                    nMethodForCancel = IMessage::SESSION_CANCEL;
                }
                else if (!SIPStatusCode::IsFinal(piResponse->GetStatusCode()))
                {
                    nMethodForCancel = IMessage::SESSION_CANCEL;
                }
            }
        }
    }

    UpdateResponseOnReceived(nMethodForCancel, piSCC);

    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        return IMS_SUCCESS;
    }
    else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        // The response of CANCEL request needs to be notified to the application
        // regardless of the current session state.
        // if (nMethodForCancel == IMessage::SESSION_CANCEL)
        {
            PostMessage(AMSG_SESSION_CANCEL_DELIVERED, 0, 0);
        }
    }
    else
    {
        // The response of CANCEL request needs to be notified to the application
        // regardless of the current session state.
        // if (nMethodForCancel == IMessage::SESSION_CANCEL)
        {
            PostMessage(AMSG_SESSION_CANCEL_DELIVERY_FAILED, 0, 0);
        }
    }

    CloseConnection(nMethodForCancel);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::HandleResponseToINVITE(IN ISIPClientConnection *piSCC)
{
    ISIPMessage *piSIPMsg = piSCC->GetMessage();
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    if (SIPStatusCode::IsProvisional(nStatusCode)
            || SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        CheckNCreateDialog(piSCC);
    }

    IMS_SINT32 nState = GetState();
    IMS_SINT32 nServiceMethod = IMessage::SERVICEMETHOD_INVALID;

    if (nState == STATE_NEGOTIATING)
        nServiceMethod = IMessage::SESSION_START;
    else if (nState == STATE_RENEGOTIATING)
        nServiceMethod = IMessage::SESSION_UPDATE;
    else if (nState == STATE_TERMINATING)
    {
        IMS_SINT32 nCallState = GetCallState();

        // STATE_IDLE : when receiving non-2xx response, J180 will send ACK automatically.
        if ((nCallState == CallState::STATE_IDLE)
                || (nCallState == CallState::STATE_INVITE_1XX_RECEIVED)
                || (nCallState == CallState::STATE_INVITE_2XX_RECEIVED)
                || (nCallState == CallState::STATE_INVITE_NON2XX_RECEIVED))
        {
            nServiceMethod = IMessage::SESSION_START;
        }
        else
        {
            //4 Check it
            nServiceMethod = IMessage::SESSION_UPDATE;
        }
    }

    // INVITE specific handling
    UpdateResponseOnReceived(nServiceMethod, piSCC);

    // Check the termination pending & do the termination procedure
    if (CheckNTerminateSession(piSIPMsg))
    {
        return IMS_SUCCESS;
    }

    // CALLER_PREFERENCE_MANAGER
    IMessage *piMessage = GetPreviousRequest(nServiceMethod);

    if (piMessage != IMS_NULL)
    {
        UpdateCallerPreference(piMessage->GetMessage(), nStatusCode);
    }

    // Handle 1xx response first...
    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        IMS_RESULT nResult = HandleProvisionalResponse(piSCC);

        if ((nState == STATE_NEGOTIATING)
                && SIPStatusCode::IsProvisional(nStatusCode)
                && (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
        {
            // 'Replaces' header handling ...
            // For explicit call transfer in early dialog state
            AddSessionToCallControlHelperIfNotPresent();
        }

        return nResult;
    }

    // AUTH_SIP_DIGEST {
    // Handle 401/407 response
    if (((nState == STATE_NEGOTIATING) || (nState == STATE_RENEGOTIATING))
            && ((nStatusCode == SIPStatusCode::SC_401)
                || (nStatusCode == SIPStatusCode::SC_407)))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return IMS_SUCCESS;
        }
    }
    // }

    bFlag_UpdateRequestor = IMS_FALSE;

    // Handle a final response according to the status code ...
    switch (nStatusCode)
    {
    case SIPStatusCode::SC_200:
        {
            // Check & create a session descriptor when an initial offer received
            CheckNCreateSessionDescriptor();

            IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSIPMsg);

            if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
                    || (nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
                    || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
            {
                // Update the media state
                if (nState == STATE_NEGOTIATING)
                    UpdateMedia(Media::SESSION_STARTED);
                else if (nState == STATE_RENEGOTIATING)
                    UpdateMedia(Media::SESSION_UPDATED);
                else if (nState == STATE_TERMINATING)
                    UpdateMedia(Media::SESSION_TERMINATED);
                else
                    RestoreOfferAnswerState();

                if (nState == STATE_NEGOTIATING)
                {
                    // 'Replaces' header handling ...
                    if (nOAResult != SdpOfferAnswer::RESULT_NOT_CHANGED)
                    {
                        // Replace the existing session id to new one
                        AddSessionToCallControlHelper();
                    }
                    else
                    {
                        AddSessionToCallControlHelperIfNotPresent();
                    }

                    SetState(STATE_ESTABLISHED);

                    PostMessage(AMSG_SESSION_STARTED, 0, 0);
                    return IMS_SUCCESS;
                }
                else if (nState == STATE_RENEGOTIATING)
                {
                    SetState(STATE_ESTABLISHED);
                    // RACE_CONDITION : SESSION_UPDATE
                    SetSessionUpdateNotificationState(IMS_TRUE);

                    PostMessage(AMSG_SESSION_UPDATED, 0, 0);
                    return IMS_SUCCESS;
                }
                else if (nState == STATE_TERMINATING)
                {
                    SetState(STATE_TERMINATED);
                    CleanupMedia();

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                }

                // Send ACK to 2xx - INVITE
                SendRequestToACK(piSCC, nServiceMethod);

                // Send BYE request if the session is in TERMINATING state
                if (nState == STATE_TERMINATING)
                {
                    //4 Set Reason header
                    SetReasonHeaderFromPreviousRequest(IMessage::SESSION_TERMINATE);
                    SendRequestToBYE();
                }
            }
            else
            {
                // Update the media state
                if (nState == STATE_NEGOTIATING)
                    UpdateMedia(Media::SESSION_START_FAILED);
                else if (nState == STATE_RENEGOTIATING)
                    UpdateMedia(Media::SESSION_UPDATE_FAILED);
                else if (nState == STATE_TERMINATING)
                    UpdateMedia(Media::SESSION_TERMINATED);

                // Send ACK to 2xx - INVITE
                SendRequestToACK(piSCC, nServiceMethod);
                //piSCC->Close();
                //SetInviteConnection(IMS_NULL);

                // This can only happen if the other end sends response to an INVITE/re-INVITE
                // with unsupported media descriptors.
                // This should not happen and if it does there is no easy way for us to signal that
                // we do not accept the response.
                // We simply ignore the response and do not send an ACK which should cause the
                // other side to time out.

                RestoreOfferAnswerState();
                RestoreEx();

                if (nState == STATE_NEGOTIATING)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
                }
                else if (nState == STATE_RENEGOTIATING)
                {
                    SetState(STATE_ESTABLISHED);
                    // RACE_CONDITION : SESSION_UPDATE
                    SetSessionUpdateNotificationState(IMS_TRUE);

                    PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
                }
                else if (nState == STATE_TERMINATING)
                {
                    SetState(STATE_TERMINATED);
                    CleanupMedia();

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                    // Send BYE request if the session is in TERMINATING state
                    //4 Set Reason header
                    SetReasonHeaderFromPreviousRequest(IMessage::SESSION_TERMINATE);
                    SendRequestToBYE();
                }
            }
            break;
        }

    case SIPStatusCode::SC_422:
        if (nState == STATE_NEGOTIATING)
        {
            IMS_BOOL bMinSEPresent = piSIPMsg->IsHeaderPresent(ISIPHeader::MIN_SE);

            CloseConnection(nServiceMethod);

            if (bMinSEPresent)
            {
                SetState(STATE_INITIATED);

                if (SendRequestToINVITEOn422Received() == IMS_SUCCESS)
                {
                    return IMS_SUCCESS;
                }
            }

            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            CloseConnection(nServiceMethod);

            SetState(STATE_ESTABLISHED);

            if (SendRequestToINVITEOn422Received() == IMS_SUCCESS)
            {
                return IMS_SUCCESS;
            }

            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        // A cancelled INVITE
        else if (nState == STATE_TERMINATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        break;

    case SIPStatusCode::SC_486: // Busy here
    case SIPStatusCode::SC_480: // Temporarily unavailable
        // 486 : INVITE is rejected by the other side
        // 480 : INVITE is rejected by the proxy because the remote user is not registered
        if (nState == STATE_NEGOTIATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            UpdateMedia(Media::SESSION_UPDATE_FAILED);

            SetState(STATE_ESTABLISHED);
            // RACE_CONDITION : SESSION_UPDATE
            SetSessionUpdateNotificationState(IMS_TRUE);
            RestoreOfferAnswerState();
            RestoreEx();

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }
        // A cancelled INVITE
        else if (nState == STATE_TERMINATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        break;

    case SIPStatusCode::SC_487: // Request terminated
        // This is the response to our cancelled INVITE, in the terminating session.
        if (nState == STATE_NEGOTIATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            UpdateMedia(Media::SESSION_UPDATE_FAILED);

            SetState(STATE_ESTABLISHED);
            // RACE_CONDITION : SESSION_UPDATE
            SetSessionUpdateNotificationState(IMS_TRUE);
            RestoreOfferAnswerState();
            RestoreEx();

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }
        // A cancelled INVITE
        else if (nState == STATE_TERMINATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        break;

    case SIPStatusCode::SC_488: // Not acceptable here
        // INVITE is rejected because the media negotiation failed
        if (nState == STATE_NEGOTIATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            UpdateMedia(Media::SESSION_UPDATE_FAILED);

            SetState(STATE_ESTABLISHED);
            // RACE_CONDITION : SESSION_UPDATE
            SetSessionUpdateNotificationState(IMS_TRUE);
            RestoreOfferAnswerState();
            RestoreEx();

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }
        // A cancelled INVITE
        else if (nState == STATE_TERMINATING)
        {
            SetState(STATE_TERMINATED);
            CleanupMedia();

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }
        break;

    default:
        {
            if (nState == STATE_NEGOTIATING)
            {
                SetState(STATE_TERMINATED);
                CleanupMedia();

                PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            }
            else if (nState == STATE_RENEGOTIATING)
            {
                UpdateMedia(Media::SESSION_UPDATE_FAILED);

                SetState(STATE_ESTABLISHED);
                // RACE_CONDITION : SESSION_UPDATE
                SetSessionUpdateNotificationState(IMS_TRUE);
                RestoreOfferAnswerState();
                RestoreEx();

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
            // A cancelled INVITE
            else if (nState == STATE_TERMINATING)
            {
                SetState(STATE_TERMINATED);
                CleanupMedia();

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }

            // 2XX response case except for 200
            if (SIPStatusCode::IsFinalSuccess(nStatusCode))
            {
                SendRequestToACK(piSCC, nServiceMethod);
            }
        }
        break;
    }

    CloseConnection(nServiceMethod);

    return IMS_SUCCESS;
}

/*

Remarks
 RACE_CONDITION : SESSION_UPDATE
*/
PRIVATE
IMS_BOOL Session::IsSessionUpdateNotificationInProgress() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_SessionUpdateNotificationInProgress;
}

/*

Remarks
 ACK_RETRANSMISSION_TO_2XX
*/
PRIVATE
void Session::RemoveStrayAcks()
{
    //---------------------------------------------------------------------------------------------

    if (piAckPackage != IMS_NULL)
    {
        piAckPackage->RemoveStrayAcks();
    }
}

/*

Remarks

*/
PRIVATE
SIPMethod Session::SelectUpdateMethod() const
{
    // 1. Select a re-INVITE if a media has been added
    // 2. Select a re-INVITE if an updated application-specific offer
    // 3. Select an UPDATE if any media was removed or direction was changed
    // 4. Select an UPDATE if the session refresh required

    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ESTABLISHED)
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return SIPMethod();
    }

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        const Media *pMedia = objMedias.GetAt(i);

        if ((pMedia->GetState() == Media::STATE_INACTIVE)
                || (pMedia->GetUpdateState() == Media::UPDATE_MODIFIED))
        {
            IMS_TRACE_D("SESSION REFRESH METHOD :: INVITE", 0, 0, 0);
            return SIPMethod(SIPMethod::INVITE);
        }
    }

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        const Media *pMedia = objMedias.GetAt(i);

        if ((pMedia->GetState() == Media::STATE_ACTIVE)
                && (pMedia->GetUpdateState() == Media::UPDATE_REMOVED))
        {
            IMS_TRACE_D("SESSION REFRESH METHOD :: UPDATE", 0, 0, 0);
            return SIPMethod(SIPMethod::UPDATE);
        }
    }

    IMS_TRACE_D("SESSION REFRESH METHOD :: UPDATE (default)", 0, 0, 0);

    return SIPMethod(SIPMethod::UPDATE);
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestForRefresh(IN IMS_SINT32 nMethod /* = SIPMethod::INVALID */)
{
    SIPMethod objMethod(nMethod);
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if (nState == STATE_ESTABLISHED)
    {
        // Send a refresh request : UPDATE or re-INVITE
        if (objMethod.Equals(SIPMethod::INVALID))
        {
            objMethod = pRefreshHelper->GetRefreshMethod();
        }

        if (!objMethod.Equals(SIPMethod::INVITE) && !objMethod.Equals(SIPMethod::UPDATE))
        {
            IMS_TRACE_E(0, "Session refresh method is an invalid", 0, 0, 0);
            return IMS_FAILURE;
        }
    }
    else if (!SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            GetSlotId(), GetService()->GetSIPProfile())
            && ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING)))
    {
        // Send a refresh request : UPDATE
        if (objMethod.Equals(SIPMethod::INVALID))
        {
            objMethod = pRefreshHelper->GetRefreshMethod();
        }

        if (!objMethod.Equals(SIPMethod::UPDATE))
        {
            IMS_TRACE_E(0, "Session refresh method is an invalid", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    ISIPClientConnection *piSCC = CreateConnectionL(GetDialog(), objMethod);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection for a session refresh failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Set the headers for the invitation

    // User-Agent : configuration options ?
    if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSIPProfile()))
    {
        UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                GetService()->GetSIPProfile(), GetService()->GetServiceId(),
                GetService()->GetIPAddress(), GetSlotId(), piSIPMsg);
    }

    // Set SDP message if any offer; According to the configuration options
    if (objMethod.Equals(SIPMethod::INVITE))
    {
        SetSDPBodyPartFromCurrentView(piSIPMsg);
    }

    if (pRefreshHelper->SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        piSCC->Close();

        IMS_TRACE_E(0, "Sending a session refresh request (%s) failed",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    if (objMethod.Equals(SIPMethod::INVITE))
    {
        // Update the call state
        UpdateCallStateOnMessageSent(piSCC->GetMessage());

        // Update the Offer/Answer state
        // UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());
    }

    // ACK_RETRANSMISSION_TO_2XX
    RemoveStrayAcks();

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToACK(IN ISIPClientConnection *piSCC, IN IMS_SINT32 nServiceMethod)
{
    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    if (!GetService()->InitAck(piSCC))
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    if (bFlag_ImplicitRoutingRequired)
    {
        const AStringArray &objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piSCC->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }

    // Set SDP message if any offer
    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // FIX_TIMING_ISSUE_UPDATE_N_INVITE_200OK
    if (!IsEarlyUpdateInProgress())
    {
        CheckNSetSDPBodyPart(piSIPMsg);
    }

    const SipConfigV *pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray &objMethods = pSipConfigV->GetAllowMethods();

        for (IMS_SINT32 i = 0; i < objMethods.GetCount(); ++i)
        {
            if (piSIPMsg->AddHeader(ISIPHeader::ALLOW, objMethods.GetElementAt(i)) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                IMS::SetLastError(IMSError::GENERAL_ERROR);
                return IMS_FAILURE;
            }
        }
    }

    // Sets User-Agent header field
    if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSIPProfile()))
    {
        UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                GetService()->GetSIPProfile(), GetService()->GetServiceId(),
                GetService()->GetIPAddress(), GetSlotId(), piSIPMsg);
    }

    // Add a specific header for ACK request (Require, ...)
    Message *pMessage = GetPreviousRequest(nServiceMethod);

    if (pMessage != IMS_NULL)
    {
        // Sets Require header fields from the previous INVITE request
        IMSList<AString> objRequires = pMessage->GetMessage()->GetHeaders(ISIPHeader::REQUIRE);

        for (IMS_UINT32 i = 0; i < objRequires.GetSize(); ++i)
        {
            piSIPMsg->AddHeader(ISIPHeader::REQUIRE, objRequires.GetAt(i));
        }

        // Sets Proxy-Require header fields from the previous INVITE request
        IMSList<AString> objProxyRequires = pMessage->GetMessage()->GetHeaders(
                ISIPHeader::UNKNOWN, SIPHeaderName::PROXY_REQUIRE);

        for (IMS_UINT32 i = 0; i < objProxyRequires.GetSize(); ++i)
        {
            piSIPMsg->AddHeader(ISIPHeader::UNKNOWN,
                    objProxyRequires.GetAt(i), SIPHeaderName::PROXY_REQUIRE);
        }
    }

    if (!SendNUpdateRequest(IMessage::SESSION_ACK, piSCC))
    {
        // ACK transaction will be closed by the caller
        ClearConnection(IMessage::SESSION_ACK);

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // ACK transaction will be closed by the caller
    ClearConnection(IMessage::SESSION_ACK);

    // Update the call state
    UpdateCallStateOnMessageSent(piSCC->GetMessage());
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());

    // Update the media state
    if (nServiceMethod == IMessage::SESSION_START)
        UpdateMedia(Media::SESSION_STARTED);
    else if (nServiceMethod == IMessage::SESSION_UPDATE)
        UpdateMedia(Media::SESSION_UPDATED);
    else
        RestoreOfferAnswerState();

    // ACK_RETRANSMISSION_TO_2XX
    if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        if (piAckPackage == IMS_NULL)
        {
            piAckPackage = piSCC->GrabAck();
        }
        else
        {
            ISIPAckPackage *piTempPackage = piSCC->GrabAck();

            if ((piTempPackage != IMS_NULL) && (piTempPackage != piAckPackage))
            {
                IMS_TRACE_D("Session :: ACK package is different (fatal error)", 0, 0, 0);
            }
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToBYE()
{
    SIPMethod objMethod(SIPMethod::BYE);
    ISIPClientConnection *piSCC = CreateConnectionL(GetDialog(), objMethod);

    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Set the headers for the session termination
    piSCC->SetExtensionTokenForViaBranch(strTerminationReasonFromApp);

    // Update a session refresh timer info.
    pRefreshHelper->StopSessionTimer(piSCC);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    ISIPMessage* piSIPMsg = piSCC->GetMessage();
    SetSDPBodyPartFromRefusedView(piSIPMsg);

    // Try to send an BYE request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_TERMINATE, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSCC->GetMessage());

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToBYEInternal()
{
    SIPMethod objMethod(SIPMethod::BYE);
    ISIPClientConnection *piSCC = CreateConnectionL(GetDialog(), objMethod);

    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Set the headers for the session termination
    piSCC->SetExtensionTokenForViaBranch(strTerminationReasonFromApp);

    // Update a session refresh timer info.
    pRefreshHelper->StopSessionTimer(piSCC);

    #if 0
    // KR
    {
        if ((nTerminationReason == TERMINATION_REASON_REFRESH_TIMEOUT)
            || (nTerminationReason == TERMINATION_REASON_REFRESH_TXN_TIMEOUT))
        {
            piSCC->AddHeader(SIPHeaderName::REASON,
                    "SIP; cause=103; text=\"Session-Expire\"; fc=9602");
            piSCC->AddHeader("P-SKT-BYE-CAUSE", "no_upd");
        }
        else
        {
            if ((nTerminationReason == TERMINATION_REASON_REFRESH_408)
                || (nTerminationReason == TERMINATION_REASON_REFRESH_481)
                || (nTerminationReason == TERMINATION_REASON_REMOTE_ACTION))
            {
                piSCC->AddHeader(SIPHeaderName::REASON,
                        "ETC; cause=104; text=\"Unknown\"; fc=9999");
            }
            else if (nTerminationReason == TERMINATION_REASON_USER_ACTION)
            {
                piSCC->AddHeader(SIPHeaderName::REASON,
                        "USER; cause=101;text=\"USER triggered\"; fc=9501");
            }
            else
            {
                piSCC->AddHeader(SIPHeaderName::REASON,
                        "ETC; cause=104; text=\"Unknown\"; fc=9999");
            }

            piSCC->AddHeader("P-SKT-BYE-CAUSE", "normal");
        }
    }
    // VZW
    {
        if ((nTerminationReason == TERMINATION_REASON_REFRESH_TIMEOUT)
                || (nTerminationReason == TERMINATION_REASON_REFRESH_TXN_TIMEOUT))
        {
            piSCC->AddHeader(SIPHeaderName::REASON, "USER;text=\"Session Expired\"");
        }
    }
    #endif

    // SIP_MESSAGE_MEDIATOR
    (void) AdjustMessage(piSCC->GetMessage(), MESSAGE_CLASS_INTERNAL_BYE);

    // Try to send an BYE request to the network
    if (piSCC->Send() != IMS_SUCCESS)
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSCC->GetMessage());

    if (piSCC_BYE != IMS_NULL)
    {
        piSCC_BYE->Close();
    }

    piSCC_BYE = piSCC;

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToCANCEL()
{
    //Message *pInviteRequest = GetPreviousRequest(IMessage::SESSION_START);

    //---------------------------------------------------------------------------------------------

    //if (pInviteRequest == IMS_NULL)
    //{
    //    return IMS_FAILURE;
    //}

    ISIPClientConnection *piSCC = IMS_NULL;

    if (GetState() == STATE_RENEGOTIATING)
        piSCC = GetClientConnection(IMessage::SESSION_UPDATE);
    else
        piSCC = GetClientConnection(IMessage::SESSION_START);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't retrieve the previous SIP connection (INVITE)", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPClientConnection *piCancel = CreateCancelConnection(piSCC);

    if (piCancel == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating CANCEL request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
    IMS_SINT32 nMethodForCancel = IMessage::SESSION_TERMINATE;

    if (GetState() >= STATE_ESTABLISHED)
    {
        nMethodForCancel = IMessage::SESSION_CANCEL;
    }

    ISIPMessage *piSIPMsg = piCancel->GetMessage();

    // Sets User-Agent header field
    if (SIPConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSIPProfile()))
    {
        UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                GetService()->GetSIPProfile(), GetService()->GetServiceId(),
                GetService()->GetIPAddress(), GetSlotId(), piSIPMsg);
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    SetSDPBodyPartFromRefusedView(piSIPMsg);

    // Try to send an CANCEL request to the network
    if (!SendNUpdateRequest(nMethodForCancel, piCancel))
    {
        piCancel->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piCancel->GetMessage());

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToINVITE(IN IMS_BOOL bSessionRefresh /* = IMS_FALSE */)
{
    IMS_SINT32 nServiceMethod;
    ISIPClientConnection *piSCC = IMS_NULL;
    SIPMethod objMethod(SIPMethod::INVITE);

    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_INITIATED)
    {
        nServiceMethod = IMessage::SESSION_START;
        piSCC = CreateConnection(objMethod);
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
        piSCC = CreateConnectionL(GetDialog(), objMethod);
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Set the headers for the invitation

    // Min-SE : configuration option
    // Session-Expires : configuration options
    // Supported : configuration options
    if (pRefreshHelper != IMS_NULL)
    {
        if (!SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
        {
            if (GetState() != STATE_INITIATED)
            {
                if (!pRefreshHelper->AddSpecificHeaderWithoutParameterChange(piSCC))
                {
                    piSCC->Close();
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
            else
            {
                if (!pRefreshHelper->AddSpecificHeader(piSCC))
                {
                    piSCC->Close();
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            if (!pRefreshHelper->AddSpecificHeader(piSCC))
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    if (bSessionRefresh)
    {
        // Set SDP message from the current view
        SetSDPBodyPartFromCurrentView(piSIPMsg);
    }
    else
    {
        // Set SDP message if any offer
        CheckNSetSDPBodyPart(piSIPMsg);
    }

    // Update a session refresh timer info.
    if (SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            GetSlotId(), GetService()->GetSIPProfile()))
    {
        if (pRefreshHelper != IMS_NULL)
        {
            pRefreshHelper->StopSessionTimer(piSCC);
        }
    }

    // CALLER_PREFERENCE_MANAGER
    if (GetState() != STATE_INITIATED)
    {
        objPreviousCallerPreference
                = CallerPreferenceManager::GetInstance()->GetAcceptContactsByName(GetName());
    }

    // Try to send an INVITE request to the network
    if (!SendNUpdateRequest(nServiceMethod, piSCC))
    {
        // CALLER_PREFERENCE_MANAGER
        objPreviousCallerPreference.Clear();

        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSCC->GetMessage());
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());

    // Update the media state
    if (GetState() == STATE_INITIATED)
        UpdateMedia(Media::SESSION_START);
    else
        UpdateMedia(Media::SESSION_UPDATE);

    if ((GetState() == STATE_INITIATED) && !objMedias.IsEmpty())
    {
        bFlag_SDPInInitialINVITE = IMS_TRUE;
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToINVITEOn422Received()
{
    IMS_SINT32 nState = GetState();
    IMS_SINT32 nServiceMethod;
    ISIPClientConnection *piSCC = IMS_NULL;
    SIPMethod objMethod(SIPMethod::INVITE);

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("SendRequestToINVITEOn422Received()", 0, 0, 0);

    if (nState == STATE_INITIATED)
    {
        nServiceMethod = IMessage::SESSION_START;
        piSCC = CreateConnection(objMethod);
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
        piSCC = CreateConnectionL(GetDialog(), objMethod);
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMessage *piPreviousRequest = GetPreviousRequest(nServiceMethod);

    if (piPreviousRequest == IMS_NULL)
    {
        piSCC->Close();

        IMS_TRACE_E(0, "Previous request is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Copy all the headers and message bodies from the previous request message
    if (piSIPMsg->CopyHeadersAndBodyParts(piPreviousRequest->GetMessage()) != IMS_SUCCESS)
    {
        piSCC->Close();

        IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Remove the transaction-related headers
    piSIPMsg->RemoveHeader(ISIPHeader::VIA);

    piSIPMsg->RemoveHeader(ISIPHeader::MIN_SE);
    piSIPMsg->RemoveHeader(ISIPHeader::SESSION_EXPIRES);

    // Correct the header value if it is required
    AString strCSeqHdr = piSIPMsg->GetHeader(ISIPHeader::CSEQ);
    ISIPHeader *piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::CSEQ, strCSeqHdr);

    if (piHeader != IMS_NULL)
    {
        IMSList<AString> objTokens = piHeader->GetValue().Split(' ');

        if (!objTokens.IsEmpty())
        {
            // Command sequence number (1st one)
            IMS_SINT32 nCSeqNumber = objTokens.GetAt(0).ToInt32();

            if (nCSeqNumber > 0)
            {
                ++nCSeqNumber;

                AString strNewCSeqHdr;
                strNewCSeqHdr.Sprintf("%d INVITE", nCSeqNumber);

                IMS_TRACE_D("New CSeq header :: %s", strNewCSeqHdr.GetStr(), 0, 0);

                piSIPMsg->SetHeader(ISIPHeader::CSEQ, strNewCSeqHdr);
            }
        }

        piHeader->Destroy();
    }

    // Min-SE : configuration option
    // Session-Expires : configuration options
    // Supported : configuration options
    if (pRefreshHelper != IMS_NULL)
    {
        if (!SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSIPProfile()))
        {
            if (nState != STATE_INITIATED)
            {
                if (!pRefreshHelper->AddSpecificHeaderWithoutParameterChange(piSCC))
                {
                    piSCC->Close();
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
            else
            {
                if (!pRefreshHelper->AddSpecificHeader(piSCC))
                {
                    piSCC->Close();
                    IMS::SetLastError(IMSError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            if (!pRefreshHelper->AddSpecificHeader(piSCC))
            {
                piSCC->Close();
                IMS::SetLastError(IMSError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    // Update a session refresh timer info.
    if (SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            GetSlotId(), GetService()->GetSIPProfile()))
    {
        if (pRefreshHelper != IMS_NULL)
        {
            pRefreshHelper->StopSessionTimer(piSCC);
        }
    }

    // CALLER_PREFERENCE_MANAGER
    if (nState != STATE_INITIATED)
    {
        objPreviousCallerPreference
                = CallerPreferenceManager::GetInstance()->GetAcceptContactsByName(GetName());
    }

    // Try to send an INVITE request to the network
    if (!SendNUpdateRequest(nServiceMethod, piSCC))
    {
        // CALLER_PREFERENCE_MANAGER
        objPreviousCallerPreference.Clear();

        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSCC->GetMessage());
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());

    // Update the media state
    if (nState == STATE_INITIATED)
    {
        UpdateMedia(Media::SESSION_START);
        SetState(STATE_NEGOTIATING);
    }
    else
    {
        UpdateMedia(Media::SESSION_UPDATE);
        SetState(STATE_RENEGOTIATING);
    }

    if ((nState == STATE_INITIATED) && !objMedias.IsEmpty())
    {
        bFlag_SDPInInitialINVITE = IMS_TRUE;
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendRequestToUPDATE(IN IMS_BOOL bSessionRefresh /* = IMS_FALSE */)
{
    SIPMethod objMethod(SIPMethod::UPDATE);
    ISIPClientConnection *piSCC = CreateConnectionL(GetDialog(), objMethod);

    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Set the headers for the invitation

    // Min-SE : configuration option
    // Session-Expires : configuration options
    // Supported : configuration options
    if (pRefreshHelper != IMS_NULL)
    {
        if (!pRefreshHelper->AddSpecificHeader(piSCC))
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (bSessionRefresh)
    {
        // Set SDP message from the current view
        SetSDPBodyPartFromCurrentView(piSIPMsg);
    }
    else
    {
        // Set SDP message if any offer
        CheckNSetSDPBodyPart(piSIPMsg);
    }

    // Update a session refresh timer info.
    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->StopSessionTimer(piSCC);
    }

    // Try to send an UPDATE request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_UPDATE, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());

    // Update the media state
    UpdateMedia(Media::SESSION_UPDATE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Session::SendResponseEx(IN ISIPServerConnection *piSSC, IN IMS_SINT32 nServiceMethod,
        IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // Update a new SIP response message
    if (piSSC->GetMethod().Equals(SIPMethod::INVITE))
    {
        switch (nStatusCode)
        {
        case SIPStatusCode::SC_180:
            break;
        case SIPStatusCode::SC_200:
            break;
        default:
            break;
        }
    }

    if (!SendNUpdateResponse(nServiceMethod, piSSC))
    {
        IMS_TRACE_E(0, "Sending a response failed - SIPError (%d)",
                SIPError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC->GetMessage());

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
void Session::SetReasonHeaderFromPreviousRequest(IN IMS_SINT32 nRequest)
{
    Message *pPrevRequest = GetPreviousRequest(nRequest);

    //---------------------------------------------------------------------------------------------

    if (pPrevRequest == IMS_NULL)
    {
        return;
    }

    Message *pNextRequest = GetNextRequest();

    if (pNextRequest == IMS_NULL)
    {
        return;
    }

    ISIPMessage *piPrevSIPMsg = pPrevRequest->GetMessage();
    ISIPMessage *piNextSIPMsg = pNextRequest->GetMessage();

    if ((piPrevSIPMsg != IMS_NULL) && (piNextSIPMsg != IMS_NULL))
    {
        const AString REASON(SIPHeaderName::REASON);
        AString strReason = piPrevSIPMsg->GetHeader(ISIPHeader::UNKNOWN, 0, REASON);

        if (strReason.GetLength() > 0)
        {
            piNextSIPMsg->SetHeader(ISIPHeader::UNKNOWN, strReason, REASON);
        }

        // KR requirements
        {
            const AString P_SKT_BYE_CAUSE("P-SKT-BYE-CAUSE");
            AString strByeCause = piPrevSIPMsg->GetHeader(
                    ISIPHeader::UNKNOWN, 0, P_SKT_BYE_CAUSE);

            if (strByeCause.GetLength() > 0)
            {
                piNextSIPMsg->SetHeader(ISIPHeader::UNKNOWN, strByeCause, P_SKT_BYE_CAUSE);
            }
        }
    }
}

/*

Remarks
 RACE_CONDITION : SESSION_UPDATE
*/
PRIVATE
void Session::SetSessionUpdateNotificationState(IN IMS_BOOL bInProgress)
{
    //---------------------------------------------------------------------------------------------

    bFlag_SessionUpdateNotificationInProgress = bInProgress;
}

/*

Remarks

*/
PRIVATE
void Session::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Session :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
void Session::Start2xxRetransmission()
{
    //---------------------------------------------------------------------------------------------

#ifndef __IMS_SESSION_RETRY_TASK__
    pRetransmissionHelper = new RetransmissionHelper();

    if (pRetransmissionHelper == IMS_NULL)
    {
        return;
    }

    const ISipConfigV *piSipConfigV = GetService()->GetISipConfigV();
    IMS_SINT32 nTH = SIPConfigProxy::GetTimerValueTH(GetSlotId(),
            GetService()->GetSIPProfile(), piSipConfigV, IMS_FALSE);

    if (nTH > 0)
    {
        pRetransmissionHelper->SetMaxDuration(nTH);
    }
    else
    {
        IMS_SINT32 nTB = SIPConfigProxy::GetTimerValueTB(GetSlotId(),
                GetService()->GetSIPProfile(), piSipConfigV, IMS_FALSE);

        if (nTB > 0)
        {
            pRetransmissionHelper->SetMaxDuration(nTB);
        }
    }

    pRetransmissionHelper->SetListener(this);
    pRetransmissionHelper->Start();
#else
    pRetransmissionTask = new RetryTaskHelper();

    if (pRetransmissionTask == IMS_NULL)
    {
        return;
    }

    pRetransmissionTask->SetListener(this);
    pRetransmissionTask->SetCommand(this);

    RetryTimer *pTimer = new RetryTimer();

    if (pTimer == IMS_NULL)
    {
        return;
    }

    const SIPProfile* pSIPProfile = GetService()->GetSIPProfile();
    const ISipConfigV* piSipConfigV = GetService()->GetISipConfigV();

    IMS_SINT32 nT1 = SIPConfigProxy::GetTimerValueT1(GetSlotId(), pSIPProfile, piSipConfigV);
    IMS_SINT32 nT2 = SIPConfigProxy::GetTimerValueT2(GetSlotId(), pSIPProfile, piSipConfigV);
    IMS_SINT32 nTH = nT1 * 64;

    IMS_SINT32 nTV = nT1;
    IMS_SINT32 nTxnExpires = nTH;
    IMS_SINT32 nMultiplier = 1;

    while (1)
    {
        if (nTV == nT2)
        {
            nTV = nT2;
        }
        else
        {
            nTV = IMS_MIN(nMultiplier * nT1, nT2);
            nMultiplier *= 2;

            // Exceptional case
            if ((nTV <= 0) || (nTV > nTxnExpires))
            {
                IMS_TRACE_D("2XX retransmission timer value fallback :: TV(%d), T2(%d)",
                        nTV, nT2, 0);

                nTV = nT2;
            }
        }

        pTimer->AddValue(nTV);

        nTH -= nTV;

        if (nTH < nT2)
        {
            if (nTH > 0)
            {
                pTimer->AddValue(nTH);
            }
            break;
        }
    }

    pRetransmissionTask->SetTimer(pTimer);

    pRetransmissionTask->Start(RetryTaskHelper::START_TIMER);
#endif // __IMS_SESSION_RETRY_TASK__
}

/*

Remarks

*/
PRIVATE
void Session::Stop2xxRetransmission()
{
    //---------------------------------------------------------------------------------------------

    if (pRetransmissionHelper != IMS_NULL)
    {
        pRetransmissionHelper->Stop();

        delete pRetransmissionHelper;
        pRetransmissionHelper = IMS_NULL;
    }

#ifdef __IMS_SESSION_RETRY_TASK__
    if (pRetransmissionTask != IMS_NULL)
    {
        pRetransmissionTask->Terminate();

        RetryTimer *pTimer = pRetransmissionTask->SetTimer(IMS_NULL);

        if (pTimer != IMS_NULL)
            delete pTimer;

        delete pRetransmissionTask;
        pRetransmissionTask = IMS_NULL;
    }
#endif
}

/*

Remarks

*/
PRIVATE
void Session::TerminateOnNegotiating()
{
    //---------------------------------------------------------------------------------------------

    // Session : MO
    if (IsMobileOriginated())
    {
        IMS_SINT32 nCallState = GetCallState();

        if (nCallState == CallState::STATE_INVITE_SENT)
        {
            // Set the flag and wait & send CANCEL when receiving 1xx response
            bFlag_TerminatePending = IMS_TRUE;

            IMS_TRACE_I("_____ No 1xx response to INVITE received _____", 0, 0, 0);
            return;
        }
        else if (nCallState == CallState::STATE_INVITE_1XX_RECEIVED)
        {
            if (bFlag_TerminateMethodBYE)
            {
                // Send BYE
                if (SendRequestToBYE() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                    IMS_TRACE_E(0, "Sending BYE request failed", 0, 0, 0);
                    return;
                }

                // Abort the current INVITE client connection
                CloseConnection(IMessage::SESSION_START);
            }
            else
            {
                // Send CANCEL
                if (SendRequestToCANCEL() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                    IMS_TRACE_E(0, "Sending CANCEL request failed", 0, 0, 0);
                    return;
                }

                TerminateForkedSessionsOnNegotiating();
            }

            SetState(STATE_TERMINATING);
        }
        else if (nCallState == CallState::STATE_INVITE_2XX_RECEIVED)
        {
            // Send BYE
            if (SendRequestToBYE() != IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                IMS_TRACE_E(0, "Sending BYE request failed", 0, 0, 0);
                return;
            }

            SetState(STATE_TERMINATING);
        }
    }
    // Session : MT
    else
    {
        ISIPServerConnection *piSSC = GetServerConnection(IMessage::SESSION_START);

        if (piSSC == IMS_NULL)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            return;
        }

        if (SendResponseEx(piSSC, IMessage::SESSION_START, SIPStatusCode::SC_486) != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending 486 to INVITE request failed", 0, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

/*

Remarks

*/
PRIVATE
void Session::TerminateOnEstablishing()
{
    //---------------------------------------------------------------------------------------------

    bFlag_TerminatePending = IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void Session::TerminateOnReNegotiating()
{
    //---------------------------------------------------------------------------------------------

    if (bFlag_UpdateRequestor)
    {
        // Abort the ongoing transaction
        CloseConnection(IMessage::SESSION_UPDATE);

        if (SendRequestToBYE() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending BYE request failed", 0, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
    else
    {
        Message *pMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if (pMessage != IMS_NULL)
        {
            ISIPServerConnection *piSSC = GetServerConnection(IMessage::SESSION_UPDATE);

            if (piSSC == IMS_NULL)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                IMS_TRACE_E(0, "Getting SIP server connection failed", 0, 0, 0);
                return;
            }

            if (SendResponseEx(piSSC,
                    IMessage::SESSION_UPDATE, SIPStatusCode::SC_488) != IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                IMS_TRACE_E(0, "Sending 488 to INVITE request failed", 0, 0, 0);
                return;
            }
        }

        if (SendRequestToBYE() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending BYE request failed", 0, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

/*

Remarks

*/
PRIVATE
void Session::TerminateOnReEstablishing()
{
    //---------------------------------------------------------------------------------------------

    bFlag_TerminatePending = IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void Session::TerminateForkedSessionsOnNegotiating()
{
    if (pForkedSessions.IsNull())
    {
        return;
    }

    const IMSList<Method*>& objMethods = pForkedSessions->GetMethods();

    if (objMethods.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); i++)
    {
        Session* pSession = DYNAMIC_CAST(Session*, objMethods.GetAt(i));

        if (pSession != IMS_NULL)
        {
            pSession->TerminateForkedSession();
        }
    }
}

/*

Remarks

*/
PRIVATE
void Session::TerminateForkedSession()
{
    IMS_TRACE_I("TerminateForkedSession :: state=%d", GetState(), 0, 0);

    if (GetState() == STATE_NEGOTIATING)
    {
        SetState(STATE_TERMINATING);
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::AddMedia(IN Media *pMedia)
{
    //---------------------------------------------------------------------------------------------

    if (!objMedias.Append(pMedia))
    {
        IMS_TRACE_E(0, "Appending Media object failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void Session::CleanupMedia()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        Media *pMedia = objMedias.GetAt(i);

        if (pMedia != IMS_NULL)
            pMedia->CleanupMedia();
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::CreateMediaFromSDP()
{
    SessionParameter *pSessionParam = pOAState->GetProposalView();
    IMS_SINT32 nMediaCount = pSessionParam->GetMediaCount();

    //---------------------------------------------------------------------------------------------

    if (nMediaCount != 0)
    {
        // Iterate over all offered media parameters and update and create a media object.
        IMS_UINT32 nMediaIndex = 0;
        IMSList<SdpMediaParameter*> objGroupMediaParams;

        for (IMS_SINT32 i = 0; i < nMediaCount; ++i)
        {
            objGroupMediaParams.Clear();

            // See if the current media parameter is first in a group of parameters that should
            // make up one media object.
            if (pSessionParam->FindGroupStartingWithMediaParameter(i, objGroupMediaParams))
            {
                const SdpMediaParameter *pMediaParam = objGroupMediaParams.GetAt(0);

                switch (pMediaParam->GetMedia().GetTransportProtocol())
                {
                case SdpMedia::TRANSPORT_RTP_AVP:
                case SdpMedia::TRANSPORT_RTP_AVPF:
                case SdpMedia::TRANSPORT_RTP_SAVP:
                case SdpMedia::TRANSPORT_RTP_SAVPF:
                case SdpMedia::TRANSPORT_TCP_MSRP:
                case SdpMedia::TRANSPORT_TCP_TLS_MSRP:
                case SdpMedia::TRANSPORT_UDP:
                case SdpMedia::TRANSPORT_TCP:
                    break;

                default:
                    IMS_TRACE_I("Unsupported media type(%s, %s) received; So, it will be ignored",
                            pMediaParam->GetMedia().GetTypeEx().GetStr(),
                            pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);
                    continue;
                }

                if (nMediaIndex >= objMedias.GetSize())
                {
                    IMSList<IMS_SINT32> objMids;

                    for (IMS_UINT32 j = 0; j < objGroupMediaParams.GetSize(); ++j)
                    {
                        const SdpMediaParameter *pMediaParam = objGroupMediaParams.GetAt(j);

                        IMS_TRACE_I("New media type(%s, %s) added",
                                pMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                        objMids.Append(pMediaParam->GetMid());
                    }

                    // New media; Create and add to the current session.
                    Media *pMedia = MediaFactory::CreateIncomingMedia(
                                        pMediaParam->GetMedia().GetTransportProtocol(),
                                        GetService(), pOAState, objMids);

                    if (pMedia == IMS_NULL)
                        continue;

                    if (!AddMedia(pMedia))
                    {
                        MediaFactory::DestroyMedia(pMedia);
                        continue;
                    }
                }

                ++nMediaIndex;
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::IsMediaInitializationDone() const
{
    //---------------------------------------------------------------------------------------------

    // Check if the created media is initialized or not
    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        const Media *pMedia = objMedias.GetAt(i);

        if (!pMedia->IsInitializationDone())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger)
{
    Media *pMedia;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        pMedia = objMedias.GetAt(i);

        if (pMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting Media object failed", 0, 0, 0);
            continue;
        }

        pMedia->TransitMedia(nTrigger, Media::ANSWER_RECEIVED);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger)
{
    Media *pMedia;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        pMedia = objMedias.GetAt(i);

        if (pMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting Media object failed", 0, 0, 0);
            continue;
        }

        pMedia->TransitMedia(nTrigger, Media::ANSWER_SENT);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger)
{
    SessionParameter *pSessionParam = pOAState->GetProposalView();
    IMS_SINT32 nMediaCount = pSessionParam->GetMediaCount();

    //---------------------------------------------------------------------------------------------

    if (nMediaCount != 0)
    {
        // Iterate over all offered media parameters and update and create a media object.
        IMS_UINT32 nMediaIndex = 0;
        IMSList<SdpMediaParameter*> objGroupMediaParams;

        for (IMS_SINT32 i = 0; i < nMediaCount; ++i)
        {
            objGroupMediaParams.Clear();

            // See if the current media parameter is first in a group of parameters that should
            // make up one media object.
            if (pSessionParam->FindGroupStartingWithMediaParameter(i, objGroupMediaParams))
            {
                SdpMediaParameter *pMediaParam = objGroupMediaParams.GetAt(0);

                switch (pMediaParam->GetMedia().GetTransportProtocol())
                {
                case SdpMedia::TRANSPORT_RTP_AVP:
                case SdpMedia::TRANSPORT_RTP_AVPF:
                case SdpMedia::TRANSPORT_RTP_SAVP:
                case SdpMedia::TRANSPORT_RTP_SAVPF:
                case SdpMedia::TRANSPORT_UDP_TLS_RTP_SAVP:
                case SdpMedia::TRANSPORT_TCP_MSRP:
                case SdpMedia::TRANSPORT_TCP_TLS_MSRP:
                case SdpMedia::TRANSPORT_UDP:
                case SdpMedia::TRANSPORT_TCP:
                    break;

                default:
                    IMS_TRACE_I("Unsupported media type(%s, %s) received; So, it will be ignored",
                            pMediaParam->GetMedia().GetTypeEx().GetStr(),
                            pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                    pMediaParam->MarkRejectedOrRemoved();
                    continue;
                }

                if (nMediaIndex >= objMedias.GetSize())
                {
                    IMSList<IMS_SINT32> objMids;

                    for (IMS_UINT32 j = 0; j < objGroupMediaParams.GetSize(); ++j)
                    {
                        const SdpMediaParameter *pMediaParam = objGroupMediaParams.GetAt(j);

                        IMS_TRACE_I("New media type(%s, %s) added",
                                pMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                        objMids.Append(pMediaParam->GetMid());
                    }

                    // New media; Create and add to the current session.
                    Media *pMedia = MediaFactory::CreateIncomingMedia(
                                        pMediaParam->GetMedia().GetTransportProtocol(),
                                        GetService(), pOAState, objMids);

                    if (pMedia == IMS_NULL)
                        continue;

                    pMedia->TransitMedia(nTrigger, Media::OFFER_RECEIVED);

                    if (!AddMedia(pMedia))
                    {
                        MediaFactory::DestroyMedia(pMedia);
                        continue;
                    }
                }
                else
                {
                    // Existing media; check if the SDP has been changed
                    Media *pMedia = objMedias.GetAt(nMediaIndex);

                    pMedia->TransitMedia(nTrigger, Media::OFFER_RECEIVED);
                }

                ++nMediaIndex;
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Session::UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger)
{
    Media *pMedia;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        pMedia = objMedias.GetAt(i);

        if (pMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting Media object failed", 0, 0, 0);
            continue;
        }

        pMedia->TransitMedia(nTrigger, Media::OFFER_SENT);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* Session::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
    case STATE_CREATED:
        return "STATE_CREATED";
    case STATE_INITIATED:
        return "STATE_INITIATED";
    case STATE_NEGOTIATING:
        return "STATE_NEGOTIATING";
    case STATE_ESTABLISHING:
        return "STATE_ESTABLISHING";
    case STATE_ESTABLISHED:
        return "STATE_ESTABLISHED";
    case STATE_RENEGOTIATING:
        return "STATE_RENEGOTIATING";
    case STATE_REESTABLISHING:
        return "STATE_REESTABLISHING";
    case STATE_TERMINATING:
        return "STATE_TERMINATING";
    case STATE_TERMINATED:
        return "STATE_TERMINATED";
    default:
        return "__INVALID__";
    }
}
