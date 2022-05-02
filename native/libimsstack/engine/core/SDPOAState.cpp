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
#include "private/ConfigurationManager.h"
#include "private/MediaConfig.h"
#include "ISipMessage.h"
#include "SipDebug.h"
#include "SipStatusCode.h"
#include "Service.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpSessionParameter.h"
#include "SDPOAState.h"

__IMS_TRACE_TAG_IMS_CORE__;



class SessionCapabilities
{
public:
    inline SessionCapabilities()
    {
    }

    inline ~SessionCapabilities()
    {
    }

    inline IMS_BOOL Create(IN CONST SdpSessionDescription &objSessionDesc)
    {
        IMSList<SdpMediaDescription> objMediaDescs;

        //-----------------------------------------------------------------------------------------

        if (!objCapabilities.Create(objSessionDesc, objMediaDescs))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    inline IMS_BOOL Create(IN CONST SdpSessionDescription &objSessionDesc,
            IN CONST IMSList<SdpMediaDescription> &objMediaDescs)
    {
        //-----------------------------------------------------------------------------------------

        if (!objCapabilities.Create(objSessionDesc, objMediaDescs))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    inline const SessionParameter& GetCapabilities() const
    {
        return objCapabilities;
    }

    inline SessionParameter& GetCapabilities()
    {
        return objCapabilities;
    }

#if 0
    inline SessionParameter* GetCapabilities() const
    {
        return &objCapabilities;
    }

    inline void GetBasicReliableMedias(OUT IMSList<SdpMediaParameter*> &objMediaParams) const
    {
        const IMSList<SdpMediaParameter*> &objMedias = objCapabilities.GetMediaParameters();

        //-----------------------------------------------------------------------------------------

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

    inline void GetBasicUnreliableMedias(OUT IMSList<SdpMediaParameter*> &objMediaParams) const
    {
        const IMSList<SdpMediaParameter*> &objMedias = objCapabilities.GetMediaParameters();

        //-----------------------------------------------------------------------------------------

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

    inline void GetFramedMedias(OUT IMSList<SdpMediaParameter*> &objMediaParams) const
    {
        const IMSList<SdpMediaParameter*> &objMedias = objCapabilities.GetMediaParameters();

        //-----------------------------------------------------------------------------------------

        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const SdpMediaParameter *pMediaParam = objMedias.GetAt(i);
            IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

            if ((nTransportProtocol == SdpMedia::TRANSPORT_TCP_MSRP)
                || (nTransportProtocol == SdpMedia::TRANSPORT_TCP_TLS_MSRP))
            {
                objMediaParams.Append(pMediaParam);
            }
        }
    }

    inline void GetStreamMedias(OUT IMSList<SdpMediaParameter*> &objMediaParams) const
    {
        const IMSList<SdpMediaParameter*> &objMedias = objCapabilities.GetMediaParameters();

        //-----------------------------------------------------------------------------------------

        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            const SdpMediaParameter *pMediaParam = objMedias.GetAt(i);
            IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

            if ((nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVP)
                    || (nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVPF)
                    || (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP)
                    || (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVPF))
            {
                objMediaParams.Append(pMediaParam);
            }
        }
    }

public:
    SessionParameter objCapabilities;
#endif

public:
    SessionParameter objCapabilities;
};



PRIVATE GLOBAL
const IMS_SINT32 SDPOAState::STATE_SENT[SDPOAState::STATE_MAX][SDPOAState::TRIGGER_MAX] =
{
    //STATE_IDLE
    {
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_INVITE
        SDPOAState::STATE_IDLE,                    // TRIGGER_ACK
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_PRACK
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_RPR
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_IDLE,                    // TRIGGER_CANCEL
        SDPOAState::STATE_IDLE                     // TRIGGER_BYE
    },

    //STATE_OFFER_SENT -- Offer sent by Engine
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_INVALID,                 // TRIGGER_ACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_INVALID,                 // TRIGGER_RPR
        SDPOAState::STATE_INVALID,                 // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_SENT               // TRIGGER_BYE
    },

    // STATE_OFFER_RECEIVED -- Offer received by Engine
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_RECEIVED           // TRIGGER_BYE
    },

    // STATE_ESTABLISHED -- Offer/Answer exchange done
    {
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_INVITE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_PRACK
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_RPR
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_CANCEL
        SDPOAState::STATE_ESTABLISHED              // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_SENT -- Counter offer made
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_INVALID,                 // TRIGGER_ACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_INVALID,                 // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_RPR
        SDPOAState::STATE_INVALID,                 // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_CHANGE_SENT        // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_RECEIVED -- Counter offer received
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED    // TRIGGER_BYE
    }
};

PRIVATE GLOBAL
const IMS_SINT32 SDPOAState::STATE_RECEIVED[SDPOAState::STATE_MAX][SDPOAState::TRIGGER_MAX] =
{
    //STATE_IDLE
    {
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_INVITE
        SDPOAState::STATE_INVALID,                 // TRIGGER_ACK
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_PRACK
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_RPR
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_INVALID,                 // TRIGGER_CANCEL
        SDPOAState::STATE_INVALID                  // TRIGGER_BYE
    },

    //STATE_OFFER_SENT -- Offer sent by Engine
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_IDLE,                    // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_SENT,              // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_SENT               // TRIGGER_BYE
    },

    // STATE_OFFER_RECEIVED -- Offer received by Engine
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_INVALID,                 // TRIGGER_ACK
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_RPR
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_RECEIVED,          // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_RECEIVED           // TRIGGER_BYE
    },

    // STATE_ESTABLISHED -- Offer/Answer exchange done
    {
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_INVITE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PRACK
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_RPR
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_CANCEL
        SDPOAState::STATE_ESTABLISHED              // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_SENT -- Counter offer made
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_ACK
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_RPR
        SDPOAState::STATE_ESTABLISHED,             // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_CHANGE_SENT,       // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_CHANGE_SENT        // TRIGGER_BYE
    },

    // STATE_OFFER_CHANGE_RECEIVED -- Counter offer received
    {
        SDPOAState::STATE_INVALID,                 // TRIGGER_INVITE
        SDPOAState::STATE_INVALID,                 // TRIGGER_ACK
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PRACK
        SDPOAState::STATE_INVALID,                 // TRIGGER_UPDATE
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_SUCCESS_RESP
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_FAILURE_RESP
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_RPR
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_PROVISIONAL_RESP
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED,   // TRIGGER_CANCEL
        SDPOAState::STATE_OFFER_CHANGE_RECEIVED    // TRIGGER_BYE
    }
};



