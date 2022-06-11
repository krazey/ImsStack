/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "base/Ims.h"
#include "Service.h"
#include "ISdpOaState.h"
#include "media/IOnMediaListener.h"
#include "media/MediaProposal.h"
#include "media/Media.h"

__IMS_TRACE_TAG_IMS_CORE__;

// 3  100503
// 3 To support the connection in the media object,
// 3 it needs to be modified using any active class to send/receive MSG.

PUBLIC
Media::Media(IN Service* pService_, IN ISdpOaState* piOAState_) :
        pService(pService_),
        piOAState(piOAState_),
        nState(STATE_INACTIVE),
        nUpdateState(UPDATE_UNCHANGED),
        nDirection(DIRECTION_NONE),
        objDescriptors(IMSList<MediaDescriptor*>()),
        piListener(IMS_NULL),
        bFlag_DirectionOnlyUpdated(IMS_FALSE),
        bFlag_InitializationDone(IMS_FALSE),
        bFlag_InitialOfferReceived(IMS_FALSE),
        pMediaProposal(IMS_NULL)
{
}

PUBLIC VIRTUAL Media::~Media()
{
    //---------------------------------------------------------------------------------------------

    if (!objDescriptors.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
        {
            MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);

            if (pDescriptor != IMS_NULL)
                delete pDescriptor;
        }

        objDescriptors.Clear();
    }

    if (pMediaProposal != IMS_NULL)
    {
        delete pMediaProposal;
        pMediaProposal = IMS_NULL;
    }

    IMS_TRACE_D("Destructor :: Media", 0, 0, 0);
}

PUBLIC
IMS_SINT32 Media::GetDirection() const
{
    //---------------------------------------------------------------------------------------------

    return nDirection;
}

PUBLIC
const IMSList<MediaDescriptor*>& Media::GetMediaDescriptors() const
{
    //---------------------------------------------------------------------------------------------

    return objDescriptors;
}

PUBLIC
MediaProposal* Media::GetProposal(IN IMS_BOOL bIMSExtension /* = IMS_TRUE */) const
{
    //---------------------------------------------------------------------------------------------

    if ((GetState() != STATE_ACTIVE) || (GetUpdateState() != UPDATE_MODIFIED))
    {
        if (bIMSExtension && (GetState() == STATE_ACTIVE) && (GetUpdateState() == UPDATE_REMOVED))
        {
        }
        else
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);
            return IMS_NULL;
        }
    }

    return pMediaProposal;
}

PUBLIC
IMS_SINT32 Media::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC
IMS_SINT32 Media::GetUpdateState() const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return UPDATE_INVALID;
    }

    return nUpdateState;
}

PUBLIC
IMS_RESULT Media::SetDirection(IN IMS_SINT32 nDirection)
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_INACTIVE) && (nState != STATE_ACTIVE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if ((nDirection < DIRECTION_INACTIVE) || (nDirection > DIRECTION_SEND_RECEIVE))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    this->nDirection = nDirection;

    IMS_SINT32 nSDPDirection = ConvertDirectionMediaToSDP(nDirection);

    for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
    {
        const MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);
        SdpMediaParameter* pMediaParam = GetProposalMediaParameter(pDescriptor->GetMid());

        if (pMediaParam != IMS_NULL)
        {
            pMediaParam->SetDirection(nSDPDirection);
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
void Media::SetMediaListener(IN IOnMediaListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PUBLIC
MediaDescriptor* Media::GetMediaDescriptor() const
{
    //---------------------------------------------------------------------------------------------

    if (objDescriptors.IsEmpty())
        return IMS_NULL;

    return objDescriptors.GetAt(0);
}

PUBLIC
void Media::RemoveMediaDescriptor(IN IMS_UINT32 nPosition)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_INACTIVE)
    {
        if (nPosition >= objDescriptors.GetSize())
        {
            return;
        }

        objDescriptors.RemoveAt(nPosition);
    }
    else
    {
        // Reject ????
    }
}

PUBLIC
void Media::SetMid(IN IMS_SINT32 nMid)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
    {
        MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);
        pDescriptor->SetMid(nMid);
    }
}

PUBLIC
IMS_BOOL Media::Equals(IN Media* pMedia) const
{
    //---------------------------------------------------------------------------------------------

    if (pMedia == IMS_NULL)
        return IMS_FALSE;

    return (this == pMedia);
}

