/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100503  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "base/IMS.h"
#include "media/MediaDescriptor.h"
#include "media/FramedMediaProposalImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
FramedMediaProposalImpl::FramedMediaProposalImpl(IN FramedMediaProposal *pMediaProposal_)
    : pMediaProposal(pMediaProposal_)
{
}

PUBLIC VIRTUAL
FramedMediaProposalImpl::~FramedMediaProposalImpl()
{
}

PRIVATE VIRTUAL
IMS_SINT32 FramedMediaProposalImpl::GetDirection() const
{
    //---------------------------------------------------------------------------------------------

    return pMediaProposal->GetDirection();
}

PRIVATE VIRTUAL
IMSList<IMediaDescriptor*> FramedMediaProposalImpl::GetMediaDescriptors() const
{
    const IMSList<MediaDescriptor*> &objMediaDescriptors = pMediaProposal->GetMediaDescriptors();

    //---------------------------------------------------------------------------------------------
    /*
    if (IMSError::GetLastError() != IMSError::NO_ERROR)
    {
        IMS_TRACE_E(0, "Getting MediaDescriptors failed - %d", IMSError::GetLastError(), 0, 0);
        return IMSList<IMediaDescriptor*>();
    }
    */

    if (objMediaDescriptors.IsEmpty())
    {
        IMS_TRACE_E(0, "No media descriptors in the current media", 0, 0, 0);
        return IMSList<IMediaDescriptor*>();
    }

    IMSList<IMediaDescriptor*> objIMediaDescriptors;

    for (IMS_UINT32 i = 0; i < objMediaDescriptors.GetSize(); ++i)
    {
        objIMediaDescriptors.Append(objMediaDescriptors.GetAt(i));
    }

    return objIMediaDescriptors;
}

PRIVATE VIRTUAL
IMedia* FramedMediaProposalImpl::GetProposal(IN IMS_BOOL /* bIMSExtension = IMS_TRUE */) const
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_E(0, "Operation not allowed in media proposal", 0, 0, 0);

    IMS::SetLastError(IMSError::ILLEGAL_STATE);

    return IMS_NULL;
}

PRIVATE VIRTUAL
IMS_SINT32 FramedMediaProposalImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return STATE_PROPOSAL;
}

PRIVATE VIRTUAL
IMS_SINT32 FramedMediaProposalImpl::GetUpdateState() const
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_E(0, "Operation not allowed in media proposal", 0, 0, 0);

    IMS::SetLastError(IMSError::ILLEGAL_STATE);

    return UPDATE_INVALID;
}

PRIVATE VIRTUAL
IMS_RESULT FramedMediaProposalImpl::SetDirection(IN IMS_SINT32 nDirection)
{
    //---------------------------------------------------------------------------------------------

    (void) nDirection;

    IMS_TRACE_E(0, "Operation not allowed in media proposal", 0, 0, 0);

    IMS::SetLastError(IMSError::ILLEGAL_STATE);

    return IMS_FAILURE;
}

PRIVATE VIRTUAL
IMediaDescriptor* FramedMediaProposalImpl::GetMediaDescriptor() const
{
    //---------------------------------------------------------------------------------------------

    return pMediaProposal->GetMediaDescriptor();
}

PRIVATE VIRTUAL
IMS_SINT32 FramedMediaProposalImpl::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return pMediaProposal->GetType();
}

PRIVATE VIRTUAL
void FramedMediaProposalImpl::RemoveMediaDescriptor(IN IMS_UINT32 nPosition)
{
    //---------------------------------------------------------------------------------------------

    (void) nPosition;
}
