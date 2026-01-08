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

#include "private/ConfigurationManager.h"
#include "private/MediaConfig.h"

#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpSessionParameter.h"

#include "ISipMessage.h"
#include "ISipMessageBodyPart.h"
#include "SdpOaState.h"
#include "Service.h"
#include "SipDebug.h"
#include "SipStatusCode.h"

__IMS_TRACE_TAG_IMS_CORE__;

class SessionCapabilities
{
public:
    inline SessionCapabilities() {}

    inline ~SessionCapabilities() {}

    inline IMS_BOOL Create(IN const SdpSessionDescription& objSessionDesc)
    {
        ImsList<SdpMediaDescription> objMediaDescs;

        if (!m_objCapabilities.Create(objSessionDesc, objMediaDescs))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    inline IMS_BOOL Create(IN const SdpSessionDescription& objSessionDesc,
            IN const ImsList<SdpMediaDescription>& objMediaDescs)
    {
        if (!m_objCapabilities.Create(objSessionDesc, objMediaDescs))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    inline const SessionParameter& GetCapabilities() const { return m_objCapabilities; }

    inline SessionParameter& GetCapabilities() { return m_objCapabilities; }

#if 0
    inline SessionParameter* GetCapabilities() const
    {
        return &objCapabilities;
    }

    inline void GetBasicReliableMedias(OUT ImsList<SdpMediaParameter*> &objMediaParams) const
    {
        const ImsList<SdpMediaParameter*> &objMedias = m_objCapabilities.GetMediaParameters();

        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const SdpMediaParameter *pMediaParam = objMedias.GetAt(i);
            IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

            if (nTransportProtocol == SdpMedia::TRANSPORT_TCP)
            {
                objMediaParams.Append(pMediaParam);
            }
        }
    }

    inline void GetBasicUnreliableMedias(OUT ImsList<SdpMediaParameter*> &objMediaParams) const
    {
        const ImsList<SdpMediaParameter*> &objMedias = m_objCapabilities.GetMediaParameters();

        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const SdpMediaParameter *pMediaParam = objMedias.GetAt(i);
            IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

            if (nTransportProtocol == SdpMedia::TRANSPORT_UDP)
            {
                objMediaParams.Append(pMediaParam);
            }
        }
    }

    inline void GetFramedMedias(OUT ImsList<SdpMediaParameter*> &objMediaParams) const
    {
        const ImsList<SdpMediaParameter*> &objMedias = m_objCapabilities.GetMediaParameters();

        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const SdpMediaParameter *pMediaParam = objMedias.GetAt(i);
            IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

            if ((nTransportProtocol == SdpMedia::TRANSPORT_TCP_MSRP) ||
                    (nTransportProtocol == SdpMedia::TRANSPORT_TCP_TLS_MSRP))
            {
                objMediaParams.Append(pMediaParam);
            }
        }
    }

    inline void GetStreamMedias(OUT ImsList<SdpMediaParameter*> &objMediaParams) const
    {
        const ImsList<SdpMediaParameter*> &objMedias = m_objCapabilities.GetMediaParameters();

        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const SdpMediaParameter *pMediaParam = objMedias.GetAt(i);
            IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

            if ((nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVP) ||
                    (nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVPF) ||
                    (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP) ||
                    (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVPF))
            {
                objMediaParams.Append(pMediaParam);
            }
        }
    }
#endif

public:
    SessionParameter m_objCapabilities;
};

// clang-format off
PRIVATE GLOBAL
const IMS_SINT32 SdpOaState::STATE_SENT[SdpOaState::STATE_MAX][SdpOaState::TRIGGER_MAX] =
{
    //STATE_IDLE
    {
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_INVITE
        SdpOaState::STATE_IDLE,                    // TRIGGER_ACK
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_PRACK
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_RPR
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_IDLE,                    // TRIGGER_CANCEL
        SdpOaState::STATE_IDLE                     // TRIGGER_BYE
    },

    //STATE_OFFER_SENT -- Offer sent by Engine
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_INVALID,                 // TRIGGER_ACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_INVALID,                 // TRIGGER_RPR
        SdpOaState::STATE_INVALID,                 // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_SENT               // TRIGGER_BYE
    },

    // STATE_OFFER_RECEIVED -- Offer received by Engine
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_RECEIVED           // TRIGGER_BYE
    },

    // STATE_ESTABLISHED -- Offer/Answer exchange done
    {
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_INVITE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_PRACK
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_RPR
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_CANCEL
        SdpOaState::STATE_ESTABLISHED              // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_SENT -- Counter offer made
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_INVALID,                 // TRIGGER_ACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_INVALID,                 // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_RPR
        SdpOaState::STATE_INVALID,                 // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_CHANGE_SENT        // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_RECEIVED -- Counter offer received
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED    // TRIGGER_BYE
    }
};

PRIVATE GLOBAL
const IMS_SINT32 SdpOaState::STATE_RECEIVED[SdpOaState::STATE_MAX][SdpOaState::TRIGGER_MAX] =
{
    //STATE_IDLE
    {
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_INVITE
        SdpOaState::STATE_INVALID,                 // TRIGGER_ACK
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_PRACK
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_RPR
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_INVALID,                 // TRIGGER_CANCEL
        SdpOaState::STATE_INVALID                  // TRIGGER_BYE
    },

    //STATE_OFFER_SENT -- Offer sent by Engine
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_SENT,              // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_SENT               // TRIGGER_BYE
    },

    // STATE_OFFER_RECEIVED -- Offer received by Engine
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_INVALID,                 // TRIGGER_ACK
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_RPR
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_RECEIVED,          // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_RECEIVED           // TRIGGER_BYE
    },

    // STATE_ESTABLISHED -- Offer/Answer exchange done
    {
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_INVITE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PRACK
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_RPR
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_CANCEL
        SdpOaState::STATE_ESTABLISHED              // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_SENT -- Counter offer made
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SdpOaState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_CHANGE_SENT        // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_RECEIVED -- Counter offer received
    {
        SdpOaState::STATE_INVALID,                 // TRIGGER_INVITE
        SdpOaState::STATE_INVALID,                 // TRIGGER_ACK
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PRACK
        SdpOaState::STATE_INVALID,                 // TRIGGER_UPDATE
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_SUCCESS_RESP
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_FAILURE_RESP
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_RPR
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PROVISIONAL_RESP
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_CANCEL
        SdpOaState::STATE_OFFER_CHANGE_RECEIVED    // TRIGGER_BYE
    }
};
// clang-format on