PUBLIC
IMS_BOOL Media::IsDirectionOnlyUpdated() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_DirectionOnlyUpdated;
}

PUBLIC
IMS_BOOL Media::IsInitializationDone() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_InitializationDone;
}

// When an error occurred in Session handling, Session will invoke this method.
// It will cleanup all the resources which assign to this media.
PUBLIC
void Media::CleanupMedia()
{
    //---------------------------------------------------------------------------------------------

    PreviewCleanupMedia();

    SetState(STATE_DELETED);

    PostCleanupMedia();
}

// Inside of RemoveMedia() in Session, Session will invoke this method.
PUBLIC
void Media::RemoveMedia()
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    PreviewRemoveMedia();

    if (nState == STATE_ACTIVE)
    {
        if (pMediaProposal == IMS_NULL)
        {
            pMediaProposal = CreateMediaProposal(piOAState);

            if (pMediaProposal != IMS_NULL)
            {
                if (piListener != IMS_NULL)
                {
                    piListener->OnMedia_FictitiousMediaCreated(this);
                }
            }
        }

        if (pMediaProposal != IMS_NULL)
        {
            const IMSList<MediaDescriptor*>& objDescriptors = pMediaProposal->GetMediaDescriptors();

            for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
            {
                MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);

                if (pDescriptor == IMS_NULL)
                    continue;

                piOAState->MarkRejectedOrRemoved(pDescriptor->GetMid());
            }
        }

        SetUpdateState(UPDATE_REMOVED);
    }
    else if ((nState == STATE_INACTIVE) || (nState == STATE_PENDING))
    {
        if (bFlag_InitialOfferReceived)
        {
            for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
            {
                MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);

                if (pDescriptor == IMS_NULL)
                    continue;

                piOAState->MarkRejectedOrRemoved(pDescriptor->GetMid());
            }
        }

        SetState(STATE_DELETED);
    }

    // Remove the media parameters for all media descriptors
    if (!bFlag_InitialOfferReceived && (GetState() == STATE_DELETED))
    {
        for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
        {
            const MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);

            if (pDescriptor == IMS_NULL)
                continue;

            piOAState->RemoveMediaParameter(pDescriptor->GetMid());
        }
    }

    PostRemoveMedia();

    piListener = IMS_NULL;
}

// After SessionUpdate has completed and if the session is not negotiated,
// then it will restore the media state to the previous state.
PUBLIC
void Media::RestoreMedia()
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if (nState == STATE_INACTIVE)
    {
        bFlag_InitialOfferReceived = IMS_FALSE;
        SetState(STATE_DELETED);
    }
    else if (nState == STATE_PENDING)
    {
        SetState(STATE_DELETED);
    }
    else if (nState == STATE_ACTIVE)
    {
        SetUpdateState(UPDATE_UNCHANGED);

        if (pMediaProposal != IMS_NULL)
        {
            delete pMediaProposal;
            pMediaProposal = IMS_NULL;
        }

        if (piListener != IMS_NULL)
        {
            piListener->OnMedia_FictitiousMediaDestroyed(this);
        }
    }
}

