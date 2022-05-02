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
#include "base/IMS.h"
#include "media/MediaDescriptor.h"
#include "media/StreamMediaProposalImpl.h"
#include "media/StreamMediaImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
StreamMediaImpl::StreamMediaImpl(IN StreamMedia *pStreamMedia_)
    : pStreamMediaProposal(IMS_NULL)
    , pStreamMedia(pStreamMedia_)
{
    pStreamMedia->SetMediaListener(this);
}

PUBLIC VIRTUAL
StreamMediaImpl::~StreamMediaImpl()
{
    if (pStreamMediaProposal != IMS_NULL)
    {
        delete pStreamMediaProposal;
        pStreamMediaProposal = IMS_NULL;
    }
}

PRIVATE VIRTUAL
IMS_BOOL StreamMediaImpl::Equals(IN CONST IMedia *piMedia) const
{
    const StreamMediaImpl *pMediaImpl = DYNAMIC_CAST(const StreamMediaImpl*, piMedia);

    //---------------------------------------------------------------------------------------------

    if (pMediaImpl == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (this == pMediaImpl);
}

PRIVATE VIRTUAL
IMedia* StreamMediaImpl::GetInterface()
{
    //---------------------------------------------------------------------------------------------

    return this;
}

PRIVATE VIRTUAL
Media* StreamMediaImpl::GetMedia() const
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia;
}

// IMedia interface
PRIVATE VIRTUAL
IMS_SINT32 StreamMediaImpl::GetDirection() const
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia->GetDirection();
}

PRIVATE VIRTUAL
IMSList<IMediaDescriptor*> StreamMediaImpl::GetMediaDescriptors() const
{
    const IMSList<MediaDescriptor*> &objMediaDescriptors = pStreamMedia->GetMediaDescriptors();

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
IMedia* StreamMediaImpl::GetProposal(IN IMS_BOOL bIMSExtension /* = IMS_TRUE */) const
{
    //---------------------------------------------------------------------------------------------

    if ((GetState() != STATE_ACTIVE) || (GetUpdateState() != UPDATE_MODIFIED))
    {
        if (bIMSExtension && (GetState() == STATE_ACTIVE) && (GetUpdateState() == UPDATE_REMOVED))
        {
        }
        else
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);
            return IMS_NULL;
        }
    }

    return pStreamMediaProposal;
}

PRIVATE VIRTUAL
IMS_SINT32 StreamMediaImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia->GetState();
}

PRIVATE VIRTUAL
IMS_SINT32 StreamMediaImpl::GetUpdateState() const
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia->GetUpdateState();
}

PRIVATE VIRTUAL
IMS_RESULT StreamMediaImpl::SetDirection(IN IMS_SINT32 nDirection)
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia->SetDirection(nDirection);
}

PRIVATE VIRTUAL
IMediaDescriptor* StreamMediaImpl::GetMediaDescriptor() const
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia->GetMediaDescriptor();
}

PRIVATE VIRTUAL
IMS_SINT32 StreamMediaImpl::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return pStreamMedia->GetType();
}

PRIVATE VIRTUAL
void StreamMediaImpl::RemoveMediaDescriptor(IN IMS_UINT32 nPosition)
{
    //---------------------------------------------------------------------------------------------

    pStreamMedia->RemoveMediaDescriptor(nPosition);
}

PRIVATE VIRTUAL
void StreamMediaImpl::OnMedia_FictitiousMediaCreated(IN Media *pMedia)
{
    //---------------------------------------------------------------------------------------------

    if (pStreamMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    StreamMediaProposal *pMediaProposal
            = DYNAMIC_CAST(StreamMediaProposal*, pStreamMedia->GetProposal());

    if (pMediaProposal == IMS_NULL)
    {
        // Do nothing
        IMS_TRACE_E(0, "NO MEDIA PROPOSAL", 0, 0, 0);
        return;
    }

    if (pStreamMediaProposal != IMS_NULL)
    {
        delete pStreamMediaProposal;
    }

    pStreamMediaProposal = new StreamMediaProposalImpl(pMediaProposal);

    if (pStreamMediaProposal == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO MEMORY", 0, 0, 0);
        return;
    }
}

PRIVATE VIRTUAL
void StreamMediaImpl::OnMedia_FictitiousMediaDestroyed(IN Media *pMedia)
{
    //---------------------------------------------------------------------------------------------

    if (pStreamMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    if (pStreamMediaProposal != IMS_NULL)
    {
        delete pStreamMediaProposal;
        pStreamMediaProposal = IMS_NULL;
    }
}