/*

Remarks

*/
PUBLIC
SDPOAState::SDPOAState(IN IMS_BOOL bSDPVersionCheck_ /* = IMS_TRUE */,
        IN IMS_BOOL bAlwaysIncreaseSDPVersion_/* = IMS_FALSE*/)
    : nState(STATE_IDLE)
    , nOldState(STATE_IDLE)
    , nMode(MODE_IDLE)
    , bStateChanged(IMS_FALSE)
    , bOfferProgress(IMS_FALSE)
    , bProvisionalRespWithSdp(IMS_FALSE)
    , bSDPVersionCheck(bSDPVersionCheck_)
    , bAlwaysIncreaseSDPVersion(bAlwaysIncreaseSDPVersion_)
    , pLastOfferMade(IMS_NULL)
    , pCurrentView(IMS_NULL)
    , pPeerView(IMS_NULL)
    , pProposalView(IMS_NULL)
    , pRefusedView(IMS_NULL)
    , pCapabilities(new SessionCapabilities())
{
}

/*

Remarks

*/
PUBLIC
SDPOAState::SDPOAState(IN CONST SDPOAState &objRHS)
    : nState(objRHS.nState)
    , nOldState(objRHS.nOldState)
    , nMode(objRHS.nMode)
    , bStateChanged(objRHS.bStateChanged)
    , bOfferProgress(objRHS.bOfferProgress)
    , bProvisionalRespWithSdp(objRHS.bProvisionalRespWithSdp)
    , bSDPVersionCheck(objRHS.bSDPVersionCheck)
    , bAlwaysIncreaseSDPVersion(objRHS.bAlwaysIncreaseSDPVersion)
    , pLastOfferMade(IMS_NULL)
    , pCurrentView(IMS_NULL)
    , pPeerView(IMS_NULL)
    , pProposalView(IMS_NULL)
    , pRefusedView(IMS_NULL)
    , pCapabilities(IMS_NULL)
{
    if (objRHS.pLastOfferMade)
        pLastOfferMade = new SessionParameter(*(objRHS.pLastOfferMade));

    if (objRHS.pCurrentView)
        pCurrentView = new SessionParameter(*(objRHS.pCurrentView));

    if (objRHS.pPeerView)
        pPeerView = new SessionParameter(*(objRHS.pPeerView));

    if (objRHS.pProposalView)
        pProposalView = new SessionParameter(*(objRHS.pProposalView));

    if (objRHS.pRefusedView)
        pRefusedView = new SessionParameter(*(objRHS.pRefusedView));

    if (objRHS.pCapabilities)
        pCapabilities = new SessionCapabilities(*(objRHS.pCapabilities));
}

/*

Remarks

*/
PUBLIC VIRTUAL
SDPOAState::~SDPOAState()
{
    //---------------------------------------------------------------------------------------------

    if (pLastOfferMade != IMS_NULL)
        delete pLastOfferMade;

    if (pCurrentView != IMS_NULL)
        delete pCurrentView;

    if (pPeerView != IMS_NULL)
        delete pPeerView;

    if (pProposalView != IMS_NULL)
        delete pProposalView;

    if (pRefusedView != IMS_NULL)
        delete pRefusedView;

    if (pCapabilities != IMS_NULL)
        delete pCapabilities;
}

/*
 Aborts the proposal session parameter.

Remarks

*/
PUBLIC VIRTUAL
void SDPOAState::AbortProposal()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("SDP Offer/Answer :: AbortProposal (%s)",
            (pProposalView == IMS_NULL) ? "NULL" : "non-NULL", 0, 0);

    if (pProposalView == IMS_NULL)
        return;

    delete pProposalView;
    pProposalView = IMS_NULL;

    nMode = MODE_IDLE;
    bOfferProgress = IMS_FALSE;
}