PUBLIC
void Media::TransitMedia(IN IMS_SINT32 nSessionTransition, IN IMS_SINT32 nOAStatus)
{
    //---------------------------------------------------------------------------------------------

    // Update the MediaDescriptors
    if ((nOAStatus == OFFER_SENT) || (nOAStatus == OFFER_RECEIVED) || (nOAStatus == ANSWER_SENT) ||
            (nOAStatus == ANSWER_RECEIVED))
    {
        // Store the session-level connection address
        // This field can be overwritten by the media descriptor
        // if the specific media has its own connection line.
        if ((nOAStatus == OFFER_RECEIVED) || (nOAStatus == ANSWER_RECEIVED))
        {
        }

        // UpdateMediaDescriptors();
    }

    IMS_SINT32 nState = GetState();

    if (nState == STATE_INACTIVE)
    {
        if (nOAStatus == OFFER_RECEIVED)
        {
            // When the initial offer received, do not anything ...
            // To access the MediaDescriptor
            bFlag_InitialOfferReceived = IMS_TRUE;
            return;
        }
        else if (nOAStatus == ANSWER_SENT)
        {
            bFlag_InitialOfferReceived = IMS_FALSE;

            if ((nSessionTransition == SESSION_START_FAILED) ||
                    (nSessionTransition == SESSION_UPDATE_FAILED))
            {
                SetState(STATE_DELETED);
            }
            else if ((nSessionTransition == SESSION_STARTED) ||
                    (nSessionTransition == SESSION_UPDATED) ||
                    (nSessionTransition == SESSION_EARLY_UPDATE))
            {
                if (IsMediaAccepted())
                {
                    SetState(STATE_ACTIVE);
                }
                else
                {
                    SetState(STATE_DELETED);
                }
            }
            else
            {
                SetState(STATE_ACTIVE);
            }
        }
        else if (nOAStatus == OFFER_SENT)
        {
            if (nSessionTransition == SESSION_EARLY_UPDATE)
            {
                SetState(STATE_PENDING);
            }
        }

        if ((nSessionTransition == SESSION_START) || (nSessionTransition == SESSION_UPDATE))
        {
            SetState(STATE_PENDING);
        }
        else if (nSessionTransition == SESSION_TERMINATED)
        {
            SetState(STATE_DELETED);
        }
    }
    else if (nState == STATE_PENDING)
    {
        IMS_BOOL bIsMediaAccepted = IsMediaAccepted();

        if (nOAStatus == ANSWER_RECEIVED)
        {
            SetState(STATE_ACTIVE);
        }

        if ((nSessionTransition == SESSION_STARTED) || (nSessionTransition == SESSION_UPDATED) ||
                (nSessionTransition == SESSION_EARLY_UPDATE))
        {
            if (bIsMediaAccepted)
            {
                SetState(STATE_ACTIVE);
            }
            else
            {
                SetState(STATE_DELETED);
            }
        }
        else if ((nSessionTransition == SESSION_START_FAILED) ||
                (nSessionTransition == SESSION_UPDATE_FAILED))
        {
            SetState(STATE_DELETED);
        }
    }
    else if (nState == STATE_ACTIVE)
    {
        if (nSessionTransition == SESSION_TERMINATED)
        {
            SetState(STATE_DELETED);
        }

        // Set the update state
        if ((nOAStatus == ANSWER_SENT) || (nOAStatus == ANSWER_RECEIVED))
        {
            IMS_SINT32 nUpdateState = GetUpdateState();

            // If the session received non-2xx response to re-INVITE or UPDATE request,
            // then aborts the proposal media after completing the transaction.
            if ((nUpdateState == UPDATE_MODIFIED) && (nSessionTransition == SESSION_UPDATE_FAILED))
            {
                piOAState->AbortProposal();
            }

            // 4 Check if the proposal media is accepted or not...
            // 4 for state change (ACTIVE to DELETED)

            SetUpdateState(UPDATE_UNCHANGED);

            if (nUpdateState == UPDATE_REMOVED)
            {
                SetState(STATE_DELETED);
            }

            // Remove the proposal media
            if (pMediaProposal != IMS_NULL)
            {
                delete pMediaProposal;
                pMediaProposal = IMS_NULL;

                if (piListener != IMS_NULL)
                {
                    piListener->OnMedia_FictitiousMediaDestroyed(this);
                }
            }
        }
        else if (nOAStatus == OFFER_RECEIVED)
        {
            if (GetUpdateState() == UPDATE_UNCHANGED)
            {
                if (IsMediaProposed())
                    SetUpdateState(UPDATE_MODIFIED);
                else
                    SetUpdateState(UPDATE_REMOVED);

                if (pMediaProposal != IMS_NULL)
                {
                    delete pMediaProposal;
                    pMediaProposal = IMS_NULL;

                    if (piListener != IMS_NULL)
                    {
                        piListener->OnMedia_FictitiousMediaDestroyed(this);
                    }
                }

                pMediaProposal = CreateMediaProposal(piOAState);

                if (pMediaProposal != IMS_NULL)
                {
                    if (piListener != IMS_NULL)
                    {
                        piListener->OnMedia_FictitiousMediaCreated(this);
                    }
                }
            }
        }
        else if (nOAStatus == OFFER_SENT)
        {
            if ((nSessionTransition == SESSION_UPDATE) ||
                    (nSessionTransition == SESSION_EARLY_UPDATE))
            {
                if (!IsMediaAccepted() && (GetUpdateState() == UPDATE_MODIFIED))
                {
                    // Re-arrange the update state according to the media port information...
                    SetUpdateState(UPDATE_REMOVED);
                }
            }
        }
    }
    // Reuse the "slot" used by an old media stream which had been disabled by setting its port to
    // zero.
    else if (nState == STATE_DELETED)
    {
        if ((nSessionTransition == SESSION_UPDATE) || (nSessionTransition == SESSION_EARLY_UPDATE))
        {
            if ((nOAStatus == OFFER_RECEIVED) && IsMediaProposed())
            {
                SetState(STATE_INACTIVE);
                bFlag_InitialOfferReceived = IMS_TRUE;

                SetUpdateState(UPDATE_UNCHANGED);

                if (pMediaProposal != IMS_NULL)
                {
                    delete pMediaProposal;
                    pMediaProposal = IMS_NULL;

                    if (piListener != IMS_NULL)
                    {
                        piListener->OnMedia_FictitiousMediaDestroyed(this);
                    }
                }

                pMediaProposal = CreateMediaProposal(piOAState);

                if (pMediaProposal != IMS_NULL)
                {
                    if (piListener != IMS_NULL)
                    {
                        piListener->OnMedia_FictitiousMediaCreated(this);
                    }
                }
            }
        }
    }
}

