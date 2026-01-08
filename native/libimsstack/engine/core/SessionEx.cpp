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
#include "ServiceSystemTime.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "offeranswer/SdpOfferAnswer.h"

#include "IOnSessionExListener.h"
#include "ISipClientConnection.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ReliableProvResponseHelper.h"
#include "SdpOaState.h"
#include "Service.h"
#include "SessionEx.h"
#include "SessionRefreshHelper.h"
#include "Sip.h"
#include "SipError.h"
#include "SipHeaderUtils.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/Ims.h"
#include "media/Media.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SessionEx::SessionEx(IN Service* pService) :
        Session(pService),
        m_nEarlyState(EARLY_STATE_IDLE),
        m_pRprHelper(IMS_NULL),
        m_piListener(IMS_NULL),
        m_bEarlyUpdateNotificationInProgress(IMS_FALSE),
        m_piSscPendingUpdate(IMS_NULL),
        m_piTimerPendingUpdate(IMS_NULL),
        m_nLastEarlyUpdateCompletedTimeSec(0),
        m_nLastEarlyUpdateCompletedTimeMicroSec(0)
{
}

PUBLIC VIRTUAL SessionEx::~SessionEx()
{
    DestroyRprHelper();

    StopPendingUpdateTimer();

    if (m_piSscPendingUpdate != IMS_NULL)
    {
        m_piSscPendingUpdate->Close();
    }
}