/*
 Creates a new proposal session parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::CreateProposalView()
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_IDLE)
            && (nState != STATE_OFFER_RECEIVED)
            && (nState != STATE_ESTABLISHED)
            && (nState != STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_E(0, "Invalid state (%d)", nState, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    if (pProposalView != IMS_NULL)
    {
        return ISDPOAState::RESULT_ALREADY_EXIST;
    }

    // Create a new SDP offer
    if (!InitiateOffer(OFFER_CHANGE))
    {
        IMS_TRACE_E(0, "Initiating new offer failed", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Gets the current local session parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::GetSessionCurrentView(OUT SdpSessionParameter *&pSessionParam) const
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_ESTABLISHED)
    {
        IMS_TRACE_D("SessionCurrentView :: Invalid state (%d)", nState, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    if (pCurrentView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Current View does not exist)", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    pSessionParam = &(pCurrentView->GetSessionParameterNC());

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Gets the current remote session parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::GetSessionPeerView(OUT SdpSessionParameter *&pSessionParam) const
{
    //---------------------------------------------------------------------------------------------

    if (pPeerView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Peer View does not exist)", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    pSessionParam = &(pPeerView->GetSessionParameterNC());

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Gets the current proposal session parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::GetSessionProposalView(OUT SdpSessionParameter *&pSessionParam) const
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_IDLE)
            && (nState != STATE_OFFER_SENT)
            && (nState != STATE_OFFER_RECEIVED)
            && (nState != STATE_ESTABLISHED)
            && (nState != STATE_OFFER_CHANGE_SENT)
            && (nState != STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_E(0, "Invalid state (%d)", nState, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    if (pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Proposed View does not exist)", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    pSessionParam = &(pProposalView->GetSessionParameterNC());

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Creates a new media parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::CreateMediaParameter(OUT SdpMediaParameter *&pMediaParam)
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid state (Proposed View is NULL)", 0, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    if (!bOfferProgress)
    {
        IMS_TRACE_E(0, "Invalid state (Offer is not in progress)", 0, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    pMediaParam = pProposalView->CreateMediaParameter();

    if (pMediaParam == IMS_NULL)
        return ISDPOAState::RESULT_ERROR;

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Gets the current local media parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::GetMediaCurrentView(IN IMS_SINT32 nMid,
        OUT SdpMediaParameter *&pMediaParam) const
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_ESTABLISHED)
    {
        IMS_TRACE_D("MediaCurrentView :: Invalid state (%d)", nState, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    if (pCurrentView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Current View does not exist)", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    pMediaParam = pCurrentView->GetMediaParameter(nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find the media parameter (%d) from the current view", nMid, 0, 0);
        return ISDPOAState::RESULT_NOT_FOUND;
    }

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Gets the current remote media parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::GetMediaPeerView(IN IMS_SINT32 nMid,
        OUT SdpMediaParameter *&pMediaParam) const
{
    //---------------------------------------------------------------------------------------------

    /*
    if (nState != STATE_ESTABLISHED)
    {
        IMS_TRACE_E(0, "Invalid state (%d)", nState, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }
    */

    if (pPeerView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Peer View does not exist)", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    pMediaParam = pPeerView->GetMediaParameter(nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_D("Can't find the media parameter (%d) from the peer view", nMid, 0, 0);

        if (pProposalView != IMS_NULL)
        {
            pMediaParam = pProposalView->GetMediaParameter(nMid);

            if (pMediaParam == IMS_NULL)
            {
                IMS_TRACE_E(0, "Can't find the media parameter (%d) from the proposal view",
                        nMid, 0, 0);
                return ISDPOAState::RESULT_NOT_FOUND;
            }
        }
        else
        {
            return ISDPOAState::RESULT_NOT_FOUND;
        }
    }

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Gets the current proposal media parameter.

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SDPOAState::GetMediaProposalView(IN IMS_SINT32 nMid,
        OUT SdpMediaParameter *&pMediaParam) const
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_IDLE)
            && (nState != STATE_OFFER_SENT)
            && (nState != STATE_OFFER_RECEIVED)
            && (nState != STATE_ESTABLISHED)
            && (nState != STATE_OFFER_CHANGE_SENT)
            && (nState != STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_E(0, "Invalid state (%d)", nState, 0, 0);
        return ISDPOAState::RESULT_INVALID_STATE;
    }

    if (pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error (Proposed View does not exist)", 0, 0, 0);
        return ISDPOAState::RESULT_ERROR;
    }

    pMediaParam = pProposalView->GetMediaParameter(nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find the media parameter (%d) from the proposed view", nMid, 0, 0);
        return ISDPOAState::RESULT_NOT_FOUND;
    }

    return ISDPOAState::RESULT_SUCCESS;
}

/*
 Marks the current proposal media parameter as a rejected or removed.

Remarks

*/
PUBLIC VIRTUAL
void SDPOAState::MarkRejectedOrRemoved(IN IMS_SINT32 nMid)
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is rejected or removed from the proposal view", nMid, 0, 0);
        pProposalView->RemoveMediaParameter(nMid, IMS_TRUE);
        return;
    }

    if (pCurrentView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is rejected or removed from the current view", nMid, 0, 0);
        pCurrentView->RemoveMediaParameter(nMid, IMS_TRUE);
    }
}

/*
 Removes the media parameter which the specified mid matches.

Remarks

*/
PUBLIC VIRTUAL
void SDPOAState::RemoveMediaParameter(IN IMS_SINT32 nMid)
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is removed from the proposal view", nMid, 0, 0);
        pProposalView->RemoveMediaParameter(nMid, IMS_FALSE);
        return;
    }

    if (pCurrentView != IMS_NULL)
    {
        IMS_TRACE_I("___ Media (%d) is removed from the current view", nMid, 0, 0);
        pCurrentView->RemoveMediaParameter(nMid, IMS_FALSE);
    }
}

/*
 Creates a new session parameter for the local capabilities.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::CreateCapabilities(IN Service *pService, IN CONST AString &strUserID,
        IN IMS_BOOL bMProf /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_IDLE)
            && (nState != STATE_OFFER_RECEIVED))
    {
        IMS_TRACE_E(0, "Not allowed to create SDP capabilities in state (%d)", nState, 0, 0);
        return IMS_FALSE;
    }

    SdpSessionDescription objSessionDesc;

    if (!objSessionDesc.CreateMandatoryLines(strUserID, pService->GetIPAddress()))
    {
        return IMS_FALSE;
    }

    if (!bMProf)
    {
        if (!pCapabilities->Create(objSessionDesc))
        {
            IMS_TRACE_E(0, "Creating an SDP capabilities for only session-level description",
                    0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        // Read the media capabilities from the media profile
        const CoreServiceConfig *pServiceConfig = pService->GetServiceConfig();
        const AString &strMProf = (pServiceConfig != IMS_NULL) ? \
                    pServiceConfig->GetMediaProfile() : AString::ConstNull();
        const MediaConfig *pMediaConfig = ConfigurationManager::GetInstance()->GetMediaConfig(
                pService->GetSlotId());
        IMSList<SdpMediaDescription> objMediaDescs;

        if (pMediaConfig != IMS_NULL)
        {
            // StreamMedia (audio)
            const AStringArray &objAudioProfile
                    = pMediaConfig->GetMediaProfile(strMProf, IMediaConfig::STREAM_AUDIO);

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
            const AStringArray &objVideoProfile
                    = pMediaConfig->GetMediaProfile(strMProf, IMediaConfig::STREAM_VIDEO);

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
            const AStringArray &objFramedProfile
                    = pMediaConfig->GetMediaProfile(strMProf, IMediaConfig::FRAMED);

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

        if (!pCapabilities->Create(objSessionDesc, objMediaDescs))
        {
            IMS_TRACE_E(0, "Creating an SDP capabilities failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*
 Returns the current capabilities session parameter.

Remarks

*/
PUBLIC
const SessionParameter* SDPOAState::GetCapabilities() const
{
    const SessionParameter &objSessionParam = pCapabilities->GetCapabilities();

    //---------------------------------------------------------------------------------------------

    return &objSessionParam;
}

/*
 Returns the current local session parameter.

Remarks

*/
PUBLIC
SessionParameter* SDPOAState::GetCurrentView() const
{
    //---------------------------------------------------------------------------------------------

    return pCurrentView;
}

/*
 Returns the peer session parameter.

Remarks

*/
PUBLIC
SessionParameter* SDPOAState::GetPeerView() const
{
    //---------------------------------------------------------------------------------------------

    return pPeerView;
}

