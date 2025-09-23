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
#include "ImsIdentity.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "private/SipConfigV.h"

#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpProfile.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SdpOaState.h"
#include "SessionDescriptor.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "VirtualSession.h"
#include "base/Ims.h"
#include "media/MediaFactory.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
VirtualSession::VirtualSession(IN Service* pService, IN const SipAddress* pUserAor) :
        RcObject(),
        m_pService(pService),
        m_nState(STATE_CREATED),
        m_pOaState(IMS_NULL),
        m_pSessionDescriptor(IMS_NULL)
{
    IMS_TRACE_I("VirtualSession - C", 0, 0, 0);

    Init(pUserAor);
}

PUBLIC
VirtualSession::VirtualSession(IN const VirtualSession& other) :
        RcObject(other),
        m_pService(other.m_pService),
        m_objUserAor(other.m_objUserAor),
        m_nState(STATE_CREATED),
        m_pOaState(IMS_NULL),
        m_pSessionDescriptor(IMS_NULL)
{
    Init(&m_objUserAor);
}

PUBLIC VIRTUAL VirtualSession::~VirtualSession()
{
    IMS_TRACE_I("VirtualSession - D", 0, 0, 0);

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

PUBLIC
IMS_BOOL VirtualSession::CheckNSetSdpBodyPart(IN_OUT ISipMessage*& piSipMsg)
{
    IMS_SINT32 nOaState = GetOfferAnswerState();

    if ((nOaState != SdpOaState::STATE_OFFER_RECEIVED) &&
            (nOaState != SdpOaState::STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_I("No SDP offer in VritualSession", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_objMedias.IsEmpty())
    {
        RestoreOfferAnswerState();
        IMS_TRACE_D("There is no media", 0, 0, 0);
        return IMS_FALSE;
    }

    // SDP message to be set
    AString strSdp;

    if (!m_pOaState->GetSdp(strSdp))
    {
        RestoreOfferAnswerState();
        IMS_TRACE_D("There is no SDP message body", 0, 0, 0);
        return IMS_FALSE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMsg->CreateSdpBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        RestoreOfferAnswerState();
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

PUBLIC
IMS_RESULT VirtualSession::Notify18xResponse(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        SetState(STATE_TERMINATED);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ %d response received on %s", piSipMsg->GetStatusCode(),
            StateToString(GetState()), 0);

    if (!UpdateOfferAnswerStateOnMessageReceived(piSipMsg))
    {
        SetState(STATE_TERMINATED);
        return IMS_FAILURE;
    }

    // Check & create a session descriptor when an initial offer received
    if (!CheckNCreateSessionDescriptor())
    {
        SetState(STATE_TERMINATED);
        return IMS_FAILURE;
    }

    IMS_SINT32 nOaResult = HandleSdpOfferAnswer(piSipMsg);

    if ((nOaResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOaResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        SetState(STATE_TERMINATED);
        return IMS_FAILURE;
    }

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);

    return IMS_SUCCESS;
}

PUBLIC
void VirtualSession::NotifyPrackSent(IN const ISipMessage* piSipMsg)
{
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSipMsg);

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);
}

PUBLIC
Media* VirtualSession::CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
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
            strType, nDirection, m_pService, m_pOaState, nCountOfDescriptor);

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
const ImsList<Media*>& VirtualSession::GetMedia() const
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
SessionDescriptor* VirtualSession::GetSessionDescriptor()
{
    if (!CheckNCreateSessionDescriptor())
    {
        return IMS_NULL;
    }

    return m_pSessionDescriptor;
}

PUBLIC
IMS_RESULT VirtualSession::RemoveMedia(IN Media* pMedia)
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
IMS_RESULT VirtualSession::RemoveMedia(IN IMS_UINT32 nIndex)
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
        for (IMS_UINT32 j = nIndex; j < m_objMedias.GetSize(); ++j)
        {
            pMedia = m_objMedias.GetAt(j);
            pMedia->SetMid(j);
        }
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL const AString& VirtualSession::GetConnectionAddress() const
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

PROTECTED VIRTUAL IMS_SINT32 VirtualSession::GetSessionState() const
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

PROTECTED VIRTUAL SdpSessionParameter* VirtualSession::GetSessionParameter() const
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

PROTECTED VIRTUAL const AString& VirtualSession::GetPeerConnectionAddress() const
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

PROTECTED VIRTUAL SdpSessionParameter* VirtualSession::GetPeerSessionParameter() const
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    m_pOaState->GetSessionPeerView(pSessionParam);

    return pSessionParam;
}

PROTECTED VIRTUAL SdpSessionParameter* VirtualSession::GetProposalSessionParameter()
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