PROTECTED VIRTUAL const AString& Media::GetConnectionAddress() const
{
    SdpSessionParameter* pSessionParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    // First, check the current view
    piOAState->GetSessionCurrentView(pSessionParam);

    if (pSessionParam != IMS_NULL)
    {
        return pSessionParam->GetConnectionAddress();
    }

    // If the current view does not exist, then check the proposed view
    piOAState->GetSessionProposalView(pSessionParam);

    if (pSessionParam != IMS_NULL)
    {
        return pSessionParam->GetConnectionAddress();
    }

    return AString::ConstNull();
}

PROTECTED VIRTUAL IMS_SINT32 Media::GetMediaState() const
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    if (nState == STATE_INACTIVE)
        return MEDIA_STATE_INACTIVE;
    else if (nState == STATE_PENDING)
        return MEDIA_STATE_PENDING;
    else if (nState == STATE_ACTIVE)
        return MEDIA_STATE_ACTIVE;
    else if (nState == STATE_DELETED)
        return MEDIA_STATE_DELETED;
    else if (nState == STATE_PROPOSAL)
        return MEDIA_STATE_PROPOSAL;
    else
    {
        if (bFlag_InitialOfferReceived)
        {
            return MEDIA_STATE_INACTIVE_PROPOSAL;
        }

        return MEDIA_STATE_INACTIVE;
    }
}