/*
 Returns the current proposal session parameter.

Remarks

*/
PUBLIC
SessionParameter* SDPOAState::GetProposalView() const
{
    //---------------------------------------------------------------------------------------------

    return pProposalView;
}

/*
 Returns the mode of SDP offer/answer model.

Remarks

*/
PUBLIC
IMS_SINT32 SDPOAState::GetMode() const
{
    //---------------------------------------------------------------------------------------------

    return nMode;
}

/*
 Gets the SDP message according to the state & other condition.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::GetSDP(OUT AString &strSDP) const
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView == IMS_NULL)
    {
        IMS_TRACE_D("SDP Offer/Answer :: There is no proposed view ...", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((nState == STATE_OFFER_SENT)
            || (nState == STATE_OFFER_CHANGE_SENT))
    {
        // Do not transmit the SDP
        IMS_TRACE_D("SDP Offer/Answer :: OFFER PROGRESSING ...", 0, 0, 0);
        return IMS_FALSE;
    }

    strSDP = pProposalView->ToSDP();

    return IMS_TRUE;
}

/*
 Returns the state of SDP offer/answer.

Remarks

*/
PUBLIC
IMS_SINT32 SDPOAState::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*
 Initiates the offer according to the specified type.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::InitiateOffer(IN IMS_SINT32 nType)
{
    //---------------------------------------------------------------------------------------------

    if ((nType <= OFFER_INVALID) || (nType >= OFFER_MAX))
        return IMS_FALSE;

    if ((nState != STATE_IDLE)
            && (nState != STATE_ESTABLISHED))
    {
        return IMS_FALSE;
    }

    SessionParameter *pOffer = new SessionParameter();

    if (pOffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (nType == OFFER_NEW)
    {
        // Generate an offer from the capabilities;
        // but in this moment, just create a SDP session parameter.
        (*pOffer) = pCapabilities->GetCapabilities();
    }
    else if (nType == OFFER_REFRESH)
    {
        // Get session parameter from the current session parameters
        (*pOffer) = (*pCurrentView);
    }
    else if (nType == OFFER_CHANGE)
    {
        (*pOffer) = (*pCurrentView);

        // Increase the session version information
        pOffer->GetSessionParameterNC().IncreaseSessionVersion();

        // (Operator-Specific)
        // Even though the session modification is rejected by the peer,
        // the device needs to upgrade of SDP version field.
        if (bAlwaysIncreaseSDPVersion)
        {
            pCurrentView->GetSessionParameterNC().IncreaseSessionVersion();
        }
    }

    SetProposedView(pOffer);

    nMode = MODE_OFFERER;
    bOfferProgress = IMS_TRUE;

    return IMS_TRUE;
}

#if 0
/*

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::InitiateMediaOffer(IN CONST AString &strMediaType,
        OUT IMSList<SdpMediaParameter*> &objNewMediaParams)
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_IDLE)
            && (nState != STATE_ESTABLISHED))
    {
        return IMS_FALSE;
    }

    if (pProposedView == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (strMediaType.EqualsIgnoreCase("StreamMedia"))
    {
        pCapabilities->GetStreamMedias(objNewMediaParams);
    }
    else if (strMediaType.EqualsIgnoreCase("FramedMedia"))
    {
        pCapabilities->GetFramedMedias(objNewMediaParams);
    }
    else if (strMediaType.EqualsIgnoreCase("BasicReliableMedia"))
    {
        pCapabilities->GetBasicReliableMedias(objNewMediaParams);
    }
    else if (strMediaType.EqualsIgnoreCase("BasicUnreliabledMedia"))
    {
        pCapabilities->GetBasicUnreliableMedias(objNewMediaParams);
    }

    if (!objNewMediaParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objNewMediaParams.GetSize(); ++i)
        {
            const SdpMediaParameter *pNewMediaParam = objNewMediaParams.GetAt(i);
            SdpMediaParameter *pMediaParam = pProposedView->CreateMediaParameter();

            if (pMediaParam)
            {
                (*pMediaParam) = (*pNewMediaParam);
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
/*PUBLIC
IMS_BOOL SDPOAState::Create(IN CONST SdpParser &objParser)
{
    SessionParameter *pSessionParam = new SessionParameter();

    //---------------------------------------------------------------------------------------------

    if (pSessionParam == IMS_NULL)
        return IMS_FALSE;

    if (!pSessionParam->Create(
            objParser.GetSessionDescription(), objParser.GetMediaDescriptions()))
    {
        delete pSessionParam;
        return IMS_FALSE;
    }

    // Update the last offer received field of the media state if it is an offer
    if ((nState == STATE_OFFER_RECEIVED)
            || (nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        SetLastOfferReceived(pSessionParam);
        return IMS_TRUE;
    }

    delete pSessionParam;

    return IMS_FALSE;
}*/
#endif

/*
 Checks if the offer is on progress or not.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::IsOfferProgress() const
{
    //---------------------------------------------------------------------------------------------

    return bOfferProgress;
}

/*
 Checks if the session is changed or not.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::IsSessionChanged() const
{
    //---------------------------------------------------------------------------------------------

    return bStateChanged;
}

/*
 Handles SDP offer/answer on the received/sent SIP message.

Remarks

*/
PUBLIC
IMS_SINT32 SDPOAState::HandleOfferAnswer(IN CONST ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (bStateChanged == IMS_FALSE)
    {
        IMS_TRACE_I("SDP Offer/Answer - NO STATE CHANGED", 0, 0, 0);
        return SdpOfferAnswer::RESULT_NOT_CHANGED;
    }

    // In case of any pending SDP answer, the message may not include the SDP message body.
    // So, in here, checks the message if it includes the body or not
    ISIPMessageBodyPart *piBodyPart = piSIPMsg->GetSDPBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_I("SDP Offer/Answer - NO SDP", 0, 0, 0);
        return SdpOfferAnswer::RESULT_NOT_CHANGED;
    }

    const ByteArray &objSDP = piBodyPart->GetContent();
    const IMS_CHAR *pSDPBody = reinterpret_cast<const IMS_CHAR*>(objSDP.GetData());

    AString strSDP(pSDPBody, objSDP.GetLength());
    SdpParser objParser;

    if (!objParser.Decode(strSDP))
    {
        IMS_TRACE_E(0, "Decoding SDP message failed\r\n-----> SDP\r\n%s",
                SIPDebug::GetCharA1(strSDP.GetStr(), 32, '\n'), 0, 0);

        // OA_STATE_ROLLBACK_FOR_MALFORMED_SDP
        if ((nState == STATE_ESTABLISHED)
                && ((nOldState == STATE_OFFER_SENT) || (nOldState == STATE_OFFER_CHANGE_SENT)))
        {
            // Ignore the SDP answer if SDP body part is malformed...
            SetState(nOldState);

            if (nOldState == STATE_OFFER_SENT)
                SetOldState(STATE_IDLE);
            else
                SetOldState(STATE_ESTABLISHED);
        }
        else if (((nState == STATE_OFFER_RECEIVED) && (nOldState == STATE_IDLE))
                || ((nState == STATE_OFFER_CHANGE_RECEIVED) && (nOldState == STATE_ESTABLISHED)))
        {
            // Revert the Offer/Answer mode
            nMode = MODE_IDLE;
            // Revert the flag for tracking state changes
            bStateChanged = IMS_FALSE;
            // Ignore the SDP offer if SDP body part is malformed...
            SetState(nOldState);
        }

        return SdpOfferAnswer::RESULT_FAILURE;
    }

    switch (nState)
    {
    case STATE_ESTABLISHED:
        {
            SessionParameter *pPrevLastOfferMade = IMS_NULL;

            // Set the previous proposed view to the last offer made view
            if (nMode == MODE_OFFERER)
            {
                pPrevLastOfferMade = pLastOfferMade;
                pLastOfferMade = IMS_NULL;

                // Update the pLastOfferMade
                SetLastOfferMade(pProposalView);
                pProposalView = IMS_NULL;
            }

            IMS_SINT32 nResult = HandleAnswer(objParser);

            if (nMode == MODE_OFFERER)
            {
                if ((nResult == SdpOfferAnswer::RESULT_NOT_DONE)
                        || (nResult == SdpOfferAnswer::RESULT_FAILURE)
                        || (nResult == SdpOfferAnswer::RESULT_NOT_FOUND))
                {
                    if ((pLastOfferMade != IMS_NULL) && (pPrevLastOfferMade != IMS_NULL))
                    {
                        pPrevLastOfferMade->UpdateRemoteVersion(
                                pLastOfferMade->GetRemoteVersion());

                        SessionParameter *pTemp = pLastOfferMade;

                        pLastOfferMade = pPrevLastOfferMade;
                        pPrevLastOfferMade = IMS_NULL;

                        delete pTemp;
                    }
                    else
                    {
                        if (pLastOfferMade != IMS_NULL)
                        {
                            delete pLastOfferMade;
                            pLastOfferMade = IMS_NULL;
                        }

                        if (pPrevLastOfferMade != IMS_NULL)
                        {
                            pLastOfferMade = pPrevLastOfferMade;
                            pPrevLastOfferMade = IMS_NULL;
                        }
                    }
                }

                if (pPrevLastOfferMade != IMS_NULL)
                {
                    delete pPrevLastOfferMade;
                }
            }

            return nResult;
        }

    case STATE_OFFER_RECEIVED:
    case STATE_OFFER_CHANGE_RECEIVED:
        return HandleOffer(objParser);

    default:
        IMS_TRACE_E(0, "SDP Offer/Answer :: INVALID STATE (%d)", nState, 0, 0);
        break;
    }

    return SdpOfferAnswer::RESULT_NOT_CHANGED;
}