PROTECTED
IMS_BOOL VirtualSession::CheckNCreateSessionDescriptor()
{
    if (m_pSessionDescriptor != IMS_NULL)
    {
        IMS_TRACE_D("SessionDescriptor already exists", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nOaState = GetOfferAnswerState();

    // IDLE :: 183 response w/o SDP
    // OFFER_RECEIVED :: 183 response w/ SDP
    if ((nOaState != SdpOaState::STATE_IDLE) && (nOaState != SdpOaState::STATE_OFFER_RECEIVED))
    {
        IMS_TRACE_E(0, "__ SessionDescriptor can't be created in offer/answer state (%d) __",
                nOaState, 0, 0);
        return IMS_FALSE;
    }

    // Create a media capabilities for this service & session
    if (!m_pOaState->CreateCapabilities(m_pService))
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

    return IMS_TRUE;
}

PROTECTED
IMS_SINT32 VirtualSession::GetOfferAnswerState() const
{
    if (m_pOaState == IMS_NULL)
    {
        return SdpOaState::STATE_IDLE;
    }

    return m_pOaState->GetState();
}

PROTECTED
IMS_SINT32 VirtualSession::HandleSdpOfferAnswer(IN const ISipMessage* piSipMsg)
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
void VirtualSession::RestoreEx()
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
IMS_BOOL VirtualSession::UpdateMedia(IN IMS_SINT32 nTrigger)
{
    if (!m_pOaState->IsSessionChanged())
    {
        IMS_TRACE_D("UpdateMedia :: No session changed", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bResult = IMS_FALSE;

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
                    bResult = UpdateMediaOnAnswerReceived(nTrigger);
                else
                    bResult = UpdateMediaOnAnswerSent(nTrigger);
            }
            else
            {
                IMS_TRACE_D("UpdateMedia :: SDP Offer/Answer is IDLE", 0, 0, 0);
            }
            break;
        case SdpOaState::STATE_OFFER_RECEIVED:  // FALL-THROUGH
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
IMS_BOOL VirtualSession::RestoreOfferAnswerState()
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pOaState->RestoreState();
}

PROTECTED
IMS_BOOL VirtualSession::UpdateOfferAnswerStateOnMessageReceived(IN const ISipMessage* piSipMsg)
{
    if (m_pOaState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = GetState();
    IMS_BOOL bIsCallEstablished = IMS_FALSE;

    // RFC 6337: Section 3.1.1
    // After the UAC has received the answer in a reliable provisional response to the INVITE,
    //[RFC3261] requires that any SDP in subsequent responses be ignored.
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

    return m_pOaState->UpdateState(
            piSipMsg, SdpOaState::MESSAGE_RECEIVED, bIsCallEstablished, IMS_FALSE);
}

PROTECTED
IMS_BOOL VirtualSession::UpdateOfferAnswerStateOnMessageSent(IN const ISipMessage* piSipMsg)
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

    return m_pOaState->UpdateState(
            piSipMsg, SdpOaState::MESSAGE_SENT, bIsCallEstablished, IMS_FALSE);
}

PRIVATE
void VirtualSession::Init(IN const SipAddress* pUserAor)
{
    if (pUserAor != IMS_NULL)
    {
        m_objUserAor = *pUserAor;
    }
    else
    {
        AString strTempAor("sip:IMS-UE");
        const AString& strHdn = ImsIdentity::GetHomeDomainName(GetSlotId());

        strTempAor += "@";
        strTempAor += strHdn;

        m_objUserAor.Create(strTempAor);
    }

    // Instantiate SDP offer/answer object
    IMS_BOOL bSdpVersionCheck = IMS_TRUE;
    const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        bSdpVersionCheck = pSipConfigV->IsSessionSdpVersionCheckSupported();
    }

    m_pOaState = new SdpOaState(bSdpVersionCheck, IMS_TRUE);

    SetState(STATE_NEGOTIATING);
}

PRIVATE
void VirtualSession::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Session :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE
IMS_BOOL VirtualSession::AddMedia(IN Media* pMedia)
{
    if (!m_objMedias.Append(pMedia))
    {
        IMS_TRACE_E(0, "Appending Media object failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void VirtualSession::CleanupMedia()
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
IMS_BOOL VirtualSession::UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger)
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
IMS_BOOL VirtualSession::UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger)
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
IMS_BOOL VirtualSession::UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger)
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
                            pMediaParam->GetMedia().GetTransportProtocol(), m_pService, m_pOaState,
                            objMids);

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
IMS_BOOL VirtualSession::UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger)
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

PRIVATE GLOBAL const IMS_CHAR* VirtualSession::StateToString(IN IMS_SINT32 nState)
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
