/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20201023  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ImsIdentity.h"
#include "private/SipConfigV.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpProfile.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SipMethod.h"
#include "SipStatusCode.h"

#include "base/IMS.h"
#include "Service.h"
#include "SDPOAState.h"
#include "SessionDescriptor.h"
#include "media/MediaFactory.h"
#include "VirtualSession.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
VirtualSession::VirtualSession(IN Service* pService_, IN const SipAddress* pUserAoR) :
        RCObject(),
        pService(pService_),
        nState(STATE_CREATED),
        pOAState(IMS_NULL),
        pSessionDescriptor(IMS_NULL)
{
    IMS_TRACE_I("VirtualSession - C", 0, 0, 0);

    Init(pUserAoR);
}

PUBLIC
VirtualSession::VirtualSession(IN const VirtualSession& objRHS) :
        RCObject(objRHS)
{
}

PUBLIC VIRTUAL VirtualSession::~VirtualSession()
{
    IMS_TRACE_I("VirtualSession - D", 0, 0, 0);

    if (pSessionDescriptor != IMS_NULL)
    {
        delete pSessionDescriptor;
        pSessionDescriptor = IMS_NULL;
    }

    if (!objMedias.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
        {
            Media* pMedia = objMedias.GetAt(i);

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
PUBLIC
IMS_BOOL VirtualSession::CheckNSetSDPBodyPart(IN_OUT ISipMessage*& piSIPMsg)
{
    IMS_SINT32 nOAState = GetOfferAnswerState();

    if ((nOAState != SDPOAState::STATE_OFFER_RECEIVED) &&
            (nOAState != SDPOAState::STATE_OFFER_CHANGE_RECEIVED))
    {
        IMS_TRACE_I("No SDP offer in VritualSession", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objMedias.IsEmpty())
    {
        RestoreOfferAnswerState();
        IMS_TRACE_D("There is no media", 0, 0, 0);
        return IMS_FALSE;
    }

    // SDP message to be set
    AString strSDP;

    if (!pOAState->GetSDP(strSDP))
    {
        RestoreOfferAnswerState();
        IMS_TRACE_D("There is no SDP message body", 0, 0, 0);
        return IMS_FALSE;
    }

    ISipMessageBodyPart* piBodyPart = piSIPMsg->CreateSdpBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        RestoreOfferAnswerState();
        IMS::SetLastError(IMSError::NO_MEMORY);

        IMS_TRACE_E(0, "Creating SDP body part failed", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objSDP(strSDP);

    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, Sip::STR_APPLICATION_SDP);
    piBodyPart->SetContent(objSDP);

    // Set the Content-Length header
    AString strCLEN;
    strCLEN.SetNumber(objSDP.GetLength());

    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_UNKNOWN, strCLEN, SipHeaderName::CONTENT_LENGTH);

    IMS_TRACE_D("SDP is formed by SDP offer/answer context", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT VirtualSession::Notify18xResponse(IN const ISipMessage* piSIPMsg)
{
    if (piSIPMsg == IMS_NULL)
    {
        SetState(STATE_TERMINATED);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("___ %d response received on %s", piSIPMsg->GetStatusCode(),
            StateToString(GetState()), 0);

    if (!UpdateOfferAnswerStateOnMessageReceived(piSIPMsg))
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

    IMS_SINT32 nOAResult = HandleSDPOfferAnswer(piSIPMsg);

    if ((nOAResult == SdpOfferAnswer::RESULT_FAILURE) ||
            (nOAResult == SdpOfferAnswer::RESULT_NOT_DONE))
    {
        SetState(STATE_TERMINATED);
        return IMS_FAILURE;
    }

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void VirtualSession::NotifyPRAckSent(IN const ISipMessage* piSIPMsg)
{
    // Update the Offer/Answer state
    UpdateOfferAnswerStateOnMessageSent(piSIPMsg);

    // Update the media state
    UpdateMedia(Media::SESSION_EARLY_UPDATE);
}

/*

Remarks

*/
PUBLIC
Media* VirtualSession::CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
        IN IMS_SINT32 nCountOfDescriptor /* = 0 */, IN IMS_BOOL bIMSExtension /* = IMS_TRUE */)
{
    if (pOAState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SDPOAState is null", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_NULL;
    }

    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED) && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSDPOAState = GetOfferAnswerState();

        if ((nSDPOAState != SDPOAState::STATE_IDLE) &&
                (nSDPOAState != SDPOAState::STATE_ESTABLISHED))
        {
            IMS_TRACE_E(0,
                    "To create a media, the state MUST be an INITIALIZED or ESTABLISHED; "
                    "(%s, %d)",
                    StateToString(nState), nSDPOAState, 0);
            IMS::SetLastError(IMSError::ILLEGAL_STATE);
            return IMS_NULL;
        }
    }

    if ((nDirection != Media::DIRECTION_INACTIVE) && (nDirection != Media::DIRECTION_RECEIVE) &&
            (nDirection != Media::DIRECTION_SEND) && (nDirection != Media::DIRECTION_SEND_RECEIVE))
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

    Media* pMedia = MediaFactory::CreateOutgoingMedia(
            strType, nDirection, pService, pOAState, nCountOfDescriptor);

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
const IMSList<Media*>& VirtualSession::GetMedia() const
{
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
SessionDescriptor* VirtualSession::GetSessionDescriptor()
{
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
IMS_SINT32 VirtualSession::GetState() const
{
    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT VirtualSession::RemoveMedia(IN Media* pMedia)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED) && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSDPOAState = GetOfferAnswerState();

        if ((nSDPOAState != SDPOAState::STATE_IDLE) &&
                (nSDPOAState != SDPOAState::STATE_ESTABLISHED))
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);

            IMS_TRACE_E(0,
                    "To remove a media, the state MUST be a INITIATED or ESTABLISHED; "
                    "(%s, %d)",
                    StateToString(nState), nSDPOAState, 0);
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
        const Media* pExistingMedia = objMedias.GetAt(i);

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
    Media* pMatchedMedia = objMedias.GetAt(i);

    pMatchedMedia->RemoveMedia();

    if ((pMatchedMedia->GetState() == Media::STATE_INACTIVE) ||
            (pMatchedMedia->GetState() == Media::STATE_DELETED))
    {
        objMedias.RemoveAt(i);
    }

    if (bMidSyncRequired)
    {
        for (IMS_UINT32 j = i; j < objMedias.GetSize(); j++)
        {
            Media* pMedia = objMedias.GetAt(j);
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
IMS_RESULT VirtualSession::RemoveMedia(IN IMS_UINT32 nIndex)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INITIATED) && (nState != STATE_ESTABLISHED))
    {
        IMS_SINT32 nSDPOAState = GetOfferAnswerState();

        if ((nSDPOAState != SDPOAState::STATE_IDLE) &&
                (nSDPOAState != SDPOAState::STATE_ESTABLISHED))
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);

            IMS_TRACE_E(0,
                    "To remove a media, the state MUST be a INITIATED or ESTABLISHED; "
                    "(%s, %d)",
                    StateToString(nState), nSDPOAState, 0);
            return IMS_FAILURE;
        }
    }

    if (nIndex >= objMedias.GetSize())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Invalid index (%d) in the total size (%d)", nIndex, objMedias.GetSize(), 0);
        return IMS_FAILURE;
    }

    IMS_BOOL bMidSyncRequired = (nIndex < (objMedias.GetSize() - 1));
    Media* pMedia = objMedias.GetAt(nIndex);

    if (pMedia == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pMedia->RemoveMedia();

    if ((pMedia->GetState() == Media::STATE_INACTIVE) ||
            (pMedia->GetState() == Media::STATE_DELETED))
    {
        objMedias.RemoveAt(nIndex);
    }

    if (bMidSyncRequired)
    {
        for (IMS_UINT32 j = nIndex; j < objMedias.GetSize(); j++)
        {
            Media* pMedia = objMedias.GetAt(j);
            pMedia->SetMid(j);
        }
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PROTECTED VIRTUAL const AString& VirtualSession::GetConnectionAddress() const
{
    if (pOAState == IMS_NULL)
    {
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

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
PROTECTED VIRTUAL IMS_SINT32 VirtualSession::GetSessionState() const
{
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
PROTECTED VIRTUAL SdpSessionParameter* VirtualSession::GetSessionParameter() const
{
    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

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
PROTECTED VIRTUAL const AString& VirtualSession::GetPeerConnectionAddress() const
{
    if (pOAState == IMS_NULL)
    {
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

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
PROTECTED VIRTUAL SdpSessionParameter* VirtualSession::GetPeerSessionParameter() const
{
    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    pOAState->GetSessionPeerView(pSessionParam);

    return pSessionParam;
}

/*

Remarks

*/
PROTECTED VIRTUAL SdpSessionParameter* VirtualSession::GetProposalSessionParameter()
{
    if (pOAState == IMS_NULL)
    {
        return IMS_NULL;
    }

    SdpSessionParameter* pSessionParam = IMS_NULL;

    pOAState->GetSessionProposalView(pSessionParam);

    if (pSessionParam == IMS_NULL)
    {
        // Checks and create a proposal view if it does not exist
        if (!pOAState->IsOfferProgress())
        {
            IMS_SINT32 nResult = pOAState->CreateProposalView();

            if ((nResult != ISDPOAState::RESULT_SUCCESS) &&
                    (nResult != ISDPOAState::RESULT_ALREADY_EXIST))
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
PROTECTED
IMS_BOOL VirtualSession::CheckNCreateSessionDescriptor()
{
    if (pSessionDescriptor != IMS_NULL)
    {
        IMS_TRACE_D("SessionDescriptor already exists", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nOAState = GetOfferAnswerState();

    // IDLE :: 183 response w/o SDP
    // OFFER_RECEIVED :: 183 response w/ SDP
    if ((nOAState != SDPOAState::STATE_IDLE) && (nOAState != SDPOAState::STATE_OFFER_RECEIVED))
    {
        IMS_TRACE_E(0, "__ SessionDescriptor can't be created in offer/answer state (%d) __",
                nOAState, 0, 0);
        return IMS_FALSE;
    }

    // Create a media capabilities for this service & session
    const SipAddress::UserInfoPart* pUserInfo = objUserAoR.GetUserInfoPart();
    const AString& strUserId = (pUserInfo != IMS_NULL) ? pUserInfo->GetUser()
            : objUserAoR.IsSchemeTel()                 ? objUserAoR.GetHost()
                                                       : objUserAoR.GetUser();

    if (!pOAState->CreateCapabilities(pService, strUserId))
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

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_SINT32 VirtualSession::GetOfferAnswerState() const
{
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
IMS_SINT32 VirtualSession::HandleSDPOfferAnswer(IN const ISipMessage* piSIPMsg)
{
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

        if (((nOAResult == SdpOfferAnswer::RESULT_SUCCESS) ||
                    (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)) &&
                ((nOAState == SDPOAState::STATE_OFFER_RECEIVED) ||
                        (nOAState == SDPOAState::STATE_OFFER_CHANGE_RECEIVED)))
        {
            SessionParameter* pSessionParam = pOAState->GetProposalView();

            if (pSessionParam != IMS_NULL)
            {
                const IMSList<SdpMediaParameter*>& objMediaParams =
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

    return nOAResult;
}

/*

Remarks

*/
PROTECTED
void VirtualSession::RestoreEx()
{
    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        Media* pMedia = objMedias.GetAt(i);

        if (pMedia->GetState() == Media::STATE_INACTIVE)
        {
            // Delete the media from the list ?????
        }

        // 4 Add the code to restore the medias
        pMedia->RestoreMedia();
    }
}

/*

Remarks

*/
PROTECTED
IMS_BOOL VirtualSession::UpdateMedia(IN IMS_SINT32 nTrigger)
{
    if (!pOAState->IsSessionChanged())
    {
        IMS_TRACE_D("UpdateMedia :: No session changed", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bResult = IMS_FALSE;

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
    if ((GetOfferAnswerState() == SDPOAState::STATE_IDLE) ||
            (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
    {
        pOAState->CompleteExchange();
    }

    return bResult;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL VirtualSession::RestoreOfferAnswerState()
{
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
IMS_BOOL VirtualSession::UpdateOfferAnswerStateOnMessageReceived(IN const ISipMessage* piSIPMsg)
{
    if (pOAState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nState = GetState();
    IMS_BOOL bIsCallEstablished = IMS_FALSE;

    // RFC 6337: Section 3.1.1
    // After the UAC has received the answer in a reliable provisional response to the INVITE,
    //[RFC3261] requires that any SDP in subsequent responses be ignored.
    {
        if ((piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE) &&
                piSIPMsg->GetMethod().Equals(SipMethod::INVITE))
        {
            if ((nState == STATE_NEGOTIATING) &&
                    (GetOfferAnswerState() == SDPOAState::STATE_ESTABLISHED))
            {
                IMS_TRACE_D("UpdateOfferAnswerStateOnMessageReceived() :: "
                            "Ignore the SDP in subsequent RPR as offer-answer is completed",
                        0, 0, 0);

                pOAState->UpdateStateOnTransactionCompleted(piSIPMsg, SDPOAState::MESSAGE_RECEIVED);
                return IMS_TRUE;
            }
        }
    }

    if ((nState == STATE_ESTABLISHED) || (nState == STATE_RENEGOTIATING) ||
            (nState == STATE_REESTABLISHING))
    {
        bIsCallEstablished = IMS_TRUE;
    }

    return pOAState->UpdateState(
            piSIPMsg, SDPOAState::MESSAGE_RECEIVED, bIsCallEstablished, IMS_FALSE);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL VirtualSession::UpdateOfferAnswerStateOnMessageSent(IN const ISipMessage* piSIPMsg)
{
    if (pOAState == IMS_NULL)
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

    return pOAState->UpdateState(piSIPMsg, SDPOAState::MESSAGE_SENT, bIsCallEstablished, IMS_FALSE);
}

/*

Remarks

*/
PRIVATE
void VirtualSession::Init(IN const SipAddress* pUserAoR)
{
    if (pUserAoR != IMS_NULL)
    {
        objUserAoR = *pUserAoR;
    }
    else
    {
        AString strTempAoR("sip:IMS-UE");
        const AString& strHDN = ImsIdentity::GetHomeDomainName(GetSlotId());

        strTempAoR += "@";
        strTempAoR += strHDN;

        objUserAoR.Create(strTempAoR);
    }

    // Instantiate SDP offer/answer object
    IMS_BOOL bSDPVersionCheck = IMS_TRUE;
    const SipConfigV* pSipConfigV = pService->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        bSDPVersionCheck = pSipConfigV->IsSessionSDPVersionCheckSupported();
    }

    pOAState = new SDPOAState(bSDPVersionCheck, IMS_TRUE);

    SetState(STATE_NEGOTIATING);
}

/*

Remarks

*/
PRIVATE
void VirtualSession::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Session :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL VirtualSession::AddMedia(IN Media* pMedia)
{
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
void VirtualSession::CleanupMedia()
{
    for (IMS_UINT32 i = 0; i < objMedias.GetSize(); ++i)
    {
        Media* pMedia = objMedias.GetAt(i);

        if (pMedia != IMS_NULL)
            pMedia->CleanupMedia();
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL VirtualSession::UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger)
{
    Media* pMedia;

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
IMS_BOOL VirtualSession::UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger)
{
    Media* pMedia;

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
IMS_BOOL VirtualSession::UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger)
{
    SessionParameter* pSessionParam = pOAState->GetProposalView();
    IMS_SINT32 nMediaCount = pSessionParam->GetMediaCount();

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
                SdpMediaParameter* pMediaParam = objGroupMediaParams.GetAt(0);

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
                        IMS_TRACE_I(
                                "Unsupported media type(%s, %s) received; So, it will be ignored",
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
                        const SdpMediaParameter* pMediaParam = objGroupMediaParams.GetAt(j);

                        IMS_TRACE_I("New media type(%s, %s) added",
                                pMediaParam->GetMedia().GetTypeEx().GetStr(),
                                pMediaParam->GetMedia().GetTransportProtocolEx().GetStr(), 0);

                        objMids.Append(pMediaParam->GetMid());
                    }

                    // New media; Create and add to the current session.
                    Media* pMedia = MediaFactory::CreateIncomingMedia(
                            pMediaParam->GetMedia().GetTransportProtocol(), pService, pOAState,
                            objMids);

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
                    Media* pMedia = objMedias.GetAt(nMediaIndex);

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
IMS_BOOL VirtualSession::UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger)
{
    Media* pMedia;

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