PUBLIC
IMS_RESULT SessionEx::RespondToEarlyUpdate(
        IN IMS_SINT32 nStatusCode, IN const AString& strReason /*= AString::ConstNull()*/)
{
    if (m_nEarlyState != EARLY_STATE_UPDATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Early UPDATE :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::SESSION_EARLY_UPDATE);

    if (piSsc == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    SessionRefreshHelper* pRefreshHelper = GetRefreshHelper();
    IMS_BOOL bTimerOptionSupported = IMS_FALSE;

    if (pRefreshHelper != IMS_NULL)
    {
        bTimerOptionSupported = pRefreshHelper->IsSessionTimerSupported(piSsc, IMS_FALSE);
    }

    if (CreateResponse(piSsc, nStatusCode, strReason) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS_SINT32 nState = GetState();
        IMS_BOOL bSessionTimerControlRequired;
        AString strRequestSe(AString::ConstNull());

        if ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING))
        {
            bSessionTimerControlRequired = IMS_TRUE;
        }
        else
        {
            const Message* pMessage = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
            const ISipMessage* piSipMsg =
                    (pMessage != IMS_NULL) ? pMessage->GetMessage() : IMS_NULL;

            bSessionTimerControlRequired = (piSipMsg != IMS_NULL)
                    ? piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES)
                    : IMS_FALSE;

            if (bSessionTimerControlRequired && (piSipMsg != IMS_NULL))
            {
                strRequestSe = (piSipMsg != IMS_NULL)
                        ? piSipMsg->GetHeader(ISipHeader::SESSION_EXPIRES)
                        : AString::ConstNull();
            }
        }

        if (bSessionTimerControlRequired)
        {
            if ((pRefreshHelper != IMS_NULL) &&
                    !pRefreshHelper->AddSpecificHeaderOnEarlyUpdate(piSsc, bTimerOptionSupported))
            {
                IMS_TRACE_E(0, "Adding the session refresh specific headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }

            if ((nState != STATE_RENEGOTIATING) && (nState != STATE_REESTABLISHING))
            {
                AString strResponseSe = piSsc->GetMessage()->GetHeader(ISipHeader::SESSION_EXPIRES);
                AString strNewSe = AdjustSessionExpiresHeader(strRequestSe, strResponseSe);

                if (strNewSe.GetLength() > 0)
                {
                    piSsc->GetMessage()->SetHeader(ISipHeader::SESSION_EXPIRES, strNewSe);
                    IMS_TRACE_D("Session-Expires is changed; %s >> %s", strResponseSe.GetStr(),
                            strNewSe.GetStr(), 0);
                }
            }
        }
    }

    ISipMessage* piSipMsg = piSsc->GetMessage();

    // If the UAS receives a UPDATE with an offer,
    // it MUST place the answer in the 2xx to the UPDATE
    IMS_BOOL bHasSdpAnswer = IMS_FALSE;
    IMS_BOOL bIsEarlyUpdateRejected = IMS_FALSE;

    if (SipStatusCode::IsFinal(nStatusCode) &&
            GetOfferAnswerState() == SdpOaState::STATE_OFFER_CHANGE_RECEIVED)
    {
        IMS_BOOL bHasSdpInRequest = IMS_FALSE;

        const Message* pMessage = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

        if (pMessage != IMS_NULL)
        {
            const ISipMessage* piSipMsgUpdate = pMessage->GetMessage();

            if (piSipMsgUpdate->GetSdpBodyPart() != IMS_NULL)
            {
                bHasSdpInRequest = IMS_TRUE;
            }
        }

        if (bHasSdpInRequest)
        {
            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                bHasSdpAnswer = IMS_TRUE;
            }
            else
            {
                bIsEarlyUpdateRejected = IMS_TRUE;
            }
        }
    }

    if (bHasSdpAnswer)
    {
        CheckNSetSdpBodyPart(piSipMsg);
    }

    if (!SendNUpdateResponse(IMessage::SESSION_EARLY_UPDATE, piSsc))
    {
        // Restore Offer/Answer state
        if (bHasSdpAnswer || bIsEarlyUpdateRejected)
        {
            RestoreOfferAnswerState();
            RestoreEx();
        }

        CloseConnection(IMessage::SESSION_EARLY_UPDATE);

        SetEarlyState(EARLY_STATE_IDLE);

        return IMS_FAILURE;
    }

    if (bHasSdpAnswer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
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

PUBLIC
IMS_RESULT SessionEx::RespondToPrack(
        IN IMS_SINT32 nStatusCode, IN const AString& strReason /*= AString::ConstNull()*/)
{
    if (m_pRprHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_pRprHelper->GetState() != ReliableProvResponseHelper::STATE_PRACK_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::SESSION_PRACK);

    if (piSsc == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, nStatusCode, strReason) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    // If the UAS receives a PRACK with an offer, it MUST place the answer in the 2xx to the PRACK
    IMS_BOOL bHasSdpAnswer = IMS_FALSE;
    IMS_BOOL bIsPrackRejected = IMS_FALSE;

    if (SipStatusCode::IsFinal(nStatusCode) &&
            GetOfferAnswerState() == SdpOaState::STATE_OFFER_CHANGE_RECEIVED)
    {
        IMS_BOOL bHasSdpInRequest = IMS_FALSE;
        const Message* pMessage = GetPreviousRequest(IMessage::SESSION_PRACK);

        if (pMessage != IMS_NULL)
        {
            const ISipMessage* piSipMsgPrack = pMessage->GetMessage();

            if (piSipMsgPrack->GetSdpBodyPart() != IMS_NULL)
            {
                bHasSdpInRequest = IMS_TRUE;
            }
        }

        if (bHasSdpInRequest)
        {
            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                bHasSdpAnswer = IMS_TRUE;
            }
            else
            {
                bIsPrackRejected = IMS_TRUE;
            }
        }
    }

    // Set SDP message if any offer ;; According to the configuration options
    if (bHasSdpAnswer)
    {
        ISipMessage* piSipMsg = piSsc->GetMessage();

        CheckNSetSdpBodyPart(piSipMsg);
    }

    if (!SendNUpdateResponse(IMessage::SESSION_PRACK, piSsc))
    {
        // Restore Offer/Answer state
        if (bHasSdpAnswer || bIsPrackRejected)
        {
            RestoreOfferAnswerState();
            RestoreEx();
        }

        CloseConnection(IMessage::SESSION_PRACK);
        m_pRprHelper->UpdateOnOperationFailed();
        return IMS_FAILURE;
    }

    if (bHasSdpAnswer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }
    else if (bIsPrackRejected)
    {
        // Restore Offer/Answer state
        RestoreOfferAnswerState();
        RestoreEx();
    }

    m_pRprHelper->UpdateOnMessageSent(piSsc->GetMessage());

    CloseConnection(IMessage::SESSION_PRACK);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SessionEx::SendPrack()
{
    if (m_pRprHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_pRprHelper->GetState() != ReliableProvResponseHelper::STATE_RPR_RECEIVED)
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
    ISipClientConnection* piScc = CreateConnectionL(piDialog, objMethod);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Set headers
    m_pRprHelper->SetRAckHeader(piSipMsg);

    VirtualSession* pEarlySession = GetVirtualEarlySession();

    if (pEarlySession != IMS_NULL)
    {
        IMS_TRACE_I("PRACK is sent via virtual early session.", 0, 0, 0);

        IMS_BOOL bHasSdpAnswer = pEarlySession->CheckNSetSdpBodyPart(piSipMsg);

        // Try to send a PRACK request to the network
        if (!SendNUpdateRequest(IMessage::SESSION_PRACK, piScc))
        {
            piScc->Close();
            return IMS_FAILURE;
        }

        piSipMsg = piScc->GetMessage();

        if (bHasSdpAnswer)
        {
            pEarlySession->NotifyPrackSent(piSipMsg);
        }

        m_pRprHelper->UpdateOnMessageSent(piSipMsg);

        Ims::SetLastError(ImsError::NO_ERROR);

        return IMS_SUCCESS;
    }

    // Case1) If the UAC receives a reliable provisional response with an offer
    //        (this would occur if the UAC sent an INVITE without an offer,
    //        in which case the first reliable provisional response will contain the offer),
    //        it MUST generate an answer in the PRACK.
    // Case2) PRACK can be used to send a SDP re-offer for session update.
    IMS_BOOL bMayIncludeSdp = IMS_FALSE;
    IMS_SINT32 nOaState = GetOfferAnswerState();

    if ((nOaState == SdpOaState::STATE_OFFER_RECEIVED) ||
            (nOaState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        bMayIncludeSdp = IMS_TRUE;

        // Checks if the early UPDATE is already received when sending PRACK request
        const IMessage* piRequest = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
        const IMessage* piResponse = GetPreviousResponse(IMessage::SESSION_EARLY_UPDATE);

        if ((piRequest != IMS_NULL) && (piResponse == IMS_NULL))
        {
            if (piRequest->GetState() == IMessage::STATE_RECEIVED)
            {
                const ISipMessage* piSipMsgUpdate = piRequest->GetMessage();

                if (piSipMsgUpdate->GetSdpBodyPart() != IMS_NULL)
                {
                    // New SDP offer is made by the UPDATE request
                    bMayIncludeSdp = IMS_FALSE;
                }
            }
        }
    }
    else if (nOaState == SdpOaState::STATE_ESTABLISHED)
    {
        bMayIncludeSdp = IMS_TRUE;
    }

    // Set SDP message if any offer ;; According to the configuration options
    if (bMayIncludeSdp)
    {
        CheckNSetSdpBodyPart(piSipMsg);
    }

    // Try to send a PRACK request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_PRACK, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the media information
    if (bMayIncludeSdp)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }

    m_pRprHelper->UpdateOnMessageSent(piScc->GetMessage());

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SessionEx::SendRpr(IN IMS_SINT32 nStatusCode,
        IN const AString& strReason /*= AString::ConstNull()*/, IN IMS_BOOL bSdp /*= IMS_TRUE*/,
        IN IMS_SINT32 nFlags /*= 0*/)
{
    if (m_pRprHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_pRprHelper->GetState() != ReliableProvResponseHelper::STATE_IDLE)
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

    const Message* pMessage = GetPreviousRequest(nServiceMethod);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    const ISipMessage* piSipMsg = pMessage->GetMessage();

    // Do nothing if the handling method is not INVITE
    if (!piSipMsg->GetMethod().Equals(SipMethod::INVITE))
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(nServiceMethod);

    if (piSsc == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIP server connection is null", 0, 0, 0);
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ RPR :: SENDING %d \"%s\" ___", nStatusCode,
            strReason.IsNULL() ? SipStatusCode::GetReasonPhrase(nStatusCode) : strReason.GetStr(),
            0);

    if (CreateResponse(piSsc, nStatusCode, strReason) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SipError (%d)", SipError::GetLastError(),
                0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsgRpr = piSsc->GetMessage();

    // Check & set the Require ('100rel') & RSeq header
    if (piSipMsgRpr->PrependHeader(ISipHeader::REQUIRE, "100rel") != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Adding Require header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!m_pRprHelper->SetRSeqHeader(piSipMsgRpr))
    {
        IMS_TRACE_E(0, "Adding RSeq header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Set SDP body if the condition meets...
    if (bSdp)
    {
        CheckNSetSdpBodyPart(piSipMsgRpr);
    }

    // REMOVE_RECORD_ROUTE_HEADERS
    if ((nFlags & FLAG_REMOVE_RECORD_ROUTES) != 0)
    {
        RemoveRecordRouteHeaders(piSsc->GetMessage());
    }

    if (!SendNUpdateResponse(nServiceMethod, piSsc))
    {
        IMS_TRACE_E(0, "Sending a response failed - SipError (%d)", SipError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // Check & create a SIP dialog
    CheckNCreateDialog(piSsc);

    // Update the call state
    UpdateCallStateOnMessageSent(piSsc->GetMessage());

    if (bSdp)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);

        if ((nState == STATE_NEGOTIATING) &&
                (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
        {
            AddSessionToCallControlHelperIfNotPresent();
        }
    }

    m_pRprHelper->UpdateOnMessageSent(piSsc->GetMessage());

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SessionEx::UpdateEarlyMedia()
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Early UPDATE :: invalid state", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_piSscPendingUpdate != IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Early UPDATE :: PendingUpdate", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetLastEarlyUpdateCompletedTime(0);

    if (nState == STATE_RENEGOTIATING)
    {
        // If re-INVITE/UPDATE transaction and SDP negotiation is in progress,
        // we could not send the early UPDATE here.
        if (GetPreviousRequest(IMessage::SESSION_UPDATE) != IMS_NULL &&
                GetOfferAnswerState() != SdpOaState::STATE_ESTABLISHED)
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);

            IMS_TRACE_E(0, "Early UPDATE :: invalid state - SDP negotiation in progress", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (m_nEarlyState != EARLY_STATE_IDLE)
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
    ISipClientConnection* piScc = CreateConnectionL(GetDialog(), objMethod);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (nState == STATE_RENEGOTIATING)
    {
        if (!AddRefreshSpecificHeaders(piScc))
        {
            IMS_TRACE_E(0, "Adding the session refresh specific headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Set the headers

    IMS_BOOL bHasSdpOffer = IMS_FALSE;

    if (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED)
    {
        bHasSdpOffer = IMS_TRUE;
    }

    // Set SDP message if any offer ;; According to the configuration options
    if (bHasSdpOffer)
    {
        CheckNSetSdpBodyPart(piSipMsg);
    }

    // Try to send an UPDATE request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_EARLY_UPDATE, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    if (bHasSdpOffer)
    {
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());
        // Update the media state
        UpdateMedia(Media::SESSION_EARLY_UPDATE);
    }

    SetEarlyState(EARLY_STATE_UPDATE_SENT);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
void SessionEx::AbortEarlyUpdateTransaction()
{
    if (m_nEarlyState != EARLY_STATE_UPDATE_SENT)
    {
        return;
    }

    IMS_TRACE_D("Abort early UPDATE transaction", 0, 0, 0);

    // Restore SDP offer/answer state and media state.
    RestoreOfferAnswerState();
    RestoreEx();

    // Clear early update related state.
    SetEarlyState(EARLY_STATE_IDLE);
    SetEarlyUpdateNotificationState(IMS_FALSE);
    SetLastEarlyUpdateCompletedTime(0);

    // Abort SIP transaction.
    CloseConnection(IMessage::SESSION_EARLY_UPDATE);
}

PROTECTED VIRTUAL IMS_BOOL SessionEx::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SESSIONEX_EARLY_MEDIA_UPDATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_EarlyMediaUpdated(this);
            }

            SetEarlyUpdateNotificationState(IMS_FALSE);
            return IMS_TRUE;
        case AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_EarlyMediaUpdateFailed(this);
            }

            SetEarlyUpdateNotificationState(IMS_FALSE);
            return IMS_TRUE;
        case AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_RECEIVED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_EarlyMediaUpdateReceived(this);
            }
            return IMS_TRUE;
        case AMSG_SESSIONEX_PRACK_DELIVERED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_PrackDelivered(this);
            }
            return IMS_TRUE;
        case AMSG_SESSIONEX_PRACK_DELIVERY_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_PrackDeliveryFailed(this);
            }
            return IMS_TRUE;
        case AMSG_SESSIONEX_PRACK_RECEIVED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_PrackReceived(this);
            }
            return IMS_TRUE;
        case AMSG_SESSIONEX_RPR_DELIVERY_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_RprDeliveryFailed(this);
            }
            return IMS_TRUE;
        case AMSG_SESSIONEX_RPR_RECEIVED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSessionEx_RprReceived(
                        this, reinterpret_cast<VirtualSession*>(objMsg.nWparam), objMsg.nLparam);
            }
            return IMS_TRUE;
        case AMSG_SESSION_STARTED:  // FALL-THROUGH
        case AMSG_SESSION_UPDATED:
            SetLastEarlyUpdateCompletedTime(0);
            break;
        default:
            break;
    }

    return Session::DispatchMessage(objMsg);
}

PROTECTED VIRTUAL IMS_BOOL SessionEx::NotifySipRequest(IN ISipServerConnection* piSsc)
{
    // Incoming INVITE request only
    if (!CheckNCreateRprHelper(piSsc->GetMessage()))
    {
        return IMS_FALSE;
    }

    return Session::NotifySipRequest(piSsc);
}

PROTECTED VIRTUAL void SessionEx::NotifySipResponse(IN ISipClientConnection* piScc)
{
    IMS_SINT32 nSipMethod = piScc->GetMethod().ToInt();

    // If a final response to INVITE request is received, then abort PRACK transaction.
    if ((nSipMethod == SipMethod::INVITE) && SipStatusCode::IsFinal(piScc->GetStatusCode()))
    {
        if ((m_pRprHelper != IMS_NULL) &&
                (m_pRprHelper->GetState() == ReliableProvResponseHelper::STATE_PRACK_SENT))
        {
            m_pRprHelper->UpdateOnOperationFailed();
            CloseConnection(IMessage::SESSION_PRACK);

            if (SipStatusCode::IsFinalSuccess(piScc->GetStatusCode()))
            {
                PostMessage(AMSG_SESSIONEX_PRACK_DELIVERED, 0, 0);
            }
        }
    }

    Session::NotifySipResponse(piScc);

    switch (nSipMethod)
    {
        case SipMethod::INVITE:
            if (GetState() == STATE_ESTABLISHED)
            {
                DestroyRprHelper();
            }
            break;
        case SipMethod::PRACK:
            if (piScc == GetClientConnection(IMessage::SESSION_PRACK))
            {
                HandleResponseToPrack(piScc);
            }
            else
            {
                IMS_TRACE_E(0, "%s is not handled", piScc->GetMethod().ToString().GetStr(), 0, 0);

                piScc->Close();
            }
            break;
        case SipMethod::UPDATE:
            if (piScc == GetClientConnection(IMessage::SESSION_EARLY_UPDATE))
            {
                HandleResponseToUpdate(piScc);
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

PROTECTED VIRTUAL void SessionEx::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    IMS_SINT32 nSipMethod = piSc->GetMethod().ToInt();

    if ((nSipMethod == SipMethod::INVITE) && (GetState() == STATE_NEGOTIATING) &&
            (m_pRprHelper != IMS_NULL) &&
            (m_pRprHelper->GetState() == ReliableProvResponseHelper::STATE_RPR_SENT))
    {
        IMS_TRACE_D("SessionEx :: RPR transaction timer is expired", 0, 0, 0);

        if (!IsTerminatePending())
        {
            PostMessage(AMSG_SESSIONEX_RPR_DELIVERY_FAILED, 0, 0);
        }
        else
        {
            RejectEx(SipStatusCode::SC_580);
        }
        return;
    }

    Session::NotifySipError(piSc, nCode, strMessage);

    switch (nSipMethod)
    {
        case SipMethod::PRACK:
            if ((m_pRprHelper != IMS_NULL) &&
                    (m_pRprHelper->GetState() == ReliableProvResponseHelper::STATE_PRACK_SENT))
            {
                m_pRprHelper->UpdateOnOperationFailed();
                CloseConnection(IMessage::SESSION_PRACK);

                PostMessage(AMSG_SESSIONEX_PRACK_DELIVERY_FAILED, 0, 0);
            }
            break;
        case SipMethod::UPDATE:
            if (m_nEarlyState == EARLY_STATE_UPDATE_SENT)
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

PROTECTED VIRTUAL IMS_BOOL SessionEx::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piScc->GetMethod();

    if (objMethod.Equals(SipMethod::PRACK) && (m_pRprHelper != IMS_NULL))
    {
        if (m_pRprHelper->GetState() != ReliableProvResponseHelper::STATE_PRACK_SENT)
        {
            IMS_TRACE_E(0, "PRACK state mismatch", 0, 0, 0);
            return IMS_FALSE;
        }

        // Clear the connection to preserve the SIP connection
        ClearConnection(IMessage::SESSION_PRACK);

        // Try to send a PRACK request to the network
        if (!SendNUpdateRequestEx(IMessage::SESSION_PRACK, piScc, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(IMessage::SESSION_PRACK, piScc);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }
    else if (objMethod.Equals(SipMethod::UPDATE) && (m_nEarlyState != EARLY_STATE_UPDATE_SENT))
    {
        // Clear the connection to preserve the SIP connection
        ClearConnection(IMessage::SESSION_EARLY_UPDATE);

        // Try to send an UPDATE request to the network
        if (!SendNUpdateRequestEx(IMessage::SESSION_EARLY_UPDATE, piScc, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(IMessage::SESSION_EARLY_UPDATE, piScc);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    return Session::SendRequestToChallenge(piScc);
}

PROTECTED VIRTUAL IMS_BOOL SessionEx::Dialog_NotifyRequest(IN ISipServerConnection* piSsc)
{
    IMS_SINT32 nSipMethod = piSsc->GetMethod().ToInt();
    const ISipMessage* piSipMsg = piSsc->GetMessage();

    if (nSipMethod == SipMethod::INVITE)
    {
        // Incoming INVITE request only
        if (!CheckNCreateRprHelper(piSipMsg))
        {
            return IMS_FALSE;
        }
    }

    // RACE_CONDITION: 200 OK to UPDATE and incoming UPDATE
    if (nSipMethod == SipMethod::UPDATE)
    {
        if (m_piSscPendingUpdate != IMS_NULL)
        {
            GetService()->SendResponse(piSsc, SipStatusCode::SC_500);
            piSsc->Close();
            return IMS_TRUE;
        }

        IMS_BOOL bPendingUpdateRequired = IMS_FALSE;

        if (piSipMsg->GetSdpBodyPart() != IMS_NULL)
        {
            if (IsEarlyUpdateNotificationInProgress() || IsIncomingEarlyUpdateReceivedInShortTime())
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
                m_piSscPendingUpdate = piSsc;

                if (!StartPendingUpdateTimer())
                {
                    GetService()->SendResponse(piSsc, SipStatusCode::SC_491);
                    piSsc->Close();

                    m_piSscPendingUpdate = IMS_NULL;
                }
                return IMS_TRUE;
            }
        }
    }

    if (!Session::Dialog_NotifyRequest(piSsc))
    {
        return IMS_FALSE;
    }

    switch (nSipMethod)
    {
        case SipMethod::ACK:
            if (GetState() == STATE_ESTABLISHED)
            {
                DestroyRprHelper();
            }
            break;
        case SipMethod::PRACK:
            HandleRequestToPrack(piSsc);
            break;
        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL Session* SessionEx::CreateSession()
{
    SessionEx* pSession = new SessionEx(GetService());

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

PROTECTED VIRTUAL IMS_RESULT SessionEx::HandleProvisionalResponse(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nServiceMethod)
{
    const ISipMessage* piSipMsg = piScc->GetMessage();

    // Check if the message is a reliable provisional response or not
    if (!piSipMsg->IsMessageRpr())
    {
        return Session::HandleProvisionalResponse(piScc, nServiceMethod);
    }

    if (!CheckNCreateRprHelper(piSipMsg))
    {
        return IMS_FAILURE;
    }

    m_pRprHelper->UpdateOnMessageReceived(piSipMsg);

    VirtualSession* pEarlySession = GetVirtualEarlySession();

    if (pEarlySession == IMS_NULL)
    {
        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSipMsg);

        if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
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
    ImsList<Message*> objResponses = GetPreviousResponses(nServiceMethod);

    if (!objResponses.IsEmpty())
    {
        PostMessage(AMSG_SESSIONEX_RPR_RECEIVED, reinterpret_cast<IMS_UINTP>(pEarlySession),
                objResponses.GetSize() - 1);
    }

    if (piSipMsg->GetStatusCode() == SipStatusCode::SC_180)
    {
        NotifyAlerting();
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_RESULT SessionEx::HandleRequestToUpdate(IN ISipServerConnection* piSsc)
{
    IMS_SINT32 nState = GetState();

    if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
    {
        IMS_TRACE_D("UPDATE is received in TERMINATING or TERMINATED state (%d)", nState, 0, 0);

        GetService()->SendResponse(piSsc, SipStatusCode::SC_488);
        piSsc->Close();
        return IMS_SUCCESS;
    }

    IMS_SINT32 nStatusCode = SipStatusCode::SC_INVALID;
    IMS_SINT32 nOfferAnswerState = GetOfferAnswerState();
    const ISipMessage* piSipMsg = piSsc->GetMessage();

    // If we have received an UPDATE request when we have already sent an UPDATE,
    // the received UPDATE should be rejected with 491 Request Pending response.
    if ((nOfferAnswerState == SdpOaState::STATE_OFFER_SENT) ||
            (nOfferAnswerState == SdpOaState::STATE_OFFER_CHANGE_SENT))
    {
        if ((piSipMsg != IMS_NULL) && (piSipMsg->GetSdpBodyPart() != IMS_NULL))
        {
            nStatusCode = SipStatusCode::SC_491;
        }
    }

    // If we have received an UPDATE request when we are already processing another
    // UPDATE, the 2nd UPDATE should be rejected with 500 Server Internal Error response.
    else if ((nOfferAnswerState == SdpOaState::STATE_OFFER_RECEIVED) ||
            (nOfferAnswerState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        if ((piSipMsg != IMS_NULL) && (piSipMsg->GetSdpBodyPart() != IMS_NULL))
        {
            nStatusCode = SipStatusCode::SC_500;
        }
    }
    else
    {
        if (m_nEarlyState == EARLY_STATE_UPDATE_RECEIVED)
        {
            nStatusCode = SipStatusCode::SC_500;
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

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
            if ((piSipMsg != IMS_NULL) && (piSipMsg->GetSdpBodyPart() != IMS_NULL))
            {
                const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

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
        else if (IsSdpOaInPreviewMode())
        {
            if ((piSipMsg != IMS_NULL) && (piSipMsg->GetSdpBodyPart() != IMS_NULL))
            {
                nStatusCode = SipStatusCode::SC_491;
            }
        }
    }

    if (nStatusCode != SipStatusCode::SC_INVALID)
    {
        IMS_TRACE_I("Rejecting an UPDATE with %d ... in SessionEx", nStatusCode, 0, 0);

        if (GetService()->CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
        {
            piSsc->Close();

            IMS_TRACE_E(0, "Rejecting an early UPDATE failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Sets Retry-After header field
        AString strRetryAfter;
        strRetryAfter.SetNumber(SipHeaderUtils::GenerateRetryAfterSeconds(10));

        if (piSsc->GetMessage()->SetHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfter) !=
                IMS_SUCCESS)
        {
            piSsc->Close();

            IMS_TRACE_E(0, "Setting Retry-After header failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSsc->Send() != IMS_SUCCESS)
        {
            piSsc->Close();

            IMS_TRACE_E(0, "Rejecting an early UPDATE failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        piSsc->Close();

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
        return Session::HandleRequestToUpdate(piSsc);
    }

    // Check if we have sent a session refresh request
    // 150831, SESSION_REFRESH_RECEIVED_ON_INVITE_SENT
    if ((piSipMsg != IMS_NULL) && (piSipMsg->GetSdpBodyPart() == IMS_NULL))
    {
        if (SendResponseToRefreshUpdate(piSsc) != IMS_SUCCESS)
        {
            piSsc->Close();
            return IMS_FAILURE;
        }

        piSsc->Close();
        return IMS_SUCCESS;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSsc->GetMessage());

    IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSsc->GetMessage());

    if ((nOaResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOaResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (!GetService()->SendResponse(piSsc, SipStatusCode::SC_400))
        {
            RestoreOfferAnswerState();
            piSsc->Close();

            IMS_TRACE_E(0, "Rejecting an early UPDATE failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        RestoreOfferAnswerState();

        piSsc->Close();

        return IMS_SUCCESS;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (GetService()->CreateResponse(piSsc, SipStatusCode::SC_606) == IMS_FALSE)
        {
            RestoreOfferAnswerState();
            piSsc->Close();

            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        (void)piSsc->GetMessage()->SetHeader(ISipHeader::WARNING, WARNING_304);
        (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSsc->Send() != IMS_SUCCESS)
        {
            RestoreOfferAnswerState();
            piSsc->Close();

            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        RestoreOfferAnswerState();

        piSsc->Close();

        return IMS_SUCCESS;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        IMS_TRACE_D("QoS precondition is required ...", 0, 0, 0);
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
    {
        IMS_TRACE_D("NO MEDIA STATE CHANGED", 0, 0, 0);
    }

    UpdateRequestOnReceived(IMessage::SESSION_EARLY_UPDATE, piSsc);

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);

    SetEarlyState(EARLY_STATE_UPDATE_RECEIVED);

    PostMessage(AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_RECEIVED, 0, 0);

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_RESULT SessionEx::HandleResponseToUpdate(IN ISipClientConnection* piScc)
{
    if (m_nEarlyState != EARLY_STATE_UPDATE_SENT)
    {
        IMS_TRACE_D("IT'S NOT AN EARLY UPDATE", 0, 0, 0);

        return Session::HandleResponseToUpdate(piScc);
    }

    IMS_SINT32 nStatusCode = piScc->GetMessage()->GetStatusCode();

    UpdateResponseOnReceived(IMessage::SESSION_EARLY_UPDATE, piScc);

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
        if (RespondToChallenge(piScc))
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
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

        if (piMessage != IMS_NULL)
        {
            UpdateCallerPreference(piMessage->GetMessage(), nStatusCode);
        }

        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piScc->GetMessage());

        if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
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

PROTECTED VIRTUAL IMS_BOOL SessionEx::HasPendingPrack() const
{
    if (m_pRprHelper == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = m_pRprHelper->GetState();

    if ((nState == ReliableProvResponseHelper::STATE_RPR_SENT) ||
            (nState == ReliableProvResponseHelper::STATE_PRACK_RECEIVED))
    {
        // Check if the reliable provisional response contains a session description
        IMS_SINT32 nServiceMethod;

        if (GetState() == STATE_NEGOTIATING)
        {
            nServiceMethod = IMessage::SESSION_START;
        }
        else
        {
            nServiceMethod = IMessage::SESSION_UPDATE;
        }

        const Message* pMessage = GetPreviousResponse(nServiceMethod);

        if (pMessage == IMS_NULL)
        {
            return IMS_FALSE;
        }

        const ISipMessage* piSipMsg = pMessage->GetMessage();

        if (!piSipMsg->GetMethod().Equals(SipMethod::INVITE))
        {
            return IMS_FALSE;
        }

        if (piSipMsg->GetSdpBodyPart() == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/**
 * This case is following:
 *     1) Received UPDATE request with SDP body part
 *     2) Received 200 OK (w/o SDP body part) to INVITE request
 *     3) Sending ACK request w/ SDP body part : issue
 *        >> It MUST be checked before sending ACK request
 */
PRIVATE
IMS_BOOL SessionEx::IsEarlyUpdateInProgress() const
{
    if (m_nEarlyState != EARLY_STATE_UPDATE_RECEIVED)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nOaState = GetOfferAnswerState();

    if ((nOaState == SdpOaState::STATE_OFFER_RECEIVED) ||
            (nOaState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        // Checks if the early UPDATE is already received when sending PRACK request
        const IMessage* piRequest = GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
        const IMessage* piResponse = GetPreviousResponse(IMessage::SESSION_EARLY_UPDATE);

        if ((piRequest != IMS_NULL) && (piResponse == IMS_NULL))
        {
            if (piRequest->GetState() == IMessage::STATE_RECEIVED)
            {
                const ISipMessage* piSipMsgUpdate = piRequest->GetMessage();

                if (piSipMsgUpdate->GetSdpBodyPart() != IMS_NULL)
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
    if (m_piTimerPendingUpdate == piTimer)
    {
        StopPendingUpdateTimer();

        if (m_piSscPendingUpdate != IMS_NULL)
        {
            IMS_TRACE_I("Pending early update is notified", 0, 0, 0);

            Session::Dialog_NotifyRequest(m_piSscPendingUpdate);

            m_piSscPendingUpdate = IMS_NULL;
        }
    }
}

PRIVATE
AString SessionEx::AdjustSessionExpiresHeader(
        IN const AString& strRequestSe, IN const AString& strResponseSe)
{
    AString strNewSe(AString::ConstNull());
    ISipHeader* piReqHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SESSION_EXPIRES, strRequestSe);
    ISipHeader* piRespHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SESSION_EXPIRES, strResponseSe);

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
            strNewSe = piRespHeader->ToStringWithoutName();
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

    return strNewSe;
}

PRIVATE
IMS_BOOL SessionEx::CheckNCreateRprHelper(IN const ISipMessage* piSipMsg)
{
    if (m_pRprHelper != IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        if (piSipMsg->IsOptionSupported(Sip::STR_100REL) ||
                piSipMsg->IsOptionRequired(Sip::STR_100REL))
        {
            IMS_TRACE_I(
                    "Remote endpoint supports a reliable provisional response (INVITE)", 0, 0, 0);

            m_pRprHelper = new ReliableProvResponseHelper(IMS_FALSE);

            if (m_pRprHelper == IMS_NULL)
            {
                return IMS_FALSE;
            }

            m_pRprHelper->Initialize(piSipMsg);
        }
    }
    else
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (!SipStatusCode::IsProvisional(nStatusCode))
        {
            return IMS_TRUE;
        }

        if (piSipMsg->IsMessageRpr())
        {
            IMS_TRACE_I("Remote endpoint requires a reliable provisional response (RPR)", 0, 0, 0);

            m_pRprHelper = new ReliableProvResponseHelper(IMS_TRUE);

            if (m_pRprHelper == IMS_NULL)
            {
                return IMS_FALSE;
            }

            m_pRprHelper->Initialize(piSipMsg);
        }
    }

    return IMS_TRUE;
}

PRIVATE
void SessionEx::DestroyRprHelper()
{
    if (m_pRprHelper != IMS_NULL)
    {
        delete m_pRprHelper;
        m_pRprHelper = IMS_NULL;
    }
}

PRIVATE
void SessionEx::HandleRequestToPrack(IN ISipServerConnection* piSsc)
{
    if (m_pRprHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);

        GetService()->SendResponse(piSsc, SipStatusCode::SC_481);
        piSsc->Close();
        return;
    }

    if (m_pRprHelper->GetState() != ReliableProvResponseHelper::STATE_RPR_SENT)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);

        GetService()->SendResponse(piSsc, SipStatusCode::SC_481);
        piSsc->Close();
        return;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSsc->GetMessage());

    IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSsc->GetMessage());

    if ((nOaResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOaResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        GetService()->SendResponse(piSsc, SipStatusCode::SC_400);
        piSsc->Close();
        return;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (GetService()->CreateResponse(piSsc, SipStatusCode::SC_606) == IMS_FALSE)
        {
            piSsc->Close();
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return;
        }

        (void)piSsc->GetMessage()->SetHeader(ISipHeader::WARNING, WARNING_304);
        (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSsc->Send() != IMS_SUCCESS)
        {
            piSsc->Close();
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return;
        }

        piSsc->Close();
        return;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        IMS_TRACE_D("QoS precondition is required ...", 0, 0, 0);
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED)
    {
        IMS_TRACE_D("NO MEDIA STATE CHANGED", 0, 0, 0);
    }

    UpdateRequestOnReceived(IMessage::SESSION_PRACK, piSsc);

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);

    m_pRprHelper->UpdateOnMessageReceived(piSsc->GetMessage());

    PostMessage(AMSG_SESSIONEX_PRACK_RECEIVED, 0, 0);
}

PRIVATE
void SessionEx::HandleResponseToPrack(IN ISipClientConnection* piScc)
{
    if (m_pRprHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper is null", 0, 0, 0);
        return;
    }

    if (m_pRprHelper->GetState() != ReliableProvResponseHelper::STATE_PRACK_SENT)
    {
        IMS_TRACE_E(0, "ReliableProvResponseHelper :: invalid state", 0, 0, 0);
        return;
    }

    IMS_SINT32 nStatusCode = piScc->GetMessage()->GetStatusCode();

    UpdateResponseOnReceived(IMessage::SESSION_PRACK, piScc);

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
        if (RespondToChallenge(piScc))
        {
            return;
        }
    }
    // }

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piScc->GetMessage());

        if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
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

    m_pRprHelper->UpdateOnMessageReceived(piScc->GetMessage());

    CloseConnection(IMessage::SESSION_PRACK);
}

PRIVATE
void SessionEx::SetEarlyState(IN IMS_UINT32 nState)
{
    IMS_TRACE_I("SessionEx :: %s to %s", EarlyStateToString(m_nEarlyState),
            EarlyStateToString(nState), 0);

    m_nEarlyState = nState;
}

PRIVATE
IMS_BOOL SessionEx::IsIncomingEarlyUpdateReceivedInShortTime() const
{
    if (m_nLastEarlyUpdateCompletedTimeSec == 0)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nNewSec = IMS_SYS_GetTimeInSeconds();
    IMS_UINT32 nNewMicroSec = IMS_SYS_GetTimeInMicroSeconds();

    if (nNewSec > (m_nLastEarlyUpdateCompletedTimeSec + 1))
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nTimeGap = 0;

    if (nNewSec == (m_nLastEarlyUpdateCompletedTimeSec + 1))
    {
        nTimeGap = 1000000 - m_nLastEarlyUpdateCompletedTimeMicroSec;
        nTimeGap += nNewMicroSec;
    }
    else
    {
        nTimeGap = nNewMicroSec - m_nLastEarlyUpdateCompletedTimeMicroSec;
    }

    if (nTimeGap >= 100000)
    {
        return IMS_FALSE;
    }

    // When an incoming early UPDATE is received in 100ms after completion early UPDATE
    return IMS_TRUE;
}

PRIVATE
void SessionEx::SetLastEarlyUpdateCompletedTime(IN IMS_SINT32 nExplicitTime /*= (-1)*/)
{
    IMS_UINT32 nOldSec = m_nLastEarlyUpdateCompletedTimeSec;
    IMS_UINT32 nOldMicroSec = m_nLastEarlyUpdateCompletedTimeMicroSec;

    m_nLastEarlyUpdateCompletedTimeSec =
            (nExplicitTime < 0) ? IMS_SYS_GetTimeInSeconds() : nExplicitTime;

    m_nLastEarlyUpdateCompletedTimeMicroSec =
            (nExplicitTime < 0) ? IMS_SYS_GetTimeInMicroSeconds() : nExplicitTime;

    if ((nOldSec != m_nLastEarlyUpdateCompletedTimeSec) ||
            (nOldMicroSec != m_nLastEarlyUpdateCompletedTimeMicroSec))
    {
        AString strLog;

        strLog.Sprintf("sec(%u >> %u), m-sec(%u >> %u)", nOldSec,
                m_nLastEarlyUpdateCompletedTimeSec, nOldMicroSec,
                m_nLastEarlyUpdateCompletedTimeMicroSec);

        IMS_TRACE_D("SetLastEarlyUpdateCompletedTime :: %s", strLog.GetStr(), 0, 0);
    }
}

PRIVATE
IMS_BOOL SessionEx::StartPendingUpdateTimer()
{
    if (m_piTimerPendingUpdate != IMS_NULL)
    {
        StopPendingUpdateTimer();
    }

    m_piTimerPendingUpdate = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimerPendingUpdate != IMS_NULL)
    {
        m_piTimerPendingUpdate->SetTimer(100, this);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void SessionEx::StopPendingUpdateTimer()
{
    if (m_piTimerPendingUpdate != IMS_NULL)
    {
        m_piTimerPendingUpdate->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimerPendingUpdate);
    }
}

PRIVATE
const IMS_CHAR* SessionEx::EarlyStateToString(IN IMS_SINT32 nState)
{
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