PUBLIC
SdpOaState::SdpOaState(IN IMS_BOOL bSdpVersionCheck /*= IMS_TRUE*/,
        IN IMS_BOOL bAlwaysIncreaseSdpVersion /*= IMS_FALSE*/) :
        m_nState(STATE_IDLE),
        m_nOldState(STATE_IDLE),
        m_nMode(MODE_IDLE),
        m_bPreviewModeSupported(IMS_FALSE),
        m_bPreviewMode(IMS_FALSE),
        m_bStateChanged(IMS_FALSE),
        m_bOfferProgress(IMS_FALSE),
        m_bProvisionalRespWithSdp(IMS_FALSE),
        m_bSdpVersionCheck(bSdpVersionCheck),
        m_bAlwaysIncreaseSdpVersion(bAlwaysIncreaseSdpVersion),
        m_pLastOfferMade(IMS_NULL),
        m_pCurrentView(IMS_NULL),
        m_pPeerView(IMS_NULL),
        m_pProposalView(IMS_NULL),
        m_pRefusedView(IMS_NULL),
        m_pCapabilities(new SessionCapabilities())
{
}

PUBLIC
SdpOaState::SdpOaState(IN const SdpOaState& other) :
        m_nState(other.m_nState),
        m_nOldState(other.m_nOldState),
        m_nMode(other.m_nMode),
        m_bPreviewModeSupported(other.m_bPreviewModeSupported),
        m_bPreviewMode(other.m_bPreviewMode),
        m_bStateChanged(other.m_bStateChanged),
        m_bOfferProgress(other.m_bOfferProgress),
        m_bProvisionalRespWithSdp(other.m_bProvisionalRespWithSdp),
        m_bSdpVersionCheck(other.m_bSdpVersionCheck),
        m_bAlwaysIncreaseSdpVersion(other.m_bAlwaysIncreaseSdpVersion),
        m_pLastOfferMade(IMS_NULL),
        m_pCurrentView(IMS_NULL),
        m_pPeerView(IMS_NULL),
        m_pProposalView(IMS_NULL),
        m_pRefusedView(IMS_NULL),
        m_pCapabilities(IMS_NULL)
{
    if (other.m_pLastOfferMade)
    {
        m_pLastOfferMade = new SessionParameter(*(other.m_pLastOfferMade));
    }

    if (other.m_pCurrentView)
    {
        m_pCurrentView = new SessionParameter(*(other.m_pCurrentView));
    }

    if (other.m_pPeerView)
    {
        m_pPeerView = new SessionParameter(*(other.m_pPeerView));
    }

    if (other.m_pProposalView)
    {
        m_pProposalView = new SessionParameter(*(other.m_pProposalView));
    }

    if (other.m_pRefusedView)
    {
        m_pRefusedView = new SessionParameter(*(other.m_pRefusedView));
    }

    if (other.m_pCapabilities)
    {
        m_pCapabilities = new SessionCapabilities(*(other.m_pCapabilities));
    }
}

PUBLIC VIRTUAL SdpOaState::~SdpOaState()
{
    if (m_pLastOfferMade != IMS_NULL)
    {
        delete m_pLastOfferMade;
    }

    if (m_pCurrentView != IMS_NULL)
    {
        delete m_pCurrentView;
    }

    if (m_pPeerView != IMS_NULL)
    {
        delete m_pPeerView;
    }

    if (m_pProposalView != IMS_NULL)
    {
        delete m_pProposalView;
    }

    if (m_pRefusedView != IMS_NULL)
    {
        delete m_pRefusedView;
    }

    if (m_pCapabilities != IMS_NULL)
    {
        delete m_pCapabilities;
    }
}

/**
 * @brief Aborts the proposal session parameter.
 */
PUBLIC VIRTUAL void SdpOaState::AbortProposal()
{
    IMS_TRACE_D("SDP Offer/Answer :: AbortProposal (%s)",
            (m_pProposalView == IMS_NULL) ? "NULL" : "non-NULL", 0, 0);

    if (m_pProposalView == IMS_NULL)
    {
        return;
    }

    delete m_pProposalView;
    m_pProposalView = IMS_NULL;

    m_nMode = MODE_IDLE;
    m_bOfferProgress = IMS_FALSE;
}

/**
 * @brief Creates a new proposal session parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::CreateProposalView()
{
    if ((m_nState != STATE_IDLE) && (m_nState != STATE_OFFER_RECEIVED) &&
            (m_nState != STATE_ESTABLISHED) && (m_nState != STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_E(0, "Invalid state (%d)", m_nState, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    if (m_pProposalView != IMS_NULL)
    {
        return ISdpOaState::RESULT_ALREADY_EXIST;
    }

    // Create a new SDP offer
    if (!InitiateOffer(OFFER_CHANGE))
    {
        IMS_TRACE_E(0, "Initiating new offer failed", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Gets the current local session parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::GetSessionCurrentView(
        OUT SdpSessionParameter*& pSessionParam) const
{
    if (m_nState != STATE_ESTABLISHED)
    {
        IMS_TRACE_D("SessionCurrentView :: Invalid state (%d)", m_nState, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    if (m_pCurrentView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Current View does not exist)", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    pSessionParam = &(m_pCurrentView->GetSessionParameterNc());

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Gets the current remote session parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::GetSessionPeerView(
        OUT SdpSessionParameter*& pSessionParam) const
{
    if (m_pPeerView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Peer View does not exist)", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    pSessionParam = &(m_pPeerView->GetSessionParameterNc());

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Gets the current proposal session parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::GetSessionProposalView(
        OUT SdpSessionParameter*& pSessionParam) const
{
    if ((m_nState != STATE_IDLE) && (m_nState != STATE_OFFER_SENT) &&
            (m_nState != STATE_OFFER_RECEIVED) && (m_nState != STATE_ESTABLISHED) &&
            (m_nState != STATE_OFFER_CHANGE_SENT) && (m_nState != STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_E(0, "Invalid state (%d)", m_nState, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    if (m_pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Proposed View does not exist)", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    pSessionParam = &(m_pProposalView->GetSessionParameterNc());

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Creates a new media parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::CreateMediaParameter(OUT SdpMediaParameter*& pMediaParam)
{
    if (m_pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid state (Proposed View is NULL)", 0, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    if (!m_bOfferProgress)
    {
        IMS_TRACE_E(0, "Invalid state (Offer is not in progress)", 0, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    pMediaParam = m_pProposalView->CreateMediaParameter();

    if (pMediaParam == IMS_NULL)
    {
        return ISdpOaState::RESULT_ERROR;
    }

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Gets the current local media parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::GetMediaCurrentView(
        IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const
{
    if (m_nState != STATE_ESTABLISHED)
    {
        IMS_TRACE_D("MediaCurrentView :: Invalid state (%d)", m_nState, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    if (m_pCurrentView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Current View does not exist)", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    pMediaParam = m_pCurrentView->GetMediaParameter(nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find the media parameter (%d) from the current view", nMid, 0, 0);
        return ISdpOaState::RESULT_NOT_FOUND;
    }

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Gets the current remote media parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::GetMediaPeerView(
        IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const
{
    if (m_pPeerView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Peer View does not exist)", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    pMediaParam = m_pPeerView->GetMediaParameter(nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_D("Can't find the media parameter (%d) from the peer view", nMid, 0, 0);

        if (m_pProposalView != IMS_NULL)
        {
            pMediaParam = m_pProposalView->GetMediaParameter(nMid);

            if (pMediaParam == IMS_NULL)
            {
                IMS_TRACE_E(0, "Can't find the media parameter (%d) from the proposal view", nMid,
                        0, 0);
                return ISdpOaState::RESULT_NOT_FOUND;
            }
        }
        else
        {
            return ISdpOaState::RESULT_NOT_FOUND;
        }
    }

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Gets the current proposal media parameter.
 */