/*
 Completes the offer/answer exchange.

Remarks

*/
PUBLIC
void SDPOAState::CompleteExchange()
{
    //---------------------------------------------------------------------------------------------

    if (nState == STATE_ESTABLISHED)
    {
        if (pProposalView != IMS_NULL)
        {
            SetCurrentView(pProposalView);

            // Update the direction information if pLastOfferMade is present...
            if ((nMode == MODE_ANSWERER) && (pLastOfferMade != IMS_NULL))
            {
                pLastOfferMade->UpdateDirection(pProposalView);
            }
        }
        else
        {
            IMS_TRACE_D("SDP Offer/Answer :: CompleteExchange - no proposal view", 0, 0, 0);
        }
    }
    else
    {
        if (pProposalView != IMS_NULL)
        {
            delete pProposalView;
        }
    }

    pProposalView = IMS_NULL;

    // Revert the Offer/Answer mode
    nMode = MODE_IDLE;

    // Revert the flag for tracking state changes
    bStateChanged = IMS_FALSE;
}

/*
 Restores the offer/answer state.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::RestoreState()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SDP Offer/Answer - Restore ...", 0, 0, 0);

    if (pProposalView != IMS_NULL)
    {
        delete pProposalView;
        pProposalView = IMS_NULL;
    }

    // Revert the Offer/Answer mode
    nMode = MODE_IDLE;

    // Revert the flag for tracking state changes
    bStateChanged = IMS_FALSE;
    bOfferProgress = IMS_FALSE;

    switch (nState)
    {
    case STATE_OFFER_SENT:
    case STATE_OFFER_RECEIVED:
        SetState(STATE_IDLE);
        break;

    case STATE_OFFER_CHANGE_SENT:
    case STATE_OFFER_CHANGE_RECEIVED:
        SetState(STATE_ESTABLISHED);
        break;

    default:
        break;
    }

    return IMS_TRUE;
}

/*
 Updates the offer/answer state on the received/sent SIP message.

Remarks

*/
PUBLIC
IMS_BOOL SDPOAState::UpdateState(IN CONST ISIPMessage *piSIPMsg, IN IMS_SINT32 nMessageFlow,
        IN IMS_BOOL bIsCallEstablished, IN IMS_BOOL bAllowOAForNonRPR /* = IMS_FALSE */)
{
    const SIPMethod &objMethod = piSIPMsg->GetMethod();
    IMS_SINT32 nTrigger = TRIGGER_NONE;
    IMS_BOOL bMessageWithSDP = (piSIPMsg->GetSDPBodyPart() != IMS_NULL) ? IMS_TRUE : IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    // Revert the flag
    if (nMessageFlow == MESSAGE_SENT)
    {
        bOfferProgress = IMS_FALSE;
    }

    bStateChanged = IMS_FALSE;

    switch (objMethod.ToInt())
    {
    case SIPMethod::INVITE:
        nTrigger = TRIGGER_INVITE;
        break;
    case SIPMethod::ACK:
        nTrigger = TRIGGER_ACK;
        break;
    case SIPMethod::PRACK:
        nTrigger = TRIGGER_PRACK;
        break;
    case SIPMethod::UPDATE:
        nTrigger = TRIGGER_UPDATE;
        break;

    default:
        // No Offer/Answer state transition
        IMS_TRACE_D("SDP Offer/Answer :: No state transition (Method: %s)",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nOldState = GetState();
    IMS_SINT32 nType = piSIPMsg->GetType();

    if (nType == ISIPMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (nStatusCode == SIPStatusCode::SC_100)
        {
            // No Offer/Answer state transition.
            IMS_TRACE_D("SDP Offer/Answer :: No state transition (100 Trying)", 0, 0, 0);
            return IMS_TRUE;
        }
        else if (SIPStatusCode::IsProvisional(nStatusCode))
        {
            IMS_BOOL bIsRPR = piSIPMsg->IsMessageRPR();

            // We check if the media state is established.
            // If so, we also check if an answer was already received before.
            // Normally, the answers are echoed in the provisional responses.
            // This should be ignored.
            if (nOldState == STATE_ESTABLISHED)
            {
                // Consider as the device received the changed offer only if RPR is received
                // if (bProvisionalRespWithSdp == IMS_TRUE)
                if ((bProvisionalRespWithSdp == IMS_TRUE) && (bIsRPR != IMS_TRUE))
                {
                    // We don't have to revert the flag here.
                    // This is because there could be still more RPR's coming
                    // and all those should also be ignored.
                    IMS_TRACE_D("SDP Offer/Answer :: Provisional response with SDP" \
                            " on ESTABLISHED state", 0, 0, 0);
                    return IMS_TRUE;
                }
            }

            // Check if the incoming message is a RPR
            if (bIsRPR == IMS_TRUE)
                nTrigger = TRIGGER_RPR;
            else
                nTrigger = TRIGGER_PROVISIONAL_RESP;
        }
        else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
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
                if ((bProvisionalRespWithSdp == IMS_TRUE)
                        && (objMethod.Equals(SIPMethod::INVITE)))
                {
                    // We revert back the flag now.
                    // This is because the transaction is complete now.
                    bProvisionalRespWithSdp = IMS_FALSE;
                    IMS_TRACE_D("SDP Offer/Answer :: Final response received &" \
                            " ProvisionalRespWithSDP on ESTABLISHED state", 0, 0, 0);
                    // Consider as the device received the changed offer
                    if (!bAllowOAForNonRPR)
                    {
                        //4 It should be verified through more testing
                        return IMS_TRUE;
                    }
                }
            }

            nTrigger = TRIGGER_SUCCESS_RESP;
        }
        else
        {
            if ((bProvisionalRespWithSdp == IMS_TRUE)
                    && (objMethod.Equals(SIPMethod::INVITE)))
            {
                // We revert back the flag now.
                // This is because the transaction is complete now.
                bProvisionalRespWithSdp = IMS_FALSE;
                IMS_TRACE_D("SDP Offer/Answer :: Final failure response received &" \
                        " ProvisionalRespWithSDP", 0, 0, 0);
            }

            nTrigger = TRIGGER_FAILURE_RESP;
        }
    }

    if (nTrigger == TRIGGER_PROVISIONAL_RESP)
    {
        if (bAllowOAForNonRPR)
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
    if (!bMessageWithSDP && (nTrigger != TRIGGER_FAILURE_RESP))
    {
        IMS_TRACE_D("SDP Offer/Answer :: SDP body is not present", 0, 0, 0);
        return IMS_TRUE;
    }

    // 401/407 response to non-INVITE will be ignored when transiting the SDP Offer/Answer state
    if ((nMessageFlow == MESSAGE_RECEIVED)
            && (nTrigger == TRIGGER_FAILURE_RESP) && (!objMethod.Equals(SIPMethod::INVITE)))
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if ((nStatusCode == SIPStatusCode::SC_401) || (nStatusCode == SIPStatusCode::SC_407))
        {
            IMS_TRACE_D("SDP Offer/Answer :: Ignored; It will be resubmitted ...", 0, 0, 0);

            if (bIsCallEstablished && objMethod.Equals(SIPMethod::UPDATE))
            {
                // Do not ignore the message in case of non-Early UPDATE
            }
            else
            {
                return IMS_TRUE;
            }
        }
    }

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
            bStateChanged = IMS_TRUE;

            IMS_TRACE_D("SDP Offer/Answer :: SENT - STATE CHANGED", 0, 0, 0);
        }

        if ((nUpdateState == STATE_OFFER_SENT)
                || (nUpdateState == STATE_OFFER_CHANGE_SENT))
        {
            nMode = MODE_OFFERER;
        }

        // Case 1 : Initial offer sent, Case 2 : Initial offer received & its answer sent
        if ((nUpdateState == STATE_OFFER_SENT)
                || (nOldState == STATE_OFFER_RECEIVED))
        {
            //3 check the failure response
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

        if (objMethod.Equals(SIPMethod::UPDATE)
                && (nTrigger == TRIGGER_SUCCESS_RESP)
                && (nUpdateState == STATE_OFFER_CHANGE_RECEIVED))
        {
            IMS_TRACE_I("SDP Offer/Answer :: Ignored; Stray SDP answer...", 0, 0, 0);
            return IMS_TRUE;
        }

        SetState(nUpdateState);

        if ((GetState() != nOldState) && (bMessageWithSDP == IMS_TRUE))
        {
            bStateChanged = IMS_TRUE;

            IMS_TRACE_D("SDP Offer/Answer :: RECEIVED - STATE CHANGED", 0, 0, 0);
        }

        if ((nUpdateState == STATE_OFFER_RECEIVED)
                || (nUpdateState == STATE_OFFER_CHANGE_RECEIVED))
        {
            nMode = MODE_ANSWERER;
        }
    }

    // After the state changed, do something for the special cases.
    if (((nTrigger == TRIGGER_RPR) || (nTrigger == TRIGGER_PROVISIONAL_RESP))
            && (bStateChanged == IMS_TRUE))
    {
        // Set the flag for the provisional response with SDP.
        // This will be checked against when a 200 OK is sent or received.

        bProvisionalRespWithSdp = IMS_TRUE;

        IMS_TRACE_D("SDP Offer/Answer :: Provisional response is received with an SDP", 0, 0, 0);
    }

    // OA_STATE_ROLLBACK_FOR_MALFORMED_SDP
    if (bStateChanged)
    {
        SetOldState(nOldState);
    }

    IMS_TRACE_I("SDP Offer/Answer :: UPDATING OFFER/ANSWER SUCCEEDED", 0, 0, 0);

    return IMS_TRUE;
}