PROTECTED VIRTUAL SdpMediaParameter* Media::GetMediaParameter(IN IMS_SINT32 nMid) const
{
    IMS_SINT32 nState = GetState();
    SdpMediaParameter* pMediaParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if ((nState == STATE_ACTIVE) || (nState == STATE_DELETED))
    {
        IMS_SINT32 nUpdateState = GetUpdateState();

        if ((nUpdateState == UPDATE_MODIFIED) || (nUpdateState == UPDATE_REMOVED))
        {
            if (piOAState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
            {
                return IMS_NULL;
            }
        }
        else
        {
            // Read-only; Using this media parameter, the application MUST NOT modify any
            // parameters.
            if (piOAState->GetMediaCurrentView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
            {
                return IMS_NULL;
            }
        }
    }
    else
    {
        if (piOAState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
        {
            return IMS_NULL;
        }
    }

    return pMediaParam;
}

PROTECTED VIRTUAL const AString& Media::GetPeerConnectionAddress() const
{
    SdpSessionParameter* pSessionParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    piOAState->GetSessionPeerView(pSessionParam);

    if (pSessionParam != IMS_NULL)
    {
        return pSessionParam->GetConnectionAddress();
    }

    return AString::ConstNull();
}

PROTECTED VIRTUAL SdpMediaParameter* Media::GetPeerMediaParameter(IN IMS_SINT32 nMid) const
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (piOAState->GetMediaPeerView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        return IMS_NULL;
    }

    return pMediaParam;
}

PROTECTED VIRTUAL SdpMediaParameter* Media::GetProposalMediaParameter(IN IMS_SINT32 nMid)
{
    IMS_SINT32 nState = GetState();
    IMS_SINT32 nUpdateState = GetUpdateState();

    //---------------------------------------------------------------------------------------------

    if (nState == STATE_ACTIVE)
    {
        if (nUpdateState == UPDATE_UNCHANGED)
        {
            if (pMediaProposal != IMS_NULL)
            {
                delete pMediaProposal;
                pMediaProposal = IMS_NULL;

                if (piListener != IMS_NULL)
                {
                    piListener->OnMedia_FictitiousMediaDestroyed(this);
                }
            }

            pMediaProposal = CreateMediaProposal(piOAState);

            if (pMediaProposal != IMS_NULL)
            {
                SetUpdateState(UPDATE_MODIFIED);

                if (piListener != IMS_NULL)
                {
                    piListener->OnMedia_FictitiousMediaCreated(this);
                }
            }
            else
            {
                return IMS_NULL;
            }
        }

        nUpdateState = GetUpdateState();

        if ((nUpdateState == UPDATE_MODIFIED) || (nUpdateState == UPDATE_REMOVED))
        {
            SdpMediaParameter* pMediaParam = IMS_NULL;

            if (piOAState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
            {
                IMS_TRACE_E(0, "There is no proposed view", 0, 0, 0);
                return IMS_NULL;
            }

            return pMediaParam;
        }
    }
    else if (nState == STATE_INACTIVE)
    {
        SdpMediaParameter* pMediaParam = IMS_NULL;

        if (piOAState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
        {
            IMS_TRACE_E(0, "There is no proposed view", 0, 0, 0);
            return IMS_NULL;
        }

        return pMediaParam;
    }

    return IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL Media::PreviewInitInstance()
{
    // no-op
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Media::PostInitInstance()
{
    // no-op
    return IMS_TRUE;
}

PROTECTED VIRTUAL void Media::PreviewCleanupMedia()
{
    // no-op
}

PROTECTED VIRTUAL void Media::PostCleanupMedia()
{
    // no-op
}

PROTECTED VIRTUAL void Media::PreviewRemoveMedia()
{
    // no-op
}

PROTECTED VIRTUAL void Media::PostRemoveMedia()
{
    // no-op
}

PROTECTED
IMS_BOOL Media::InitInstance(IN IMS_SINT32 nCountOfDescriptor, IN IMS_SINT32 nDirection)
{
    IMS_SINT32 nResult = piOAState->CreateProposalView();

    //---------------------------------------------------------------------------------------------

    if ((nResult != ISdpOaState::RESULT_SUCCESS) && (nResult != ISdpOaState::RESULT_ALREADY_EXIST))
    {
        IMS_TRACE_E(0, "Creating a proposal view failed", 0, 0, 0);
        return IMS_FALSE;
    }

    this->nDirection = nDirection;

    // If nCountOfDescriptor is zero, the descriptor will be created from the media profile
    // with full capabilities.

    IMSList<IMS_SINT32> objMids;
    SdpMediaParameter* pMediaParam;
    IMS_SINT32 nSDPDirection = ConvertDirectionMediaToSDP(nDirection);

    if (nCountOfDescriptor == 0)
    {
    }
    else
    {
        for (IMS_SINT32 i = 0; i < nCountOfDescriptor; ++i)
        {
            nResult = piOAState->CreateMediaParameter(pMediaParam);

            if (nResult != ISdpOaState::RESULT_SUCCESS)
            {
                IMS_TRACE_E(0, "Creating a SDP media parameter (%d) failed", i, 0, 0);
                return IMS_FALSE;
            }

            pMediaParam->SetDirection(nSDPDirection);

            objMids.Append(pMediaParam->GetMid());
        }
    }

    return InitInstance(objMids);
}

PROTECTED
IMS_BOOL Media::InitInstance(IN CONST IMSList<IMS_SINT32>& objMids)
{
    //---------------------------------------------------------------------------------------------

    if (!PreviewInitInstance())
    {
        IMS_TRACE_E(0, "Creating a media (preview) failed", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objMids.GetSize(); ++i)
    {
        MediaDescriptor* pDescriptor = new MediaDescriptor(this, objMids.GetAt(i));

        if (pDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a MediaDescriptor (%d) failed", i, 0, 0);
            return IMS_FALSE;
        }

        SdpMediaParameter* pMediaParam = GetProposalMediaParameter(pDescriptor->GetMid());

        if (pMediaParam != IMS_NULL)
        {
            nDirection = ConvertDirectionSDPToMedia(pMediaParam->GetDirection());
        }

        if (!objDescriptors.Append(pDescriptor))
        {
            delete pDescriptor;
            return IMS_FALSE;
        }
    }

    if (!PostInitInstance())
    {
        IMS_TRACE_E(0, "Creating a media (post) failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL Media::IsMediaAccepted() const
{
    MediaDescriptor* pDescriptor = GetMediaDescriptor();

    //---------------------------------------------------------------------------------------------

    if (pDescriptor != IMS_NULL)
    {
        SdpMediaParameter* pMediaParameter = GetMediaParameter(pDescriptor->GetMid());

        if (pMediaParameter != IMS_NULL)
        {
            return pMediaParameter->IsMediaAccepted();
        }
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL Media::IsMediaProposed() const
{
    MediaDescriptor* pDescriptor = GetMediaDescriptor();

    //---------------------------------------------------------------------------------------------

    if (pDescriptor != IMS_NULL)
    {
        SdpMediaParameter* pMediaParameter = GetPeerMediaParameter(pDescriptor->GetMid());

        if (pMediaParameter != IMS_NULL)
        {
            return pMediaParameter->IsMediaAccepted();
        }
    }

    return IMS_TRUE;
}

PROTECTED
Service* Media::GetService() const
{
    //---------------------------------------------------------------------------------------------

    IMS_ASSERT(pService != IMS_NULL);

    return pService;
}

PROTECTED
void Media::SetInitializationDone(IN IMS_BOOL bInitializationDone)
{
    //---------------------------------------------------------------------------------------------

    this->bFlag_InitializationDone = bInitializationDone;
}

PROTECTED
void Media::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Media :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

PROTECTED
void Media::SetUpdateState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I(
            "Media :: %s to %s", UpdateStateToString(nUpdateState), UpdateStateToString(nState), 0);

    this->nUpdateState = nState;
}

PRIVATE
void Media::UpdateMediaDescriptors()
{
    /*
    IMS_SINT32 nResult;
    SdpMediaParameter *pMediaParam;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
    {
        MediaDescriptor *pDescriptor = objDescriptors.GetAt(i);

        nResult = piOAState->GetProposedView(pDescriptor->GetMid(), pMediaParam);

        if (nResult != ISdpOaState::RESULT_SUCCESS)
        {
            IMS_TRACE_E(0, "Getting a proposed media descriptor (%d) failed", i, 0, 0);
            return;
        }

        pDescriptor->SetMediaParameter(pMediaParam);
    }
    */
}

PRIVATE GLOBAL IMS_SINT32 Media::ConvertDirectionSDPToMedia(IN IMS_SINT32 nDirection)
{
    //---------------------------------------------------------------------------------------------

    if (nDirection == Sdp::DIRECTION_INACTIVE)
        return DIRECTION_INACTIVE;
    else if (nDirection == Sdp::DIRECTION_RECVONLY)
        return DIRECTION_RECEIVE;
    else if (nDirection == Sdp::DIRECTION_SENDONLY)
        return DIRECTION_SEND;
    else if (nDirection == Sdp::DIRECTION_SENDRECV)
        return DIRECTION_SEND_RECEIVE;
    else
        return DIRECTION_NONE;
}

PRIVATE GLOBAL IMS_SINT32 Media::ConvertDirectionMediaToSDP(IN IMS_SINT32 nDirection)
{
    //---------------------------------------------------------------------------------------------

    if (nDirection == DIRECTION_INACTIVE)
        return Sdp::DIRECTION_INACTIVE;
    else if (nDirection == DIRECTION_RECEIVE)
        return Sdp::DIRECTION_RECVONLY;
    else if (nDirection == DIRECTION_SEND)
        return Sdp::DIRECTION_SENDONLY;
    else if (nDirection == DIRECTION_SEND_RECEIVE)
        return Sdp::DIRECTION_SENDRECV;
    else if (nDirection == DIRECTION_NONE)
        return Sdp::DIRECTION_NONE;
    else
        return Sdp::DIRECTION_NONE;
}

PRIVATE GLOBAL const IMS_CHAR* Media::StateToString(IN IMS_SINT32 nState)
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
        case STATE_DELETED:
            return "STATE_DELETED";
        case STATE_PROPOSAL:
            return "STATE_PROPOSAL";
        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* Media::UpdateStateToString(IN IMS_SINT32 nUpdateState)
{
    //---------------------------------------------------------------------------------------------

    switch (nUpdateState)
    {
        case UPDATE_INVALID:
            return "UPDATE_INVALID";
        case UPDATE_UNCHANGED:
            return "UPDATE_UNCHANGED";
        case UPDATE_MODIFIED:
            return "UPDATE_MODIFIED";
        case UPDATE_REMOVED:
            return "UPDATE_REMOVED";
        default:
            return "__INVALID__";
    }
}
