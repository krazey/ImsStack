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
#include "RetryTimer.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "private/SipConfigV.h"

#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpProfile.h"

#include "CallControlHelper.h"
#include "Capabilities.h"
#include "IOnSessionListener.h"
#include "IReasonHeaderSetter.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImsCoreContext.h"
#include "Publication.h"
#include "Reference.h"
#include "Replaces.h"
#include "SdpOaState.h"
#include "SdpReader.h"
#include "Service.h"
#include "Session.h"
#include "SessionDescriptor.h"
#include "SessionRefreshHelper.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipError.h"
#include "SipHeaderName.h"
#include "SipHeaderUtils.h"
#include "SipMethod.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipTimerValuesHelper.h"
#include "Subscription.h"
#include "base/IRefreshListener.h"
#include "base/Ims.h"
#include "media/MediaFactory.h"
#include "util/CallerPreferenceManager.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"
#include "util/OperatorFeatureResolver.h"
#include "util/UserAgentHeader.h"

#define __IMS_SEND_ACK_IN_TERMINATING_STATE__

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
Session::Session(IN Service* pService) :
        ServiceMethod(pService),
        m_nState(STATE_CREATED),
        m_bAlerting(IMS_FALSE),
        m_bSdpInInitialInvite(IMS_FALSE),
        m_bTerminatePending(IMS_FALSE),
        m_bUpdateRequestor(IMS_FALSE),
        m_bSdpNonRprAllowed(IMS_FALSE),
        m_bTerminateMethodBye(IMS_FALSE),
        m_bSessionUpdateNotificationInProgress(IMS_FALSE),
        m_bImplicitRoutingRequired(IMS_TRUE),
        m_bSessionCanceledOnAccepted(IMS_FALSE),
        m_nConfigValue(
                CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE | CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR),
        m_nCompletedListenerCalls(0),
        m_nTerminationReason(TERMINATION_REASON_UNKNOWN),
        m_strTerminationReasonFromApp(AString::ConstNull()),
        m_pOaState(IMS_NULL),
        m_pSessionDescriptor(IMS_NULL),
        m_pRemoteMediaCapabilities(IMS_NULL),
        m_piSessionListener(IMS_NULL),
        m_piRefreshListener(IMS_NULL),
        m_pRefreshHelper(IMS_NULL),
        m_piReferredMessageListener(IMS_NULL),
        m_pRetransmissionTask(IMS_NULL),
        m_strSessionIdForCallControl(AString::ConstNull()),
        m_piSccBye(IMS_NULL),
        m_objPreviousCallerPreference(ImsList<AString>()),
        m_pForkedSessions(IMS_NULL),
        m_pVirtualEarlySession(IMS_NULL),
        m_piReasonHeaderSetter(IMS_NULL)
{
}

PUBLIC VIRTUAL Session::~Session()
{
    CleanupOnDestroy();

    if (m_pRefreshHelper != IMS_NULL)
    {
        delete m_pRefreshHelper;
        m_pRefreshHelper = IMS_NULL;
    }

    if (m_pRetransmissionTask != IMS_NULL)
    {
        delete m_pRetransmissionTask;
        m_pRetransmissionTask = IMS_NULL;
    }

    if (m_pRemoteMediaCapabilities != IMS_NULL)
    {
        delete m_pRemoteMediaCapabilities;
    }

    if (m_pSessionDescriptor != IMS_NULL)
    {
        delete m_pSessionDescriptor;
        m_pSessionDescriptor = IMS_NULL;
    }

    if (!m_objMedias.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
        {
            Media* pMedia = m_objMedias.GetAt(i);

            MediaFactory::DestroyMedia(pMedia);
        }

        m_objMedias.Clear();
    }

    if (m_pOaState != IMS_NULL)
    {
        delete m_pOaState;
        m_pOaState = IMS_NULL;
    }
}

PUBLIC VIRTUAL void Session::Destroy()
{
    CleanupOnDestroy();
    ServiceMethod::Destroy();
    GetService()->DeregisterMethod(this);
}

PUBLIC VIRTUAL void Session::SetMessageMediator(IN IMessageMediator* piMediator)
{
    Method::SetMessageMediator(piMediator);

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->SetMessageMediator(piMediator);
    }
}

PUBLIC
IMS_RESULT Session::Accept()
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0,
                "To accept a session, the state MUST be a NEGOTIATING or RENEGOTIATING; (%s)",
                StateToString(nState), 0, 0);
        return IMS_FAILURE;
    }

    if (HasPendingPrack())
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "UAS MUST delay sending 2XX until the provisional response is acknowledged",
                0, 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = IMS_NULL;

    // Create an answer if offer received

    if (nState == STATE_NEGOTIATING)
    {
        piSsc = GetServerConnection(IMessage::SESSION_START);
    }
    else
    {
        piSsc = GetServerConnection(IMessage::SESSION_UPDATE);
    }

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piSsc->GetMessage();

    // Session-Expires
    // Require
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (!SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
        {
            if ((nState == STATE_RENEGOTIATING) && piSsc->GetMethod().Equals(SipMethod::INVITE))
            {
                if (!m_pRefreshHelper->AddSpecificHeaderWithoutParameterChange(piSsc))
                {
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
            else
            {
                if (!m_pRefreshHelper->AddSpecificHeader(piSsc))
                {
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            if (!m_pRefreshHelper->AddSpecificHeader(piSsc))
            {
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    if (nState == STATE_NEGOTIATING)
    {
        CheckNSetSdpBodyPart(piSipMsg);

        if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
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
        // INVITE_TXN_HANDLING_CORRECTION
        // CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());

        // Update the media information
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());

        // Case) Initial offer is sent by 200 OK
        if (GetOfferAnswerState() == SdpOaState::STATE_OFFER_SENT)
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
        IMS_BOOL bSdpFromCurrentView = IMS_TRUE;

        // 1) If re-INVITE is received with SDP, it will be set by the SDP offer/answer context.
        // 2) If no SDP in the re-INVITE, generate the SDP from the current view (capabilities).
        if (piSipMsg->GetMethod().Equals(SipMethod::INVITE) &&
                GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED &&
                (m_pOaState == IMS_NULL || !m_pOaState->IsOfferProgress()))
        {
            // Current view will be used when the following conditions match:
            // 1) re-INVITE w/o SDP received and no responses.
            // 2) re-INVITE w/o SDP received and any provisional response w/o SDP sent.
            const Message* pRequest = GetPreviousRequest(IMessage::SESSION_UPDATE);

            if (pRequest != IMS_NULL && pRequest->GetMessage()->GetSdpBodyPart() == IMS_NULL)
            {
                ImsList<Message*> objResponses = GetPreviousResponses(IMessage::SESSION_UPDATE);

                for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
                {
                    const Message* pResponse = objResponses.GetAt(i);

                    if (pResponse != IMS_NULL &&
                            pResponse->GetMessage()->GetSdpBodyPart() != IMS_NULL)
                    {
                        bSdpFromCurrentView = IMS_FALSE;
                        break;
                    }
                }
            }
            else
            {
                bSdpFromCurrentView = IMS_FALSE;
            }
        }
        else
        {
            bSdpFromCurrentView = IMS_FALSE;
        }

        if (bSdpFromCurrentView)
        {
            SetSdpBodyPartFromCurrentView(piSipMsg);
        }
        else
        {
            CheckNSetSdpBodyPart(piSipMsg);
        }

        // For in-dialog INVITE request... (when ACK waiting timer expired)
        if (piSipMsg->GetMethod().Equals(SipMethod::INVITE))
        {
            piSsc->SetErrorListener(this);
        }

        if (!SendNUpdateResponse(IMessage::SESSION_UPDATE, piSsc))
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
        // INVITE_TXN_HANDLING_CORRECTION
        // CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
        // Update the media information
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());

        // Case) Offer is sent by 200 OK
        if (GetOfferAnswerState() == SdpOaState::STATE_OFFER_CHANGE_SENT)
        {
            UpdateMedia(Media::SESSION_UPDATE);
        }
        else
        {
            UpdateMedia(Media::SESSION_UPDATED);
        }

        if (piSsc->GetMethod().Equals(SipMethod::INVITE))
        {
            SetState(STATE_REESTABLISHING);
        }
        else
        {
            SetState(STATE_ESTABLISHED);
        }
    }

    // Create a dialog after sending 2xx without 1xx response with a tag
    CheckNCreateDialog(piSsc);

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->UpdateOnMessageSent(piSsc);
    }

    Stop2xxRetransmission();

    if (piSsc->GetMethod().Equals(SipMethod::INVITE))
    {
        Start2xxRetransmission();

        // CALLER_PREFERENCE_MANAGER
        if (nState == STATE_NEGOTIATING)
        {
            ISipDialog* piDialog = GetDialog();

            if (piDialog != IMS_NULL)
            {
                ImsCoreContext::GetInstance()->GetCallerPreferenceManager()->UpdateDialogId(
                        GetName(), piDialog->GetDialogId());
            }
        }
    }

    IMS_TRACE_D("Session is accepted", 0, 0, 0);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
Capabilities* Session::CreateCapabilities()
{
    if (!IsMidDialogTransactionCreatable())
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Capabilities can't be created in the state(%s)", StateToString(GetState()),
                0, 0);
        return IMS_NULL;
    }

    // If the Capabilities can not be created, throw exception
    Capabilities* pCapabilities = new Capabilities(GetService());

    if (pCapabilities == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Capabilities failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pCapabilities->InitMethod(this))
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
Media* Session::CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
        IN IMS_SINT32 nCountOfDescriptor /*= 0*/)
{
    if (m_pOaState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SdpOaState is null", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_NULL;
    }

    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED) && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSdpOaState = GetOfferAnswerState();

        if ((nSdpOaState != SdpOaState::STATE_IDLE) &&
                (nSdpOaState != SdpOaState::STATE_ESTABLISHED))
        {
            IMS_TRACE_E(0,
                    "To create a media, the state MUST be an INITIALIZED or ESTABLISHED; "
                    "(%s, %d)",
                    StateToString(nState), nSdpOaState, 0);
            Ims::SetLastError(ImsError::ILLEGAL_STATE);
            return IMS_NULL;
        }
    }

    if ((nDirection != Media::DIRECTION_INACTIVE) && (nDirection != Media::DIRECTION_RECEIVE) &&
            (nDirection != Media::DIRECTION_SEND) && (nDirection != Media::DIRECTION_SEND_RECEIVE))
    {
        if (nDirection == Media::DIRECTION_NONE)
        {
            // Do not specify the SDP direction in the media-level description
        }
        else
        {
            Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Direction argument is invalid", 0, 0, 0);
            return IMS_NULL;
        }
    }

    if (m_objMedias.IsEmpty())
    {
        if (!CheckNCreateSessionDescriptor())
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Can't create an initial SDP offer", 0, 0, 0);
            return IMS_NULL;
        }
    }

    // Create a new SDP media parameter

    Media* pMedia = MediaFactory::CreateOutgoingMedia(
            strType, nDirection, GetService(), m_pOaState, nCountOfDescriptor);

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

    Ims::SetLastError(ImsError::NO_ERROR);

    return pMedia;
}

PUBLIC
Reference* Session::CreateReference(IN const AString& strReferTo, IN const AString& strReferMethod)
{
    if (!IsMidDialogTransactionCreatable())
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(
                0, "Reference can't be created in the state(%s)", StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    if (!Service::ValidateReferTo(strReferTo, strReferMethod))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    // If the Reference can not be created, throw exception
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    Reference* pReference = new Reference(
            GetService(), strReferTo, strReferMethod, Replaces(), m_bImplicitRoutingRequired);

    if (pReference == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pReference->InitMethod(this))
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
Subscription* Session::CreateSubscription(IN const AString& strEvent)
{
    if (!IsMidDialogTransactionCreatable())
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Subscription can't be created in the state(%s)", StateToString(GetState()),
                0, 0);
        return IMS_NULL;
    }

    // Checks an event package from the application configuration
    if (!GetService()->IsEventPackageSupported(strEvent))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    // If the subscription can not be created, throw exception
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    Subscription* pSubscription =
            new Subscription(GetService(), strEvent, m_bImplicitRoutingRequired);

    if (pSubscription == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Subscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pSubscription->InitMethod(this))
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
Publication* Session::CreatePublication(IN const AString& strEvent)
{
    if (!IsMidDialogTransactionCreatable())
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Publication can't be created in the state(%s)", StateToString(GetState()),
                0, 0);
        return IMS_NULL;
    }

    // Checks an event package from the application configuration
    if (!GetService()->IsEventPackageSupported(strEvent))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid argument: event (%s)", strEvent.GetStr(), 0, 0);
        return IMS_NULL;
    }

    // If the publication can not be created, throw exception
    Publication* pPublication = new Publication(GetService(), strEvent, m_bImplicitRoutingRequired);

    if (pPublication == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating Publication failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!pPublication->InitMethod(this))
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
ISipClientConnection* Session::CreateTransaction(IN const SipMethod& objMethod)
{
    switch (objMethod.ToInt())
    {
        case SipMethod::ACK:      // FALL-THROUGH
        case SipMethod::BYE:      // FALL-THROUGH
        case SipMethod::CANCEL:   // FALL-THROUGH
        case SipMethod::INVITE:   // FALL-THROUGH
        case SipMethod::OPTIONS:  // FALL-THROUGH
        case SipMethod::PRACK:    // FALL-THROUGH
        case SipMethod::REFER:    // FALL-THROUGH
        case SipMethod::UPDATE:
            IMS_TRACE_E(0, "Method (%s) is not allowed", objMethod.ToString().GetStr(), 0, 0);
            return IMS_NULL;
        default:
            break;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't create a mid-call transaction; Dialog is null", 0, 0, 0);
        return IMS_NULL;
    }

    ISipClientConnection* piScc = GetService()->CreateConnection(piDialog, objMethod);

    if (piScc != IMS_NULL)
    {
        // IMPLICIT_ROUTING_FOR_MID_DIALOG
        SetImplicitRouteHeader(piScc);
    }

    return piScc;
}

PUBLIC
const ISipHeader* Session::GetContactHeader() const
{
    const ISipDialog* piDialog = GetDialog();

    if (piDialog != IMS_NULL)
    {
        IMS_SINT32 nDState = piDialog->GetState();

        if ((nDState == ISipDialog::STATE_EARLY) || (nDState == ISipDialog::STATE_CONFIRMED))
        {
            return piDialog->GetContactHeader();
        }
    }

    return IMS_NULL;
}

PUBLIC
const Replaces* Session::GetReplaces() const
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_ESTABLISHING) &&
            (nState != STATE_ESTABLISHED) && (nState != STATE_RENEGOTIATING) &&
            (nState != STATE_REESTABLISHING))
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    if (GetSessionId().GetLength() <= 0)
    {
        IMS_TRACE_E(0, "Session id is invalid", 0, 0, 0);
        return IMS_NULL;
    }

    return ImsCoreContext::GetInstance()->GetCallControlHelper()->GetReplacesFromSessionId(
            GetSessionId());
}

PUBLIC
IMS_SINT32 Session::GetTerminationReason() const
{
    // Checks if session already notifies the terminated event.
    if ((m_nCompletedListenerCalls & LISTENER_CALL_TERMINATED) != 0)
    {
        return m_nTerminationReason;
    }

    if (GetState() != STATE_TERMINATED)
    {
        IMS_TRACE_E(0, "The termination reason can't be retrieved in the %s",
                StateToString(GetState()), 0, 0);
        return TERMINATION_REASON_INVALID;
    }

    return m_nTerminationReason;
}

PUBLIC
const ImsList<Media*>& Session::GetMedia() const
{
    if (GetState() == STATE_TERMINATED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To get a media, the state MUST not be a TERMINATED; (%s)",
                StateToString(GetState()), 0, 0);
        return m_objMedias;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return m_objMedias;
}

PUBLIC
SessionDescriptor* Session::GetSessionDescriptor()
{
    if (!CheckNCreateSessionDescriptor())
    {
        return IMS_NULL;
    }

    return m_pSessionDescriptor;
}