PUBLIC VIRTUAL IMS_SINT32 SdpOaState::GetMediaProposalView(
        IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const
{
    if ((m_nState != STATE_IDLE) && (m_nState != STATE_OFFER_SENT) &&
            (m_nState != STATE_OFFER_RECEIVED) && (m_nState != STATE_ESTABLISHED) &&
            (m_nState != STATE_OFFER_CHANGE_SENT) && (m_nState != STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_E(0, "Invalid state (%d)", m_nState, 0, 0);
        return ISdpOaState::RESULT_INVALID_STATE;
    }

    if (m_pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Proposed View does not exist)", 0, 0, 0);
        return ISdpOaState::RESULT_ERROR;
    }

    pMediaParam = m_pProposalView->GetMediaParameter(nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find the media parameter (%d) from the proposed view", nMid, 0, 0);
        return ISdpOaState::RESULT_NOT_FOUND;
    }

    return ISdpOaState::RESULT_SUCCESS;
}

/**
 * @brief Marks the current proposal media parameter as a rejected or removed.
 */
PUBLIC VIRTUAL void SdpOaState::MarkRejectedOrRemoved(IN IMS_SINT32 nMid)
{
    if (m_pProposalView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is rejected or removed from the proposal view", nMid, 0, 0);
        m_pProposalView->RemoveMediaParameter(nMid, IMS_TRUE);
        return;
    }

    if (m_pCurrentView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is rejected or removed from the current view", nMid, 0, 0);
        m_pCurrentView->RemoveMediaParameter(nMid, IMS_TRUE);
    }
}

/**
 * @brief Removes the media parameter which the specified mid matches.
 */
PUBLIC VIRTUAL void SdpOaState::RemoveMediaParameter(IN IMS_SINT32 nMid)
{
    if (m_pProposalView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is removed from the proposal view", nMid, 0, 0);
        m_pProposalView->RemoveMediaParameter(nMid, IMS_FALSE);
        return;
    }

    if (m_pCurrentView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is removed from the current view", nMid, 0, 0);
        m_pCurrentView->RemoveMediaParameter(nMid, IMS_FALSE);
    }
}

/**
 * @brief Creates a new session parameter for the local capabilities.
 */
PUBLIC
IMS_BOOL SdpOaState::CreateCapabilities(
        IN const Service* pService, IN IMS_BOOL bMProf /*= IMS_FALSE*/)
{
    if ((m_nState != STATE_IDLE) && (m_nState != STATE_OFFER_RECEIVED))
    {
        IMS_TRACE_E(0, "Not allowed to create SDP capabilities in state (%d)", m_nState, 0, 0);
        return IMS_FALSE;
    }

    SdpSessionDescription objSessionDesc;

    // Some carriers require to set the username field value of SDP o-line to "-".
    if (!objSessionDesc.CreateMandatoryLines(SdpOrigin::DEFAULT_USERNAME, pService->GetIpAddress()))
    {
        return IMS_FALSE;
    }

    if (!bMProf)
    {
        if (!m_pCapabilities->Create(objSessionDesc))
        {
            IMS_TRACE_E(
                    0, "Creating an SDP capabilities for only session-level description", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        // Read the media capabilities from the media profile
        const CoreServiceConfig* pServiceConfig = pService->GetServiceConfig();
        const AString& strMProf = (pServiceConfig != IMS_NULL) ? pServiceConfig->GetMediaProfile()
                                                               : AString::ConstNull();
        const MediaConfig* pMediaConfig =
                ConfigurationManager::GetInstance()->GetMediaConfig(pService->GetSlotId());
        ImsList<SdpMediaDescription> objMediaDescs;

        if (pMediaConfig != IMS_NULL)
        {
            // StreamMedia (audio)
            const AStringArray& objAudioProfile =
                    pMediaConfig->GetMediaProfile(strMProf, IMediaConfig::STREAM_AUDIO);

            if (!objAudioProfile.IsEmpty())
            {
                SdpMediaDescription objAudioDesc;

                if (!objAudioDesc.Decode(objAudioProfile))
                {
                    IMS_TRACE_E(0, "Decoding an SDP(audio) failed", 0, 0, 0);
                    return IMS_FALSE;
                }

                objMediaDescs.Append(objAudioDesc);
            }

            // StreamMedia (video)
            const AStringArray& objVideoProfile =
                    pMediaConfig->GetMediaProfile(strMProf, IMediaConfig::STREAM_VIDEO);

            if (!objVideoProfile.IsEmpty())
            {
                SdpMediaDescription objVideoDesc;

                if (!objVideoDesc.Decode(objVideoProfile))
                {
                    IMS_TRACE_E(0, "Decoding an SDP(video) failed", 0, 0, 0);
                    return IMS_FALSE;
                }

                objMediaDescs.Append(objVideoDesc);
            }

            // FramedMedia (message)
            const AStringArray& objFramedProfile =
                    pMediaConfig->GetMediaProfile(strMProf, IMediaConfig::FRAMED);

            if (!objFramedProfile.IsEmpty())
            {
                SdpMediaDescription objFramedDesc;

                if (!objFramedDesc.Decode(objFramedProfile))
                {
                    IMS_TRACE_E(0, "Decoding an SDP(framed) failed", 0, 0, 0);
                    return IMS_FALSE;
                }

                objMediaDescs.Append(objFramedDesc);
            }
        }

        if (!m_pCapabilities->Create(objSessionDesc, objMediaDescs))
        {
            IMS_TRACE_E(0, "Creating an SDP capabilities failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/**
 * @brief Returns the current capabilities session parameter.
 */
PUBLIC
const SessionParameter* SdpOaState::GetCapabilities() const
{
    const SessionParameter& objSessionParam = m_pCapabilities->GetCapabilities();
    return &objSessionParam;
}

/**
 * @brief Gets the SDP message according to the state & other condition.
 */
PUBLIC
IMS_BOOL SdpOaState::GetSdp(OUT AString& strSdp) const
{
    if (m_pProposalView == IMS_NULL)
    {
        IMS_TRACE_D("SDP Offer/Answer :: There is no proposed view ...", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((m_nState == STATE_OFFER_SENT) || (m_nState == STATE_OFFER_CHANGE_SENT))
    {
        // Do not transmit the SDP
        IMS_TRACE_D("SDP Offer/Answer :: OFFER PROGRESSING ...", 0, 0, 0);
        return IMS_FALSE;
    }

    strSdp = m_pProposalView->ToSdp();

    return IMS_TRUE;
}

/**
 * @brief Initiates the offer according to the specified type.
 */
PUBLIC
IMS_BOOL SdpOaState::InitiateOffer(IN IMS_SINT32 nType)
{
    if ((nType <= OFFER_INVALID) || (nType >= OFFER_MAX))
    {
        return IMS_FALSE;
    }

    if ((m_nState != STATE_IDLE) && (m_nState != STATE_ESTABLISHED))
    {
        return IMS_FALSE;
    }

    SessionParameter* pOffer = new SessionParameter();

    if (pOffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (nType == OFFER_NEW)
    {
        // Generate an offer from the capabilities;
        // but in this moment, just create a SDP session parameter.
        (*pOffer) = m_pCapabilities->GetCapabilities();
    }
    else if (nType == OFFER_REFRESH)
    {
        // Get session parameter from the current session parameters
        (*pOffer) = (*m_pCurrentView);
    }
    else if (nType == OFFER_CHANGE)
    {
        (*pOffer) = (*m_pCurrentView);

        // Increase the session version information
        pOffer->IncreaseSessionVersion();

        // (Operator-Specific)
        // Even though the session modification is rejected by the peer,
        // the device needs to upgrade of SDP version field.
        if (m_bAlwaysIncreaseSdpVersion)
        {
            IncreaseSessionVersion();
        }
    }

    SetProposedView(pOffer);

    m_nMode = MODE_OFFERER;
    m_bOfferProgress = IMS_TRUE;

    return IMS_TRUE;
}

/**
 * @brief Handles SDP offer/answer on the received/sent SIP message.
 */
PUBLIC
IMS_SINT32 SdpOaState::HandleOfferAnswer(IN const ISipMessage* piSipMsg)
{
    if (m_bStateChanged == IMS_FALSE)
    {
        IMS_TRACE_I("SDP Offer/Answer - NO STATE CHANGED", 0, 0, 0);
        return SdpOfferAnswer::RESULT_NOT_CHANGED;
    }

    // In case of any pending SDP answer, the message may not include the SDP message body.
    // So, in here, checks the message if it includes the body or not
    const ISipMessageBodyPart* piBodyPart = piSipMsg->GetSdpBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_I("SDP Offer/Answer - NO SDP", 0, 0, 0);
        return SdpOfferAnswer::RESULT_NOT_CHANGED;
    }

    const ByteArray& objSdp = piBodyPart->GetContent();
    const IMS_CHAR* pSdpBody = reinterpret_cast<const IMS_CHAR*>(objSdp.GetData());

    AString strSdp(pSdpBody, objSdp.GetLength());
    SdpParser objParser;

    if (!objParser.Decode(strSdp))
    {
        IMS_TRACE_E(0, "Decoding SDP message failed\r\n-----> SDP\r\n%s",
                SipDebug::GetCharA1(strSdp.GetStr(), 32, '\n'), 0, 0);

        // OA_STATE_ROLLBACK_FOR_MALFORMED_SDP
        if ((m_nState == STATE_ESTABLISHED) &&
                ((m_nOldState == STATE_OFFER_SENT) || (m_nOldState == STATE_OFFER_CHANGE_SENT)))
        {
            // Ignore the SDP answer if SDP body part is malformed...
            SetState(m_nOldState);

            if (m_nOldState == STATE_OFFER_SENT)
            {
                SetOldState(STATE_IDLE);
            }
            else
            {
                SetOldState(STATE_ESTABLISHED);
            }
        }
        else if (((m_nState == STATE_OFFER_RECEIVED) && (m_nOldState == STATE_IDLE)) ||
                ((m_nState == STATE_OFFER_CHANGE_RECEIVED) && (m_nOldState == STATE_ESTABLISHED)))
        {
            // Revert the Offer/Answer mode
            m_nMode = MODE_IDLE;
            // Revert the flag for tracking state changes
            m_bStateChanged = IMS_FALSE;
            // Ignore the SDP offer if SDP body part is malformed...
            SetState(m_nOldState);
        }

        return SdpOfferAnswer::RESULT_FAILURE;
    }

    switch (m_nState)
    {
        case STATE_ESTABLISHED:
        {
            SessionParameter* pPrevLastOfferMade = IMS_NULL;

            // Set the previous proposed view to the last offer made view
            if (m_nMode == MODE_OFFERER)
            {
                pPrevLastOfferMade = m_pLastOfferMade;
                m_pLastOfferMade = IMS_NULL;

                // Update the pLastOfferMade
                SetLastOfferMade(m_pProposalView);
                m_pProposalView = IMS_NULL;
            }

            IMS_SINT32 nResult =
                    HandleAnswer(objParser, m_nMode == MODE_OFFERER || IsInPreviewMode());

            if (m_nMode == MODE_OFFERER)
            {
                if ((nResult == SdpOfferAnswer::RESULT_NOT_DONE) ||
                        (nResult == SdpOfferAnswer::RESULT_FAILURE) ||
                        (nResult == SdpOfferAnswer::RESULT_NOT_FOUND))
                {
                    if ((m_pLastOfferMade != IMS_NULL) && (pPrevLastOfferMade != IMS_NULL))
                    {
                        pPrevLastOfferMade->UpdateRemoteVersion(
                                m_pLastOfferMade->GetRemoteVersion());

                        SessionParameter* pTemp = m_pLastOfferMade;

                        m_pLastOfferMade = pPrevLastOfferMade;
                        pPrevLastOfferMade = IMS_NULL;

                        delete pTemp;
                    }
                    else
                    {
                        if (m_pLastOfferMade != IMS_NULL)
                        {
                            delete m_pLastOfferMade;
                            m_pLastOfferMade = IMS_NULL;
                        }

                        if (pPrevLastOfferMade != IMS_NULL)
                        {
                            m_pLastOfferMade = pPrevLastOfferMade;
                            pPrevLastOfferMade = IMS_NULL;
                        }
                    }
                }
                else
                {
                    if (IsPreviewModeSupported() && m_nOldState == STATE_OFFER_SENT &&
                            piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE &&
                            !(piSipMsg->IsMessageRpr() ||
                                    SipStatusCode::IsFinalSuccess(piSipMsg->GetStatusCode())))
                    {
                        StartPreviewMode();
                    }
                }

                if (pPrevLastOfferMade != IMS_NULL)
                {
                    delete pPrevLastOfferMade;
                }
            }
            else if (IsInPreviewMode() && nResult != SdpOfferAnswer::RESULT_NOT_DONE &&
                    nResult != SdpOfferAnswer::RESULT_FAILURE &&
                    nResult != SdpOfferAnswer::RESULT_NOT_FOUND)
            {
                if (piSipMsg->GetMethod().Equals(SipMethod::INVITE) &&
                        (piSipMsg->IsMessageRpr() ||
                                SipStatusCode::IsFinalSuccess(piSipMsg->GetStatusCode())))
                {
                    StopPreviewMode();
                }
            }

            return nResult;
        }
        case STATE_OFFER_RECEIVED:
        case STATE_OFFER_CHANGE_RECEIVED:
            return HandleOffer(objParser);
        default:
            IMS_TRACE_E(0, "SDP Offer/Answer :: INVALID STATE (%d)", m_nState, 0, 0);
            break;
    }

    return SdpOfferAnswer::RESULT_NOT_CHANGED;
}

/**
 * @brief Completes the offer/answer exchange.
 */
PUBLIC
void SdpOaState::CompleteExchange()
{
    if (m_nState == STATE_ESTABLISHED)
    {
        if (m_pProposalView != IMS_NULL)
        {
            SetCurrentView(m_pProposalView);

            // Update the session parameter related information if pLastOfferMade is present.
            if ((m_nMode == MODE_ANSWERER) && (m_pLastOfferMade != IMS_NULL))
            {
                m_pLastOfferMade->UpdateDirection(m_pProposalView);
                m_pLastOfferMade->UpdateRemoteVersion(m_pProposalView->GetRemoteVersion());
            }
        }
        else
        {
            IMS_TRACE_D("SDP Offer/Answer :: CompleteExchange - no proposal view", 0, 0, 0);
        }
    }
    else
    {
        if (m_pProposalView != IMS_NULL)
        {
            delete m_pProposalView;
        }
    }

    m_pProposalView = IMS_NULL;

    // Revert the Offer/Answer mode
    m_nMode = MODE_IDLE;

    // Revert the flag for tracking state changes
    m_bStateChanged = IMS_FALSE;
}

/**
 * @brief Restores the offer/answer state.
 */
PUBLIC
IMS_BOOL SdpOaState::RestoreState()
{
    IMS_TRACE_I("SDP Offer/Answer - Restore ...", 0, 0, 0);

    if (m_pProposalView != IMS_NULL)
    {
        delete m_pProposalView;
        m_pProposalView = IMS_NULL;
    }

    // Revert the Offer/Answer mode
    m_nMode = MODE_IDLE;

    // Revert the flag for tracking state changes
    m_bStateChanged = IMS_FALSE;
    m_bOfferProgress = IMS_FALSE;

    switch (m_nState)
    {
        case STATE_OFFER_SENT:  // FALL-THROUGH
        case STATE_OFFER_RECEIVED:
            SetState(STATE_IDLE);
            break;
        case STATE_OFFER_CHANGE_SENT:  // FALL-THROUGH
        case STATE_OFFER_CHANGE_RECEIVED:
            SetState(STATE_ESTABLISHED);
            break;
        default:
            break;
    }

    return IMS_TRUE;
}

/**
 * @brief Updates the offer/answer state on the received/sent SIP message.
 */
PUBLIC
IMS_BOOL SdpOaState::UpdateState(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMessageFlow,
        IN IMS_BOOL bIsCallEstablished, IN IMS_BOOL bAllowOaForNonRpr /*= IMS_FALSE*/)
{
    const SipMethod& objMethod = piSipMsg->GetMethod();
    IMS_SINT32 nTrigger = TRIGGER_NONE;
    IMS_BOOL bMessageWithSdp = (piSipMsg->GetSdpBodyPart() != IMS_NULL) ? IMS_TRUE : IMS_FALSE;

    // Revert the flag
    if (nMessageFlow == MESSAGE_SENT)
    {
        m_bOfferProgress = IMS_FALSE;
    }

    m_bStateChanged = IMS_FALSE;

    switch (objMethod.ToInt())
    {
        case SipMethod::INVITE:
            nTrigger = TRIGGER_INVITE;
            break;
        case SipMethod::ACK:
            nTrigger = TRIGGER_ACK;
            break;
        case SipMethod::PRACK:
            nTrigger = TRIGGER_PRACK;
            break;
        case SipMethod::UPDATE:
            nTrigger = TRIGGER_UPDATE;
            break;
        default:
            // No Offer/Answer state transition
            IMS_TRACE_D("SDP Offer/Answer :: No state transition (Method: %s)",
                    objMethod.ToString().GetStr(), 0, 0);
            return IMS_TRUE;
    }

    IMS_SINT32 nOldState = GetState();
    IMS_SINT32 nType = piSipMsg->GetType();

    if (nType == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (nStatusCode == SipStatusCode::SC_100)
        {
            // No Offer/Answer state transition.
            IMS_TRACE_D("SDP Offer/Answer :: No state transition (100 Trying)", 0, 0, 0);
            return IMS_TRUE;
        }
        else if (SipStatusCode::IsProvisional(nStatusCode))
        {
            IMS_BOOL bIsRpr = piSipMsg->IsMessageRpr();

            // We check if the media state is established.
            // If so, we also check if an answer was already received before.
            // Normally, the answers are echoed in the provisional responses.
            // This should be ignored.
            if (nOldState == STATE_ESTABLISHED)
            {
                // Consider as the device received the changed offer only if RPR is received
                if (m_bProvisionalRespWithSdp && !bIsRpr && !IsInPreviewMode())
                {
                    // We don't have to revert the flag here.
                    // This is because there could be still more RPR's coming
                    // and all those should also be ignored.
                    IMS_TRACE_D("SDP Offer/Answer: Ignore non-RPR message with SDP"
                                " on ESTABLISHED state",
                            0, 0, 0);
                    return IMS_TRUE;
                }
            }

            // Check if the incoming message is a RPR
            if (bIsRpr == IMS_TRUE)
            {
                nTrigger = TRIGGER_RPR;
            }
            else
            {
                nTrigger = TRIGGER_PROVISIONAL_RESP;
            }
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            // Special case:
            //    1. When a 200 to INVITE goes after the provisional response
            //      and the provisional response
            //        had an SDP, then the state transition should be IGNORED
            //    2. When a 200 to INVITE is received after the provisional response
            //      and the provisional response
            //        had a SDP, then the state transition should be IGNORED

            if (nOldState == STATE_ESTABLISHED)
            {
                if ((m_bProvisionalRespWithSdp == IMS_TRUE) &&
                        (objMethod.Equals(SipMethod::INVITE)))
                {
                    // We revert back the flag now.
                    // This is because the transaction is complete now.
                    m_bProvisionalRespWithSdp = IMS_FALSE;
                    IMS_TRACE_D("SDP Offer/Answer :: Final response received &"
                                " ProvisionalRespWithSDP on ESTABLISHED state",
                            0, 0, 0);
                    // Consider as the device received the changed offer
                    if (!bAllowOaForNonRpr)
                    {
                        // 4 It should be verified through more testing
                        return IMS_TRUE;
                    }
                }
            }

            nTrigger = TRIGGER_SUCCESS_RESP;
        }
        else
        {
            if ((m_bProvisionalRespWithSdp == IMS_TRUE) && (objMethod.Equals(SipMethod::INVITE)))
            {
                // We revert back the flag now.
                // This is because the transaction is complete now.
                m_bProvisionalRespWithSdp = IMS_FALSE;
                IMS_TRACE_D("SDP Offer/Answer :: Final failure response received &"
                            " ProvisionalRespWithSDP",
                        0, 0, 0);
            }

            nTrigger = TRIGGER_FAILURE_RESP;
        }
    }

    if (nTrigger == TRIGGER_PROVISIONAL_RESP)
    {
        if (bAllowOaForNonRpr || IsInPreviewMode())
        {
            IMS_TRACE_D("SDP Offer/Answer :: non-RPR is allowed", 0, 0, 0);
        }
        else
        {
            // If an unreliable 1xx response contains the SDP message,
            // it needs not to change its state.
            // Either Offer or Answer received through unreliable 1xx response need
            // to be received again in the final response.
            IMS_TRACE_D("SDP Offer/Answer :: Trigger event is a provisional response.", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    // If no SDP in the message, do not transit the state
    if (!bMessageWithSdp && (nTrigger != TRIGGER_FAILURE_RESP))
    {
        IMS_TRACE_D("SDP Offer/Answer :: SDP body is not present", 0, 0, 0);
        return IMS_TRUE;
    }

    // 401/407 response to non-INVITE will be ignored when transiting the SDP Offer/Answer state
    if ((nMessageFlow == MESSAGE_RECEIVED) && (nTrigger == TRIGGER_FAILURE_RESP) &&
            (!objMethod.Equals(SipMethod::INVITE)))
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
        {
            IMS_TRACE_D("SDP Offer/Answer :: Ignored; It will be resubmitted ...", 0, 0, 0);

            if (bIsCallEstablished && objMethod.Equals(SipMethod::UPDATE))
            {
                // Do not ignore the message in case of non-Early UPDATE
            }
            else
            {
                return IMS_TRUE;
            }
        }
    }

    IMS_BOOL bNeedToChangeOldState = IMS_FALSE;

    // Now, invoke the state transition table
    if (nMessageFlow == MESSAGE_SENT)
    {
        IMS_SINT32 nUpdateState;

        if ((nUpdateState = STATE_SENT[nOldState][nTrigger]) == STATE_INVALID)
        {
            IMS_TRACE_E(0, "SDP Offer/Answer :: State transition failed", 0, 0, 0);
            return IMS_FALSE;
        }

        SetState(nUpdateState);

        if (GetState() != nOldState)
        {
            m_bStateChanged = IMS_TRUE;
            bNeedToChangeOldState = IMS_TRUE;
            IMS_TRACE_D("SDP Offer/Answer :: SENT - STATE CHANGED", 0, 0, 0);
        }

        if ((nUpdateState == STATE_OFFER_SENT) || (nUpdateState == STATE_OFFER_CHANGE_SENT))
        {
            m_nMode = MODE_OFFERER;
        }

        // Case 1 : Initial offer sent, Case 2 : Initial offer received & its answer sent
        if ((nUpdateState == STATE_OFFER_SENT) || (nOldState == STATE_OFFER_RECEIVED))
        {
            // 3 check the failure response
            SetCapabilities();
        }
    }
    else
    {
        IMS_SINT32 nUpdateState;

        if ((nUpdateState = STATE_RECEIVED[nOldState][nTrigger]) == STATE_INVALID)
        {
            IMS_TRACE_E(0, "SDP Offer/Answer :: State transition failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (objMethod.Equals(SipMethod::UPDATE) && nUpdateState == STATE_OFFER_CHANGE_RECEIVED)
        {
            if (nTrigger == TRIGGER_SUCCESS_RESP)
            {
                IMS_TRACE_I("SDP Offer/Answer: Ignored - stray SDP answer", 0, 0, 0);
                return IMS_TRUE;
            }
            else if (IsInPreviewMode())
            {
                // SessionEx handles the incoming early UPDATE request with 491.
                IMS_TRACE_I("SDP Offer/Answer: Ignored - preview state", 0, 0, 0);
                return IMS_TRUE;
            }
        }

        if (IsInPreviewMode() && nUpdateState == STATE_OFFER_CHANGE_RECEIVED &&
                (nTrigger == TRIGGER_RPR || nTrigger == TRIGGER_PROVISIONAL_RESP ||
                        nTrigger == TRIGGER_SUCCESS_RESP))
        {
            IMS_TRACE_I("SDP Offer/Answer: RECEIVED ON PREVIEW - STATE CHANGED (trigger=%d)",
                    nTrigger, 0, 0);
            m_bStateChanged = IMS_TRUE;
        }
        else
        {
            SetState(nUpdateState);

            if ((GetState() != nOldState) && (bMessageWithSdp == IMS_TRUE))
            {
                m_bStateChanged = IMS_TRUE;
                bNeedToChangeOldState = IMS_TRUE;
                IMS_TRACE_D("SDP Offer/Answer :: RECEIVED - STATE CHANGED", 0, 0, 0);
            }

            if ((nUpdateState == STATE_OFFER_RECEIVED) ||
                    (nUpdateState == STATE_OFFER_CHANGE_RECEIVED))
            {
                m_nMode = MODE_ANSWERER;
            }
        }
    }

    // After the state changed, do something for the special cases.
    if (((nTrigger == TRIGGER_RPR) || (nTrigger == TRIGGER_PROVISIONAL_RESP)) &&
            (m_bStateChanged == IMS_TRUE))
    {
        // Set the flag for the provisional response with SDP.
        // This will be checked against when a 200 OK is sent or received.

        m_bProvisionalRespWithSdp = IMS_TRUE;

        IMS_TRACE_D("SDP Offer/Answer :: Provisional response is received with an SDP", 0, 0, 0);
    }

    // OA_STATE_ROLLBACK_FOR_MALFORMED_SDP
    if (bNeedToChangeOldState)
    {
        SetOldState(nOldState);
    }

    IMS_TRACE_I("SDP Offer/Answer :: UPDATING OFFER/ANSWER SUCCEEDED", 0, 0, 0);

    return IMS_TRUE;
}

/**
 * @brief Updates offer/answer state when SIP transaction is completed without any state transition.
 */
PUBLIC
void SdpOaState::UpdateStateOnTransactionCompleted(
        IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMessageFlow)
{
    (void)nMessageFlow;

    if (!piSipMsg->GetMethod().Equals(SipMethod::INVITE))
    {
        return;
    }

    if (SipStatusCode::IsFinal(piSipMsg->GetStatusCode()))
    {
        if (m_bProvisionalRespWithSdp)
        {
            m_bProvisionalRespWithSdp = IMS_FALSE;
            IMS_TRACE_D("SDP Offer/Answer :: ProvisionalRespWithSdp will be reset", 0, 0, 0);
        }

        if (piSipMsg->GetSdpBodyPart() != IMS_NULL)
        {
            StopPreviewMode();
        }
    }
    else if (piSipMsg->IsMessageRpr())
    {
        if (piSipMsg->GetSdpBodyPart() != IMS_NULL)
        {
            StopPreviewMode();
        }
    }
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
/**
 * @brief Creates a refused SDP view.
 */
PUBLIC
void SdpOaState::CreateRefusedView()
{
    if (m_pRefusedView != IMS_NULL)
    {
        return;
    }

    if (m_pCurrentView != IMS_NULL)
    {
        m_pRefusedView = new SessionParameter(*m_pCurrentView);
    }
    else if (m_pProposalView != IMS_NULL)
    {
        m_pRefusedView = new SessionParameter(*m_pProposalView);
    }
    else if (m_pPeerView != IMS_NULL)
    {
        m_pRefusedView = new SessionParameter(*m_pPeerView);
    }
    else
    {
        m_pRefusedView = new SessionParameter(m_pCapabilities->GetCapabilities());
    }
}

/**
 * @brief Destroy a refused SDP view.
 */
PUBLIC
void SdpOaState::DestroyRefusedView()
{
    if (m_pRefusedView != IMS_NULL)
    {
        delete m_pRefusedView;
        m_pRefusedView = IMS_NULL;
    }
}
// }

/**
 * @brief Increases the local session version for the current view, the capabilities,
 *        and the last offer made view if present.
 */
PUBLIC
void SdpOaState::IncreaseSessionVersion()
{
    m_pCurrentView->IncreaseSessionVersion();
    // Increase the session version of the capabilities
    m_pCapabilities->GetCapabilities().IncreaseSessionVersion();
    // Increase the session verion of the last offered session parameter
    if (m_pLastOfferMade != IMS_NULL)
    {
        m_pLastOfferMade->IncreaseSessionVersion();
    }
}

/**
 * @brief Returns the session parameter for the current capabilities.
 */
PRIVATE
SessionParameter* SdpOaState::GetCurrentCapabilities()
{
    if (m_nState == STATE_ESTABLISHED)
    {
        if (m_pLastOfferMade != IMS_NULL)
        {
            return m_pLastOfferMade;
        }
        else
        {
            return m_pCurrentView;
        }
    }
    else if (m_nState == STATE_OFFER_RECEIVED)
    {
        return &(m_pCapabilities->GetCapabilities());
    }
    else
    {
        if (m_nState == STATE_OFFER_CHANGE_RECEIVED)
        {
            if (m_pLastOfferMade != IMS_NULL)
            {
                return m_pLastOfferMade;
            }
        }

        return m_pCurrentView;
    }
}

/**
 * @brief Creates & returns a new proposal session parameter.
 */
PRIVATE
SessionParameter*& SdpOaState::GetNewProposalView()
{
    if (m_pProposalView != IMS_NULL)
    {
        delete m_pProposalView;
    }

    m_pProposalView = new SessionParameter();

    return m_pProposalView;
}

/**
 * @brief Creates & returns a new remote session parameter.
 */
PRIVATE
SessionParameter*& SdpOaState::GetNewPeerView()
{
    if (m_pPeerView != IMS_NULL)
    {
        delete m_pPeerView;
    }

    m_pPeerView = new SessionParameter();

    return m_pPeerView;
}

/**
 * @brief Handles the SDP answer from the SDP message.
 */
PUBLIC
IMS_SINT32 SdpOaState::HandleAnswer(IN const SdpParser& objParser, IN IMS_BOOL bUpdateRemoteVersion)
{
    SessionParameter* pAnswer = new SessionParameter();

    IMS_TRACE_D("SDP Offer/Answer - HandleAnswer(...)", 0, 0, 0);

    if (pAnswer == IMS_NULL)
    {
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    if (!pAnswer->Create(objParser.GetSessionDescription(), objParser.GetMediaDescriptions()))
    {
        delete pAnswer;

        IMS_TRACE_E(0, "Creating a SessionParameter from SdpParser failed", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Update the remote session version from the origin field
    if (bUpdateRemoteVersion && m_pLastOfferMade != IMS_NULL)
    {
        m_pLastOfferMade->UpdateRemoteVersion(
                pAnswer->GetSessionParameter().GetOrigin().GetSessionVersion());
    }

#if 0
    // Update the last offer received field of the media state if it is an offer
    if ((m_nState == STATE_OFFER_RECEIVED)
            || (m_nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        SetLastOfferReceived(pOffer);
    }
#endif

    SessionParameter* pCurrentCapabilities = GetCurrentCapabilities();

    IMS_TRACE_D("Current SDP version=%s, lastSdpProvidedWithNegotiatedSdp=%s",
            pCurrentCapabilities->GetSessionParameter().GetOrigin().GetSessionVersion().GetStr(),
            _TRACE_B_((m_pCurrentView != IMS_NULL)
                            ? m_pCurrentView->IsLastSdpProvidedWithNegotiatedSdp()
                            : IMS_FALSE),
            0);

    if (pAnswer->GetMediaCount() != pCurrentCapabilities->GetMediaCount())
    {
        delete pAnswer;

        // The count of the answered m-lines is different to the offered m-lines.
        IMS_TRACE_E(0, "Answered SDP does not match with the offered SDP", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Check if the remote view was changed or not

    if ((GetNewProposalView() == IMS_NULL) || (GetNewPeerView() == IMS_NULL))
    {
        delete pAnswer;

        IMS_TRACE_E(0, "Creating a Proposal or Peer View failed", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Now, do the negotiation with the session parameter
    IMS_SINT32 nOptions = 0;
    IMS_SINT32 nOaResult =
            pCurrentCapabilities->ProcessAnswer(pAnswer, m_pProposalView, m_pPeerView, nOptions);

    delete pAnswer;

    return nOaResult;
}

/**
 * @brief Handles the SDP offer from the SDP message.
 */
PUBLIC
IMS_SINT32 SdpOaState::HandleOffer(IN const SdpParser& objParser)
{
    SessionParameter* pOffer = new SessionParameter();

    IMS_TRACE_D("SDP Offer/Answer - HandleOffer(...)", 0, 0, 0);

    if (pOffer == IMS_NULL)
    {
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    if (!pOffer->Create(objParser.GetSessionDescription(), objParser.GetMediaDescriptions()))
    {
        delete pOffer;
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Update the remote session version from the origin field
    pOffer->UpdateRemoteVersion(pOffer->GetSessionParameter().GetOrigin().GetSessionVersion());

#if 0
    // Update the last offer received field of the media state if it is an offer
    if ((m_nState == STATE_OFFER_RECEIVED)
            || (m_nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        SetLastOfferReceived(pOffer);
    }
#endif

    // Check if the remote view was changed or not

    // Now, do the negotiation with the session parameter
    SessionParameter* pCurrentCapabilities = GetCurrentCapabilities();

    if (pCurrentCapabilities == IMS_NULL)
    {
        delete pOffer;

        IMS_TRACE_E(0, "No current capabilities", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    IMS_SINT32 nOaResult;

    IMS_TRACE_D("Current SDP version=%s, lastSdpProvidedWithNegotiatedSdp=%s",
            pCurrentCapabilities->GetSessionParameter().GetOrigin().GetSessionVersion().GetStr(),
            _TRACE_B_((m_pCurrentView != IMS_NULL)
                            ? m_pCurrentView->IsLastSdpProvidedWithNegotiatedSdp()
                            : IMS_FALSE),
            0);

    if ((GetNewProposalView() == IMS_NULL) || (GetNewPeerView() == IMS_NULL))
    {
        delete pOffer;

        return SdpOfferAnswer::RESULT_NOT_DONE;
    }

    // If the initial offer & local capabilities does not exist,
    // then copies all the attributes from the offer
    if ((m_nState == STATE_OFFER_RECEIVED) && (pCurrentCapabilities->GetMediaCount() == 0))
    {
        nOaResult = pCurrentCapabilities->GenerateAnswer(pOffer, m_pProposalView, m_pPeerView);

        m_pProposalView->SetLastSdpProvidedWithNegotiatedSdp(IMS_TRUE);
    }
    else
    {
        IMS_SINT32 nOptions = SdpOfferAnswer::F_MEDIA_PARAM;

        if ((m_nState == STATE_IDLE) || (m_nState == STATE_ESTABLISHED))
        {
            nOptions |= SdpOfferAnswer::F_MEDIA_GROUP;
        }

        // Check the peer status
        if (pOffer->GetMediaCount() == 0)
        {
            if ((m_nState == STATE_OFFER_RECEIVED) ||
                    (m_nState == STATE_OFFER_CHANGE_RECEIVED &&
                            m_pCurrentView->GetMediaCount() == 0))
            {
                nOptions &= ~(SdpOfferAnswer::F_MEDIA_PARAM);
            }
        }

        // Check if any changes are present in SDP from the session-version field in o-line.
        if ((m_nState == STATE_OFFER_CHANGE_RECEIVED) &&
                (m_bSdpVersionCheck && pCurrentCapabilities->IsSameVersion(pOffer)))
        {
            if (!m_pCurrentView->IsLastSdpProvidedWithNegotiatedSdp())
            {
                m_pCurrentView->SetLastSdpProvidedWithNegotiatedSdp(IMS_TRUE);
                IncreaseSessionVersion();
            }

            (*m_pProposalView) = (*m_pCurrentView);
            (*m_pPeerView) = (*pOffer);
            delete pOffer;
            return SdpOfferAnswer::RESULT_NOT_CHANGED;
        }

        IMS_BOOL bInitialOffer = (m_nState == STATE_OFFER_CHANGE_RECEIVED) ? IMS_FALSE : IMS_TRUE;

        nOaResult = pCurrentCapabilities->GenerateAnswer(
                pOffer, m_pProposalView, m_pPeerView, nOptions, bInitialOffer);
    }

    // if ((nOaResult == SdpOfferAnswer::RESULT_SUCCESS)
    //         && ((m_nState == STATE_OFFER_RECEIVED)
    //                 || (m_nState == STATE_OFFER_CHANGE_RECEIVED)))
    if (((nOaResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)) &&
            (m_nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        // Sets this flag in the proposal view because the negotiated SDP will be sent
        // with the selected codec.
        m_pProposalView->SetLastSdpProvidedWithNegotiatedSdp(IMS_TRUE);

        // Increase the session version of the negotiated session
        m_pProposalView->IncreaseSessionVersion();
        // Increase the session version of the capabilities session
        m_pCapabilities->GetCapabilities().IncreaseSessionVersion();
        // Increase the session verion of the last offered session parameter
        if (m_pLastOfferMade != IMS_NULL)
        {
            m_pLastOfferMade->IncreaseSessionVersion();
        }
    }

    delete pOffer;

    return nOaResult;
}

/**
 * @brief Sets the capabilities from the incoming proposal parameter.
 */
PRIVATE
void SdpOaState::SetCapabilities()
{
    if (m_pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Proposed view is NULL", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("SetCapabilities() - Capabilities is updated by the received offer ...", 0, 0, 0);
    m_pCapabilities->GetCapabilities() = (*m_pProposalView);
}

/**
 * @brief Updates the proposal session parameter from the specified session parameter.
 */
PRIVATE
void SdpOaState::SetProposedView(IN SessionParameter* pSessionParam)
{
    if (m_pProposalView != IMS_NULL)
    {
        delete m_pProposalView;
    }

    m_pProposalView = pSessionParam;
}

/**
 * @brief Updates the current session parameter from the specified session parameter.
 */
PRIVATE
void SdpOaState::SetCurrentView(IN SessionParameter* pSessionParam)
{
    if (m_pCurrentView != IMS_NULL)
    {
        delete m_pCurrentView;
    }

    m_pCurrentView = pSessionParam;
}

/**
 * @brief Updates the last offered session parameter from the specified session parameter.
 */
PRIVATE
void SdpOaState::SetLastOfferMade(IN SessionParameter* pSessionParam)
{
    if (m_pLastOfferMade != IMS_NULL)
    {
        delete m_pLastOfferMade;
    }

    m_pLastOfferMade = pSessionParam;
}

/**
 * @brief Sets the old state of SDP offer/answer.
 */
PRIVATE
void SdpOaState::SetOldState(IN IMS_SINT32 nOldState)
{
    IMS_TRACE_I("SDP Offer/Answer (OldState) :: %s to %s", StateToString(m_nOldState),
            StateToString(nOldState), 0);

    m_nOldState = nOldState;
}

/**
 * @brief Sets the state of SDP offer/answer.
 */
PRIVATE
void SdpOaState::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("SDP Offer/Answer :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE
void SdpOaState::StartPreviewMode()
{
    if (!m_bPreviewMode)
    {
        m_bPreviewMode = IMS_TRUE;
        IMS_TRACE_I("SDP Offer/Answer: Preview mode started", 0, 0, 0);
    }
}

PRIVATE
void SdpOaState::StopPreviewMode()
{
    if (m_bPreviewMode)
    {
        m_bPreviewMode = IMS_FALSE;
        IMS_TRACE_I("SDP Offer/Answer: Preview mode stopped", 0, 0, 0);
    }
}

PRIVATE GLOBAL const IMS_CHAR* SdpOaState::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_IDLE:
            return "STATE_IDLE";
        case STATE_OFFER_SENT:
            return "STATE_OFFER_SENT";
        case STATE_OFFER_RECEIVED:
            return "STATE_OFFER_RECEIVED";
        case STATE_ESTABLISHED:
            return "STATE_ESTABLISHED";
        case STATE_OFFER_CHANGE_SENT:
            return "STATE_OFFER_CHANGE_SENT";
        case STATE_OFFER_CHANGE_RECEIVED:
            return "STATE_OFFER_CHANGE_RECEIVED";
        default:
            return "__INVALID__";
    }
}