/*
 Updates offer/answer state when SIP transaction is completed without any state transition.

Remarks

*/
PUBLIC
void SDPOAState::UpdateStateOnTransactionCompleted(
        IN CONST ISIPMessage *piSIPMsg, IN IMS_SINT32 nMessageFlow)
{
    //---------------------------------------------------------------------------------------------

    (void) nMessageFlow;

    if (piSIPMsg->GetMethod().Equals(SIPMethod::INVITE)
            && SIPStatusCode::IsFinal(piSIPMsg->GetStatusCode()))
    {
        if (bProvisionalRespWithSdp)
        {
            bProvisionalRespWithSdp = IMS_FALSE;
            IMS_TRACE_D("SDP Offer/Answer :: ProvisionalRespWithSDP will be reset", 0, 0, 0);
        }
    }
}

// REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
/*
 Creates a refused SDP view.

Remarks

*/
PUBLIC
void SDPOAState::CreateRefusedView()
{
    if (pRefusedView != IMS_NULL)
    {
        return;
    }

    if (pCurrentView != IMS_NULL)
    {
        pRefusedView = new SessionParameter(*pCurrentView);
    }
    else if (pProposalView != IMS_NULL)
    {
        pRefusedView = new SessionParameter(*pProposalView);
    }
    else if (pPeerView != IMS_NULL)
    {
        pRefusedView = new SessionParameter(*pPeerView);
    }
    else
    {
        pRefusedView = new SessionParameter(pCapabilities->GetCapabilities());
    }
}


