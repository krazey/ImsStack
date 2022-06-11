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
#include "ISdpOaState.h"
#include "media/Media.h"
#include "media/MediaDescriptor.h"
#include "media/MediaProposal.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
MediaProposal::MediaProposal(IN ISdpOaState* piOAState_) :
        piOAState(piOAState_),
        objDescriptors(IMSList<MediaDescriptor*>())
{
}

PUBLIC VIRTUAL MediaProposal::~MediaProposal()
{
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
}

PUBLIC VIRTUAL const IMSList<MediaDescriptor*>& MediaProposal::GetMediaDescriptors() const
{
    //---------------------------------------------------------------------------------------------

    return objDescriptors;
}

PUBLIC
IMS_BOOL MediaProposal::CreateDescriptor(IN CONST IMSList<MediaDescriptor*>& objDescriptors)
{
    IMS_SINT32 nResult = piOAState->CreateProposalView();

    //---------------------------------------------------------------------------------------------

    if ((nResult != ISdpOaState::RESULT_SUCCESS) && (nResult != ISdpOaState::RESULT_ALREADY_EXIST))
    {
        IMS_TRACE_E(0, "Creating a proposed view failed", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
    {
        const MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);

        MediaDescriptor* pNewDescriptor = new MediaDescriptor(this, pDescriptor->GetMid());

        if (pNewDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a MediaDescriptor (%d) failed", i, 0, 0);
            return IMS_FALSE;
        }

        if (!this->objDescriptors.Append(pNewDescriptor))
        {
            delete pNewDescriptor;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 MediaProposal::GetDirection() const
{
    //---------------------------------------------------------------------------------------------

    if (objDescriptors.IsEmpty())
    {
        return Media::DIRECTION_NONE;
    }

    MediaDescriptor* pDescriptor = GetMediaDescriptor();

    if (pDescriptor == IMS_NULL)
    {
        return Media::DIRECTION_NONE;
    }

    SdpMediaParameter* pMediaParam = GetPeerMediaParameter(pDescriptor->GetMid());

    if (pMediaParam == IMS_NULL)
    {
        return Media::DIRECTION_NONE;
    }

    IMS_SINT32 nDirection = pMediaParam->GetDirection();

    if (nDirection == Sdp::DIRECTION_INACTIVE)
        return Media::DIRECTION_INACTIVE;
    else if (nDirection == Sdp::DIRECTION_RECVONLY)
        return Media::DIRECTION_RECEIVE;
    else if (nDirection == Sdp::DIRECTION_SENDONLY)
        return Media::DIRECTION_SEND;
    else if (nDirection == Sdp::DIRECTION_SENDRECV)
        return Media::DIRECTION_SEND_RECEIVE;
    else
        return Media::DIRECTION_NONE;
}

PUBLIC
MediaDescriptor* MediaProposal::GetMediaDescriptor() const
{
    //---------------------------------------------------------------------------------------------

    if (objDescriptors.IsEmpty())
    {
        return IMS_NULL;
    }

    return objDescriptors.GetAt(0);
}

PUBLIC
MediaDescriptor* MediaProposal::GetMediaDescriptor(IN IMS_SINT32 nMid) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
    {
        MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);

        if (nMid == pDescriptor->GetMid())
        {
            return pDescriptor;
        }
    }

    return IMS_NULL;
}

PROTECTED VIRTUAL const AString& MediaProposal::GetConnectionAddress() const
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

PROTECTED VIRTUAL IMS_SINT32 MediaProposal::GetMediaState() const
{
    //---------------------------------------------------------------------------------------------

    return MEDIA_STATE_PROPOSAL;
}

PROTECTED VIRTUAL SdpMediaParameter* MediaProposal::GetMediaParameter(IN IMS_SINT32 nMid) const
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (piOAState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        return IMS_NULL;
    }

    return pMediaParam;
}

PROTECTED VIRTUAL const AString& MediaProposal::GetPeerConnectionAddress() const
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

PROTECTED VIRTUAL SdpMediaParameter* MediaProposal::GetPeerMediaParameter(IN IMS_SINT32 nMid) const
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (piOAState->GetMediaPeerView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        return IMS_NULL;
    }

    return pMediaParam;
}

PROTECTED VIRTUAL SdpMediaParameter* MediaProposal::GetProposalMediaParameter(IN IMS_SINT32 nMid)
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (piOAState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "There is no proposed view", 0, 0, 0);
        return IMS_NULL;
    }

    return pMediaParam;
}