PUBLIC
IMS_BOOL Session::HasPendingUpdate() const
{
    if (GetState() == STATE_ESTABLISHED)
    {
        for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
        {
            const Media* pMedia = m_objMedias.GetAt(i);

            if ((pMedia->GetState() == Media::STATE_INACTIVE) ||
                    ((pMedia->GetState() == Media::STATE_ACTIVE) &&
                            (pMedia->GetUpdateState() != Media::UPDATE_UNCHANGED)))
            {
                IMS_TRACE_D("Session has at least a pending update for the media", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    IMS_TRACE_D("Session has no pending update for the media", 0, 0, 0);

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL Session::IsFinalResponseReceivedForInitialInviteRequest() const
{
    if (m_pForkedSessions.IsNull())
    {
        return IsInviteFinalResponseReceived(IMessage::SESSION_START);
    }

    const ImsList<Method*>& objMethods = m_pForkedSessions->GetMethods();

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

PUBLIC
IMS_BOOL Session::IsReliableProvResponseSupported() const
{
    const Message* pMessage = GetPreviousRequest(IMessage::SESSION_START);

    if (pMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const ISipMessage* piSipMsg = pMessage->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piSipMsg->IsOptionSupported(Sip::STR_100REL);
}

PUBLIC
IMS_BOOL Session::IsSdpOaInPreviewMode() const
{
    return (m_pOaState != IMS_NULL) ? m_pOaState->IsInPreviewMode() : IMS_FALSE;
}

PUBLIC
IMS_RESULT Session::Reject()
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0,
                "To reject a session, the state MUST be a NEGOTIATING or RENEGOTIATING; "
                "(%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = IMS_NULL;

    // Create an answer if offer received

    if (nState == STATE_NEGOTIATING)
    {
        piSsc = GetServerConnection(IMessage::SESSION_START);
    }
    else
    {
        piSsc = GetServerConnection(IMessage::SESSION_UPDATE);
    }

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, SipStatusCode::SC_486) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    if (nState == STATE_NEGOTIATING)
    {
        if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
        {
            CloseConnection(IMessage::SESSION_START);

            Ims::SetLastError(ImsError::GENERAL_ERROR);
            SetState(STATE_TERMINATED);
            CleanupMedia();
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        UpdateMedia(Media::SESSION_START_FAILED);

        CloseConnection(IMessage::SESSION_START);

        SetState(STATE_TERMINATED);

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
    }
    else
    {
        if (!SendNUpdateResponse(IMessage::SESSION_UPDATE, piSsc))
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            Ims::SetLastError(ImsError::GENERAL_ERROR);
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
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        UpdateMedia(Media::SESSION_UPDATE_FAILED);
        RestoreEx();

        if (SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
        {
            m_pRefreshHelper->UpdateOnMessageSent(piSsc);
        }

        CloseConnection(IMessage::SESSION_UPDATE);

        SetState(STATE_ESTABLISHED);

        PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
    }

    IMS_TRACE_D("Session :: Reject ()", 0, 0, 0);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::Reject(IN IMS_SINT32 nStatusCode)
{
    if (GetState() != STATE_NEGOTIATING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To reject a session, the state MUST be a NEGOTIATING; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    if ((nStatusCode != STATUSCODE_433_ANONYMITY_DISALLOWED) &&
            (nStatusCode != STATUSCODE_480_TEMPORARILY_UNAVAILABLE) &&
            (nStatusCode != STATUSCODE_486_BUSY_HERE) &&
            (nStatusCode != STATUSCODE_488_NOT_ACCEPTABLE_HERE) &&
            (nStatusCode != STATUSCODE_600_BUSY_EVERYWHERE) &&
            (nStatusCode != STATUSCODE_603_DECLINE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Unsupported status code (%d) to reject a session", nStatusCode, 0, 0);
        return IMS_FAILURE;
    }

    // Send a reject response

    // Create an answer if offer received

    ISipServerConnection* piSsc = GetServerConnection(IMessage::SESSION_START);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    ISipMessage* piSipMsg = piSsc->GetMessage();
    SetSdpBodyPartFromRefusedView(piSipMsg);

    if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
    {
        CloseConnection(IMessage::SESSION_START);

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        SetState(STATE_TERMINATED);
        CleanupMedia();
        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

        return IMS_FAILURE;
    }

    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    // Update the call state
    UpdateCallStateOnMessageSent(piSsc->GetMessage());
    UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
    UpdateMedia(Media::SESSION_START_FAILED);

    CloseConnection(IMessage::SESSION_START);
    SetState(STATE_TERMINATED);

    IMS_TRACE_D("Session :: Reject (%d)", nStatusCode, 0, 0);

    PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::RejectEx(
        IN IMS_SINT32 nStatusCode, IN const AString& strReasonPhrase /*= AString::ConstNull()*/)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0,
                "To reject a session, the state MUST be a NEGOTIATING or RENEGOTIATING; "
                "(%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = IMS_NULL;

    // Create an answer if offer received

    if (nState == STATE_NEGOTIATING)
    {
        piSsc = GetServerConnection(IMessage::SESSION_START);
    }
    else
    {
        piSsc = GetServerConnection(IMessage::SESSION_UPDATE);
    }

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, nStatusCode, strReasonPhrase) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    ISipMessage* piSipMsg = piSsc->GetMessage();
    SetSdpBodyPartFromRefusedView(piSipMsg);

    if (nState == STATE_NEGOTIATING)
    {
        if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
        {
            CloseConnection(IMessage::SESSION_START);

            Ims::SetLastError(ImsError::GENERAL_ERROR);
            SetState(STATE_TERMINATED);
            CleanupMedia();
            CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

            PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

            return IMS_FAILURE;
        }

        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        UpdateMedia(Media::SESSION_START_FAILED);

        CloseConnection(IMessage::SESSION_START);

        SetState(STATE_TERMINATED);

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
    }
    else
    {
        if (!SendNUpdateResponse(IMessage::SESSION_UPDATE, piSsc))
        {
            CloseConnection(IMessage::SESSION_UPDATE);

            Ims::SetLastError(ImsError::GENERAL_ERROR);
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
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        UpdateMedia(Media::SESSION_UPDATE_FAILED);
        RestoreEx();

        if (SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
        {
            m_pRefreshHelper->UpdateOnMessageSent(piSsc);
        }

        CloseConnection(IMessage::SESSION_UPDATE);

        SetState(STATE_ESTABLISHED);

        PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
    }

    IMS_TRACE_D("Session :: RejectEx (%d)", nStatusCode, 0, 0);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::RejectWithDiversion(IN const AString& strAlternativeUserAddress)
{
    if (GetState() != STATE_NEGOTIATING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To reject a session, the state MUST be a NEGOTIATING; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, strAlternativeUserAddress);

    if (piHeader == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "No contact headers in 3xx response", 0, 0, 0);
        return IMS_FAILURE;
    }

    piHeader->Destroy();

    // Send "302 - Moved Temporarily" response with the given Contact address

    // Create an answer if offer received

    ISipServerConnection* piSsc = GetServerConnection(IMessage::SESSION_START);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, SipStatusCode::SC_302) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piSsc->GetMessage();

    // Set Contact header
    piSipMsg->SetHeader(ISipHeader::CONTACT_ANY, strAlternativeUserAddress);

    if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
    {
        CloseConnection(IMessage::SESSION_START);

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        SetState(STATE_TERMINATED);
        CleanupMedia();
        CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

        return IMS_FAILURE;
    }

    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    // Update the call state
    UpdateCallStateOnMessageSent(piSsc->GetMessage());
    UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
    UpdateMedia(Media::SESSION_START_FAILED);

    CloseConnection(IMessage::SESSION_START);
    SetState(STATE_TERMINATED);

    PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::RemoveMedia(IN Media* pMedia)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED) && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSdpOaState = GetOfferAnswerState();

        if ((nSdpOaState != SdpOaState::STATE_IDLE) &&
                (nSdpOaState != SdpOaState::STATE_ESTABLISHED))
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);

            IMS_TRACE_E(0,
                    "To remove a media, the state MUST be a INITIATED or ESTABLISHED; "
                    "(%s, %d)",
                    StateToString(nState), nSdpOaState, 0);
            return IMS_FAILURE;
        }
    }

    if (pMedia == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_UINT32 i;

    for (i = 0; i < m_objMedias.GetSize(); ++i)
    {
        const Media* pExistingMedia = m_objMedias.GetAt(i);

        if (pExistingMedia->Equals(pMedia))
        {
            break;
        }
    }

    if (i >= m_objMedias.GetSize())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "No matched media (%p)", pMedia, 0, 0);
        return IMS_FAILURE;
    }

    IMS_BOOL bMidSyncRequired = (i < (m_objMedias.GetSize() - 1));
    Media* pMatchedMedia = m_objMedias.GetAt(i);

    pMatchedMedia->RemoveMedia();

    if ((pMatchedMedia->GetState() == Media::STATE_INACTIVE) ||
            (pMatchedMedia->GetState() == Media::STATE_DELETED))
    {
        m_objMedias.RemoveAt(i);
    }

    if (bMidSyncRequired)
    {
        for (IMS_UINT32 j = i; j < m_objMedias.GetSize(); j++)
        {
            Media* pTempMedia = m_objMedias.GetAt(j);
            pTempMedia->SetMid(j);
        }
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::RemoveMedia(IN IMS_UINT32 nIndex)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED) && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSdpOaState = GetOfferAnswerState();

        if ((nSdpOaState != SdpOaState::STATE_IDLE) &&
                (nSdpOaState != SdpOaState::STATE_ESTABLISHED))
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);

            IMS_TRACE_E(0,
                    "To remove a media, the state MUST be a INITIATED or ESTABLISHED; "
                    "(%s, %d)",
                    StateToString(nState), nSdpOaState, 0);
            return IMS_FAILURE;
        }
    }

    if (nIndex >= m_objMedias.GetSize())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(
                0, "Invalid index (%d) in the total size (%d)", nIndex, m_objMedias.GetSize(), 0);
        return IMS_FAILURE;
    }

    IMS_BOOL bMidSyncRequired = (nIndex < (m_objMedias.GetSize() - 1));
    Media* pMedia = m_objMedias.GetAt(nIndex);

    if (pMedia == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pMedia->RemoveMedia();

    if ((pMedia->GetState() == Media::STATE_INACTIVE) ||
            (pMedia->GetState() == Media::STATE_DELETED))
    {
        m_objMedias.RemoveAt(nIndex);
    }

    if (bMidSyncRequired)
    {
        for (IMS_UINT32 j = nIndex; j < m_objMedias.GetSize(); j++)
        {
            pMedia = m_objMedias.GetAt(j);
            pMedia->SetMid(j);
        }
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::Restore()
{
    if (GetState() != STATE_ESTABLISHED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To restore a media, the state MUST be a ESTABLISHED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    // Restore the proposal view
    RestoreOfferAnswerState();
    // Remove the local capabilities that are not negotiated?
    RestoreEx();

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::SendAck()
{
    IMS_SINT32 nCallState = GetCallState();

    if ((nCallState != CallState::STATE_INVITE_2XX_RECEIVED) &&
            (nCallState != CallState::STATE_REINVITE_2XX_RECEIVED))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Invalid call state (%d) to send ACK", nCallState, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nServiceMethod;

    if (nCallState == CallState::STATE_INVITE_2XX_RECEIVED)
    {
        nServiceMethod = IMessage::SESSION_START;
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
    }

    ISipClientConnection* piScc = GetClientConnection(nServiceMethod);

    if (piScc == IMS_NULL)
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (SendRequestToAck(piScc, nServiceMethod) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending ACK request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    CloseConnection(nServiceMethod);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
        IN const AString& strReason /*= AString::ConstNull()*/, IN IMS_SINT32 nFlags /*= 0*/)
{
    if (!SipStatusCode::IsProvisional(nStatusCode))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid provisional response code (%d, %s)", nStatusCode,
                strReason.GetStr(), 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = GetState();

    if (m_bUpdateRequestor || (IsMobileOriginated() && (nState == STATE_NEGOTIATING)))
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);

        IMS_TRACE_E(0, "Session is the mobile-terminated session", 0, 0, 0);
        return IMS_FAILURE;
    }

    if ((nState != STATE_NEGOTIATING) && (nState != STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        IMS_TRACE_E(0,
                "To send a provisional response, the state MUST be a NEGOTIATING "
                "or RENEGOTIATING; (%s)",
                StateToString(GetState()), 0, 0);
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
        Ims::SetLastError(ImsError::INVALID_OPERATION);

        IMS_TRACE_E(0, "SIP server connection is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ PR :: SENDING %d \"%s\" ___", nStatusCode,
            strReason.IsNULL() ? SipStatusCode::GetReasonPhrase(nStatusCode) : strReason.GetStr(),
            0);

    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SipError (%d)", SipError::GetLastError(),
                0, 0);
        return IMS_FAILURE;
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

    // Update the call state
    UpdateCallStateOnMessageSent(piSsc->GetMessage());

    CheckNCreateDialog(piSsc);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::SetCallerPreference(IN const ImsList<AString>& objCallerPreference)
{
    // CALLER_PREFERENCE_MANAGER
    m_objPreviousCallerPreference.Clear();
    m_objPreviousCallerPreference = objCallerPreference;

    ImsCoreContext::GetInstance()->GetCallerPreferenceManager()->UpdateAcceptContacts(
            GetName(), objCallerPreference);

    return IMS_SUCCESS;
}

// CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
PUBLIC
IMS_RESULT Session::SetContactParameter(
        IN const AString& strParameter, IN IMS_SINT32 nOperation /*= 0 (0: ADD, 1: REMOVE)*/)
{
    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piDialog->SetContactParameter(strParameter, nOperation);
}

PUBLIC
void Session::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    m_bImplicitRoutingRequired = bFlag;
}

PUBLIC
void Session::SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
        IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt)
{
    if (m_pRefreshHelper == IMS_NULL)
    {
        return;
    }

    switch (nPolicy)
    {
        case REFRESH_POLICY_NO_REFRESH:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_NO_REFRESH, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_SPEC:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_SPEC, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_REMAIN_TIME:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_REMAIN_TIME, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_RATIO:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_RATIO, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        default:
            IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
            break;
    }
}

PUBLIC
IMS_RESULT Session::Start()
{
    if (GetState() != STATE_INITIATED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To start a session, the state MUST be an INITIATED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    // 1 Its checking is moved to the enabler implementation
#if 0
    if (objMedias.IsEmpty())
    {
        IMS_TRACE_E(0, "No media in a session", 0, 0, 0);
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }
#endif

    // Check if the created media is initialized or not
    if (!IsMediaInitializationDone())
    {
        IMS_TRACE_E(0, "Media is not ready", 0, 0, 0);
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Send INVITE request
    if (SendRequestToInvite() != IMS_SUCCESS)
    {
        SetState(STATE_TERMINATED);
        CleanupMedia();

        PostMessage(AMSG_SESSION_START_FAILED, 0, 0);

        return IMS_FAILURE;
    }

    SetState(STATE_NEGOTIATING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::Terminate()
{
    IMS_SINT32 nState = GetState();

    if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
    {
        Ims::SetLastError(ImsError::NO_ERROR);

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
        case STATE_CREATED:  // FALL-THROUGH
        case STATE_INITIATED:
            SetState(STATE_TERMINATED);
            Ims::SetLastError(ImsError::NO_ERROR);

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
            if (m_pRefreshHelper != IMS_NULL)
            {
                if (m_pRefreshHelper->IsRequestPending())
                {
                    IMS_TRACE_D("Stopping the session refresh on Terminate()", 0, 0, 0);

                    m_pRefreshHelper->AbortConnection();
                }
            }

            SetState(STATE_TERMINATING);

            if (SendRequestToBye() != IMS_SUCCESS)
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
        default:
            break;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::TerminateEx(IN IMS_BOOL bTerminateMethodBye /*= IMS_FALSE*/)
{
    IMS_SINT32 nState = GetState();

    IMS_TRACE_D("TerminateEx()", 0, 0, 0);

    if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
    {
        Ims::SetLastError(ImsError::NO_ERROR);

        IMS_TRACE_D("Session is already terminating or terminated in Terminate()", 0, 0, 0);
        return IMS_SUCCESS;
    }

    m_bTerminateMethodBye = bTerminateMethodBye;

    // Cease the 2xx retransmission
    Stop2xxRetransmission();

    IMS_SINT32 nCallState = GetCallState();

    if (!m_bTerminateMethodBye && m_bUpdateRequestor && (nState == STATE_RENEGOTIATING) &&
            ((nCallState == CallState::STATE_REINVITE_SENT) ||
                    (nCallState == CallState::STATE_REINVITE_1XX_RECEIVED)))
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
        case STATE_CREATED:  // FALL-THROUGH
        case STATE_INITIATED:
            SetState(STATE_TERMINATED);
            Ims::SetLastError(ImsError::NO_ERROR);

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
            if (m_pRefreshHelper != IMS_NULL)
            {
                if (m_pRefreshHelper->IsRequestPending())
                {
                    IMS_TRACE_D("Stopping the session refresh on Terminate()", 0, 0, 0);

                    m_pRefreshHelper->AbortConnection();
                }
            }

            SetState(STATE_TERMINATING);

            if (SendRequestToBye() != IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                IMS_TRACE_E(0, "Sending BYE request on ESTABLISHED failed", 0, 0, 0);
                return IMS_FAILURE;
            }
            break;
        case STATE_RENEGOTIATING:
            if (m_bTerminateMethodBye)
            {
                TerminateOnReNegotiating();
                break;
            }

            if (m_bUpdateRequestor)
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
                    if (SendRequestToCancel() != IMS_SUCCESS)
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
        case STATE_TERMINATING:  // FALL-THROUGH
        case STATE_TERMINATED:   // FALL-THROUGH
        default:
            break;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::Update()
{
    if (GetState() != STATE_ESTABLISHED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To update a session, the state MUST be an ESTABLISHED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    if (!HasPendingUpdate())
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        IMS_TRACE_E(0, "There are no updates to be made to the session", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Checks if the session refresh is ongoing...
    if (SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSipProfile()))
    {
        if ((m_pRefreshHelper != IMS_NULL) && (m_pRefreshHelper->IsRequestPending()))
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);
            IMS_TRACE_E(0, "Session refresh is ongoing", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (m_objMedias.IsEmpty() || !IsMediaInitializationDone())
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        IMS_TRACE_E(0, "There is no media or all the medias are not ready to update the session", 0,
                0, 0);
        return IMS_FAILURE;
    }

    SipMethod objMethod = SelectUpdateMethod();

    if (objMethod.Equals(SipMethod::INVITE))
    {
        if (SendRequestToInvite() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        if (SendRequestToUpdate() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }

    m_bUpdateRequestor = IMS_TRUE;
    SetState(STATE_RENEGOTIATING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Session::UpdateEx(
        IN IMS_SINT32 nMethod /*= SipMethod::INVALID*/, IN IMS_BOOL bSessionRefresh /*= IMS_FALSE*/)
{
    // CASE: session refresh
    if (bSessionRefresh)
    {
        return SendRequestForRefresh(nMethod);
    }

    // re-INVITE is selected as the default
    if (nMethod == SipMethod::INVALID)
    {
        nMethod = SipMethod::INVITE;
    }

    if ((nMethod != SipMethod::INVITE) && (nMethod != SipMethod::UPDATE))
    {
        IMS_TRACE_E(0, "To update a session, the method MUST be an INVITE or UPDATE; (%d)", nMethod,
                0, 0);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_ESTABLISHED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "To update a session, the state MUST be an ESTABLISHED; (%s)",
                StateToString(GetState()), 0, 0);
        return IMS_FAILURE;
    }

    // Checks if the session refresh is ongoing...
    if (SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSipProfile()))
    {
        if ((m_pRefreshHelper != IMS_NULL) && (m_pRefreshHelper->IsRequestPending()))
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);
            IMS_TRACE_E(0, "Session refresh is ongoing", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (m_objMedias.IsEmpty() || !IsMediaInitializationDone())
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        IMS_TRACE_E(0, "There is no media or all the medias are not ready to update the session", 0,
                0, 0);
        return IMS_FAILURE;
    }

    if (nMethod == SipMethod::INVITE)
    {
        if (SendRequestToInvite() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        if (SendRequestToUpdate() != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }

    m_bUpdateRequestor = IMS_TRUE;
    SetState(STATE_RENEGOTIATING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
PUBLIC
IMS_RESULT Session::CreateFailureSdp()
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    m_pOaState->CreateRefusedView();

    return IMS_SUCCESS;
}

PUBLIC
void Session::DestroyFailureSdp()
{
    if (m_pOaState == IMS_NULL)
    {
        return;
    }

    m_pOaState->DestroyRefusedView();
}

PUBLIC
ISessionParameter* Session::GetFailureSdp() const
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_pOaState->GetRefusedView();
}
// }

PUBLIC
IMS_BOOL Session::IsSessionRefreshInProgress() const
{
    return (m_pRefreshHelper != IMS_NULL) && m_pRefreshHelper->IsRequestPending();
}

PROTECTED VIRTUAL IMS_BOOL Session::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SESSION_INVITATION_RECEIVED:
            GetService()->HandleSessionInvitationReceived(this);
            return IMS_TRUE;
        case AMSG_SESSION_ALERTING:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_Alerting(this);
            }
            return IMS_TRUE;
        case AMSG_SESSION_REFERENCE_RECEIVED:
        {
            Reference* pReference = reinterpret_cast<Reference*>(objMsg.nLparam);

            if (pReference == IMS_NULL)
            {
                return IMS_TRUE;
            }

            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_ReferenceReceived(this, pReference);
            }
            else
            {
                // 4 Find a proper response code
                pReference->Reject();
                pReference->Destroy();
            }
            return IMS_TRUE;
        }
        case AMSG_SESSION_STARTED:
            if (m_piSessionListener != IMS_NULL)
            {
                if (CheckNSetListenerCall(LISTENER_CALL_STARTED))
                {
                    m_piSessionListener->OnSession_Started(this);
                }
            }
            return IMS_TRUE;
        case AMSG_SESSION_START_FAILED:
            if (m_piSessionListener != IMS_NULL)
            {
                if (CheckNSetListenerCall(LISTENER_CALL_START_FAILED))
                {
                    m_piSessionListener->OnSession_StartFailed(this);
                }
            }
            return IMS_TRUE;
        case AMSG_SESSION_TERMINATED:
            // 'Replaces' header handling ...
            RemoveSessionFromCallControlHelper();

            if (m_piSessionListener != IMS_NULL)
            {
                if (CheckNSetListenerCall(LISTENER_CALL_TERMINATED))
                {
                    m_piSessionListener->OnSession_Terminated(this);
                }
            }
            return IMS_TRUE;
        case AMSG_SESSION_UPDATED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_Updated(this);
            }

            // RACE_CONDITION : SESSION_UPDATE
            SetSessionUpdateNotificationInProgress(IMS_FALSE);
            return IMS_TRUE;
        case AMSG_SESSION_UPDATE_FAILED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_UpdateFailed(this);
            }

            // RACE_CONDITION : SESSION_UPDATE
            SetSessionUpdateNotificationInProgress(IMS_FALSE);
            return IMS_TRUE;
        case AMSG_SESSION_UPDATE_RECEIVED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_UpdateReceived(this);
            }
            return IMS_TRUE;
        case AMSG_SESSION_CANCELED_ON_ACCEPTED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_CanceledOnAccepted(this);
            }
            return IMS_TRUE;
        case AMSG_SESSION_CANCEL_DELIVERED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_CancelDelivered(this);
            }
            return IMS_TRUE;
        case AMSG_SESSION_CANCEL_DELIVERY_FAILED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_CancelDeliveryFailed(this);
            }
            return IMS_TRUE;
        case AMSG_SESSION_FORKED_RESPONSE_RECEIVED:
        {
            Session* pForkedSession = reinterpret_cast<Session*>(objMsg.nLparam);

            if (pForkedSession == IMS_NULL)
            {
                return IMS_TRUE;
            }

            // RACE_CONDITION (MO CANCEL & forked INVITE response)
            IMS_BOOL bTerminated = (m_piSessionListener == IMS_NULL) ? IMS_TRUE : IMS_FALSE;
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
                const IMessage* piMessage =
                        pForkedSession->GetPreviousResponse(IMessage::SESSION_START);

                if (piMessage != IMS_NULL)
                {
                    nStatusCode = piMessage->GetStatusCode();

                    if (SipStatusCode::IsFinalSuccess(nStatusCode))
                    {
                        pForkedSession->SendAck();

                        pForkedSession->SetTerminationReason(TERMINATION_REASON_USER_ACTION);
                        pForkedSession->SendRequestToByeInternal();
                    }
                }

                pForkedSession->Destroy();

                if ((nState == STATE_TERMINATING) && SipStatusCode::IsFinal(nStatusCode) &&
                        (m_piSessionListener != IMS_NULL))
                {
                    SetState(STATE_TERMINATED);
                    m_piSessionListener->OnSession_Terminated(this);
                }
                return IMS_TRUE;
            }

            if (!m_piSessionListener->OnSession_ForkedResponseReceived(this, pForkedSession))
            {
                pForkedSession->Destroy();
            }
            return IMS_TRUE;
        }
        case AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED:
            if (m_piSessionListener != IMS_NULL)
            {
                m_piSessionListener->OnSession_ProvisionalResponseReceived(this, objMsg.nLparam);
            }
            return IMS_TRUE;
        case AMSG_SESSION_TRANSACTION_RECEIVED:
        {
            ISipServerConnection* piSsc = reinterpret_cast<ISipServerConnection*>(objMsg.nLparam);

            if (piSsc == IMS_NULL)
            {
                return IMS_TRUE;
            }

            if (m_piSessionListener != IMS_NULL)
            {
                if (!m_piSessionListener->OnSession_TransactionReceived(this, piSsc))
                {
                    GetService()->SendResponse(piSsc, SipStatusCode::SC_480);
                    piSsc->Close();
                }
            }
            else
            {
                // 4 Find a proper response code
                GetService()->SendResponse(piSsc, SipStatusCode::SC_480);
                piSsc->Close();
            }
            return IMS_TRUE;
        }
        case AMSG_SESSION_DELAYED_DIALOG_TRANSACTION_RECEIVED:
        {
            ISipServerConnection* piSsc = reinterpret_cast<ISipServerConnection*>(objMsg.nLparam);

            if (piSsc != IMS_NULL)
            {
                IMS_TRACE_I("Handle DELAYED_DIALOG_TRANSACTION_RECEIVED", 0, 0, 0);
                Dialog_NotifyRequest(piSsc);
            }
            return IMS_TRUE;
        }
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PROTECTED VIRTUAL void Session::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    IMS_SINT32 nState = GetState();

    (void)nErrorCode;

    // If the error code is SERVICE_CLOSING, do something ...
    IMS_TRACE_D("Session :: Exception_NotifyError() ... Error (%d) on %s", nErrorCode,
            StateToString(nState), 0);

    if (nState == STATE_TERMINATED)
    {
        return;
    }

    if ((nState == STATE_NEGOTIATING) || (nState == STATE_ESTABLISHING) /*ACK wait timeout*/)
    {
        CloseConnection(IMessage::SESSION_START);
        CleanupMedia();

        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        if (!m_bTerminatePending)
        {
            // To send a BYE by enabler
            if (IsMobileOriginated() && (nState == STATE_NEGOTIATING))
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
    else if (((nState == STATE_RENEGOTIATING) ||
                     (nState == STATE_REESTABLISHING) /*ACK wait timeout*/) &&
            !IsConfigurationSet(CONFIG_IGNORE_DEREG_ON_SESSION_UPDATE))
    {
        CloseConnection(IMessage::SESSION_UPDATE);
        RestoreOfferAnswerState();
        RestoreEx();

        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        if (!m_bTerminatePending)
        {
            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            return;
        }
    }

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    if (m_bTerminatePending || (nState == STATE_TERMINATING))
    {
        if (nErrorCode == ImsError::SERVICE_CLOSED)
        {
            SetTerminationReason(TERMINATION_REASON_SERVICE_CLOSED);
        }

        SetState(STATE_TERMINATED);
        CleanupMedia();

        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }
}

PROTECTED VIRTUAL IMS_BOOL Session::InitInstance()
{
    if (GetState() != STATE_CREATED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FALSE;
    }

    // Instantiate SDP offer/answer object
    IMS_BOOL bSdpVersionCheck = IMS_TRUE;
    const SipConfigV* pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        m_bSdpNonRprAllowed = pSipConfigV->IsSessionSdpNonRprAllowed();
        bSdpVersionCheck = pSipConfigV->IsSessionSdpVersionCheckSupported();

        if (pSipConfigV->ShouldIgnoreSubsequentSdpAnswerInPreviewMode())
        {
            SetConfiguration(
                    GetConfiguration() | CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE);
        }
    }

    m_pOaState = new SdpOaState(bSdpVersionCheck, IMS_TRUE);

    if (m_pOaState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_FALSE;
    }

    // Create a session refresh helper
    m_pRefreshHelper = CreateRefreshHelper();

    if (m_pRefreshHelper == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
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
    ImsCoreContext::GetInstance()->GetCallerPreferenceManager()->CreatePreferenceWrapper(
            GetName(), AString::ConstNull());

    GetService()->RegisterMethod(this);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Session::NotifySipRequest(IN ISipServerConnection* piSsc)
{
    const ISipMessage* piSipMsg = piSsc->GetMessage();

    IMS_TRACE_D("___>>> INCOMING SESSION RECEIVED <<<___", 0, 0, 0);

    // Update the call state
    UpdateCallStateOnMessageReceived(piSipMsg);

    // Negotiate session timer info.
    switch (m_pRefreshHelper->UpdateOnMessageReceived(piSsc))
    {
        case SessionRefreshHelper::RESULT_ERROR:  // FALL-THROUGH
        case SessionRefreshHelper::RESULT_REJECT_500:
            GetService()->SendResponse(piSsc, SipStatusCode::SC_500);
            return IMS_FALSE;

        case SessionRefreshHelper::RESULT_REJECT_422:
            CreateResponse(piSsc, SipStatusCode::SC_422);
            m_pRefreshHelper->AddSpecificHeader(piSsc);
            piSsc->Send();
            return IMS_FALSE;

        default:
            break;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSipMsg);

    if (HandleRequestToInvite(piSsc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Handling INVITE request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Session::NotifySipForkedResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc)
{
    if (piScc != GetClientConnection(IMessage::SESSION_START))
    {
        IMS_TRACE_E(0, "No client connection matches for a forked response", 0, 0, 0);

        piForkedScc->Close();
        return IMS_FALSE;
    }

    if (!piForkedScc->GetMethod().Equals(SipMethod::INVITE))
    {
        IMS_TRACE_E(0, "SIP forked response is non-INVITE, so the session ignores it", 0, 0, 0);

        piForkedScc->Close();
        return IMS_FALSE;
    }

    // Create a session from the SIP forked response
    Session* pSession = CreateSession();

    if (pSession == IMS_NULL)
    {
        piForkedScc->Close();
        return IMS_FALSE;
    }

    // Management of forked sessions
    if (m_pForkedSessions.IsNull())
    {
        m_pForkedSessions = new MethodManager();

        // First forked session detected
        m_pForkedSessions->AddMethod(this);
        m_pForkedSessions->AddMethod(pSession);
    }
    else
    {
        // New forked session
        m_pForkedSessions->AddMethod(pSession);
    }

    pSession->m_pForkedSessions = m_pForkedSessions;

    // Set a configuration as the original configuration
    pSession->m_nConfigValue = m_nConfigValue;

    // Update the dialog info. enforcelly
    pSession->CheckNCreateDialog(piForkedScc, IMS_TRUE);

    if (m_bSdpInInitialInvite)
    {
        // INVITE request has been sent with SDP, so creates a media from the SDP.
        pSession->m_bSdpInInitialInvite = IMS_TRUE;
        pSession->CheckNCreateSessionDescriptor();

        // Update the proposed view from this session capabilities
        const SessionParameter* pCapabilities = m_pOaState->GetCapabilities();
        SessionParameter* pProposalView = pSession->m_pOaState->GetProposalView();

        (*pProposalView) = (*pCapabilities);

        // Create all the medias from the proposal view
        pSession->CreateMediaFromSdp();
    }

    if (!pSession->UpdateRequestOnSent(IMessage::SESSION_START, piForkedScc))
    {
        pSession->Destroy();
        piForkedScc->Close();

        IMS_TRACE_E(0, "Updating request on sent failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Set listener for the forked client connection
    piForkedScc->SetListener(pSession);
    piForkedScc->SetErrorListener(pSession);

    // Update the call state
    pSession->UpdateCallStateOnMessageSent(piForkedScc->GetMessage());
    // Update the Offer/Answer state
    pSession->UpdateOfferAnswerStateOnMessageSent(piForkedScc->GetMessage());
    // Update the media state
    pSession->UpdateMedia(Media::SESSION_START);

    pSession->SetState(STATE_NEGOTIATING);

    PostMessage(AMSG_SESSION_FORKED_RESPONSE_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(pSession));

    pSession->HandleAllSipResponse(piForkedScc);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Session::NotifySipResponse(IN ISipClientConnection* piScc)
{
    IMS_TRACE_I("___ Response received at the state, %s", StateToString(GetState()), 0, 0);

    if (GetState() == STATE_TERMINATED)
    {
        IMS_TRACE_E(0, "Response message will not be handled in the TERMINATED state", 0, 0, 0);

        if (piScc->GetMethod().Equals(SipMethod::INVITE) &&
                (piScc->GetStatusCode() >= SipStatusCode::SC_200))
        {
            RestoreOfferAnswerState();

            SendRequestToAck(piScc, IMessage::SERVICEMETHOD_INVALID);
        }
        return;
    }

    // UAC: 2XX-INVITE retransmission received.
    if ((GetCallState() == CallState::STATE_INVITE_2XX_RECEIVED ||
                GetCallState() == CallState::STATE_REINVITE_2XX_RECEIVED) &&
            piScc->GetMethod().Equals(SipMethod::INVITE) &&
            SipStatusCode::IsFinalSuccess(piScc->GetStatusCode()))
    {
        IMS_TRACE_I("UAC: 2XX-INVITE RETRANSMISSION", 0, 0, 0);
        piScc->RetransmitAck();
        return;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Update the call state
    UpdateCallStateOnMessageReceived(piSipMsg);

    // Handle the response according to the SIP method.
    const SipMethod& objMethod = piSipMsg->GetMethod();

    // Update a session refresh timer
    if (m_pRefreshHelper->UpdateOnMessageReceived(piScc) != SessionRefreshHelper::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Updating a session refresh timer failed", 0, 0, 0);
    }

    // Update the Offer/Answer state
    if (objMethod.Equals(SipMethod::PRACK) &&
            (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
    {
        // no-op :: ignore PRACK response if SDP OA state is in ESTABLISHED
        IMS_TRACE_D("Session :: PRACK response doesn't participate in SDP OAE", 0, 0, 0);
    }
    else
    {
        if (IsConfigurationSet(CONFIG_SUPPORT_EARLY_SESSION_MODEL) &&
                (GetState() == STATE_NEGOTIATING) &&
                (piSipMsg->GetStatusCode() == SipStatusCode::SC_183) &&
                OperatorFeatureResolver::IsMessageForEarlySessionModel(piSipMsg))
        {
            if (m_pVirtualEarlySession.IsNull())
            {
                m_pVirtualEarlySession = new VirtualSession(GetService(), GetUserAor());
            }

            m_pVirtualEarlySession->Notify18xResponse(piSipMsg);
        }
        else
        {
            if ((GetCallState() == CallState::STATE_INVITE_2XX_RECEIVED) ||
                    (piSipMsg->GetSdpBodyPart() != IMS_NULL) ||
                    (piSipMsg->GetStatusCode() == SipStatusCode::SC_180))
            {
                // KT
                // 183 (to-tag1) -> 183 (early-session, to-tag2) -> 200-INVITE (to-tag2)
                // 183 (early-session, to-tag1) -> 183 (forking, to-tag2) -> 200-INVITE (to-tag3)
                m_pVirtualEarlySession = IMS_NULL;
            }

            UpdateOfferAnswerStateOnMessageReceived(piSipMsg);
        }
    }

    switch (objMethod.ToInt())
    {
        // ACK request will be sent inside of HandleResponseToInvite() method.
        case SipMethod::INVITE:
            HandleResponseToInvite(piScc);
            break;
        case SipMethod::BYE:
            // 3 check if the same connection exists or not
            HandleResponseToBye(piScc);
            break;
        case SipMethod::CANCEL:
            HandleResponseToCancel(piScc);
            break;
        case SipMethod::UPDATE:
            if (piScc != GetClientConnection(IMessage::SESSION_UPDATE))
            {
                // Do nothing ...
                break;
            }

            HandleResponseToUpdate(piScc);
            break;
        case SipMethod::PRACK:
            break;
        default:
            IMS_TRACE_E(0, "Not handled method (%s)", objMethod.ToString().GetStr(), 0, 0);
            break;
    }
}

PROTECTED VIRTUAL void Session::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    switch (objMethod.ToInt())
    {
        case SipMethod::INVITE:
        {
            IMS_SINT32 nOldState = GetState();

            if ((nOldState == STATE_NEGOTIATING) ||
                    (nOldState == STATE_ESTABLISHING) /*ACK wait timeout*/)
            {
                CloseConnection(IMessage::SESSION_START);

                CleanupMedia();

                // Cease the 2xx retransmission
                Stop2xxRetransmission();

                if (m_bTerminatePending)
                {
                    m_bTerminatePending = IMS_FALSE;

                    if (nOldState == STATE_NEGOTIATING)
                    {
                        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                    }
                    else
                    {
                        // ACK wait timer expired ...
                        if (SendRequestToBye() != IMS_SUCCESS)
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
                    if (IsMobileOriginated() && (nOldState == STATE_NEGOTIATING))
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
            else if ((nOldState == STATE_RENEGOTIATING) ||
                    (nOldState == STATE_REESTABLISHING) /*ACK wait timeout*/)
            {
                // CALLER_PREFERENCE_MANAGER
                if (nOldState == STATE_RENEGOTIATING)
                {
                    const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

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

                if (m_bTerminatePending)
                {
                    m_bTerminatePending = IMS_FALSE;

                    if (SendRequestToBye() != IMS_SUCCESS)
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
                        SetSessionUpdateNotificationInProgress(IMS_TRUE);
                    }

                    // It contains the incoming INVITE case when the error is occurred
                    // in the SIP transport layer
                    PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
                }
            }
            break;
        }
        case SipMethod::BYE:
            if (piSc == m_piSccBye)
            {
                if (m_piSccBye != IMS_NULL)
                {
                    m_piSccBye->Close();
                    m_piSccBye = IMS_NULL;
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
        case SipMethod::CANCEL:
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
                        const IMessage* piRequest = GetPreviousRequest(IMessage::SESSION_CANCEL);

                        if (piRequest != IMS_NULL)
                        {
                            const IMessage* piResponse =
                                    GetPreviousResponse(IMessage::SESSION_CANCEL);

                            if ((piResponse == IMS_NULL) ||
                                    !SipStatusCode::IsFinal(piResponse->GetStatusCode()))
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
        case SipMethod::UPDATE:
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
            SetSessionUpdateNotificationInProgress(IMS_TRUE);

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            break;
        case SipMethod::ACK:  // FALL-THROUGH
        case SipMethod::PRACK:
            // If the transaction timer of PRACK is expired before the call is established,
            // it SHOULD be notified to the application and the application SHOULD terminate the
            // call. (RFC 3261 recommendation)
            break;
        default:
            IMS_TRACE_E(0, "Not handled method (%s)", objMethod.ToString().GetStr(), 0, 0);
            piSc->Close();
            break;
    }
}

PROTECTED VIRTUAL IMS_BOOL Session::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // For session refresh case
    if (piScc == m_pRefreshHelper->GetConnection())
    {
        if (m_pRefreshHelper->SendRefreshRequest(piScc) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piScc->GetMessage());

        return IMS_TRUE;
    }

    IMS_SINT32 nServiceMethod;

    if (GetState() == STATE_NEGOTIATING)
    {
        nServiceMethod = IMessage::SESSION_START;
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
    }

    // Try to send an INVITE request to the network

    // Clear the connection to preserve the SIP connection
    ClearConnection(nServiceMethod);

    if (!SendNUpdateRequestEx(nServiceMethod, piScc, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(nServiceMethod, piScc);
        return IMS_FALSE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piScc->GetMessage());

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());

    // Update the media state also.
    if (GetState() == STATE_NEGOTIATING)
    {
        UpdateMedia(Media::SESSION_START);
    }
    else
    {
        UpdateMedia(Media::SESSION_UPDATE);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Session::SetReferredMessageListener(
        IN IReferredMessageListener* piListener)
{
    m_piReferredMessageListener = piListener;
    return IMS_TRUE;
}

PROTECTED VIRTUAL const AString& Session::GetConnectionAddress() const
{
    if (m_pOaState == IMS_NULL)
    {
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    m_pOaState->GetSessionCurrentView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        m_pOaState->GetSessionProposalView(pSessionParam);
    }

    if (pSessionParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "No current or proposal view exists", 0, 0, 0);
        return AString::ConstNull();
    }

    return pSessionParam->GetConnectionAddress();
}

PROTECTED VIRTUAL IMS_SINT32 Session::GetSessionState() const
{
    if (m_nState == STATE_CREATED)
    {
        return SESSION_STATE_CREATED;
    }
    else if (m_nState == STATE_INITIATED)
    {
        return SESSION_STATE_INITIATED;
    }
    else if (m_nState == STATE_NEGOTIATING)
    {
        return SESSION_STATE_NEGOTIATING;
    }
    else if (m_nState == STATE_ESTABLISHING)
    {
        return SESSION_STATE_ESTABLISHING;
    }
    else if (m_nState == STATE_ESTABLISHED)
    {
        return SESSION_STATE_ESTABLISHED;
    }
    else if (m_nState == STATE_RENEGOTIATING)
    {
        return SESSION_STATE_RENEGOTIATING;
    }
    else if (m_nState == STATE_REESTABLISHING)
    {
        return SESSION_STATE_REESTABLISHING;
    }
    else if (m_nState == STATE_TERMINATING)
    {
        return SESSION_STATE_TERMINATING;
    }
    else if (m_nState == STATE_TERMINATED)
    {
        return SESSION_STATE_TERMINATED;
    }

    return (-1);
}

PROTECTED VIRTUAL SdpSessionParameter* Session::GetSessionParameter() const
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    if (m_nState == STATE_ESTABLISHED)
    {
        if (m_pOaState->GetSessionCurrentView(pSessionParam) != ISdpOaState::RESULT_SUCCESS)
        {
            return IMS_NULL;
        }
    }
    else
    {
        if (m_pOaState->GetSessionProposalView(pSessionParam) != ISdpOaState::RESULT_SUCCESS)
        {
            return IMS_NULL;
        }
    }

    return pSessionParam;
}

PROTECTED VIRTUAL const AString& Session::GetPeerConnectionAddress() const
{
    if (m_pOaState == IMS_NULL)
    {
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    m_pOaState->GetSessionPeerView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Getting a peer view failed", 0, 0, 0);
        return AString::ConstNull();
    }

    return pSessionParam->GetConnectionAddress();
}

PROTECTED VIRTUAL SdpSessionParameter* Session::GetPeerSessionParameter() const
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    m_pOaState->GetSessionPeerView(pSessionParam);

    return pSessionParam;
}

PROTECTED VIRTUAL SdpSessionParameter* Session::GetProposalSessionParameter()
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    m_pOaState->GetSessionProposalView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        // Checks and create a proposal view if it does not exist
        if (!m_pOaState->IsOfferProgress())
        {
            IMS_SINT32 nResult = m_pOaState->CreateProposalView();

            if ((nResult != ISdpOaState::RESULT_SUCCESS) &&
                    (nResult != ISdpOaState::RESULT_ALREADY_EXIST))
            {
                return IMS_NULL;
            }

            m_pOaState->GetSessionProposalView(pSessionParam);
        }
    }

    return pSessionParam;
}

PROTECTED VIRTUAL IMS_BOOL Session::Cancellable_Compare(IN ISipServerConnection* piSscCancel) const
{
    IMS_TRACE_I("CANCEL comparing ... Session (%d), Call (%d)", GetState(), GetCallState(), 0);

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!GetService()->IsImsConnected())
    {
        IMS_TRACE_E(0, "There is no registration", 0, 0, 0);
        return IMS_FALSE;
    }
#endif

    const ISipServerConnection* piSscInvite = IMS_NULL;
    IMS_SINT32 nState = GetState();

    if (nState == STATE_NEGOTIATING)
    {
        if (IsMobileOriginated())
        {
            IMS_TRACE_D("CANCEL is ignored ... (not mobile-terminated session)", 0, 0, 0);
            return IMS_FALSE;
        }

        piSscInvite = GetServerConnection(IMessage::SESSION_START);
    }
    else if (nState == STATE_RENEGOTIATING)
    {
        if (m_bUpdateRequestor)
        {
            IMS_TRACE_D("CANCEL is ignored ... (not update receiver)", 0, 0, 0);
            return IMS_FALSE;
        }

        piSscInvite = GetServerConnection(IMessage::SESSION_UPDATE);
    }
    // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
    else
    {
        // INVITE_TXN_HANDLING_CORRECTION
        if (nState == STATE_ESTABLISHING)
        {
            if (IsMobileOriginated())
            {
                IMS_TRACE_D("CANCEL is ignored ... (not mobile-terminated session) "
                            "in ESTABLISHING state",
                        0, 0, 0);
                return IMS_FALSE;
            }

            piSscInvite = GetServerConnection(IMessage::SESSION_START);
        }
    }

    if (piSscInvite == IMS_NULL)
    {
        IMS_TRACE_D("No INVITE server transaction in %s", StateToString(nState), 0, 0);
        return IMS_FALSE;
    }

    return piSscCancel->IsSameTransaction(piSscInvite);
}

PROTECTED VIRTUAL IMS_BOOL Session::Cancellable_NotifyRequest(IN ISipServerConnection* piSscCancel)
{
    const ISipMessage* piSipMsg = piSscCancel->GetMessage();

    if (!piSipMsg->GetMethod().Equals(SipMethod::CANCEL))
    {
        return IMS_FALSE;
    }

    // Checks if Request-URI is matched or not
    const AString& strRequestUri = piSscCancel->GetRequestUri();
    SipAddress objRequestUri(strRequestUri);

    if (!GetService()->ValidateRequestUri(objRequestUri))
    {
        IMS_BOOL bRUriMatched = IMS_FALSE;
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_START);
        const ISipMessage* piOrigSipMsg =
                (piMessage != IMS_NULL) ? piMessage->GetMessage() : IMS_NULL;

        if (piOrigSipMsg != IMS_NULL)
        {
            SipAddress objOrigRUri(piOrigSipMsg->GetRequestUri());
            bRUriMatched = objRequestUri.Equals(objOrigRUri);
        }

        if (!bRUriMatched)
        {
            IMS_TRACE_D("Request-URI (%s) in a CANCEL request is not matched",
                    SipDebug::GetUri1(strRequestUri).GetStr(), 0, 0);

            (void)CreateResponse(piSscCancel, SipStatusCode::SC_404);
            (void)piSscCancel->Send();
            piSscCancel->Close();
            return IMS_TRUE;
        }
    }

    IMS_SINT32 nCallState = GetCallState();

    IMS_TRACE_I(
            "CANCEL request received ... State : Session (%d), Call (%d)", m_nState, nCallState, 0);

    if ((nCallState != CallState::STATE_INVITE_RECEIVED) &&
            (nCallState != CallState::STATE_INVITE_1XX_SENT) &&
            (nCallState != CallState::STATE_REINVITE_RECEIVED) &&
            (nCallState != CallState::STATE_REINVITE_1XX_SENT))
    {
        IMS_TRACE_D("Ignore the CANCEL request and maintain the session state", 0, 0, 0);

        // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
        // RFC 5407, Appendix C.  UA's Behavior for CANCEL.
        // After receiving 200 OK for CANCEL request,
        // UAC will send BYE request to terminate this session.
        m_bSessionCanceledOnAccepted = IMS_TRUE;
        PostMessage(AMSG_SESSION_CANCELED_ON_ACCEPTED, 0, 0);

        (void)CreateResponse(piSscCancel, SipStatusCode::SC_200);
        (void)piSscCancel->Send();
        piSscCancel->Close();
        return IMS_TRUE;
    }

    // CALL_TERMINATION_AFTER_CANCEL_OF_REINVITE
    IMS_SINT32 nMethodForCancel = IMessage::SESSION_TERMINATE;

    if (GetState() >= STATE_ESTABLISHED)
    {
        nMethodForCancel = IMessage::SESSION_CANCEL;
    }

    if (!UpdateRequestOnReceived(nMethodForCancel, piSscCancel))
    {
        IMS_TRACE_E(0, "Updating SIP message failed", 0, 0, 0);
    }

    HandleRequestToCancel(piSscCancel);

    CloseConnection(nMethodForCancel);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Session::Dialog_Compare(IN ISipServerConnection* piSsc) const
{
    // Filters some method which does not handle in the session

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!GetService()->IsImsConnected())
    {
        return IMS_FALSE;
    }
#endif

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSsc->GetMethod();

    if (objMethod.Equals(SipMethod::REFER))
    {
        // If the server transaction has the same dialog identifier with a dialog of this session,
        // then it will be handled by this session.
        ISipDialog* piReferDialog = piSsc->GetDialog();

        if (piReferDialog == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strDialogId = piDialog->GetDialogId();
        AString strReferDialogId = piReferDialog->GetDialogId();

        if (strDialogId.Equals(strReferDialogId))
        {
            IMS_TRACE_D("Session :: Dialog (%s), Refer's Dialog (%s)",
                    SipDebug::GetCharA1(strDialogId.GetStr(), 8, '@'),
                    SipDebug::GetCharA2(strReferDialogId.GetStr(), 8, '@'), 0);
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }
    else if (objMethod.Equals(SipMethod::OPTIONS))
    {
        // 1  In this moment, OPTIONS request will not be handled by Session
        // return IMS_FALSE;
    }

    if (!piDialog->IsSameDialog(piSsc))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Session::Dialog_NotifyRequest(IN ISipServerConnection* piSsc)
{
    const ISipMessage* piSipMsg = piSsc->GetMessage();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    IMS_TRACE_I("Dialog_NotifyRequest: %s request received", objMethod.ToString().GetStr(), 0, 0);

    if (IsSessionUpdateNotificationInProgress() &&
            (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::UPDATE)) &&
            (piSipMsg->GetSdpBodyPart() != IMS_NULL))
    {
        IMS_TRACE_I("Processing of incoming request will be delayed", 0, 0, 0);
        PostMessage(AMSG_SESSION_DELAYED_DIALOG_TRANSACTION_RECEIVED, 0,
                reinterpret_cast<IMS_UINTP>(piSsc));
        return IMS_TRUE;
    }

    if (SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(
                GetSlotId(), GetService()->GetSipProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString& strRequestUri = piSsc->GetRequestUri();
        SipAddress objRequestUri(strRequestUri);

        if (!GetService()->ValidateRequestUri(objRequestUri, piSsc->GetDialog(), IMS_TRUE))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SipDebug::GetUri1(strRequestUri).GetStr(), 0, 0);

            // PATCH_SIP_DIALOG_TERMINATED_STATE
            if (objMethod.Equals(SipMethod::BYE))
            {
                UpdateRequestOnReceived(IMessage::SESSION_TERMINATE, piSsc);
            }

            if (!objMethod.Equals(SipMethod::ACK))
            {
                GetService()->SendResponse(piSsc, SipStatusCode::SC_404);
            }

            // PATCH_SIP_DIALOG_TERMINATED_STATE
            // The dialog state will be transited to "TERMINATED" state,
            // so notify the application that this session is terminated...
            if (piSsc->GetMethod().Equals(SipMethod::BYE))
            {
                UpdateResponseOnSent(IMessage::SESSION_TERMINATE, piSsc);

                m_bTerminatePending = IMS_FALSE;

                // Cease the 2xx retransmission
                Stop2xxRetransmission();
                CleanupMedia();

                // Clear the previous SIP connection
                ClearConnection(IMessage::SESSION_TERMINATE);

                SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }

            piSsc->Close();

            return IMS_FALSE;
        }
    }

    // Update the call state
    UpdateCallStateOnMessageReceived(piSipMsg);

    // Update a session refresh timer
    if (objMethod.Equals(SipMethod::INVITE) &&
            !SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
    {
        IMS_TRACE_D("re-INVITE request will not participate in the session refresh", 0, 0, 0);

        m_pRefreshHelper->UpdateTimerOptionOnRequestReceived(piSsc);
    }
    else
    {
        switch (m_pRefreshHelper->UpdateOnMessageReceived(piSsc))
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
        case SipMethod::INVITE:
            if (HandleRequestToInviteWithinDialog(piSsc) != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
        case SipMethod::ACK:
            HandleRequestToAck(piSsc);

            // When ACK request is received, the server connection has been created newly...
            // So, close the server connection directly.
            piSsc->Close();
            break;
        case SipMethod::BYE:
            if (HandleRequestToBye(piSsc) != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
        case SipMethod::INFO:
            if (m_piSessionListener == IMS_NULL)
            {
                IMS_TRACE_D("No session listener", 0, 0, 0);

                // 4 Find a proper response code
                GetService()->CreateResponse(piSsc, SipStatusCode::SC_480);
                piSsc->Send();
                piSsc->Close();
                break;
            }

            PostMessage(AMSG_SESSION_TRANSACTION_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSsc));
            break;
        case SipMethod::REFER:
            if (HandleRequestToRefer(piSsc) != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
        case SipMethod::OPTIONS:
            if (Capabilities::HandleOptionsRequestWithinDialog(GetService(), this, piSsc) !=
                    IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
        case SipMethod::UPDATE:
            if (HandleRequestToUpdate(piSsc) != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }
            break;
        case SipMethod::PRACK:
            // PRACK method will be handled by SessionEx class.
            break;
        default:
            // Unknown method received
            if (m_piSessionListener == IMS_NULL)
            {
                IMS_TRACE_D("No session listener", 0, 0, 0);

                // 4 Find a proper response code
                GetService()->CreateResponse(piSsc, SipStatusCode::SC_480);
                piSsc->Send();
                piSsc->Close();
                break;
            }

            PostMessage(AMSG_SESSION_TRANSACTION_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSsc));
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Session::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    IMS_TRACE_I("___ SESSION REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyCompleted(piScc);
    }

    // In case, the session refresh request is successfully done.
    if (nCode == 0)
    {
        const SipMethod& objMethod = piScc->GetMethod();
        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        // Update the call state
        UpdateCallStateOnMessageReceived(piScc->GetMessage());

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (objMethod.Equals(SipMethod::INVITE))
            {
                if (SendRequestToAck(piScc, IMessage::SESSION_UPDATE) != IMS_SUCCESS)
                {
                    // Clean up the session resources
                    // SetState(STATE_TERMINATED);
                    // CleanupMedia();
                    // PostEvent(AMSG_SESSION_TERMINATED, IMS_NULL);
                    return;
                }
            }

            IMS_TRACE_I("___ SESSION REFRESH IS SUCCESSFULLY DONE", 0, 0, 0);
        }
        else if ((nStatusCode == SipStatusCode::SC_408) || (nStatusCode == SipStatusCode::SC_481))
        {
            CleanupMedia();

            if (GetState() != STATE_ESTABLISHED)
            {
                IMS_TRACE_D("BYE can't be sent in the state (%s)", StateToString(GetState()), 0, 0);
                return;
            }

            if (nStatusCode == SipStatusCode::SC_408)
            {
                SetTerminationReason(TERMINATION_REASON_REFRESH_408);
            }
            else
            {
                SetTerminationReason(TERMINATION_REASON_REFRESH_481);
            }

            // Send BYE request
            if (SendRequestToByeInternal() != IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                return;
            }

            SetState(STATE_TERMINATING);
        }

        // AUTH_SIP_DIGEST {
        if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
        {
            // In case of other method except for REGISTER,
            // the UE only supports the authentication algorithm, MD5
            if (RespondToChallenge(piScc))
            {
                return;
            }
        }

        ResetChallengeCount(piScc);
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
        if (SendRequestToByeInternal() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

PROTECTED VIRTUAL IMS_BOOL Session::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    IMS_SINT32 nState = GetState();

    IMS_TRACE_I("___ SESSION REFRESH STARTED ... State(%d)", nState, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
    }

    IMS_BOOL bRefreshable = IMS_FALSE;

    if (nState == STATE_ESTABLISHED)
    {
        // Send a refresh request : UPDATE or re-INVITE
        bRefreshable = IMS_TRUE;
    }
    else
    {
        if (!SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
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

PROTECTED VIRTUAL void Session::Refreshable_RefreshTerminated()
{
    IMS_TRACE_D("___ SESSION REFRESH TERMINATED ...", 0, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyTerminated();
    }

    if (GetState() == STATE_ESTABLISHED)
    {
        CleanupMedia();

        SetTerminationReason(TERMINATION_REASON_REFRESH_TIMEOUT);

        if (SendRequestToByeInternal() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

PROTECTED VIRTUAL IMS_RESULT Session::ExecuteCmd()
{
    ISipServerConnection* piSscInvite = IMS_NULL;

    IMS_TRACE_D("RetransmissionTask :: RETRANSMIT", 0, 0, 0);

    if (GetState() == STATE_ESTABLISHING)
    {
        piSscInvite = GetServerConnection(IMessage::SESSION_START);
    }
    else
    {
        piSscInvite = GetServerConnection(IMessage::SESSION_UPDATE);
    }

    if (piSscInvite == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (piSscInvite->Send() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_pRefreshHelper->UpdateOnMessageSent(piSscInvite);

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void Session::RetryTaskHelper_OnCompleted(
        IN RetryTaskHelper* pTaskHelper, IN RetryCmd* pCmd, IN IMS_SINT32 nCode /*= 0*/)
{
    (void)pCmd;

    if (m_pRetransmissionTask == IMS_NULL)
    {
        IMS_TRACE_E(0, "Stray notification received", 0, 0, 0);
        return;
    }

    if (m_pRetransmissionTask != pTaskHelper)
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

                if (m_bTerminatePending)
                {
                    m_bTerminatePending = IMS_FALSE;

                    // ACK wait timer expired ...
                    if (SendRequestToBye() != IMS_SUCCESS)
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

                if (m_bTerminatePending)
                {
                    m_bTerminatePending = IMS_FALSE;

                    // ACK wait timer expired ...
                    if (SendRequestToBye() != IMS_SUCCESS)
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

                if (m_bTerminatePending)
                {
                    m_bTerminatePending = IMS_FALSE;

                    // ACK wait timer expired ...
                    if (SendRequestToBye() != IMS_SUCCESS)
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

                if (m_bTerminatePending)
                {
                    m_bTerminatePending = IMS_FALSE;

                    // ACK wait timer expired ...
                    if (SendRequestToBye() != IMS_SUCCESS)
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

PROTECTED VIRTUAL Session* Session::CreateSession()
{
    Session* pSession = new Session(GetService());

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

PROTECTED VIRTUAL SessionRefreshHelper* Session::CreateRefreshHelper()
{
    return new SessionRefreshHelper(GetService(), this);
}

PROTECTED VIRTUAL IMS_RESULT Session::HandleProvisionalResponse(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nServiceMethod)
{
    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    if (nStatusCode == SipStatusCode::SC_100)
    {
        if (IsConfigurationSet(CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED))
        {
            // INDEX_FOR_PROVISIONAL_RESPONSE_MESSAGE
            ImsList<Message*> objResponses = GetPreviousResponses(nServiceMethod);

            if (!objResponses.IsEmpty())
            {
                IMS_TRACE_D("100 Trying is received - handled by the application", 0, 0, 0);
                PostMessage(
                        AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED, 0, objResponses.GetSize() - 1);
            }
        }
        else
        {
            IMS_TRACE_D("100 Trying is received - ignored", 0, 0, 0);
        }

        return IMS_SUCCESS;
    }

    if (m_bSdpNonRprAllowed || IsConfigurationSet(CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR))
    {
        // Check & create a session descriptor when an initial offer received
        CheckNCreateSessionDescriptor();

        IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piScc->GetMessage());

        if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT) ||
                (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND))
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
        PostMessage(AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED, 0, objResponses.GetSize() - 1);
    }

    if (nStatusCode == SipStatusCode::SC_180)
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

PROTECTED VIRTUAL IMS_RESULT Session::HandleRequestToUpdate(IN ISipServerConnection* piSsc)
{
    // 4 Check if we have sent a session refresh request
    const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

    if (piMessage != IMS_NULL)
    {
        if (piMessage->GetMethod().Equals(SipMethod::INVITE))
        {
            const IMessage* piResponse = GetPreviousResponse(IMessage::SESSION_UPDATE);

            // 140818, CONDITION_ACK_WAITING_STATE
            // It needs to properly handle UPDATE request(session refresh) before receiving ACK.
            if ((piResponse == IMS_NULL) || (piResponse->GetStatusCode() < SipStatusCode::SC_200) ||
                    (GetState() == STATE_REESTABLISHING))
            {
                // 4 check SDP presentity
                IMS_TRACE_D("re-INVITE transaction is in progress...; do implicitly answer "
                            "to the UPDATE request",
                        0, 0, 0);

                if (SendResponseToRefreshUpdate(piSsc) != IMS_SUCCESS)
                {
                    piSsc->Close();
                    return IMS_FAILURE;
                }

                piSsc->Close();
                return IMS_SUCCESS;
            }
        }
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSsc->GetMessage());

    IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSsc->GetMessage());

    if ((nOaResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOaResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (GetService()->SendResponse(piSsc, SipStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
        }

        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        RestoreOfferAnswerState();

        piSsc->Close();

        return IMS_SUCCESS;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (GetService()->CreateResponse(piSsc, SipStatusCode::SC_606) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);

            RestoreOfferAnswerState();
            return IMS_FAILURE;
        }

        (void)piSsc->GetMessage()->SetHeader(ISipHeader::WARNING, WARNING_304);
        (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSsc->Send() != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
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

    // RACE_CONDITION : SESSION_UPDATE
    if (IsSessionUpdateNotificationInProgress())
    {
        CopyPreviousMessage(IMessage::SESSION_UPDATE, IMessage::SESSION_STALE_UPDATE);
    }

    UpdateRequestOnReceived(IMessage::SESSION_UPDATE, piSsc);

    // Update the media state
    UpdateMedia(Media::SESSION_UPDATE);

    SetState(STATE_RENEGOTIATING);

    PostMessage(AMSG_SESSION_UPDATE_RECEIVED, 0, 0);

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_RESULT Session::HandleResponseToUpdate(IN ISipClientConnection* piScc)
{
    IMS_SINT32 nStatusCode = piScc->GetMessage()->GetStatusCode();

    UpdateResponseOnReceived(IMessage::SESSION_UPDATE, piScc);

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

    m_bUpdateRequestor = IMS_FALSE;

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // CALLER_PREFERENCE_MANAGER
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

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
            UpdateMedia(Media::SESSION_UPDATED);

            if (GetState() == STATE_RENEGOTIATING)
            {
                SetState(STATE_ESTABLISHED);
                // RACE_CONDITION : SESSION_UPDATE
                SetSessionUpdateNotificationInProgress(IMS_TRUE);

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
                SetSessionUpdateNotificationInProgress(IMS_TRUE);

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
            SetSessionUpdateNotificationInProgress(IMS_TRUE);

            PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
        }
    }

    CloseConnection(IMessage::SESSION_UPDATE);

    return IMS_SUCCESS;
}

PROTECTED
IMS_BOOL Session::AddRefreshSpecificHeaders(IN ISipConnection* piSc)
{
    if (m_pRefreshHelper == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return m_pRefreshHelper->AddSpecificHeader(piSc);
}

PROTECTED
IMS_BOOL Session::CheckNSetListenerCall(IN IMS_SINT32 nListenerCall)
{
    if ((m_nCompletedListenerCalls & nListenerCall) != nListenerCall)
    {
        m_nCompletedListenerCalls |= nListenerCall;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL Session::CheckNCreateSessionDescriptor()
{
    // Case 1) Initial offer from the local user agent
    if (GetState() == STATE_INITIATED)
    {
        if (m_pSessionDescriptor != IMS_NULL)
        {
            IMS_TRACE_D("SessionDescriptor already exists", 0, 0, 0);
            return IMS_TRUE;
        }

        // Create a media capabilities for this service & session
        if (!m_pOaState->CreateCapabilities(GetService()))
        {
            IMS_TRACE_E(0, "Creating SDP capabilities failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Create a new offer with only SDP session parameter
        // The media parameters are created when CreateMedia() method invoked
        if (!m_pOaState->InitiateOffer(SdpOaState::OFFER_NEW))
        {
            IMS_TRACE_E(0, "Initiating SDP offer failed", 0, 0, 0);
            return IMS_FALSE;
        }

        m_pSessionDescriptor = new SessionDescriptor(this);

        if (m_pSessionDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SessionDescriptor failed", 0, 0, 0);
            return IMS_FALSE;
        }

        IMS_TRACE_D("___ SessionDescriptor is created in INITIATED state ___", 0, 0, 0);
    }
    // Case 2) Initial offer from the peer user agent
    else
    {
        if (m_pSessionDescriptor != IMS_NULL)
        {
            IMS_TRACE_D("SessionDescriptor already exists", 0, 0, 0);
            return IMS_TRUE;
        }

        IMS_SINT32 nOaState = GetOfferAnswerState();

        // IDLE :: incoming INVITE w/o SDP
        // OFFER_RECEIVED :: incoming INVITE w/ SDP
        if ((nOaState != SdpOaState::STATE_IDLE) && (nOaState != SdpOaState::STATE_OFFER_RECEIVED))
        {
            IMS_TRACE_E(0, "__ SessionDescriptor can't be created in offer/answer state (%d) __",
                    nOaState, 0, 0);
            return IMS_FALSE;
        }

        // Create a media capabilities for this service & session
        if (!m_pOaState->CreateCapabilities(GetService()))
        {
            IMS_TRACE_E(0, "Creating SDP capabilities failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Create a new offer with only SDP session parameter
        // The media parameters are created when CreateMedia() method invoked
        if (nOaState == SdpOaState::STATE_IDLE)
        {
            if (!m_pOaState->InitiateOffer(SdpOaState::OFFER_NEW))
            {
                IMS_TRACE_E(0, "Initiating SDP offer (MT) failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        m_pSessionDescriptor = new SessionDescriptor(this);

        if (m_pSessionDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SessionDescriptor failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (nOaState == SdpOaState::STATE_IDLE)
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

PROTECTED
IMS_BOOL Session::CheckNSetSdpBodyPart(IN_OUT ISipMessage*& piSipMsg)
{
    if (m_objMedias.IsEmpty())
    {
        IMS_TRACE_D("There is no media", 0, 0, 0);
        return IMS_TRUE;
    }

    // SDP message to be set
    AString strSdp;

    if (!m_pOaState->GetSdp(strSdp))
    {
        IMS_TRACE_D("There is no SDP message body", 0, 0, 0);
        return IMS_TRUE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMsg->CreateSdpBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSdp(strSdp);

    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, Sip::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSdp);

    // Set the Content-Length header
    AString strCLen;
    strCLen.SetNumber(objSdp.GetLength());

    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_UNKNOWN, strCLen, SipHeaderName::CONTENT_LENGTH);

    IMS_TRACE_D("SDP is formed by SDP offer/answer context", 0, 0, 0);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL Session::CheckNTerminateSession(IN const ISipMessage* piSipMsg)
{
    if (!m_bTerminatePending)
    {
        IMS_TRACE_D("No pending terminate()", 0, 0, 0);
        return IMS_FALSE;
    }

    // Reset the flag
    m_bTerminatePending = IMS_FALSE;

    if (IsMobileOriginated() && (GetState() == STATE_NEGOTIATING))
    {
        // Check the SIP status code to terminate the session
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (SipStatusCode::Is1XX(nStatusCode))
        {
            CleanupMedia();

            if (m_bTerminateMethodBye)
            {
                // Send BYE
                if (SendRequestToBye() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                    return IMS_TRUE;
                }
            }
            else
            {
                // Send CANCEL
                if (SendRequestToCancel() != IMS_SUCCESS)
                {
                    SetState(STATE_TERMINATED);

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
                    return IMS_TRUE;
                }
            }

            SetState(STATE_TERMINATING);
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
#ifdef __IMS_SEND_ACK_IN_TERMINATING_STATE__
            // Send ACK
            ISipClientConnection* piScc = GetClientConnection(IMessage::SESSION_START);

            if ((piScc != IMS_NULL) && (piSipMsg->GetMethod().Equals(SipMethod::INVITE)))
            {
                // Check & create a session descriptor when an initial offer received
                CheckNCreateSessionDescriptor();

                IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSipMsg);

                if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                        (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                        (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT) ||
                        (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND))
                {
                    if (m_pOaState->IsSessionChanged())
                    {
                        if ((GetOfferAnswerState() == SdpOaState::STATE_IDLE) ||
                                (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
                        {
                            m_pOaState->CompleteExchange();
                        }
                    }
                }
                else
                {
                    RestoreOfferAnswerState();
                }

                SendRequestToAck(piScc, IMessage::SESSION_START);
            }
#endif  // __IMS_SEND_ACK_IN_TERMINATING_STATE__

            CleanupMedia();

            // Send BYE
            if (SendRequestToBye() != IMS_SUCCESS)
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
        if (SendRequestToBye() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            return IMS_TRUE;
        }

        SetState(STATE_TERMINATING);
    }

    return IMS_TRUE;
}

PROTECTED
ISipClientConnection* Session::CreateConnectionL(
        IN ISipDialog* piDialog, IN const SipMethod& objMethod)
{
    ISipClientConnection* piScc = CreateConnection(piDialog, objMethod);

    if (piScc != IMS_NULL)
    {
        // IMPLICIT_ROUTING_FOR_MID_DIALOG
        SetImplicitRouteHeader(piScc);
    }

    return piScc;
}

PROTECTED
IMS_SINT32 Session::GetOfferAnswerState() const
{
    if (m_pOaState == IMS_NULL)
    {
        return SdpOaState::STATE_IDLE;
    }

    return m_pOaState->GetState();
}

PROTECTED
IMS_SINT32 Session::HandleSdpOfferAnswer(IN const ISipMessage* piSipMsg)
{
    if (m_pOaState == IMS_NULL)
    {
        return SdpOfferAnswer::RESULT_NOT_CHANGED;
    }

    IMS_SINT32 nOaResult = m_pOaState->HandleOfferAnswer(piSipMsg);

    // If incoming SDP offer contains "qos" attributes and precondition is not supported,
    // then the corresponding SDP attributes will be removed.
    if (!SdpProfile::GetInstance()->IsAttributePreconditionSupported(GetSlotId()))
    {
        IMS_SINT32 nOaState = m_pOaState->GetState();

        if (((nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                    (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)) &&
                ((nOaState == SdpOaState::STATE_OFFER_RECEIVED) ||
                        (nOaState == SdpOaState::STATE_OFFER_CHANGE_RECEIVED)))
        {
            const SessionParameter* pSessionParam = m_pOaState->GetProposalView();

            if (pSessionParam != IMS_NULL)
            {
                const ImsList<SdpMediaParameter*>& objMediaParams =
                        pSessionParam->GetMediaParameters();

                for (IMS_UINT32 i = 0; i < objMediaParams.GetSize(); i++)
                {
                    SdpMediaParameter* pMediaParam = objMediaParams.GetAt(i);

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

    return nOaResult;
}

PROTECTED
IMS_BOOL Session::IsInviteFinalResponseReceived(IN IMS_SINT32 nServiceMethod) const
{
    const Message* pMessage = GetPreviousResponse(nServiceMethod);
    const ISipMessage* piSipMsg = (pMessage != IMS_NULL) ? pMessage->GetMessage() : IMS_NULL;

    return (piSipMsg != IMS_NULL) && piSipMsg->GetMethod().Equals(SipMethod::INVITE) &&
            (piSipMsg->GetStatusCode() >= SipStatusCode::SC_200);
}

PROTECTED
IMS_BOOL Session::IsMidDialogTransactionCreatable() const
{
    IMS_SINT32 nState = GetState();

    if ((nState == STATE_ESTABLISHED) || (nState == STATE_RENEGOTIATING) ||
            (nState == STATE_REESTABLISHING))
    {
        return IMS_TRUE;
    }
    else if ((nState == STATE_ESTABLISHING) && !IsMobileOriginated())
    {
        IMS_SINT32 nCallState = GetCallState();

        if ((nCallState == CallState::STATE_INVITE_2XX_SENT) ||
                (nCallState == CallState::STATE_ESTABLISHED))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED
void Session::NotifyAlerting()
{
    if (GetState() == STATE_NEGOTIATING)
    {
        if (!m_bAlerting)
        {
            m_bAlerting = IMS_TRUE;

            IMS_TRACE_D("Session :: Alerting ...", 0, 0, 0);

            PostMessage(AMSG_SESSION_ALERTING, 0, 0);
        }
    }
}

PROTECTED
void Session::RestoreEx()
{
    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        Media* pMedia = m_objMedias.GetAt(i);

        if (pMedia->GetState() == Media::STATE_INACTIVE)
        {
            // Delete the media from the list ?????
        }

        // 4 Add the code to restore the medias
        pMedia->RestoreMedia();
    }
}

PROTECTED
IMS_RESULT Session::SendResponseToRefreshUpdate(IN ISipServerConnection* piSsc)
{
    if (!CreateResponse(piSsc, SipStatusCode::SC_200))
    {
        IMS_TRACE_E(0, "Creating a response failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Session-Expires
    // Require
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (!m_pRefreshHelper->AddSpecificHeader(piSsc))
        {
            IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

    if (piSsc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a response to UPDATE request (session refresh) failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->UpdateOnMessageSent(piSsc);
    }

    return IMS_SUCCESS;
}

PROTECTED
IMS_BOOL Session::SetSdpBodyPartFromCurrentView(IN_OUT ISipMessage*& piSipMsg)
{
    if (m_pOaState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SdpOaState is null", 0, 0, 0);
        return IMS_FALSE;
    }

    SessionParameter* pCurrentView = m_pOaState->GetCurrentView();

    if (pCurrentView == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no current view", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nSdpVersionIncrement;
    const SipConfigV* pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        nSdpVersionIncrement = pSipConfigV->GetSessionRefreshSdpVersionIncrement();
    }
    else
    {
        nSdpVersionIncrement = SipConfigV::SESSION_REFRESH_SDP_VERSION_INCREMENT_NONE;
    }

    if (!pCurrentView->IsLastSdpProvidedWithNegotiatedSdp())
    {
        pCurrentView->SetLastSdpProvidedWithNegotiatedSdp(IMS_TRUE);
        m_pOaState->IncreaseSessionVersion();
    }
    else if ((piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            ((nSdpVersionIncrement ==
                     SipConfigV::SESSION_REFRESH_SDP_VERSION_INCREMENT_AS_OFFERER) ||
                    (nSdpVersionIncrement ==
                            SipConfigV::SESSION_REFRESH_SDP_VERSION_INCREMENT_ALL)))
    {
        m_pOaState->IncreaseSessionVersion();
    }

    ISipMessageBodyPart* piBodyPart = piSipMsg->CreateSdpBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSdp(pCurrentView->ToSdp());

    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, Sip::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSdp);

    // Set the Content-Length header
    AString strCLen;
    strCLen.SetNumber(objSdp.GetLength());

    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_UNKNOWN, strCLen, SipHeaderName::CONTENT_LENGTH);

    IMS_TRACE_D("SDP is formed by the current view", 0, 0, 0);

    return IMS_TRUE;
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE
PROTECTED
IMS_BOOL Session::SetSdpBodyPartFromRefusedView(IN_OUT ISipMessage*& piSipMsg)
{
    const SessionParameter* pRefusedView =
            (m_pOaState != IMS_NULL) ? m_pOaState->GetRefusedView() : IMS_NULL;

    if (pRefusedView == IMS_NULL)
    {
        // No operations if not present
        return IMS_TRUE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::BYE) &&
            !objMethod.Equals(SipMethod::CANCEL))
    {
        IMS_TRACE_D("Refused view is not allowed; method=%s", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMsg->CreateSdpBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSdp(pRefusedView->ToSdp());

    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, Sip::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSdp);

    // Set the Content-Length header
    AString strCLen;
    strCLen.SetNumber(objSdp.GetLength());

    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_UNKNOWN, strCLen, SipHeaderName::CONTENT_LENGTH);

    IMS_TRACE_I("SDP is formed by the refused view", 0, 0, 0);

    return IMS_TRUE;
}

PROTECTED
void Session::SetTerminationReason(IN IMS_SINT32 nReason)
{
    if (m_nTerminationReason != TERMINATION_REASON_UNKNOWN)
    {
        return;
    }

    m_nTerminationReason = nReason;

    switch (m_nTerminationReason)
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

PROTECTED
IMS_BOOL Session::UpdateMedia(IN IMS_SINT32 nTrigger)
{
    IMS_BOOL bResult = IMS_FALSE;

    if (!m_pOaState->IsSessionChanged())
    {
        IMS_TRACE_D("UpdateMedia :: No session changed", 0, 0, 0);
        return IMS_TRUE;
    }

    switch (GetOfferAnswerState())
    {
        case SdpOaState::STATE_OFFER_SENT:  // FALL-THROUGH
        case SdpOaState::STATE_OFFER_CHANGE_SENT:
            bResult = UpdateMediaOnOfferSent(nTrigger);
            break;
        case SdpOaState::STATE_ESTABLISHED:
            if (m_pOaState->GetMode() != SdpOaState::MODE_IDLE)
            {
                if (m_pOaState->GetMode() == SdpOaState::MODE_OFFERER)
                {
                    bResult = UpdateMediaOnAnswerReceived(nTrigger);
                }
                else
                {
                    bResult = UpdateMediaOnAnswerSent(nTrigger);
                }
            }
            else
            {
                IMS_TRACE_D("UpdateMedia :: SDP Offer/Answer is IDLE", 0, 0, 0);
            }
            break;
        case SdpOaState::STATE_OFFER_RECEIVED:
        case SdpOaState::STATE_OFFER_CHANGE_RECEIVED:
            bResult = UpdateMediaOnOfferReceived(nTrigger);
            break;
        default:
            IMS_TRACE_D("UpdateMedia :: NOT HANDLED", 0, 0, 0);
            return IMS_FALSE;
    }

    // Move the proposed view to the current view
    if ((GetOfferAnswerState() == SdpOaState::STATE_IDLE) ||
            (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
    {
        m_pOaState->CompleteExchange();
    }

    return bResult;
}

PROTECTED
IMS_BOOL Session::RestoreOfferAnswerState()
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pOaState->RestoreState();
}

PROTECTED
IMS_BOOL Session::UpdateOfferAnswerStateOnMessageReceived(IN const ISipMessage* piSipMsg)
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = GetState();
    IMS_BOOL bIsCallEstablished = IMS_FALSE;
    IMS_BOOL bIgnoreSubsequentSdpAnswerInPreviewMode =
            IsConfigurationSet(CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE) ||
            !m_pOaState->IsInPreviewMode();

    // RFC 6337: Section 3.1.1
    // After the UAC has received the answer in a reliable provisional response to the INVITE,
    //[RFC3261] requires that any SDP in subsequent responses be ignored.
    if (IsConfigurationSet(CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE) &&
            bIgnoreSubsequentSdpAnswerInPreviewMode)
    {
        if ((piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE) &&
                piSipMsg->GetMethod().Equals(SipMethod::INVITE))
        {
            if ((nState == STATE_NEGOTIATING) &&
                    (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
            {
                IMS_TRACE_D("UpdateOfferAnswerStateOnMessageReceived() :: "
                            "Ignore the SDP in subsequent RPR as offer-answer is completed",
                        0, 0, 0);

                m_pOaState->UpdateStateOnTransactionCompleted(
                        piSipMsg, SdpOaState::MESSAGE_RECEIVED);
                return IMS_TRUE;
            }
        }
    }

    if ((nState == STATE_ESTABLISHED) || (nState == STATE_RENEGOTIATING) ||
            (nState == STATE_REESTABLISHING))
    {
        bIsCallEstablished = IMS_TRUE;
    }

    return m_pOaState->UpdateState(piSipMsg, SdpOaState::MESSAGE_RECEIVED, bIsCallEstablished,
            (m_bSdpNonRprAllowed || IsConfigurationSet(CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR)));
}

PROTECTED
IMS_BOOL Session::UpdateOfferAnswerStateOnMessageSent(IN const ISipMessage* piSipMsg)
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = GetState();
    IMS_BOOL bIsCallEstablished = IMS_FALSE;

    if ((nState == STATE_ESTABLISHED) || (nState == STATE_RENEGOTIATING) ||
            (nState == STATE_REESTABLISHING))
    {
        bIsCallEstablished = IMS_TRUE;
    }

    return m_pOaState->UpdateState(piSipMsg, SdpOaState::MESSAGE_SENT, bIsCallEstablished,
            (m_bSdpNonRprAllowed || IsConfigurationSet(CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR)));
}

PROTECTED
void Session::UpdateCallerPreference(
        IN const ISipMessage* piPrevSipMsg, IN IMS_SINT32 nStatusCode /*= SipStatusCode::SC_200*/)
{
    if (piPrevSipMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISipMessage is null", 0, 0, 0);
        return;
    }

    CallerPreferenceManager* pCallerPreferenceManager =
            ImsCoreContext::GetInstance()->GetCallerPreferenceManager();
    const SipMethod& objMethod = piPrevSipMsg->GetMethod();

    if (objMethod.Equals(SipMethod::INVITE))
    {
        IMS_SINT32 nState = GetState();

        if (nState == STATE_NEGOTIATING)
        {
            ISipDialog* piDialog = GetDialog();

            if ((piDialog != IMS_NULL) &&
                    ((piDialog->GetState() == ISipDialog::STATE_EARLY) ||
                            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED)))
            {
                pCallerPreferenceManager->UpdateDialogId(GetName(), piDialog->GetDialogId());
                pCallerPreferenceManager->UpdateAcceptContacts(
                        GetName(), piPrevSipMsg->GetHeaders(ISipHeader::ACCEPT_CONTACT));
            }
        }
        else if (nState == STATE_RENEGOTIATING)
        {
            if (SipStatusCode::IsProvisional(nStatusCode) ||
                    SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                pCallerPreferenceManager->UpdateAcceptContacts(
                        GetName(), piPrevSipMsg->GetHeaders(ISipHeader::ACCEPT_CONTACT));
            }
            else
            {
                // If the re-INVITE has been failed, restore the caller preference
                // to the previous one
                pCallerPreferenceManager->UpdateAcceptContacts(
                        GetName(), m_objPreviousCallerPreference);
            }
        }
    }
    else if (objMethod.Equals(SipMethod::UPDATE))
    {
        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            pCallerPreferenceManager->UpdateAcceptContacts(
                    GetName(), piPrevSipMsg->GetHeaders(ISipHeader::ACCEPT_CONTACT));
        }
    }
    else
    {
        IMS_TRACE_D("The method(%s) can't modify the caller preference",
                objMethod.ToString().GetStr(), 0, 0);
    }
}

PROTECTED
void Session::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Session :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PROTECTED
IMS_RESULT Session::SendRequestToByeInternal()
{
    SipMethod objMethod(SipMethod::BYE);
    ISipClientConnection* piScc = CreateConnectionL(GetDialog(), objMethod);

    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Set the headers for the session termination
    piScc->SetExtensionTokenForViaBranch(m_strTerminationReasonFromApp);

    // Update a session refresh timer info.
    m_pRefreshHelper->StopSessionTimer(piScc);

    if (m_piReasonHeaderSetter != IMS_NULL)
    {
        m_piReasonHeaderSetter->ReasonHeaderSetter_SetHeader(
                piScc->GetMessage(), m_nTerminationReason);
    }

    (void)AdjustMessage(piScc->GetMessage(), MESSAGE_CLASS_INTERNAL_BYE);

    // Try to send a BYE request to the network
    if (piScc->Send() != IMS_SUCCESS)
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piScc->GetMessage());

    if (m_piSccBye != IMS_NULL)
    {
        m_piSccBye->Close();
    }

    m_piSccBye = piScc;

    return IMS_SUCCESS;
}

// REMOVE_RECORD_ROUTE_HEADERS
PROTECTED GLOBAL void Session::RemoveRecordRouteHeaders(IN ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("Remove Record-Route headers in {%s, %d}",
            piSipMsg->GetMethod().ToString().GetStr(), piSipMsg->GetStatusCode(), 0);

    IMS_SINT32 nHCount = piSipMsg->GetHeaderCount(ISipHeader::RECORD_ROUTE);

    while (nHCount > 0)
    {
        piSipMsg->RemoveHeader(ISipHeader::RECORD_ROUTE);
        --nHCount;
    }
}

PRIVATE
void Session::AddSessionToCallControlHelper()
{
    // If the sessionId is already present, please remove the session from the CallControlHelper
    if (m_strSessionIdForCallControl.GetLength() > 0)
    {
        RemoveSessionFromCallControlHelper();
    }

    CallControlHelper* pCallControlHelper = ImsCoreContext::GetInstance()->GetCallControlHelper();

    m_strSessionIdForCallControl = pCallControlHelper->CreateSessionId();

    // Create a Replaces header info...
    Replaces* pReplaces = CallControlHelper::CreateReplaces(GetDialog(), IsMobileOriginated());

    pCallControlHelper->AddSession(m_strSessionIdForCallControl, pReplaces);

    IMS_TRACE_D("CallControlHelper: AddSession (%s)", m_strSessionIdForCallControl.GetStr(), 0, 0);
}

PRIVATE
void Session::RemoveSessionFromCallControlHelper()
{
    if (m_strSessionIdForCallControl.GetLength() <= 0)
    {
        IMS_TRACE_D("No session id for the 3rd-party call control", 0, 0, 0);
        return;
    }

    CallControlHelper* pCallControlHelper = ImsCoreContext::GetInstance()->GetCallControlHelper();
    pCallControlHelper->RemoveSession(m_strSessionIdForCallControl);

    IMS_TRACE_D("CallControlHelper: RemoveSession (%s), Count (%d)",
            m_strSessionIdForCallControl.GetStr(), pCallControlHelper->GetSessionCount(), 0);

    m_strSessionIdForCallControl = AString::ConstNull();
}

PRIVATE
void Session::CleanupOnDestroy()
{
    m_pVirtualEarlySession = IMS_NULL;

    // Close the SIP client connection for an internal BYE
    if (m_piSccBye != IMS_NULL)
    {
        m_piSccBye->Close();
        m_piSccBye = IMS_NULL;
    }

    // Checks if the session refresh is ongoing...
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (m_pRefreshHelper->IsRequestPending())
        {
            IMS_TRACE_D("Stopping the session refresh on CleanupOnDestroy()", 0, 0, 0);

            m_pRefreshHelper->AbortConnection();
        }
    }

    // Management of forked sessions
    if (!m_pForkedSessions.IsNull())
    {
        m_pForkedSessions->RemoveMethod(this);
        m_pForkedSessions = IMS_NULL;
    }

    // Guard code for Cancellable & Dialog
    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    // CALLER_PREFERENCE_MANAGER
    ImsCoreContext::GetInstance()->GetCallerPreferenceManager()->DestroyPreferenceWrapper(
            GetName());

    // Stop 2xx retransmission if it is running...
    Stop2xxRetransmission();

    // 'Replaces' header handling ...
    RemoveSessionFromCallControlHelper();
}

PRIVATE
IMS_RESULT Session::HandleRequestToAck(IN ISipServerConnection* piSsc)
{
    IMS_SINT32 nState = GetState();

    // UAS_REINVITE_RECEIVED_BEFORE_ACK
    // It's an optimization for the race condition handling between ACK and re-INVITE.
    if ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING))
    {
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if ((piMessage != IMS_NULL) && piMessage->GetMethod().Equals(SipMethod::INVITE) &&
                (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            // Don't process ACK request anymore if it doesn't match
            IMS_UINT32 nCSeqInvite = piMessage->GetMessage()->GetCSeqNumber();
            IMS_UINT32 nCSeqAck = piSsc->GetMessage()->GetCSeqNumber();

            if (nCSeqInvite != nCSeqAck)
            {
                IMS_TRACE_D("Stray ACK is received; CSeq-INVITE=%d, CSeq-ACK=%d", nCSeqInvite,
                        nCSeqAck, 0);
                return IMS_SUCCESS;
            }
        }
    }

    const ISipMessage* piSipMsg = piSsc->GetMessage();

    // CANCEL_HANDLING_AFTER_200_OK_TO_INVITE
    // INVITE_TXN_HANDLING_CORRECTION
    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    UpdateRequestOnReceived(IMessage::SESSION_ACK, piSsc);
    // ACK transaction will be closed by the caller
    ClearConnection(IMessage::SESSION_ACK);

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageReceived(piSipMsg);

    // Check the termination pending & do the termination procedure
    if (CheckNTerminateSession(piSipMsg))
    {
        return IMS_SUCCESS;
    }

    IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSipMsg);

    if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
            (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
            (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
    {
        UpdateMedia(Media::SESSION_UPDATED);
    }
    else
    {
        UpdateMedia(Media::SESSION_UPDATE_FAILED);
    }

    if ((nState == STATE_ESTABLISHING) || (nState == STATE_REESTABLISHING))
    {
        // Notify the session establishment to MEDIA

        // Cease the 2xx retransmission
        Stop2xxRetransmission();

        // Handle the ACK according to the state
        if (nState == STATE_ESTABLISHING)
        {
            // 'Replaces' header handling ...
            // RACE_CONDITION: ACK & re-INVITE in MT
            if (nOaResult != SdpOfferAnswer::RESULT_NOT_CHANGED)
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

            SetSessionUpdateNotificationInProgress(IMS_TRUE);
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
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if ((piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::INVITE) &&
                    piMessage->GetState() == IMessage::STATE_RECEIVED) ||
                ((m_nCompletedListenerCalls & LISTENER_CALL_STARTED) != 0))
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
            SetSessionUpdateNotificationInProgress(IMS_TRUE);
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

PRIVATE
IMS_RESULT Session::HandleRequestToBye(IN ISipServerConnection* piSsc)
{
    UpdateRequestOnReceived(IMessage::SESSION_TERMINATE, piSsc);

    if (Ims::GetLastError() == ImsError::ALREADY_EXISTS)
    {
        GetService()->SendResponse(piSsc, SipStatusCode::SC_200);
        piSsc->Close();

        return IMS_SUCCESS;
    }

    IMS_SINT32 nOldState = GetState();

    // RACE CONDITION : reset the terminate request flag
    if (m_bTerminatePending)
    {
        m_bTerminatePending = IMS_FALSE;

        IMS_TRACE_D("The pending terminate request will be reset", 0, 0, 0);
    }

    // Cease the 2xx retransmission
    Stop2xxRetransmission();

    SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

    SetState(STATE_TERMINATED);

    if (CreateResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to BYE request failed", 0, 0, 0);
        goto EXIT_HandleByeRequest;
    }

    if (!SendNUpdateResponse(IMessage::SESSION_TERMINATE, piSsc))
    {
        goto EXIT_HandleByeRequest;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSsc->GetMessage());

EXIT_HandleByeRequest:

    CleanupMedia();

    if (nOldState != STATE_TERMINATED)
    {
        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
    }

    CloseConnection(IMessage::SESSION_TERMINATE);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::HandleRequestToCancel(IN ISipServerConnection* piSsc)
{
    // RACE CONDITION : reset the terminate request flag
    if (m_bTerminatePending)
    {
        m_bTerminatePending = IMS_FALSE;

        IMS_TRACE_D("The pending terminate request will be reset", 0, 0, 0);
    }

    // Send a 200 OK to CANCEL request
    if (CreateResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
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

    if (!SendNUpdateResponse(nMethodForCancel, piSsc))
    {
        return IMS_FAILURE;
    }

    // Send a failure final response (487) to an INVITE request
    IMS_SINT32 nCallState = GetCallState();
    IMS_SINT32 nServiceMethod = IMessage::SERVICEMETHOD_INVALID;

    if ((nCallState == CallState::STATE_INVITE_1XX_SENT) ||
            (nCallState == CallState::STATE_INVITE_RECEIVED))
    {
        nServiceMethod = IMessage::SESSION_START;
    }
    else if ((nCallState == CallState::STATE_REINVITE_1XX_SENT) ||
            (nCallState == CallState::STATE_REINVITE_RECEIVED))
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
    }

    ISipServerConnection* piSscInvite = GetServerConnection(nServiceMethod);

    if (piSscInvite == IMS_NULL)
    {
        IMS_TRACE_E(0, "INVITE transaction does not exist", 0, 0, 0);

        SetTerminationReason(TERMINATION_REASON_REMOTE_ACTION);

        SetState(STATE_TERMINATED);
        CleanupMedia();
        PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SINT32 nState = GetState();

    if (CreateResponse(piSscInvite, SipStatusCode::SC_487) == IMS_FALSE)
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

    if (!SendNUpdateResponse(nServiceMethod, piSscInvite))
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
    UpdateCallStateOnMessageSent(piSscInvite->GetMessage());

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

PRIVATE
IMS_RESULT Session::HandleRequestToInvite(IN ISipServerConnection* piSsc)
{
    UpdateRequestOnReceived(IMessage::SESSION_START, piSsc);

    // Restore the media state if it needs

    // Check & create a session descriptor when an initial offer received
    if (GetOfferAnswerState() != SdpOaState::STATE_IDLE)
    {
        // Incoming INVITE w/ SDP
        CheckNCreateSessionDescriptor();
    }

    IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSsc->GetMessage());

    if ((nOaResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOaResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (CreateResponse(piSsc, SipStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());

        return IMS_SUCCESS;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_D("Rejecting SDP Offer/Answer with 606 (Not Acceptable) ...", 0, 0, 0);

        if (CreateResponse(piSsc, SipStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        (void)piSsc->GetMessage()->SetHeader(ISipHeader::WARNING, WARNING_304);

        if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());

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

    // Update the media state
    UpdateMedia(Media::SESSION_START);

    // According to the offer/answer, the failure final response can be sent.

    SetState(STATE_NEGOTIATING);

    PostMessage(AMSG_SESSION_INVITATION_RECEIVED, 0, 0);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::HandleRequestToInviteWithinDialog(IN ISipServerConnection* piSsc)
{
    IMS_SINT32 nState = GetState();

    if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
    {
        IMS_TRACE_D("re-INVITE is received in TERMINATING or TERMINATED state (%d)", nState, 0, 0);

        GetService()->SendResponse(piSsc, SipStatusCode::SC_488);
        piSsc->Close();
        return IMS_SUCCESS;
    }

    IMS_SINT32 nStatusCode = SipStatusCode::SC_INVALID;

    // Checks the message validity:
    // If INVITE/re-INVITE is sent,
    // the device needs to reject the message with 491 Request Pending response.
    if ((nState == STATE_NEGOTIATING) || (nState == STATE_RENEGOTIATING) ||
            m_pOaState->IsOfferProgress())
    {
        nStatusCode = SipStatusCode::SC_491;
    }
    // Offer is sent by 200 OK, and waits for ACK w/ SDP answer
    else if ((nState == STATE_REESTABLISHING) &&
            (GetOfferAnswerState() == SdpOaState::STATE_OFFER_CHANGE_SENT))
    {
        const ISipMessage* piSipMsg = piSsc->GetMessage();

        if ((piSipMsg != IMS_NULL) && (piSipMsg->GetSdpBodyPart() != IMS_NULL))
        {
            nStatusCode = SipStatusCode::SC_500;
        }
    }

    if (nStatusCode != SipStatusCode::SC_INVALID)
    {
        IMS_TRACE_I("Rejecting re-INVITE with %d ...", nStatusCode, 0, 0);

        if (GetService()->CreateResponse(piSsc, nStatusCode))
        {
            // Sets Retry-After header field
            AString strRetryAfter;

            strRetryAfter.SetNumber(SipHeaderUtils::GenerateRetryAfterSeconds(10));

            piSsc->GetMessage()->SetHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfter);

            piSsc->Send();

            // Update the call state
            UpdateCallStateOnMessageSent(piSsc->GetMessage());
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
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 400 (Bad Request) ...", 0, 0, 0);

        if (GetService()->SendResponse(piSsc, SipStatusCode::SC_400) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
        // Update the Offer/Answer state
        UpdateOfferAnswerStateOnMessageSent(piSsc->GetMessage());
        RestoreOfferAnswerState();

        piSsc->Close();

        return IMS_SUCCESS;
    }
    else if (nOaResult == SdpOfferAnswer::RESULT_NOT_FOUND)
    {
        IMS_TRACE_I("Rejecting SDP Offer/Answer with 488 (Not Acceptable Here)", 0, 0, 0);

        if (GetService()->CreateResponse(piSsc, SipStatusCode::SC_488) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Rejecting SDP Offer/Answer failed", 0, 0, 0);

            RestoreOfferAnswerState();
            return IMS_FAILURE;
        }

        (void)piSsc->GetMessage()->SetHeader(ISipHeader::WARNING, WARNING_304);
        (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

        if (piSsc->Send() != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Sending a response message failed", 0, 0, 0);
        }

        // Update the call state
        UpdateCallStateOnMessageSent(piSsc->GetMessage());
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
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_START);

        if ((piMessage != IMS_NULL) && (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            Stop2xxRetransmission();
            CloseConnection(IMessage::SESSION_START);

            SetState(STATE_ESTABLISHED);
            PostMessage(AMSG_SESSION_STARTED, 0, 0);
        }
    }
    else if (nState == STATE_REESTABLISHING)
    {
        const IMessage* piMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if ((piMessage != IMS_NULL) && piMessage->GetMethod().Equals(SipMethod::INVITE) &&
                (piMessage->GetState() == IMessage::STATE_RECEIVED))
        {
            Stop2xxRetransmission();
            CloseConnection(IMessage::SESSION_UPDATE);

            SetState(STATE_ESTABLISHED);
            PostMessage(AMSG_SESSION_UPDATED, 0, 0);
        }
    }

    UpdateRequestOnReceived(IMessage::SESSION_UPDATE, piSsc);

    // Update the media state
    UpdateMedia(Media::SESSION_UPDATE);

    // 3 PRACK / UPDATE

    // According to the offer/answer, the failure final response can be sent.

#if 0
    // Send 180 Ringing
    IMS_DEBUG_("Sending 180 \"Ringing\" ...");

    if (CreateResponse(piSsc, SipStatusCode::SC_180) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SipError (%d)",
                SipError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    if (!SendNUpdateResponse(IMessage::SESSION_START, piSsc))
    {
        IMS_TRACE_E(0, "Sending a response failed - SipError (%d)",
                SipError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    CheckNCreateDialog(piSsc);
#endif

    SetState(STATE_RENEGOTIATING);

    PostMessage(AMSG_SESSION_UPDATE_RECEIVED, 0, 0);

    // To cancel re-INVITE request when it is received in the mid-dialog
    CancellableMethodManager::GetInstance()->AddMethod(GetName(), this);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::HandleRequestToRefer(IN ISipServerConnection* piSsc)
{
    IMS_SINT32 nState = GetState();

    if ((nState == STATE_TERMINATING) || (nState == STATE_TERMINATED))
    {
        IMS_TRACE_D("REFER is received in TERMINATING or TERMINATED state (%d)", nState, 0, 0);

        GetService()->SendResponse(piSsc, SipStatusCode::SC_488);
        piSsc->Close();
        return IMS_SUCCESS;
    }

    const ISipMessage* piSipMsg = piSsc->GetMessage();

    if (piSipMsg->GetHeaderCount(ISipHeader::REFER_TO) != 1)
    {
        GetService()->SendResponse(
                piSsc, SipStatusCode::SC_400, AString("Mandatory Header Missing"));
        piSsc->Close();
        return IMS_FAILURE;
    }

    AString strReferTo = piSipMsg->GetHeader(ISipHeader::REFER_TO);
    SipAddress objReferTo;

    if (!objReferTo.Create(strReferTo))
    {
        GetService()->SendResponse(piSsc, SipStatusCode::SC_400, AString("Invalid Header Field"));
        piSsc->Close();
        return IMS_FAILURE;
    }

    const SipParameter* pMethodP = objReferTo.GetParameter(Sip::STR_METHOD);

    if (pMethodP == IMS_NULL)
    {
        GetService()->SendResponse(
                piSsc, SipStatusCode::SC_400, AString("Mandatory Parameter Missing"));
        piSsc->Close();
        return IMS_FAILURE;
    }

    const ISipHeader* piReplaces = objReferTo.GetHeader(ISipHeader::REPLACES);
    Replaces objReplaces;

    if (piReplaces != IMS_NULL)
    {
        objReplaces.Create(piReplaces->ToString());
    }

    Reference* pReference =
            new Reference(GetService(), objReferTo.GetUri(), pMethodP->GetValue(), objReplaces);

    if (pReference == IMS_NULL)
    {
        piSsc->Close();

        IMS_TRACE_E(0, "Creating Reference failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pReference->InitMethod(this, IMS_FALSE))
    {
        piSsc->Close();
        delete pReference;
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Initializing Reference failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pReference->ServerConnection_NotifyRequest(piSsc))
    {
        pReference->Destroy();
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Handling Reference failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    PostMessage(AMSG_SESSION_REFERENCE_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(pReference));

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::HandleResponseToBye(IN ISipClientConnection* piScc)
{
    IMS_SINT32 nOldState = GetState();
    IMS_SINT32 nStatusCode = piScc->GetMessage()->GetStatusCode();

    IMS_TRACE_I("Session - Got a %d response to BYE request", nStatusCode, 0, 0);

    if (piScc == m_piSccBye)
    {
        if (m_piSccBye != IMS_NULL)
        {
            m_piSccBye->Close();
            m_piSccBye = IMS_NULL;
        }

        SetState(STATE_TERMINATED);

        if (nOldState != STATE_TERMINATED)
        {
            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
        }

        return IMS_SUCCESS;
    }

    UpdateResponseOnReceived(IMessage::SESSION_TERMINATE, piScc);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return IMS_SUCCESS;
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
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

PRIVATE
IMS_RESULT Session::HandleResponseToCancel(IN const ISipClientConnection* piScc)
{
    IMS_SINT32 nStatusCode = piScc->GetMessage()->GetStatusCode();

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
            const IMessage* piRequest = GetPreviousRequest(IMessage::SESSION_CANCEL);

            if (piRequest != IMS_NULL)
            {
                const IMessage* piResponse = GetPreviousResponse(IMessage::SESSION_CANCEL);

                if ((piResponse == IMS_NULL) ||
                        !SipStatusCode::IsFinal(piResponse->GetStatusCode()))
                {
                    nMethodForCancel = IMessage::SESSION_CANCEL;
                }
            }
        }
    }

    UpdateResponseOnReceived(nMethodForCancel, piScc);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return IMS_SUCCESS;
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // The response of CANCEL request needs to be notified to the application
        // regardless of the current session state.
        // if (nMethodForCancel == IMessage::SESSION_CANCEL)
        PostMessage(AMSG_SESSION_CANCEL_DELIVERED, 0, 0);
    }
    else
    {
        // The response of CANCEL request needs to be notified to the application
        // regardless of the current session state.
        // if (nMethodForCancel == IMessage::SESSION_CANCEL)
        PostMessage(AMSG_SESSION_CANCEL_DELIVERY_FAILED, 0, 0);
    }

    CloseConnection(nMethodForCancel);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::HandleResponseToInvite(IN ISipClientConnection* piScc)
{
    const ISipMessage* piSipMsg = piScc->GetMessage();
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    if (SipStatusCode::IsProvisional(nStatusCode) || SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        CheckNCreateDialog(piScc);
    }

    IMS_SINT32 nState = GetState();
    IMS_SINT32 nServiceMethod = IMessage::SERVICEMETHOD_INVALID;

    if (nState == STATE_NEGOTIATING)
    {
        nServiceMethod = IMessage::SESSION_START;
    }
    else if (nState == STATE_RENEGOTIATING)
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
    }
    else if (nState == STATE_TERMINATING)
    {
        IMS_SINT32 nCallState = GetCallState();

        // STATE_IDLE : when receiving non-2xx response, J180 will send ACK automatically.
        if ((nCallState == CallState::STATE_IDLE) ||
                (nCallState == CallState::STATE_INVITE_1XX_RECEIVED) ||
                (nCallState == CallState::STATE_INVITE_2XX_RECEIVED) ||
                (nCallState == CallState::STATE_INVITE_NON2XX_RECEIVED))
        {
            nServiceMethod = IMessage::SESSION_START;
        }
        else
        {
            // 4 Check it
            nServiceMethod = IMessage::SESSION_UPDATE;
        }
    }

    // INVITE specific handling
    UpdateResponseOnReceived(nServiceMethod, piScc);

    // Check the termination pending & do the termination procedure
    if (CheckNTerminateSession(piSipMsg))
    {
        return IMS_SUCCESS;
    }

    // CALLER_PREFERENCE_MANAGER
    const IMessage* piMessage = GetPreviousRequest(nServiceMethod);

    if (piMessage != IMS_NULL)
    {
        UpdateCallerPreference(piMessage->GetMessage(), nStatusCode);
    }

    // Handle 1xx response first...
    if (SipStatusCode::Is1XX(nStatusCode))
    {
        IMS_RESULT nResult = HandleProvisionalResponse(piScc, nServiceMethod);

        if ((nState == STATE_NEGOTIATING) && SipStatusCode::IsProvisional(nStatusCode) &&
                (GetOfferAnswerState() == SdpOaState::STATE_ESTABLISHED))
        {
            // 'Replaces' header handling ...
            // For explicit call transfer in early dialog state
            AddSessionToCallControlHelperIfNotPresent();
        }

        return nResult;
    }

    // AUTH_SIP_DIGEST {
    // Handle 401/407 response
    if (((nState == STATE_NEGOTIATING) || (nState == STATE_RENEGOTIATING)) &&
            ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407)))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piScc))
        {
            return IMS_SUCCESS;
        }
    }
    // }

    m_bUpdateRequestor = IMS_FALSE;

    // Handle a final response according to the status code ...
    switch (nStatusCode)
    {
        case SipStatusCode::SC_200:
        {
            // Check & create a session descriptor when an initial offer received
            CheckNCreateSessionDescriptor();

            IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSipMsg);

            if ((nOaResult == SdpOfferAnswer::RESULT_NOT_CHANGED) ||
                    (nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                    (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
            {
                // Update the media state
                if (nState == STATE_NEGOTIATING)
                {
                    UpdateMedia(Media::SESSION_STARTED);
                }
                else if (nState == STATE_RENEGOTIATING)
                {
                    UpdateMedia(Media::SESSION_UPDATED);
                }
                else if (nState == STATE_TERMINATING)
                {
                    UpdateMedia(Media::SESSION_TERMINATED);
                }
                else
                {
                    RestoreOfferAnswerState();
                }

                if (nState == STATE_NEGOTIATING)
                {
                    // 'Replaces' header handling ...
                    if (nOaResult != SdpOfferAnswer::RESULT_NOT_CHANGED)
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
                    SetSessionUpdateNotificationInProgress(IMS_TRUE);

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
                SendRequestToAck(piScc, nServiceMethod);

                // Send BYE request if the session is in TERMINATING state
                if (nState == STATE_TERMINATING)
                {
                    // 4 Set Reason header
                    SetReasonHeaderFromPreviousRequest(IMessage::SESSION_TERMINATE);
                    SendRequestToBye();
                }
            }
            else
            {
                // Update the media state
                if (nState == STATE_NEGOTIATING)
                {
                    UpdateMedia(Media::SESSION_START_FAILED);
                }
                else if (nState == STATE_RENEGOTIATING)
                {
                    UpdateMedia(Media::SESSION_UPDATE_FAILED);
                }
                else if (nState == STATE_TERMINATING)
                {
                    UpdateMedia(Media::SESSION_TERMINATED);
                }

                // Send ACK to 2xx - INVITE
                SendRequestToAck(piScc, nServiceMethod);
                // piScc->Close();
                // SetInviteConnection(IMS_NULL);

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
                    SetSessionUpdateNotificationInProgress(IMS_TRUE);

                    PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
                }
                else if (nState == STATE_TERMINATING)
                {
                    SetState(STATE_TERMINATED);
                    CleanupMedia();

                    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                    // Send BYE request if the session is in TERMINATING state
                    // 4 Set Reason header
                    SetReasonHeaderFromPreviousRequest(IMessage::SESSION_TERMINATE);
                    SendRequestToBye();
                }
            }
            break;
        }
        case SipStatusCode::SC_422:
            if (nState == STATE_NEGOTIATING)
            {
                IMS_BOOL bMinSePresent = piSipMsg->IsHeaderPresent(ISipHeader::MIN_SE);

                CloseConnection(nServiceMethod);

                if (bMinSePresent)
                {
                    SetState(STATE_INITIATED);

                    if (SendRequestToInviteOn422Received() == IMS_SUCCESS)
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

                if (SendRequestToInviteOn422Received() == IMS_SUCCESS)
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
        default:
        {
            // 480 : INVITE is rejected by the proxy because the remote user is not registered
            // 486 : INVITE is rejected by the other side
            // 487 : This is the response to our cancelled INVITE, in the terminating session
            // 488 : INVITE is rejected because the media negotiation failed
            if (nState == STATE_NEGOTIATING)
            {
                CreateRemoteMediaCapabilities(piSipMsg);
                SetState(STATE_TERMINATED);
                CleanupMedia();
                ClearForkedSessionsByTerminated();

                PostMessage(AMSG_SESSION_START_FAILED, 0, 0);
            }
            else if (nState == STATE_RENEGOTIATING)
            {
                CreateRemoteMediaCapabilities(piSipMsg);
                UpdateMedia(Media::SESSION_UPDATE_FAILED);

                SetState(STATE_ESTABLISHED);
                // RACE_CONDITION : SESSION_UPDATE
                SetSessionUpdateNotificationInProgress(IMS_TRUE);
                RestoreOfferAnswerState();
                RestoreEx();

                PostMessage(AMSG_SESSION_UPDATE_FAILED, 0, 0);
            }
            // A cancelled INVITE
            else if (nState == STATE_TERMINATING)
            {
                SetState(STATE_TERMINATED);
                CleanupMedia();
                ClearForkedSessionsByTerminated();

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            }

            // 2XX response case except for 200
            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                SendRequestToAck(piScc, nServiceMethod);
            }
        }
        break;
    }

    CloseConnection(nServiceMethod);

    return IMS_SUCCESS;
}

PRIVATE
SipMethod Session::SelectUpdateMethod() const
{
    // 1. Select a re-INVITE if a media has been added
    // 2. Select a re-INVITE if an updated application-specific offer
    // 3. Select an UPDATE if any media was removed or direction was changed
    // 4. Select an UPDATE if the session refresh required
    if (GetState() != STATE_ESTABLISHED)
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return SipMethod();
    }

    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        const Media* pMedia = m_objMedias.GetAt(i);

        if ((pMedia->GetState() == Media::STATE_INACTIVE) ||
                (pMedia->GetUpdateState() == Media::UPDATE_MODIFIED))
        {
            IMS_TRACE_D("SESSION REFRESH METHOD :: INVITE", 0, 0, 0);
            return SipMethod(SipMethod::INVITE);
        }
    }

    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        const Media* pMedia = m_objMedias.GetAt(i);

        if ((pMedia->GetState() == Media::STATE_ACTIVE) &&
                (pMedia->GetUpdateState() == Media::UPDATE_REMOVED))
        {
            IMS_TRACE_D("SESSION REFRESH METHOD :: UPDATE", 0, 0, 0);
            return SipMethod(SipMethod::UPDATE);
        }
    }

    IMS_TRACE_D("SESSION REFRESH METHOD :: UPDATE (default)", 0, 0, 0);

    return SipMethod(SipMethod::UPDATE);
}

PRIVATE
IMS_RESULT Session::SendRequestForRefresh(IN IMS_SINT32 nMethod /*= SipMethod::INVALID*/)
{
    SipMethod objMethod(nMethod);
    IMS_SINT32 nState = GetState();

    if (nState == STATE_ESTABLISHED)
    {
        // Send a refresh request : UPDATE or re-INVITE
        if (objMethod.Equals(SipMethod::INVALID))
        {
            objMethod = m_pRefreshHelper->GetRefreshMethod();
        }

        if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE))
        {
            IMS_TRACE_E(0, "Session refresh method is an invalid", 0, 0, 0);
            return IMS_FAILURE;
        }
    }
    else if (!SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                     GetSlotId(), GetService()->GetSipProfile()) &&
            ((nState == STATE_RENEGOTIATING) || (nState == STATE_REESTABLISHING)))
    {
        // Send a refresh request : UPDATE
        if (objMethod.Equals(SipMethod::INVALID))
        {
            objMethod = m_pRefreshHelper->GetRefreshMethod();
        }

        if (!objMethod.Equals(SipMethod::UPDATE))
        {
            IMS_TRACE_E(0, "Session refresh method is an invalid", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    ISipClientConnection* piScc = CreateConnectionL(GetDialog(), objMethod);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection for a session refresh failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Set the headers for the invitation

    // User-Agent : configuration options ?
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSipProfile()))
    {
        UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, GetService()->GetSipProfile(),
                GetService()->GetServiceId(), GetService()->GetIpAddress(), GetSlotId(), piSipMsg);
    }

    // Set SDP message if any offer; According to the configuration options
    if (objMethod.Equals(SipMethod::INVITE))
    {
        SetSdpBodyPartFromCurrentView(piSipMsg);
    }

    if (m_pRefreshHelper->SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        piScc->Close();

        IMS_TRACE_E(0, "Sending a session refresh request (%s) failed",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    if (objMethod.Equals(SipMethod::INVITE))
    {
        // Update the call state
        UpdateCallStateOnMessageSent(piScc->GetMessage());

        // Update the Offer/Answer state
        // UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendRequestToAck(IN ISipClientConnection* piScc, IN IMS_SINT32 nServiceMethod)
{
    if (!GetService()->InitAck(piScc))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    SetImplicitRouteHeader(piScc);

    // Set SDP message if any offer
    ISipMessage* piSipMsg = piScc->GetMessage();

    // FIX_TIMING_ISSUE_UPDATE_N_INVITE_200OK
    if (!IsEarlyUpdateInProgress())
    {
        CheckNSetSdpBodyPart(piSipMsg);
    }

    const SipConfigV* pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        const AStringArray& objMethods = pSipConfigV->GetAllowMethods();

        for (IMS_SINT32 i = 0; i < objMethods.GetCount(); ++i)
        {
            if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(i)) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Allow header failed", 0, 0, 0);
                Ims::SetLastError(ImsError::GENERAL_ERROR);
                return IMS_FAILURE;
            }
        }
    }

    // Sets User-Agent header field
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSipProfile()))
    {
        UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, GetService()->GetSipProfile(),
                GetService()->GetServiceId(), GetService()->GetIpAddress(), GetSlotId(), piSipMsg);
    }

    // Add a specific header for ACK request (Require, ...)
    const Message* pMessage = GetPreviousRequest(nServiceMethod);

    if (pMessage != IMS_NULL)
    {
        // Sets Require header fields from the previous INVITE request
        ImsList<AString> objRequires = pMessage->GetMessage()->GetHeaders(ISipHeader::REQUIRE);

        for (IMS_UINT32 i = 0; i < objRequires.GetSize(); ++i)
        {
            piSipMsg->AddHeader(ISipHeader::REQUIRE, objRequires.GetAt(i));
        }

        // Sets Proxy-Require header fields from the previous INVITE request
        ImsList<AString> objProxyRequires =
                pMessage->GetMessage()->GetHeaders(ISipHeader::PROXY_REQUIRE);

        for (IMS_UINT32 i = 0; i < objProxyRequires.GetSize(); ++i)
        {
            piSipMsg->AddHeader(ISipHeader::PROXY_REQUIRE, objProxyRequires.GetAt(i));
        }
    }

    if (!SendNUpdateRequest(IMessage::SESSION_ACK, piScc))
    {
        // ACK transaction will be closed by the caller
        ClearConnection(IMessage::SESSION_ACK);

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // ACK transaction will be closed by the caller
    ClearConnection(IMessage::SESSION_ACK);

    // Update the call state
    UpdateCallStateOnMessageSent(piScc->GetMessage());
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());

    // Update the media state
    if (nServiceMethod == IMessage::SESSION_START)
    {
        UpdateMedia(Media::SESSION_STARTED);
    }
    else if (nServiceMethod == IMessage::SESSION_UPDATE)
    {
        UpdateMedia(Media::SESSION_UPDATED);
    }
    else
    {
        RestoreOfferAnswerState();
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendRequestToBye()
{
    SipMethod objMethod(SipMethod::BYE);
    ISipClientConnection* piScc = CreateConnectionL(GetDialog(), objMethod);

    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Set the headers for the session termination
    piScc->SetExtensionTokenForViaBranch(m_strTerminationReasonFromApp);

    // Update a session refresh timer info.
    m_pRefreshHelper->StopSessionTimer(piScc);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    ISipMessage* piSipMsg = piScc->GetMessage();
    SetSdpBodyPartFromRefusedView(piSipMsg);

    // Try to send a BYE request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_TERMINATE, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piScc->GetMessage());

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendRequestToCancel()
{
    ISipClientConnection* piScc = IMS_NULL;

    if (GetState() == STATE_RENEGOTIATING)
    {
        piScc = GetClientConnection(IMessage::SESSION_UPDATE);
    }
    else
    {
        piScc = GetClientConnection(IMessage::SESSION_START);
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't retrieve the previous SIP connection (INVITE)", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipClientConnection* piCancel = CreateCancelConnection(piScc);

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

    ISipMessage* piSipMsg = piCancel->GetMessage();

    // Sets User-Agent header field
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), GetService()->GetSipProfile()))
    {
        UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, GetService()->GetSipProfile(),
                GetService()->GetServiceId(), GetService()->GetIpAddress(), GetSlotId(), piSipMsg);
    }

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    SetSdpBodyPartFromRefusedView(piSipMsg);

    // Try to send a CANCEL request to the network
    if (!SendNUpdateRequest(nMethodForCancel, piCancel))
    {
        piCancel->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piCancel->GetMessage());

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendRequestToInvite(IN IMS_BOOL bSessionRefresh /*= IMS_FALSE*/)
{
    IMS_SINT32 nServiceMethod;
    ISipClientConnection* piScc = IMS_NULL;
    SipMethod objMethod(SipMethod::INVITE);

    if (GetState() == STATE_INITIATED)
    {
        nServiceMethod = IMessage::SESSION_START;
        piScc = CreateConnection(objMethod);
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
        piScc = CreateConnectionL(GetDialog(), objMethod);
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Set the headers for the invitation

    // Min-SE : configuration option
    // Session-Expires : configuration options
    // Supported : configuration options
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (!SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
        {
            if (GetState() != STATE_INITIATED)
            {
                if (!m_pRefreshHelper->AddSpecificHeaderWithoutParameterChange(piScc))
                {
                    piScc->Close();
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
            else
            {
                if (!m_pRefreshHelper->AddSpecificHeader(piScc))
                {
                    piScc->Close();
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            if (!m_pRefreshHelper->AddSpecificHeader(piScc))
            {
                piScc->Close();
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    if (bSessionRefresh)
    {
        // Set SDP message from the current view
        SetSdpBodyPartFromCurrentView(piSipMsg);
    }
    else
    {
        // Set SDP message if any offer
        CheckNSetSdpBodyPart(piSipMsg);
    }

    // Update a session refresh timer info.
    if (SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSipProfile()))
    {
        if (m_pRefreshHelper != IMS_NULL)
        {
            m_pRefreshHelper->StopSessionTimer(piScc);
        }
    }

    // CALLER_PREFERENCE_MANAGER
    if (GetState() != STATE_INITIATED)
    {
        m_objPreviousCallerPreference = ImsCoreContext::GetInstance()
                                                ->GetCallerPreferenceManager()
                                                ->GetAcceptContactsByName(GetName());
    }

    // Try to send an INVITE request to the network
    if (!SendNUpdateRequest(nServiceMethod, piScc))
    {
        // CALLER_PREFERENCE_MANAGER
        m_objPreviousCallerPreference.Clear();

        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piScc->GetMessage());
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());

    // Update the media state
    if (GetState() == STATE_INITIATED)
    {
        UpdateMedia(Media::SESSION_START);
    }
    else
    {
        UpdateMedia(Media::SESSION_UPDATE);
    }

    if ((GetState() == STATE_INITIATED) && !m_objMedias.IsEmpty())
    {
        m_bSdpInInitialInvite = IMS_TRUE;
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendRequestToInviteOn422Received()
{
    IMS_SINT32 nState = GetState();
    IMS_SINT32 nServiceMethod;
    ISipClientConnection* piScc = IMS_NULL;
    SipMethod objMethod(SipMethod::INVITE);

    IMS_TRACE_D("SendRequestToINVITEOn422Received()", 0, 0, 0);

    if (nState == STATE_INITIATED)
    {
        nServiceMethod = IMessage::SESSION_START;
        piScc = CreateConnection(objMethod);
    }
    else
    {
        nServiceMethod = IMessage::SESSION_UPDATE;
        piScc = CreateConnectionL(GetDialog(), objMethod);
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    const IMessage* piPreviousRequest = GetPreviousRequest(nServiceMethod);

    if (piPreviousRequest == IMS_NULL)
    {
        piScc->Close();

        IMS_TRACE_E(0, "Previous request is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Copy all the headers and message bodies from the previous request message
    if (piSipMsg->CopyHeadersAndBodyParts(piPreviousRequest->GetMessage()) != IMS_SUCCESS)
    {
        piScc->Close();

        IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Remove the transaction-related headers
    piSipMsg->RemoveHeader(ISipHeader::VIA);

    piSipMsg->RemoveHeader(ISipHeader::MIN_SE);
    piSipMsg->RemoveHeader(ISipHeader::SESSION_EXPIRES);

    // Correct the header value if it is required
    AString strCSeqHdr = piSipMsg->GetHeader(ISipHeader::CSEQ);
    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::CSEQ, strCSeqHdr);

    if (piHeader != IMS_NULL)
    {
        ImsList<AString> objTokens = piHeader->GetValue().Split(' ');

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

                piSipMsg->SetHeader(ISipHeader::CSEQ, strNewCSeqHdr);
            }
        }

        piHeader->Destroy();
    }

    // Min-SE : configuration option
    // Session-Expires : configuration options
    // Supported : configuration options
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (!SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                    GetSlotId(), GetService()->GetSipProfile()))
        {
            if (nState != STATE_INITIATED)
            {
                if (!m_pRefreshHelper->AddSpecificHeaderWithoutParameterChange(piScc))
                {
                    piScc->Close();
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
            else
            {
                if (!m_pRefreshHelper->AddSpecificHeader(piScc))
                {
                    piScc->Close();
                    Ims::SetLastError(ImsError::GENERAL_ERROR);

                    IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            if (!m_pRefreshHelper->AddSpecificHeader(piScc))
            {
                piScc->Close();
                Ims::SetLastError(ImsError::GENERAL_ERROR);

                IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    // Update a session refresh timer info.
    if (SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
                GetSlotId(), GetService()->GetSipProfile()))
    {
        if (m_pRefreshHelper != IMS_NULL)
        {
            m_pRefreshHelper->StopSessionTimer(piScc);
        }
    }

    // CALLER_PREFERENCE_MANAGER
    if (nState != STATE_INITIATED)
    {
        m_objPreviousCallerPreference = ImsCoreContext::GetInstance()
                                                ->GetCallerPreferenceManager()
                                                ->GetAcceptContactsByName(GetName());
    }

    // Try to send an INVITE request to the network
    if (!SendNUpdateRequest(nServiceMethod, piScc))
    {
        // CALLER_PREFERENCE_MANAGER
        m_objPreviousCallerPreference.Clear();

        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piScc->GetMessage());
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());

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

    if ((nState == STATE_INITIATED) && !m_objMedias.IsEmpty())
    {
        m_bSdpInInitialInvite = IMS_TRUE;
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendRequestToUpdate(IN IMS_BOOL bSessionRefresh /*= IMS_FALSE*/)
{
    SipMethod objMethod(SipMethod::UPDATE);
    ISipClientConnection* piScc = CreateConnectionL(GetDialog(), objMethod);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Set the headers for the invitation

    // Min-SE : configuration option
    // Session-Expires : configuration options
    // Supported : configuration options
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (!m_pRefreshHelper->AddSpecificHeader(piScc))
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            IMS_TRACE_E(0, "Adding the session refresh headers failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (bSessionRefresh)
    {
        // Set SDP message from the current view
        SetSdpBodyPartFromCurrentView(piSipMsg);
    }
    else
    {
        // Set SDP message if any offer
        CheckNSetSdpBodyPart(piSipMsg);
    }

    // Update a session refresh timer info.
    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->StopSessionTimer(piScc);
    }

    // Try to send an UPDATE request to the network
    if (!SendNUpdateRequest(IMessage::SESSION_UPDATE, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piScc->GetMessage());

    // Update the media state
    UpdateMedia(Media::SESSION_UPDATE);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Session::SendResponseEx(
        IN ISipServerConnection* piSsc, IN IMS_SINT32 nServiceMethod, IN IMS_SINT32 nStatusCode)
{
    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing a response failed - SipError (%d)", SipError::GetLastError(),
                0, 0);
        return IMS_FAILURE;
    }

    if (!SendNUpdateResponse(nServiceMethod, piSsc))
    {
        IMS_TRACE_E(0, "Sending a response failed - SipError (%d)", SipError::GetLastError(), 0, 0);
        return IMS_FAILURE;
    }

    // Update the call state
    UpdateCallStateOnMessageSent(piSsc->GetMessage());

    return IMS_SUCCESS;
}

PRIVATE
void Session::SetImplicitRouteHeader(IN ISipClientConnection* piScc)
{
    if (m_bImplicitRoutingRequired)
    {
        const AStringArray& objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piScc->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }
}

PRIVATE
void Session::SetReasonHeaderFromPreviousRequest(IN IMS_SINT32 nRequest)
{
    Message* pPrevRequest = GetPreviousRequest(nRequest);

    if (pPrevRequest == IMS_NULL)
    {
        return;
    }

    Message* pNextRequest = GetNextRequest();

    if (pNextRequest == IMS_NULL)
    {
        return;
    }

    ISipMessage* piPrevSipMsg = pPrevRequest->GetMessage();
    ISipMessage* piNextSipMsg = pNextRequest->GetMessage();

    if ((piPrevSipMsg != IMS_NULL) && (piNextSipMsg != IMS_NULL))
    {
        AString strReason = piPrevSipMsg->GetHeader(ISipHeader::REASON);

        if (strReason.GetLength() > 0)
        {
            piNextSipMsg->SetHeader(ISipHeader::REASON, strReason);
        }

        if (m_piReasonHeaderSetter != IMS_NULL)
        {
            m_piReasonHeaderSetter->ReasonHeaderSetter_SetPrivateHeader(piPrevSipMsg, piNextSipMsg);
        }
    }
}

PRIVATE
void Session::Start2xxRetransmission()
{
    m_pRetransmissionTask = new RetryTaskHelper();

    if (m_pRetransmissionTask == IMS_NULL)
    {
        return;
    }

    m_pRetransmissionTask->SetListener(this);
    m_pRetransmissionTask->SetCommand(this);

    RetryTimer* pTimer = new RetryTimer();

    if (pTimer == IMS_NULL)
    {
        return;
    }

    const SipProfile* pProfile = GetService()->GetSipProfile();
    const ISipConfigV* piSipConfigV = GetService()->GetISipConfigV();

    IMS_SINT32 nT1 = SipConfigProxy::GetTimerValueT1(GetSlotId(), pProfile, piSipConfigV);
    IMS_SINT32 nT2 = SipConfigProxy::GetTimerValueT2(GetSlotId(), pProfile, piSipConfigV);
    IMS_SINT32 nTimerH = nT1 * 64;

    IMS_SINT32 nTimerValue = nT1;
    IMS_SINT32 nTxnExpires = nTimerH;
    IMS_SINT32 nMultiplier = 1;

    while (1)
    {
        if (nTimerValue != nT2)
        {
            nTimerValue = IMS_MIN(nMultiplier * nT1, nT2);
            nMultiplier *= 2;

            // Exceptional case
            if ((nTimerValue <= 0) || (nTimerValue > nTxnExpires))
            {
                IMS_TRACE_D("2XX retransmission timer value fallback :: TV(%d), T2(%d)",
                        nTimerValue, nT2, 0);

                nTimerValue = nT2;
            }
        }

        pTimer->AddValue(nTimerValue);

        nTimerH -= nTimerValue;

        if (nTimerH < nT2)
        {
            if (nTimerH > 0)
            {
                pTimer->AddValue(nTimerH);
            }
            break;
        }
    }

    m_pRetransmissionTask->SetTimer(pTimer);

    m_pRetransmissionTask->Start(RetryTaskHelper::START_TIMER);
}

PRIVATE
void Session::Stop2xxRetransmission()
{
    if (m_pRetransmissionTask != IMS_NULL)
    {
        m_pRetransmissionTask->Terminate();

        RetryTimer* pTimer = m_pRetransmissionTask->SetTimer(IMS_NULL);

        if (pTimer != IMS_NULL)
        {
            delete pTimer;
        }

        delete m_pRetransmissionTask;
        m_pRetransmissionTask = IMS_NULL;
    }
}

PRIVATE
void Session::TerminateOnNegotiating()
{
    // Session : MO
    if (IsMobileOriginated())
    {
        IMS_SINT32 nCallState = GetCallState();

        if (nCallState == CallState::STATE_INVITE_SENT)
        {
            // Set the flag and wait & send CANCEL when receiving 1xx response
            m_bTerminatePending = IMS_TRUE;

            IMS_TRACE_I("_____ No 1xx response to INVITE received _____", 0, 0, 0);
            return;
        }
        else if (nCallState == CallState::STATE_INVITE_1XX_RECEIVED)
        {
            if (m_bTerminateMethodBye)
            {
                // Send BYE
                if (SendRequestToBye() != IMS_SUCCESS)
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
                if (SendRequestToCancel() != IMS_SUCCESS)
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
            if (SendRequestToBye() != IMS_SUCCESS)
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
        ISipServerConnection* piSsc = GetServerConnection(IMessage::SESSION_START);

        if (piSsc == IMS_NULL)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
            return;
        }

        if (SendResponseEx(piSsc, IMessage::SESSION_START, SipStatusCode::SC_486) != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending 486 to INVITE request failed", 0, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

PRIVATE
void Session::TerminateOnReNegotiating()
{
    if (m_bUpdateRequestor)
    {
        // Abort the ongoing transaction
        CloseConnection(IMessage::SESSION_UPDATE);

        if (SendRequestToBye() != IMS_SUCCESS)
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
        const Message* pMessage = GetPreviousRequest(IMessage::SESSION_UPDATE);

        if (pMessage != IMS_NULL)
        {
            ISipServerConnection* piSsc = GetServerConnection(IMessage::SESSION_UPDATE);

            if (piSsc == IMS_NULL)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                IMS_TRACE_E(0, "Getting SIP server connection failed", 0, 0, 0);
                return;
            }

            if (SendResponseEx(piSsc, IMessage::SESSION_UPDATE, SipStatusCode::SC_488) !=
                    IMS_SUCCESS)
            {
                SetState(STATE_TERMINATED);

                PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

                IMS_TRACE_E(0, "Sending 488 to INVITE request failed", 0, 0, 0);
                return;
            }
        }

        if (SendRequestToBye() != IMS_SUCCESS)
        {
            SetState(STATE_TERMINATED);

            PostMessage(AMSG_SESSION_TERMINATED, 0, 0);

            IMS_TRACE_E(0, "Sending BYE request failed", 0, 0, 0);
            return;
        }

        SetState(STATE_TERMINATING);
    }
}

PRIVATE
void Session::TerminateForkedSessionsOnNegotiating()
{
    if (m_pForkedSessions.IsNull())
    {
        return;
    }

    const ImsList<Method*>& objMethods = m_pForkedSessions->GetMethods();

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

PRIVATE
void Session::TerminateForkedSession()
{
    IMS_TRACE_I("TerminateForkedSession :: state=%d", GetState(), 0, 0);

    if (GetState() == STATE_NEGOTIATING)
    {
        SetState(STATE_TERMINATING);
    }

    // Abort the ongoing transaction
    CloseConnection(IMessage::SESSION_PRACK);
    CloseConnection(IMessage::SESSION_EARLY_UPDATE);
}

PRIVATE
void Session::ClearForkedSessionsByTerminated()
{
    IMS_TRACE_I("ClearForkedSessionsByTerminated: %p on %s", this, StateToString(GetState()), 0);

    if (m_pForkedSessions.IsNull())
    {
        return;
    }

    const ImsList<Method*>& objMethods = m_pForkedSessions->GetMethods();

    if (objMethods.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); i++)
    {
        Session* pSession = DYNAMIC_CAST(Session*, objMethods.GetAt(i));

        if (pSession != IMS_NULL && pSession->GetState() != STATE_TERMINATED)
        {
            pSession->HandleForkedSessionTerminated();
        }
    }
}

PRIVATE
void Session::HandleForkedSessionTerminated()
{
    IMS_TRACE_I("HandleForkedSessionTerminated: %p on %s", this, StateToString(GetState()), 0);

    if (GetState() == STATE_NEGOTIATING)
    {
        SetState(STATE_TERMINATING);
    }
    SetState(STATE_TERMINATED);

    PostMessage(AMSG_SESSION_TERMINATED, 0, 0);
}

PRIVATE
IMS_BOOL Session::AddMedia(IN Media* pMedia)
{
    if (!m_objMedias.Append(pMedia))
    {
        IMS_TRACE_E(0, "Appending Media object failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void Session::CleanupMedia()
{
    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        Media* pMedia = m_objMedias.GetAt(i);

        if (pMedia != IMS_NULL)
        {
            pMedia->CleanupMedia();
        }
    }
}

PRIVATE
void Session::CreateMediaFromSdp()
{
    const SessionParameter* pSessionParam = m_pOaState->GetProposalView();
    IMS_SINT32 nMediaCount = pSessionParam->GetMediaCount();

    if (nMediaCount != 0)
    {
        // Iterate over all offered media parameters and update and create a media object.
        IMS_UINT32 nMediaIndex = 0;
        ImsList<SdpMediaParameter*> objGroupMediaParams;

        for (IMS_SINT32 i = 0; i < nMediaCount; ++i)
        {
            objGroupMediaParams.Clear();

            // See if the current media parameter is first in a group of parameters that should
            // make up one media object.
            if (pSessionParam->FindGroupStartingWithMediaParameter(i, objGroupMediaParams))
            {
                const SdpMediaParameter* pMediaParam = objGroupMediaParams.GetAt(0);

                switch (pMediaParam->GetMedia().GetTransportProtocol())
                {
                    case SdpMedia::TRANSPORT_RTP_AVP:       // FALL-THROUGH
                    case SdpMedia::TRANSPORT_RTP_AVPF:      // FALL-THROUGH
                    case SdpMedia::TRANSPORT_RTP_SAVP:      // FALL-THROUGH
                    case SdpMedia::TRANSPORT_RTP_SAVPF:     // FALL-THROUGH
                    case SdpMedia::TRANSPORT_TCP_MSRP:      // FALL-THROUGH
                    case SdpMedia::TRANSPORT_TCP_TLS_MSRP:  // FALL-THROUGH
                    case SdpMedia::TRANSPORT_UDP:           // FALL-THROUGH
                    case SdpMedia::TRANSPORT_TCP:
                        break;
                    default:
                        IMS_TRACE_I(
                                "Unsupported media type(%s, %s) received; So, it will be ignored",
                                pMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);
                        continue;
                }

                if (nMediaIndex >= m_objMedias.GetSize())
                {
                    ImsList<IMS_SINT32> objMids;

                    for (IMS_UINT32 j = 0; j < objGroupMediaParams.GetSize(); ++j)
                    {
                        const SdpMediaParameter* pGroupMediaParam = objGroupMediaParams.GetAt(j);

                        IMS_TRACE_I("New media type(%s, %s) added",
                                pGroupMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pGroupMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                        objMids.Append(pGroupMediaParam->GetMid());
                    }

                    // New media; Create and add to the current session.
                    Media* pMedia = MediaFactory::CreateIncomingMedia(
                            pMediaParam->GetMedia().GetTransportProtocol(), GetService(),
                            m_pOaState, objMids);

                    if (pMedia == IMS_NULL)
                    {
                        continue;
                    }

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
}

PRIVATE
IMS_BOOL Session::IsMediaInitializationDone() const
{
    // Check if the created media is initialized or not
    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        const Media* pMedia = m_objMedias.GetAt(i);

        if (!pMedia->IsInitializationDone())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL Session::UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger)
{
    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        Media* pMedia = m_objMedias.GetAt(i);

        if (pMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting Media object failed", 0, 0, 0);
            continue;
        }

        pMedia->TransitMedia(nTrigger, Media::ANSWER_RECEIVED);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL Session::UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger)
{
    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        Media* pMedia = m_objMedias.GetAt(i);

        if (pMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting Media object failed", 0, 0, 0);
            continue;
        }

        pMedia->TransitMedia(nTrigger, Media::ANSWER_SENT);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL Session::UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger)
{
    const SessionParameter* pSessionParam = m_pOaState->GetProposalView();
    IMS_SINT32 nMediaCount = pSessionParam->GetMediaCount();

    if (nMediaCount != 0)
    {
        // Iterate over all offered media parameters and update and create a media object.
        IMS_UINT32 nMediaIndex = 0;
        ImsList<SdpMediaParameter*> objGroupMediaParams;

        for (IMS_SINT32 i = 0; i < nMediaCount; ++i)
        {
            objGroupMediaParams.Clear();

            // See if the current media parameter is first in a group of parameters that should
            // make up one media object.
            if (pSessionParam->FindGroupStartingWithMediaParameter(i, objGroupMediaParams))
            {
                SdpMediaParameter* pMediaParam = objGroupMediaParams.GetAt(0);

                switch (pMediaParam->GetMedia().GetTransportProtocol())
                {
                    case SdpMedia::TRANSPORT_RTP_AVP:           // FALL-THROUGH
                    case SdpMedia::TRANSPORT_RTP_AVPF:          // FALL-THROUGH
                    case SdpMedia::TRANSPORT_RTP_SAVP:          // FALL-THROUGH
                    case SdpMedia::TRANSPORT_RTP_SAVPF:         // FALL-THROUGH
                    case SdpMedia::TRANSPORT_UDP_TLS_RTP_SAVP:  // FALL-THROUGH
                    case SdpMedia::TRANSPORT_TCP_MSRP:          // FALL-THROUGH
                    case SdpMedia::TRANSPORT_TCP_TLS_MSRP:      // FALL-THROUGH
                    case SdpMedia::TRANSPORT_UDP:               // FALL-THROUGH
                    case SdpMedia::TRANSPORT_TCP:
                        break;
                    default:
                        IMS_TRACE_I(
                                "Unsupported media type(%s, %s) received; So, it will be ignored",
                                pMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                        pMediaParam->MarkRejectedOrRemoved();
                        continue;
                }

                if (nMediaIndex >= m_objMedias.GetSize())
                {
                    ImsList<IMS_SINT32> objMids;

                    for (IMS_UINT32 j = 0; j < objGroupMediaParams.GetSize(); ++j)
                    {
                        const SdpMediaParameter* pGroupMediaParam = objGroupMediaParams.GetAt(j);

                        IMS_TRACE_I("New media type(%s, %s) added",
                                pGroupMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pGroupMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                        objMids.Append(pGroupMediaParam->GetMid());
                    }

                    // New media; Create and add to the current session.
                    Media* pMedia = MediaFactory::CreateIncomingMedia(
                            pMediaParam->GetMedia().GetTransportProtocol(), GetService(),
                            m_pOaState, objMids);

                    if (pMedia == IMS_NULL)
                    {
                        continue;
                    }

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
                    Media* pMedia = m_objMedias.GetAt(nMediaIndex);

                    pMedia->TransitMedia(nTrigger, Media::OFFER_RECEIVED);
                }

                ++nMediaIndex;
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL Session::UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger)
{
    for (IMS_UINT32 i = 0; i < m_objMedias.GetSize(); ++i)
    {
        Media* pMedia = m_objMedias.GetAt(i);

        if (pMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting Media object failed", 0, 0, 0);
            continue;
        }

        pMedia->TransitMedia(nTrigger, Media::OFFER_SENT);
    }

    return IMS_TRUE;
}

PRIVATE
void Session::CreateRemoteMediaCapabilities(IN const ISipMessage* piSipMsg)
{
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    if (nStatusCode == SipStatusCode::SC_488 || nStatusCode == SipStatusCode::SC_606)
    {
        const ISipMessageBodyPart* piBodyPart = piSipMsg->GetSdpBodyPart();
        const ByteArray& objSdp =
                (piBodyPart != IMS_NULL) ? piBodyPart->GetContent() : ByteArray::ConstNull();

        if (objSdp.GetLength() > 0)
        {
            if (m_pRemoteMediaCapabilities != IMS_NULL)
            {
                delete m_pRemoteMediaCapabilities;
            }

            IMS_TRACE_I("Remote media capabilities created.", 0, 0, 0);
            m_pRemoteMediaCapabilities = new SdpReader(objSdp);
        }
    }
}

PRIVATE GLOBAL const IMS_CHAR* Session::StateToString(IN IMS_SINT32 nState)
{
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