/*
 Destroy a refused SDP view.

Remarks

*/
PUBLIC
void SDPOAState::DestroyRefusedView()
{
    if (pRefusedView != IMS_NULL)
    {
        delete pRefusedView;
        pRefusedView = IMS_NULL;
    }
}

/*
 Returns a refused SDP view.

Remarks

*/
PUBLIC
SessionParameter* SDPOAState::GetRefusedView() const
{
    return pRefusedView;
}
// }

/*
 Returns the session parameter for the current capabilities.

Remarks

*/
PRIVATE
SessionParameter* SDPOAState::GetCurrentCapabilities()
{
    //---------------------------------------------------------------------------------------------

    if (nState == STATE_ESTABLISHED)
    {
        if (pLastOfferMade != IMS_NULL)
            return pLastOfferMade;
        else
            return pCurrentView;
    }
    else if (nState == STATE_OFFER_RECEIVED)
    {
        return &(pCapabilities->GetCapabilities());
    }
    else
    {
        if (nState == STATE_OFFER_CHANGE_RECEIVED)
        {
            if (pLastOfferMade != IMS_NULL)
                return pLastOfferMade;
        }

        return pCurrentView;
    }
}

/*
 Creates & returns a new proposal session parameter.

Remarks

*/
PRIVATE
SessionParameter*& SDPOAState::GetNewProposalView()
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView != IMS_NULL)
        delete pProposalView;

    pProposalView = new SessionParameter();

    return pProposalView;
}

/*
 Creates & returns a new remote session parameter.

Remarks

*/
PRIVATE
SessionParameter*& SDPOAState::GetNewPeerView()
{
    //---------------------------------------------------------------------------------------------

    if (pPeerView != IMS_NULL)
        delete pPeerView;

    pPeerView = new SessionParameter();

    return pPeerView;
}

