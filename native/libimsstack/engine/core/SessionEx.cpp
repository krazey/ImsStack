/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100615  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "ServiceSystemTime.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipClientConnection.h"
#include "ISipServerConnection.h"
#include "ISipDialog.h"
#include "Sip.h"
#include "SipStatusCode.h"
#include "SipHeaderUtils.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "base/Ims.h"
#include "Service.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "SdpOaState.h"
#include "media/Media.h"
#include "SessionRefreshHelper.h"
#include "ReliableProvResponseHelper.h"
#include "IOnSessionExListener.h"
#include "SessionEx.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SessionEx::SessionEx(IN Service* pService_) :
        Session(pService_),
        nEarlyState(EARLY_STATE_IDLE),
        pRPRHelper(IMS_NULL),
        piListener(IMS_NULL),
        bFlag_EarlyUpdateNotificationInProgress(IMS_FALSE),
        piSSC_PendingUpdate(IMS_NULL),
        piTimer_PendingUpdate(IMS_NULL),
        nLastEarlyUpdateCompletedTimeSec(0),
        nLastEarlyUpdateCompletedTimeMicroSec(0)
{
}

PUBLIC VIRTUAL SessionEx::~SessionEx()
{
    DestroyRPRHelper();

    StopPendingUpdateTimer();

    if (piSSC_PendingUpdate != IMS_NULL)
    {
        piSSC_PendingUpdate->Close();
    }
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SessionEx::RespondToEarlyUpdate(
        IN IMS_SINT32 nStatusCode, IN CONST AString& strReason /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (nEarlyState != EARLY_STATE_UPDATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Early UPDATE :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSSC = GetServerConnection(IMessage::SESSION_EARLY_UPDATE);

    if (piSSC == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    SessionRefreshHelper* pRefreshHelper = GetRefreshHelper();
    IMS_BOOL bTimerOptionSupported = IMS_FALSE;

    if (pRefreshHelper != IMS_NULL)
    {
        bTimerOptionSupported = pRefreshHelper->IsSessionTimerSupported(
                piSSC, pRefreshHelper->IsSessionTimerSupportedBySessionExpires());
    }

    if (CreateResponse(piSSC, nStatusCode, strReason) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS_SINT32 nState = GetState();
        IMS_BOOL bSessionTimerControlRequired = IMS_FALSE;
        AString strRequestSE(AString::ConstNull());

        if ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING))
        {
            bSessionTimerControlRequired = IMS_TRUE;
        }
        else
        {
            Message* pMessage = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
            ISipMessage* piSIPMsg = (pMessage != IMS_NULL) ? pMessage->GetMessage() : IMS_NULL;

            bSessionTimerControlRequired = (piSIPMsg != IMS_NULL)
                    ? piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES)
                    : IMS_FALSE;

            if (bSessionTimerControlRequired && (piSIPMsg != IMS_NULL))
            {
                strRequestSE = (piSIPMsg != IMS_NULL)
                        ? piSIPMsg->GetHeader(ISipHeader::SESSION_EXPIRES)
                        : AString::ConstNull();
            }
        }

        if (bSessionTimerControlRequired)
        {
            if ((pRefreshHelper != IMS_NULL) &&
                    !pRefreshHelper->AddSpecificHeaderOnEarlyUPDATE(piSSC, bTimerOptionSupported))
            {
                IMS_TRACE_E(0, "Adding the session refresh specific headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }

            if ((nState != STATE_RENEGOTIATING) && (nState != STATE_REESTABLISHING))
            {
                AString strResponseSE = piSSC->GetMessage()->GetHeader(ISipHeader::SESSION_EXPIRES);
                AString strNewSE = AdjustSessionExpiresHeader(strRequestSE, strResponseSE);

                if (strNewSE.GetLength() > 0)
                {
                    piSSC->GetMessage()->SetHeader(ISipHeader::SESSION_EXPIRES, strNewSE);
                    IMS_TRACE_D("Session-Expires is changed; %s >> %s", strResponseSE.GetStr(),
                            strNewSE.GetStr(), 0);
                }
            }
        }
    }

    ISipMessage* piSIPMsg = piSSC->GetMessage();

    // If the UAS receives a UPDATE with an offer,
    // it MUST place the answer in the 2xx to the UPDATE
    IMS_BOOL bHasSDPAnswer = IMS_FALSE;
    IMS_BOOL bIsEarlyUpdateRejected = IMS_FALSE;

    if (SipStatusCode::IsFinal(nStatusCode) &&
            GetOfferAnswerState() == SdpOaState::STATE_OFFER_CHANGE_RECEIVED)
    {
        IMS_BOOL bHasSDPInRequest = IMS_FALSE;

        Message* pMessage = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

        if (pMessage != IMS_NULL)
        {
            ISipMessage* piSIPMsg_UPDATE = pMessage->GetMessage();

            if (piSIPMsg_UPDATE->GetSdpBodyPart() != IMS_NULL)
            {
                bHasSDPInRequest = IMS_TRUE;
            }
        }

        if (bHasSDPInRequest)
        {
            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                bHasSDPAnswer = IMS_TRUE;
            }
            else
            {
                bIsEarlyUpdateRejected = IMS_TRUE;
            }
        }
    }

    if (bHasSDPAnswer)
    {
        CheckNSetSDPBodyPart(piSIPMsg);
    }

    if (!SendNUpdateResponse(IMessage::SESSION_EARLY_UPDATE, piSSC))
    {
        // Restore Offer/Answer state
        if (bHasSDPAnswer || bIsEarlyUpdateRejected)
        {
            RestoreOfferAnswerState();
            RestoreEx();
        }

        CloseConnection(IMessage::SESSION_EARLY_UPDATE);

        SetEarlyState(EARLY_STATE_IDLE);

        return IMS_FAILURE;
    }

    if (bHasSDPAnswer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }
    else if (bIsEarlyUpdateRejected)
    {
        // Restore Offer/Answer state
        RestoreOfferAnswerState();
        RestoreEx();
    }

    SetEarlyState(EARLY_STATE_IDLE);

    CloseConnection(IMessage::SESSION_EARLY_UPDATE);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SessionEx::RespondToPRAck(
        IN IMS_SINT32 nStatusCode, IN CONST AString& strReason /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (pRPRHelper->GetState() != ReliableProvResponseHelper::STATE_PRACK_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSSC = GetServerConnection(IMessage::SESSION_PRACK);

    if (piSSC == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    if (CreateResponse(piSSC, nStatusCode, strReason) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    // If the UAS receives a PRACK with an offer, it MUST place the answer in the 2xx to the PRACK
    IMS_BOOL bHasSDPAnswer = IMS_FALSE;
    IMS_BOOL bIsPRAckRejected = IMS_FALSE;

    if (SipStatusCode::IsFinal(nStatusCode) &&
            GetOfferAnswerState() == SdpOaState::STATE_OFFER_CHANGE_RECEIVED)
    {
        IMS_BOOL bHasSDPInRequest = IMS_FALSE;
        Message* pMessage = GetPreviousRequest(IMessage::SESSION_PRACK);

        if (pMessage != IMS_NULL)
        {
            ISipMessage* piSIPMsg_PRAck = pMessage->GetMessage();

            if (piSIPMsg_PRAck->GetSdpBodyPart() != IMS_NULL)
            {
                bHasSDPInRequest = IMS_TRUE;
            }
        }

        if (bHasSDPInRequest)
        {
            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                bHasSDPAnswer = IMS_TRUE;
            }
            else
            {
                bIsPRAckRejected = IMS_TRUE;
            }
        }
    }

    // Set SDP message if any offer ;; According to the configuration options
    if (bHasSDPAnswer)
    {
        ISipMessage* piSIPMsg = piSSC->GetMessage();

        CheckNSetSDPBodyPart(piSIPMsg);
    }

    if (!SendNUpdateResponse(IMessage::SESSION_PRACK, piSSC))
    {
        // Restore Offer/Answer state
        if (bHasSDPAnswer || bIsPRAckRejected)
        {
            RestoreOfferAnswerState();
            RestoreEx();
        }

        CloseConnection(IMessage::SESSION_PRACK);
        pRPRHelper->UpdateOnOperationFailed();
        return IMS_FAILURE;
    }

    if (bHasSDPAnswer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }
    else if (bIsPRAckRejected)
    {
        // Restore Offer/Answer state
        RestoreOfferAnswerState();
        RestoreEx();
    }

    pRPRHelper->UpdateOnMessageSent(piSSC->GetMessage());

    CloseConnection(IMessage::SESSION_PRACK);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SessionEx::SendPRAck()
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (pRPRHelper->GetState() != ReliableProvResponseHelper::STATE_RPR_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    SipMethod objMethod(SipMethod::PRACK);
    ISipClientConnection* piSCC = CreateConnectionL(piDialog, objMethod);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Set headers
    pRPRHelper->SetRAckHeader(piSIPMsg);

    VirtualSession* pEarlySession = GetVirtualEarlySession();

    if (pEarlySession != IMS_NULL)
    {
        IMS_TRACE_I("PRACK is sent via virtual early session.", 0, 0, 0);

        IMS_BOOL bHasSDPAnswer = pEarlySession->CheckNSetSDPBodyPart(piSIPMsg);

        // Try to send a PRACK request to the network
        if (!SendNUpdateRequest(IMessage::SESSION_PRACK, piSCC))
        {
            piSCC->Close();
            return IMS_FAILURE;
        }

        piSIPMsg = piSCC->GetMessage();

        if (bHasSDPAnswer)
        {
            pEarlySession->NotifyPRAckSent(piSIPMsg);
        }

        pRPRHelper->UpdateOnMessageSent(piSIPMsg);

        Ims::SetLastError(ImsError::NO_ERROR);

        return IMS_SUCCESS;
    }

    // If the UAC receives a reliable provisional response with an offer
    // (this would occur if the UAC sent an INVITE without an offer,
    // in which case the first reliable provisional response will contain the offer),
    // it MUST generate an answer in the PRACK.
    IMS_BOOL bHasSDPAnswer = IMS_FALSE;
    IMS_SINT32 nOAState = GetOfferAnswerState();

    if ((nOAState == SdpOaState::STATE_OFFER_RECEIVED) ||
            (nOAState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        bHasSDPAnswer = IMS_TRUE;

        // Checks if the early UPDATE is already received when sending PRACK request
        IMessage* piRequest = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
        IMessage* piResponse = GetPreviousResponse(IMessage::SESSION_EARLY_UPDATE);

        if ((piRequest != IMS_NULL) && (piResponse == IMS_NULL))
        {
            if (piRequest->GetState() == IMessage::STATE_RECEIVED)
            {
                ISipMessage* piSIPMsg_UPDATE = piRequest->GetMessage();

                if (piSIPMsg_UPDATE->GetSdpBodyPart() != IMS_NULL)
                {
                    // New SDP offer is made by the UPDATE request
                    bHasSDPAnswer = IMS_FALSE;
                }
            }
        }
    }

    // Set SDP message if any offer ;; According to the configuration options
    if (bHasSDPAnswer)
    {
        CheckNSetSDPBodyPart(piSIPMsg);
    }

    // Try to send a PRACK request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_PRACK, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the media information
    if (bHasSDPAnswer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }

    pRPRHelper->UpdateOnMessageSent(piSCC->GetMessage());

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SessionEx::SendRPR(IN IMS_SINT32 nStatusCode,
        IN CONST AString& strReason /* = AString::ConstNull() */, IN IMS_BOOL bSDP /* = IMS_TRUE */,
        IN IMS_SINT32 nFlags /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (pRPRHelper->GetState() != ReliableProvResponseHelper::STATE_IDLE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!SipStatusCode::IsProvisional(nStatusCode))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = GetState();

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
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

    Message* pMessage = GetPreviousRequest(nServiceMethod);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    ISipMessage* piSIPMsg = pMessage->GetMessage();

    // Do nothing if the handling method is not INVITE
    if (!piSIPMsg->GetMethod().Equals(SipMethod::INVITE))
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSSC = GetServerConnection(nServiceMethod);

    if (piSSC == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIP server connection is null", 0, 0, 0);
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ RPR :: SENDING %d \"%s\" ___", nStatusCode,
            strReason.IsNULL() ? SipStatusCode::GetReasonPhrase(nStatusCode) : strReason.GetStr(),
            0);

    if (CreateResponse(piSSC, nStatusCode, strReason) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SipError (%d)", SipError::GetLastError(),
                0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSIPMsg_RPR = piSSC->GetMessage();

    // Check & set the Require ('100rel') & RSeq header
    if (piSIPMsg_RPR->PrependHeader(ISipHeader::REQUIRE, "100rel") != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Adding Require header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pRPRHelper->SetRSeqHeader(piSIPMsg_RPR))
    {
        IMS_TRACE_E(0, "Adding RSeq header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Set SDP body if the condition meets...
    if (bSDP)
    {
        CheckNSetSDPBodyPart(piSIPMsg_RPR);
    }

    // REMOVE_RECORD_ROUTE_HEADERS
    if ((nFlags & FLAG_REMOVE_RECORD_ROUTES) != 0)
    {
        RemoveRecordRouteHeaders(piSSC->GetMessage());
    }

    if (!SendNUpdateResponse(nServiceMethod, piSSC))
    {
        IMS_TRACE_E(0, "Sending a response failed - SipError (%d)", SipError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // Check & create a SIP dialog
    CheckNCreateDialog(piSSC);

    // Update the call state
    UpdateCallStateOnMessageSent(piSSC->GetMessage());

    if (bSDP)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);

        if ((nState == STATE_NEGOTIATING) &&
                (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
        {
            AddSessionToCallControlHelperIfNotPresent();
        }
    }

    pRPRHelper->UpdateOnMessageSent(piSSC->GetMessage());

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SessionEx::UpdateEarlyMedia()
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Early UPDATE :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (piSSC_PendingUpdate != IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Early UPDATE :: PendingUpdate", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetLastEarlyUpdateCompletedTime(0);

    if (nState == STATE_RENEGOTIATING)
    {
        if (GetPreviousRequest(IMessage::SESSION_UPDATE) != IMS_NULL)
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);

            IMS_TRACE_E(0, "Early UPDATE :: invalid state", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (nEarlyState != EARLY_STATE_IDLE)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);

        IMS_TRACE_E(0, "Early UPDATE :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    SipMethod objMethod(SipMethod::UPDATE);
    ISipClientConnection* piSCC = CreateConnectionL(GetDialog(), objMethod);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (nState == STATE_RENEGOTIATING)
    {
        if (!AddRefreshSpecificHeaders(piSCC))
        {
            IMS_TRACE_E(0, "Adding the session refresh specific headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Set the headers

    IMS_BOOL bHasSDPOffer = IMS_FALSE;

    if (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED)
    {
        bHasSDPOffer = IMS_TRUE;
    }

    // Set SDP message if any offer ;; According to the configuration options
    if (bHasSDPOffer)
    {
        CheckNSetSDPBodyPart(piSIPMsg);
    }

    // Try to send an UPDATE request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_EARLY_UPDATE, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    if (bHasSDPOffer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSCC->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }

    SetEarlyState(EARLY_STATE_UPDATE_SENT);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL void SessionEx::SetExListener(IN IOnSessionExListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL SessionEx::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_SESSIONEX_EARLY_MEDIA_UPDATED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_EarlyMediaUpdated(this);
            }

            SetEarlyUpdateNotificationState(IMS_FALSE);
            return IMS_TRUE;

        case AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_EarlyMediaUpdateFailed(this);
            }

            SetEarlyUpdateNotificationState(IMS_FALSE);
            return IMS_TRUE;

        case AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_RECEIVED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_EarlyMediaUpdateReceived(this);
            }
            return IMS_TRUE;

        case AMSG_SESSIONEX_PRACK_DELIVERED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_PRAckDelivered(this);
            }
            return IMS_TRUE;

        case AMSG_SESSIONEX_PRACK_DELIVERY_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_PRAckDeliveryFailed(this);
            }
            return IMS_TRUE;

        case AMSG_SESSIONEX_PRACK_RECEIVED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_PRAckReceived(this);
            }
            return IMS_TRUE;

        case AMSG_SESSIONEX_RPR_DELIVERY_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_RPRDeliveryFailed(this);
            }
            return IMS_TRUE;

        case AMSG_SESSIONEX_RPR_RECEIVED:
            if (piListener != IMS_NULL)
            {
                piListener->OnSessionEx_RPRReceived(
                        this, reinterpret_cast<VirtualSession*>(objMSG.nWparam), objMSG.nLparam);
            }
            return IMS_TRUE;

        case AMSG_SESSION_STARTED:
        case AMSG_SESSION_UPDATED:
            SetLastEarlyUpdateCompletedTime(0);
            break;

        default:
            break;
    }

    return Session::DispatchMessage(objMSG);
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL SessionEx::NotifySIPRequest(IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    // Incoming INVITE request only
    if (!CheckNCreateRPRHelper(piSSC->GetMessage()))
    {
        return IMS_FALSE;
    }

    return Session::NotifySIPRequest(piSSC);
}

/*

Remarks

*/
PROTECTED VIRTUAL void SessionEx::NotifySIPResponse(IN ISipClientConnection* piSCC)
{
    IMS_SINT32 nSIPMethod = piSCC->GetMethod().ToInt();

    //---------------------------------------------------------------------------------------------

    // If a final response to INVITE request is received, then abort PRACK transaction.
    if ((nSIPMethod == SipMethod::INVITE) && SipStatusCode::IsFinal(piSCC->GetStatusCode()))
    {
        if ((pRPRHelper != IMS_NULL) &&
                (pRPRHelper->GetState() == ReliableProvResponseHelper::STATE_PRACK_SENT))
        {
            pRPRHelper->UpdateOnOperationFailed();
            CloseConnection(IMessage::SESSION_PRACK);

            if (SipStatusCode::IsFinalSuccess(piSCC->GetStatusCode()))
            {
                PostMessage(AMSG_SESSIONEX_PRACK_DELIVERED, 0, 0);
            }
        }
    }

    Session::NotifySIPResponse(piSCC);

    switch (nSIPMethod)
    {
        case SipMethod::INVITE:
            if (GetState() == STATE_ESTABLISHED)
            {
                DestroyRPRHelper();
            }
            break;

        case SipMethod::PRACK:
            if (piSCC == GetClientConnection(IMessage::SESSION_PRACK))
            {
                HandleResponseToPRACK(piSCC);
            }
            else
            {
                IMS_TRACE_E(0, "%s is not handled", piSCC->GetMethod().ToString().GetStr(), 0, 0);

                piSCC->Close();
            }
            break;

        case SipMethod::UPDATE:
            if (piSCC == GetClientConnection(IMessage::SESSION_EARLY_UPDATE))
            {
                HandleResponseToUPDATE(piSCC);
            }
            else
            {
                // UPDATE response was already handled by Session.
            }
            break;

        default:
            break;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void SessionEx::NotifySIPError(
        IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    IMS_SINT32 nSIPMethod = piSC->GetMethod().ToInt();

    //---------------------------------------------------------------------------------------------

    if ((nSIPMethod == SipMethod::INVITE) && (GetState() == STATE_NEGOTIATING) &&
            (pRPRHelper != IMS_NULL) &&
            (pRPRHelper->GetState() == ReliableProvResponseHelper::STATE_RPR_SENT))
    {
        IMS_TRACE_D("SessionEx :: RPR transaction timer is expired", 0, 0, 0);

        if (!IsTerminatePending())
        {
            PostMessage(AMSG_SESSIONEX_RPR_DELIVERY_FAILED, 0, 0);
        }
        else
        {
            // FIXME: select a proper status code
            RejectEx(SipStatusCode::SC_580);
        }
        return;
    }

    Session::NotifySIPError(piSC, nCode, strMessage);

    switch (nSIPMethod)
    {
        case SipMethod::PRACK:
            if ((pRPRHelper != IMS_NULL) &&
                    (pRPRHelper->GetState() == ReliableProvResponseHelper::STATE_PRACK_SENT))
            {
                pRPRHelper->UpdateOnOperationFailed();
                CloseConnection(IMessage::SESSION_PRACK);

                PostMessage(AMSG_SESSIONEX_PRACK_DELIVERY_FAILED, 0, 0);
            }
            break;

        case SipMethod::UPDATE:
            if (nEarlyState == EARLY_STATE_UPDATE_SENT)
            {
                SetEarlyState(EARLY_STATE_IDLE);
                SetEarlyUpdateNotificationState(IMS_TRUE);
                SetLastEarlyUpdateCompletedTime(0);
                CloseConnection(IMessage::SESSION_EARLY_UPDATE);

                PostMessage(AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_FAILED, 0, 0);
            }
            break;

        default:
            break;
    }
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PROTECTED VIRTUAL IMS_BOOL SessionEx::SendRequestToChallenge(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSCC->GetMethod();

    if (objMethod.Equals(SipMethod::PRACK) && (pRPRHelper != IMS_NULL))
    {
        if (pRPRHelper->GetState() != ReliableProvResponseHelper::STATE_PRACK_SENT)
        {
            IMS_TRACE_E(0, "PRACK state mismatch", 0, 0, 0);
            return IMS_FALSE;
        }

        // Clear the connection to preserve the SIP connection
        ClearConnection(IMessage::SESSION_PRACK);

        // Try to send a PRACK request to the network
        if (!SendNUpdateRequestEx(IMessage::SESSION_PRACK, piSCC, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(IMessage::SESSION_PRACK, piSCC);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }
    else if (objMethod.Equals(SipMethod::UPDATE) && (nEarlyState != EARLY_STATE_UPDATE_SENT))
    {
        // Clear the connection to preserve the SIP connection
        ClearConnection(IMessage::SESSION_EARLY_UPDATE);

        // Try to send an UPDATE request to the network
        if (!SendNUpdateRequestEx(IMessage::SESSION_EARLY_UPDATE, piSCC, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(IMessage::SESSION_EARLY_UPDATE, piSCC);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    return Session::SendRequestToChallenge(piSCC);
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL SessionEx::Dialog_NotifyRequest(IN ISipServerConnection* piSSC)
{
    IMS_SINT32 nSIPMethod = piSSC->GetMethod().ToInt();
    ISipMessage* piSIPMsg = piSSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (nSIPMethod == SipMethod::INVITE)
    {
        // Incoming INVITE request only
        if (!CheckNCreateRPRHelper(piSIPMsg))
        {
            return IMS_FALSE;
        }
    }

    // RACE_CONDITION: 200 OK to UPDATE and incoming UPDATE
    if (nSIPMethod == SipMethod::UPDATE)
    {
        if (piSSC_PendingUpdate != IMS_NULL)
        {
            GetService()->SendResponse(piSSC, SipStatusCode::SC_500);
            piSSC->Close();
            return IMS_TRUE;
        }

        IMS_BOOL bPendingUpdateRequired = IMS_FALSE;

        if (piSIPMsg->GetSdpBodyPart() != IMS_NULL)
        {
            if (IsEarlyUpdateNotificationInProgress())
            {
                bPendingUpdateRequired = IMS_TRUE;
            }
            else if (IsIncomingEarlyUpdateReceivedInShortTime())
            {
                bPendingUpdateRequired = IMS_TRUE;
            }
        }

        SetLastEarlyUpdateCompletedTime(0);

        if (bPendingUpdateRequired)
        {
            IMS_SINT32 nCallState = GetCallState();

            if ((nCallState == CallState::STATE_INVITE_1XX_RECEIVED) ||
                    (nCallState == CallState::STATE_INVITE_1XX_SENT) ||
                    (nCallState == CallState::STATE_REINVITE_1XX_RECEIVED) ||
                    (nCallState == CallState::STATE_REINVITE_1XX_SENT))
            {
                piSSC_PendingUpdate = piSSC;

                if (!StartPendingUpdateTimer())
                {
                    GetService()->SendResponse(piSSC, SipStatusCode::SC_491);
                    piSSC->Close();

                    piSSC_PendingUpdate = IMS_NULL;
                }
                return IMS_TRUE;
            }
        }
    }

    if (!Session::Dialog_NotifyRequest(piSSC))
    {
        return IMS_FALSE;
    }

    switch (nSIPMethod)
    {
        case SipMethod::ACK:
            if (GetState() == STATE_ESTABLISHED)
            {
                DestroyRPRHelper();
            }
            break;

        case SipMethod::PRACK:
            HandleRequestToPRACK(piSSC);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL Session* SessionEx::CreateSession()
{
    SessionEx* pSession = new SessionEx(GetService());

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
PROTECTED VIRTUAL IMS_RESULT SessionEx::HandleProvisionalResponse(IN ISipClientConnection* piSCC)
{
    ISipMessage* piSIPMsg = piSCC->GetMessage();

    //---------------------------------------------------------------------------------------------

    // Check if the message is a reliable provisional response or not
    if (!piSIPMsg->IsMessageRpr())
    {
        return Session::HandleProvisionalResponse(piSCC);
    }

    if (!CheckNCreateRPRHelper(piSIPMsg))
    {
        return IMS_FAILURE;
    }

    pRPRHelper->UpdateOnMessageReceived(piSIPMsg);

    VirtualSession* pEarlySession = GetVirtualEarlySession();

    if (pEarlySession == IMS_NULL)
    {
        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSIPMsg);

        if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOAResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
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

    PostMessage(AMSG_SESSIONEX_RPR_RECEIVED, reinterpret_cast<IMS_UINTP>(pEarlySession),
            objResponses.GetSize() - 1);

    if (piSIPMsg->GetStatusCode() == SipStatusCode::SC_180)
    {
        NotifyAlerting();
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_RESULT SessionEx::HandleRequestToUPDATE(IN ISipServerConnection* piSSC)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
    {
        IMS_TRACE_D("UPDATE is received in TERMINATING or TERMINATED state (%d)", nState, 0, 0);

        GetService()->SendResponse(piSSC, SipStatusCode::SC_488);
        piSSC->Close();
        return IMS_SUCCESS;
    }

    IMS_SINT32 nStatusCode = SipStatusCode::SC_INVALID;
    IMS_SINT32 nOfferAnswerState = GetOfferAnswerState();
    ISipMessage* piSIPMsg = piSSC->GetMessage();

    // If we have received an UPDATE request when we have already sent an UPDATE,
    // the received UPDATE should be rejected with 491 Request Pending response.
    if ((nOfferAnswerState == SdpOaState::STATE_OFFER_SENT) ||
            (nOfferAnswerState == SdpOaState::STATE_OFFER_CHANGE_SENT))
    {
        if ((piSIPMsg != IMS_NULL) && (piSIPMsg->GetSdpBodyPart() != IMS_NULL))
        {
            nStatusCode = SipStatusCode::SC_491;
        }
    }

    // If we have received an UPDATE request when we are already processing another
    // UPDATE, the 2nd UPDATE should be rejected with 500 Server Internal Error response.
    else if ((nOfferAnswerState == SdpOaState::STATE_OFFER_RECEIVED) ||
            (nOfferAnswerState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        if ((piSIPMsg != IMS_NULL) && (piSIPMsg->GetSdpBodyPart() != IMS_NULL))
        {
            nStatusCode = SipStatusCode::SC_500;
        }
    }
    else
    {
        if (nEarlyState == EARLY_STATE_UPDATE_RECEIVED)
        {
            nStatusCode = SipStatusCode::SC_500;
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

            if (piMessage != IMS_NULL)
            {
                if (piMessage->GetMethod().Equals(SipMethod::UPDATE) &&
                        (piMessage->GetState() == IMessage::STATE_RECEIVED))
                {
                    nStatusCode = SipStatusCode::SC_500;
                }
            }
        }
        // 140818, CONDITION_ACK_WAITING_STATE
        else if (nState == STATE_REESTABLISHING)
        {
            if ((piSIPMsg != IMS_NULL) && (piSIPMsg->GetSdpBodyPart() != IMS_NULL))
            {
                IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

                // re-INVITE only case
                if (piMessage != IMS_NULL)
                {
                    if (piMessage->GetState() == IMessage::STATE_RECEIVED)
                    {
                        nStatusCode = SipStatusCode::SC_500;
                    }
                    else
                    {
                        nStatusCode = SipStatusCode::SC_491;
                    }
                }
            }
        }
    }

    if (nStatusCode != SipStatusCode::SC_INVALID)
    {
        IMS_TRACE_I("Rejecting an UPDATE with %d ... in SessionEx", nStatusCode, 0, 0);

        if (GetService()->CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
        {
            piSSC->Close();

            IMS_TRACE_E(0, "Rejecting an early UPDATE failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Sets Retry-After header field
        AString strRAHdr;
        strRAHdr.SetNumber(SipHeaderUtils::GenerateRetryAfterSeconds(10));

        if (piSSC->GetMessage()->SetHeader(ISipHeader::RETRY_AFTER_SEC, strRAHdr) != IMS_SUCCESS)
        {
            piSSC->Close();

            IMS_TRACE_E(0, "Setting Retry-After header failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSSC->Send() != IMS_SUCCESS)
        {
            piSSC->Close();

            IMS_TRACE_E(0, "Rejecting an early UPDATE failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        piSSC->Close();

        IMS_TRACE_D("An UPDATE request is rejected by IMS engine (SessionEx)", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nCallState = GetCallState();

    if ((nCallState != CallState::STATE_INVITE_1XX_RECEIVED) &&
            (nCallState != CallState::STATE_INVITE_1XX_SENT) &&
            (nCallState != CallState::STATE_REINVITE_1XX_RECEIVED) &&
            (nCallState != CallState::STATE_REINVITE_1XX_SENT))
    {
        // It's not an early UPDATE request, so it will be handled by Session
        return Session::HandleRequestToUPDATE(piSSC);
    }

    // Check if we have sent a session refresh request
    // 150831, SESSION_REFRESH_RECEIVED_ON_INVITE_SENT
    if ((piSIPMsg != IMS_NULL) && (piSIPMsg->GetSdpBodyPart() == IMS_NULL))
    {
        if (SendResponseToRefreshUPDATE(piSSC) != IMS_SUCCESS)
        {
            piSSC->Close();
            return IMS_FAILURE;
        }

        piSSC->Close();
        return IMS_SUCCESS;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSSC->GetMessage());

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSSC->GetMessage());

    if ((nOAResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOAResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (!GetService()->SendResponse(piSSC, SipStatusCode::SC_400))
        {
            RestoreOfferAnswerState();
            piSSC->Close();

            IMS_TRACE_E(0, "Rejecting an early UPDATE failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSSC->GetMessage());
        RestoreOfferAnswerState();

        piSSC->Close();

        return IMS_SUCCESS;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (GetService()->CreateResponse(piSSC, SipStatusCode::SC_606) == IMS_FALSE)
        {
            RestoreOfferAnswerState();
            piSSC->Close();

            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        AString strWarning("304 \"Media Type Not Available\"");
        (void)piSSC->GetMessage()->SetHeader(ISipHeader::WARNING, strWarning);

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSSC->Send() != IMS_SUCCESS)
        {
            RestoreOfferAnswerState();
            piSSC->Close();

            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return IMS_FAILURE;
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

    UpdateRequestOnReceived(IMessage::SESSION_EARLY_UPDATE, piSSC);

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);

    SetEarlyState(EARLY_STATE_UPDATE_RECEIVED);

    PostMessage(AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_RECEIVED, 0, 0);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_RESULT SessionEx::HandleResponseToUPDATE(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (nEarlyState != EARLY_STATE_UPDATE_SENT)
    {
        IMS_TRACE_D("IT'S NOT AN EARLY UPDATE", 0, 0, 0);

        return Session::HandleResponseToUPDATE(piSCC);
    }

    IMS_SINT32 nStatusCode = piSCC->GetMessage()->GetStatusCode();

    UpdateResponseOnReceived(IMessage::SESSION_EARLY_UPDATE, piSCC);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return IMS_SUCCESS;
    }

    // AUTH_SIP_DIGEST {
    // Handle 401/407 response
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return IMS_SUCCESS;
        }
    }
    // }

    SetEarlyState(EARLY_STATE_IDLE);

    SetEarlyUpdateNotificationState(IMS_TRUE);
    SetLastEarlyUpdateCompletedTime();

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // CALLER_PREFERENCE_MANAGER
        IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

        if (piMessage != IMS_NULL)
        {
            UpdateCallerPreference(piMessage->GetMessage(), nStatusCode);
        }

        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSCC->GetMessage());

        if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOAResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
        {
            // Update the media state
            UpdateMedia(Media::SESSION_EARLY_UPDATE);

            PostMessage(AMSG_SESSIONEX_EARLY_MEDIA_UPDATED, 0, 0);
        }
        else
        {
            // This can only happen if the other end sends response to an INVITE/re-INVITE
            // with unsupported media descriptors.
            // This should not happen and if it does there is no easy way for us to signal that
            // we do not accept the response.
            // We simply ignore the response and do not send an ACK which should cause the
            // other side to time out.

            // 4 additional processing ?
            RestoreEx();

            PostMessage(AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_FAILED, 0, 0);
        }
    }
    else
    {
        RestoreEx();

        PostMessage(AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_FAILED, 0, 0);
    }

    CloseConnection(IMessage::SESSION_EARLY_UPDATE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL SessionEx::HasPendingPRAck() const
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = pRPRHelper->GetState();

    if ((nState == ReliableProvResponseHelper::STATE_RPR_SENT) ||
            (nState == ReliableProvResponseHelper::STATE_PRACK_RECEIVED))
    {
        // Check if the reliable provisional response contains a session description
        IMS_SINT32 nServiceMethod;

        if (GetState() == STATE_NEGOTIATING)
            nServiceMethod = IMessage::SESSION_START;
        else
            nServiceMethod = IMessage::SESSION_UPDATE;

        Message* pMessage = GetPreviousResponse(nServiceMethod);

        if (pMessage == IMS_NULL)
        {
            return IMS_FALSE;
        }

        ISipMessage* piSIPMsg = pMessage->GetMessage();

        if (!piSIPMsg->GetMethod().Equals(SipMethod::INVITE))
        {
            return IMS_FALSE;
        }

        if (piSIPMsg->GetSdpBodyPart() == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks
 This case is following:
    1) Received UPDATE request with SDP body part
    2) Received 200 OK (w/o SDP body part) to INVITE request
    3) Sending ACK request w/ SDP body part : issue
       >> It MUST be checked before sending ACK request
*/
PRIVATE
IMS_BOOL SessionEx::IsEarlyUpdateInProgress() const
{
    //---------------------------------------------------------------------------------------------

    if (nEarlyState != EARLY_STATE_UPDATE_RECEIVED)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nOAState = GetOfferAnswerState();

    if ((nOAState == SdpOaState::STATE_OFFER_RECEIVED) ||
            (nOAState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        // Checks if the early UPDATE is already received when sending PRACK request
        IMessage* piRequest = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
        IMessage* piResponse = GetPreviousResponse(IMessage::SESSION_EARLY_UPDATE);

        if ((piRequest != IMS_NULL) && (piResponse == IMS_NULL))
        {
            if (piRequest->GetState() == IMessage::STATE_RECEIVED)
            {
                ISipMessage* piSIPMsg_UPDATE = piRequest->GetMessage();

                if (piSIPMsg_UPDATE->GetSdpBodyPart() != IMS_NULL)
                {
                    // New SDP offer is made by the UPDATE request
                    return IMS_TRUE;
                }
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void SessionEx::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer_PendingUpdate == piTimer)
    {
        StopPendingUpdateTimer();

        if (piSSC_PendingUpdate != IMS_NULL)
        {
            IMS_TRACE_I("Pending early update is notified", 0, 0, 0);

            Session::Dialog_NotifyRequest(piSSC_PendingUpdate);

            piSSC_PendingUpdate = IMS_NULL;
        }
    }
}

/*

Remarks

*/
PRIVATE
AString SessionEx::AdjustSessionExpiresHeader(
        IN CONST AString& strRequestSE, IN CONST AString& strResponseSE)
{
    AString strNewSE(AString::ConstNull());
    ISipHeader* piReqHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SESSION_EXPIRES, strRequestSE);
    ISipHeader* piRespHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SESSION_EXPIRES, strResponseSE);

    if ((piReqHeader != IMS_NULL) && (piRespHeader != IMS_NULL))
    {
        IMS_BOOL bSessionExpiresChanged = IMS_FALSE;
        const SipParameter* pReqParam = piReqHeader->GetParameter("refresher");
        const SipParameter* pRespParam = piRespHeader->GetParameter("refresher");

        if ((pReqParam != IMS_NULL) && (pRespParam != IMS_NULL))
        {
            if (!pReqParam->GetValue().Equals(pRespParam->GetValue()))
            {
                piRespHeader->SetParameter("refresher", pReqParam->GetValue());
                bSessionExpiresChanged = IMS_TRUE;
            }
        }
        else if ((pReqParam != IMS_NULL) && (pRespParam == IMS_NULL))
        {
            piRespHeader->SetParameter("refresher", pReqParam->GetValue());
            bSessionExpiresChanged = IMS_TRUE;
        }

        IMS_SINT32 nReqSessionInterval = piReqHeader->GetValueInt();
        IMS_SINT32 nRespSessionInterval = piRespHeader->GetValueInt();

        // Response SHALL use the value of session interval from the request.
        if ((nReqSessionInterval > 0) && (nRespSessionInterval > 0) &&
                (nReqSessionInterval != nRespSessionInterval))
        {
            piRespHeader->SetValueInt(nReqSessionInterval);
            bSessionExpiresChanged = IMS_TRUE;
        }

        if (bSessionExpiresChanged)
        {
            strNewSE = piRespHeader->ToStringWithoutName();
        }
    }

    if (piReqHeader != IMS_NULL)
    {
        piReqHeader->Destroy();
    }

    if (piRespHeader != IMS_NULL)
    {
        piRespHeader->Destroy();
    }

    return strNewSE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SessionEx::CheckNCreateRPRHelper(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper != IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        if (piSIPMsg->IsOptionSupported(Sip::STR_100REL) ||
                piSIPMsg->IsOptionRequired(Sip::STR_100REL))
        {
            IMS_TRACE_I(
                    "Remote endpoint supports a reliable provisional response (INVITE)", 0, 0, 0);

            pRPRHelper = new ReliableProvResponseHelper(IMS_FALSE);

            if (pRPRHelper == IMS_NULL)
            {
                return IMS_FALSE;
            }

            pRPRHelper->Initialize(piSIPMsg);
        }
    }
    else
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (!SipStatusCode::IsProvisional(nStatusCode))
        {
            return IMS_TRUE;
        }

        if (piSIPMsg->IsMessageRpr())
        {
            IMS_TRACE_I("Remote endpoint requires a reliable provisional response (RPR)", 0, 0, 0);

            pRPRHelper = new ReliableProvResponseHelper(IMS_TRUE);

            if (pRPRHelper == IMS_NULL)
            {
                return IMS_FALSE;
            }

            pRPRHelper->Initialize(piSIPMsg);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void SessionEx::DestroyRPRHelper()
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
        return;

    delete pRPRHelper;
    pRPRHelper = IMS_NULL;
}

/*

Remarks

*/
PRIVATE
void SessionEx::HandleRequestToPRACK(IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);

        GetService()->SendResponse(piSSC, SipStatusCode::SC_481);
        piSSC->Close();

        return;
    }

    if (pRPRHelper->GetState() != ReliableProvResponseHelper::STATE_RPR_SENT)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);

        GetService()->SendResponse(piSSC, SipStatusCode::SC_481);
        piSSC->Close();

        return;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSSC->GetMessage());

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSSC->GetMessage());

    if ((nOAResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOAResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        GetService()->SendResponse(piSSC, SipStatusCode::SC_400);
        piSSC->Close();

        return;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (GetService()->CreateResponse(piSSC, SipStatusCode::SC_606) == IMS_FALSE)
        {
            piSSC->Close();

            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return;
        }

        AString strWarning("304 \"Media Type Not Available\"");
        (void)piSSC->GetMessage()->SetHeader(ISipHeader::WARNING, strWarning);

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSSC->Send() != IMS_SUCCESS)
        {
            piSSC->Close();

            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return;
        }

        piSSC->Close();

        return;
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        IMS_TRACE_D("QoS precondition is required ...", 0, 0, 0);
    }
    else if (nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
    {
        IMS_TRACE_D("NO MEDIA STATE CHANGED", 0, 0, 0);
    }

    UpdateRequestOnReceived(IMessage::SESSION_PRACK, piSSC);

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);

    pRPRHelper->UpdateOnMessageReceived(piSSC->GetMessage());

    PostMessage(AMSG_SESSIONEX_PRACK_RECEIVED, 0, 0);
}

/*

Remarks

*/
PRIVATE
void SessionEx::HandleResponseToPRACK(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (pRPRHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return;
    }

    if (pRPRHelper->GetState() != ReliableProvResponseHelper::STATE_PRACK_SENT)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);
        return;
    }

    IMS_SINT32 nStatusCode = piSCC->GetMessage()->GetStatusCode();

    UpdateResponseOnReceived(IMessage::SESSION_PRACK, piSCC);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return;
    }

    // AUTH_SIP_DIGEST {
    // Handle 401/407 response
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return;
        }
    }
    // }

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSCC->GetMessage());

        if ((nOAResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOAResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
        {
            // Update the media state
            UpdateMedia(Media::SESSION_EARLY_UPDATE);

            PostMessage(AMSG_SESSIONEX_PRACK_DELIVERED, 0, 0);
        }
        else
        {
            // This can only happen if the other end sends response to an INVITE/re-INVITE
            // with unsupported media descriptors.
            // This should not happen and if it does there is no easy way for us to signal that
            // we do not accept the response.
            // We simply ignore the response and do not send an ACK which should cause the
            // other side to time out.

            RestoreEx();

            PostMessage(AMSG_SESSIONEX_PRACK_DELIVERY_FAILED, 0, 0);
        }
    }
    else
    {
        RestoreEx();

        PostMessage(AMSG_SESSIONEX_PRACK_DELIVERY_FAILED, 0, 0);
    }

    pRPRHelper->UpdateOnMessageReceived(piSCC->GetMessage());

    CloseConnection(IMessage::SESSION_PRACK);
}

/*

Remarks
 RACE_CONDITION : SESSION_EARLY_UPDATE
*/
PRIVATE
IMS_BOOL SessionEx::IsEarlyUpdateNotificationInProgress() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_EarlyUpdateNotificationInProgress;
}

/*

Remarks
 RACE_CONDITION : SESSION_EARLY_UPDATE
*/
PRIVATE
void SessionEx::SetEarlyUpdateNotificationState(IN IMS_BOOL bInProgress)
{
    //---------------------------------------------------------------------------------------------

    bFlag_EarlyUpdateNotificationInProgress = bInProgress;
}

/*

Remarks

*/
PRIVATE
void SessionEx::SetEarlyState(IN IMS_UINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SessionEx :: %s to %s", EarlyStateToString(nEarlyState),
            EarlyStateToString(nState), 0);

    nEarlyState = nState;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SessionEx::IsIncomingEarlyUpdateReceivedInShortTime() const
{
    if (nLastEarlyUpdateCompletedTimeSec == 0)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nNewSec = IMS_SYS_GetTimeInSeconds();
    IMS_UINT32 nNewMicroSec = IMS_SYS_GetTimeInMicroSeconds();

    if (nNewSec > (nLastEarlyUpdateCompletedTimeSec + 1))
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nTimeGap = 0;

    if (nNewSec == (nLastEarlyUpdateCompletedTimeSec + 1))
    {
        nTimeGap = 1000000 - nLastEarlyUpdateCompletedTimeMicroSec;
        nTimeGap += nNewMicroSec;
    }
    else
    {
        nTimeGap = nNewMicroSec - nLastEarlyUpdateCompletedTimeMicroSec;
    }

    if (nTimeGap >= 100000)
    {
        return IMS_FALSE;
    }

    // When an incoming early UPDATE is received in 100ms after completion early UPDATE
    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void SessionEx::SetLastEarlyUpdateCompletedTime(IN IMS_SINT32 nExplicitTime /* = (-1)*/)
{
    IMS_UINT32 nOldSec = nLastEarlyUpdateCompletedTimeSec;
    IMS_UINT32 nOldMicroSec = nLastEarlyUpdateCompletedTimeMicroSec;

    //---------------------------------------------------------------------------------------------

    nLastEarlyUpdateCompletedTimeSec =
            (nExplicitTime < 0) ? IMS_SYS_GetTimeInSeconds() : nExplicitTime;

    nLastEarlyUpdateCompletedTimeMicroSec =
            (nExplicitTime < 0) ? IMS_SYS_GetTimeInMicroSeconds() : nExplicitTime;

    if ((nOldSec != nLastEarlyUpdateCompletedTimeSec) ||
            (nOldMicroSec != nLastEarlyUpdateCompletedTimeMicroSec))
    {
        AString strLog;

        strLog.Sprintf("sec(%u >> %u), m-sec(%u >> %u)", nOldSec, nLastEarlyUpdateCompletedTimeSec,
                nOldMicroSec, nLastEarlyUpdateCompletedTimeMicroSec);

        IMS_TRACE_D("SetLastEarlyUpdateCompletedTime :: %s", strLog.GetStr(), 0, 0);
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SessionEx::StartPendingUpdateTimer()
{
    if (piTimer_PendingUpdate != IMS_NULL)
    {
        StopPendingUpdateTimer();
    }

    piTimer_PendingUpdate = TimerService::GetTimerService()->CreateTimer();

    if (piTimer_PendingUpdate != IMS_NULL)
    {
        piTimer_PendingUpdate->SetTimer(100, this);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
void SessionEx::StopPendingUpdateTimer()
{
    if (piTimer_PendingUpdate != IMS_NULL)
    {
        piTimer_PendingUpdate->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer_PendingUpdate);
    }
}

/*

Remarks

*/
PRIVATE
const IMS_CHAR* SessionEx::EarlyStateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
        case EARLY_STATE_IDLE:
            return "EARLY_STATE_IDLE";
        case EARLY_STATE_UPDATE_RECEIVED:
            return "EARLY_STATE_UPDATE_RECEIVED";
        case EARLY_STATE_UPDATE_SENT:
            return "EARLY_STATE_UPDATE_SENT";
        default:
            return "__INVALID__";
    }
}