/*
 Handles the SDP answer from the SDP message.

Remarks

*/
PUBLIC
IMS_SINT32 SDPOAState::HandleAnswer(IN CONST SdpParser &objParser)
{
    SessionParameter *pAnswer = new SessionParameter();

    //---------------------------------------------------------------------------------------------

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
    if ((nMode == MODE_OFFERER)
            && (pLastOfferMade != IMS_NULL))
    {
        pLastOfferMade->UpdateRemoteVersion(
                pAnswer->GetSessionParameter().GetOrigin().GetSessionVersion());
    }

#if 0
    // Update the last offer received field of the media state if it is an offer
    if ((nState == STATE_OFFER_RECEIVED)
            || (nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        SetLastOfferReceived(pOffer);
    }
#endif

    SessionParameter *pCurrentCapabilities = GetCurrentCapabilities();

    IMS_TRACE_D("Current SDP version=%s",
            pCurrentCapabilities->GetSessionParameter().GetOrigin().GetSessionVersion().GetStr(),
            0, 0);

    if (pAnswer->GetMediaCount() != pCurrentCapabilities->GetMediaCount())
    {
        delete pAnswer;

        // The count of the answered m-lines is different to the offered m-lines.
        IMS_TRACE_E(0, "Answered SDP does not match with the offered SDP", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Check if the remote view was changed or not

    if ((GetNewProposalView() == IMS_NULL)
            || (GetNewPeerView() == IMS_NULL))
    {
        delete pAnswer;

        IMS_TRACE_E(0, "Creating a Proposal or Peer View failed", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Now, do the negotiation with the session parameter
    IMS_SINT32 nOptions = 0;
    IMS_SINT32 nOAResult = pCurrentCapabilities->ProcessAnswer(
            pAnswer, pProposalView, pPeerView, nOptions);

    delete pAnswer;

    return nOAResult;
}

/*
 Handles the SDP offer from the SDP message.

Remarks

*/
PUBLIC
IMS_SINT32 SDPOAState::HandleOffer(IN CONST SdpParser &objParser)
{
    SessionParameter *pOffer = new SessionParameter();

    //---------------------------------------------------------------------------------------------

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
    if ((nState == STATE_OFFER_RECEIVED)
            || (nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        SetLastOfferReceived(pOffer);
    }
#endif

    // Check if the remote view was changed or not

    // Now, do the negotiation with the session parameter
    SessionParameter *pCurrentCapabilities = GetCurrentCapabilities();

    if (pCurrentCapabilities == IMS_NULL)
    {
        delete pOffer;

        IMS_TRACE_E(0, "No current capabilities", 0, 0, 0);
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    IMS_SINT32 nOAResult;

    IMS_TRACE_D("Current SDP version=%s",
            pCurrentCapabilities->GetSessionParameter().GetOrigin().GetSessionVersion().GetStr(),
            0, 0);

    if ((GetNewProposalView() == IMS_NULL)
            || (GetNewPeerView() == IMS_NULL))
    {
        delete pOffer;

        return SdpOfferAnswer::RESULT_NOT_DONE;
    }

    // If the initial offer & local capabilities does not exist,
    // then copies all the attributes from the offer
    if ((nState == STATE_OFFER_RECEIVED)
            && (pCurrentCapabilities->GetMediaCount() == 0))
    {
        nOAResult = pCurrentCapabilities->GenerateAnswer(pOffer, pProposalView, pPeerView);
    }
    else
    {
        IMS_SINT32 nOptions = SdpOfferAnswer::F_MEDIA_PARAM;

        if ((nState == STATE_IDLE) || (nState == STATE_ESTABLISHED))
        {
            nOptions |= SdpOfferAnswer::F_MEDIA_GROUP;
        }

        // Check the peer status
        if (pOffer->GetMediaCount() == 0)
        {
            if (nState == STATE_OFFER_RECEIVED)
            {
                nOptions &= ~(SdpOfferAnswer::F_MEDIA_PARAM);
            }
            else if ((nState == STATE_OFFER_CHANGE_RECEIVED)
                    && (pCurrentView->GetMediaCount() == 0))
            {
                nOptions &= ~(SdpOfferAnswer::F_MEDIA_PARAM);
            }
        }

        // Check if any changes are present in SDP from the session-version field in o-line.
        if ((nState == STATE_OFFER_CHANGE_RECEIVED)
                && (bSDPVersionCheck && pCurrentCapabilities->IsSameVersion(pOffer)))
        {
            (*pProposalView) = (*pCurrentView);
            (*pPeerView) = (*pOffer);

            delete pOffer;

            return SdpOfferAnswer::RESULT_NOT_CHANGED;
        }

        IMS_BOOL bInitialOffer = (nState == STATE_OFFER_CHANGE_RECEIVED) ? IMS_FALSE : IMS_TRUE;

        nOAResult = pCurrentCapabilities->GenerateAnswer(
                pOffer, pProposalView, pPeerView, nOptions, bInitialOffer);
    }

    // Incoming INVITE & Initial offer received
    /*
    if (nState == STATE_OFFER_RECEIVED)
    {
        if ((GetNewProposalView() == IMS_NULL)
                || (GetNewPeerView() == IMS_NULL))
        {
            delete pOffer;

            return SdpOfferAnswer::RESULT_NOT_DONE;
        }

    nOAResult = pOffer->GenerateAnswer(pProposalView, pPeerView);

    // Update the session-level view from the capabilities
    pProposalView->GetSessionParameterNC().UpdateProperties(
            pCapabilities->GetCapabilities().GetSessionParameter());

    // TODO:: update CAPABILITY
    }
    else
    {
    }
    */

    //if ((nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
    //        && ((nState == STATE_OFFER_RECEIVED)
    //                || (nState == STATE_OFFER_CHANGE_RECEIVED)))
    if (((nOAResult == SdpOfferAnswer::RESULT_SUCCESS)
            || (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT))
                && (nState == STATE_OFFER_CHANGE_RECEIVED))
    {
        // Increase the session version of the negotiated session
        pProposalView->GetSessionParameterNC().IncreaseSessionVersion();

        // Increase the session version of the capabilities session
        pCapabilities->GetCapabilities().GetSessionParameterNC().IncreaseSessionVersion();

        // Increase the session verion of the last offered session parameter
        if (pLastOfferMade != IMS_NULL)
        {
            pLastOfferMade->GetSessionParameterNC().IncreaseSessionVersion();
        }
    }

    delete pOffer;

    return nOAResult;
}

/*
 Sets the capabilities from the incoming proposal parameter.

Remarks

*/
PRIVATE
void SDPOAState::SetCapabilities()
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView == IMS_NULL)
    {
        IMS_TRACE_E(0, "Proposed view is NULL", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("SetCapabilities() - Capabilities is updated by the received offer ...", 0, 0, 0);
    pCapabilities->GetCapabilities() = (*pProposalView);
}

/*
 Updates the proposal session parameter from the specified session parameter.

Remarks

*/
PRIVATE
void SDPOAState::SetProposedView(IN SessionParameter *pSessionParam)
{
    //---------------------------------------------------------------------------------------------

    if (pProposalView != IMS_NULL)
        delete pProposalView;

    pProposalView = pSessionParam;
}

/*
 Updates the current session parameter from the specified session parameter.

Remarks

*/
PRIVATE
void SDPOAState::SetCurrentView(IN SessionParameter *pSessionParam)
{
    //---------------------------------------------------------------------------------------------

    if (pCurrentView != IMS_NULL)
        delete pCurrentView;

    pCurrentView = pSessionParam;
}

/*
 Updates the last offered session parameter from the specified session parameter.

Remarks

*/
PRIVATE
void SDPOAState::SetLastOfferMade(IN SessionParameter *pSessionParam)
{
    //---------------------------------------------------------------------------------------------

    if (pLastOfferMade != IMS_NULL)
        delete pLastOfferMade;

    pLastOfferMade = pSessionParam;
}

/*
 Sets the old state of SDP offer/answer.

Remarks

*/
PRIVATE
void SDPOAState::SetOldState(IN IMS_SINT32 nOldState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SDP Offer/Answer (OldState) :: %s to %s",
            StateToString(this->nOldState), StateToString(nOldState), 0);

    this->nOldState = nOldState;
}

/*
 Sets the state of SDP offer/answer.

Remarks

*/
PRIVATE
void SDPOAState::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SDP Offer/Answer :: %s to %s",
            StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* SDPOAState::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

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
