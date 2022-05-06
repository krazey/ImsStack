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
#include "media/MediaDescriptor.h"
#include "media/FramedMediaProposalImpl.h"
#include "media/FramedMediaImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
FramedMediaImpl::FramedMediaImpl(IN FramedMedia* pFramedMedia_) :
        pFramedMediaProposal(IMS_NULL),
        pFramedMedia(pFramedMedia_)
{
    pFramedMedia->SetMediaListener(this);
}

PUBLIC VIRTUAL FramedMediaImpl::~FramedMediaImpl()
{
    if (pFramedMediaProposal != IMS_NULL)
    {
        delete pFramedMediaProposal;
        pFramedMediaProposal = IMS_NULL;
    }
}

PRIVATE VIRTUAL IMS_BOOL FramedMediaImpl::Equals(IN CONST IMedia* piMedia) const
{
    const FramedMediaImpl* pMediaImpl = DYNAMIC_CAST(const FramedMediaImpl*, piMedia);

    //---------------------------------------------------------------------------------------------

    if (pMediaImpl == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (this == pMediaImpl);
}

PRIVATE VIRTUAL IMedia* FramedMediaImpl::GetInterface()
{
    //---------------------------------------------------------------------------------------------

    return this;
}

PRIVATE VIRTUAL Media* FramedMediaImpl::GetMedia() const
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia;
}

PRIVATE VIRTUAL IMS_SINT32 FramedMediaImpl::GetDirection() const
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia->GetDirection();
}

PRIVATE VIRTUAL IMSList<IMediaDescriptor*> FramedMediaImpl::GetMediaDescriptors() const
{
    const IMSList<MediaDescriptor*>& objMediaDescriptors = pFramedMedia->GetMediaDescriptors();

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

PRIVATE VIRTUAL IMedia* FramedMediaImpl::GetProposal(
        IN IMS_BOOL bIMSExtension /* = IMS_TRUE */) const
{
    //---------------------------------------------------------------------------------------------

    if ((GetState() != STATE_ACTIVE) || (GetUpdateState() != UPDATE_MODIFIED))
    {
        if (bIMSExtension && (GetState() == STATE_ACTIVE) && (GetUpdateState() == UPDATE_REMOVED))
        {
        }
        else
        {
            return IMS_NULL;
        }
    }

    return pFramedMediaProposal;
}

PRIVATE VIRTUAL IMS_SINT32 FramedMediaImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia->GetState();
}

PRIVATE VIRTUAL IMS_SINT32 FramedMediaImpl::GetUpdateState() const
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia->GetUpdateState();
}

PRIVATE VIRTUAL IMS_RESULT FramedMediaImpl::SetDirection(IN IMS_SINT32 nDirection)
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia->SetDirection(nDirection);
}

PRIVATE VIRTUAL IMediaDescriptor* FramedMediaImpl::GetMediaDescriptor() const
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia->GetMediaDescriptor();
}

PRIVATE VIRTUAL IMS_SINT32 FramedMediaImpl::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return pFramedMedia->GetType();
}

PRIVATE VIRTUAL void FramedMediaImpl::RemoveMediaDescriptor(IN IMS_UINT32 nPosition)
{
    //---------------------------------------------------------------------------------------------

    pFramedMedia->RemoveMediaDescriptor(nPosition);
}

PRIVATE VIRTUAL void FramedMediaImpl::OnMedia_FictitiousMediaCreated(IN Media* pMedia)
{
    //---------------------------------------------------------------------------------------------

    if (pFramedMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    FramedMediaProposal* pMediaProposal =
            DYNAMIC_CAST(FramedMediaProposal*, pFramedMedia->GetProposal());

    if (pMediaProposal == IMS_NULL)
    {
        // Do nothing
        IMS_TRACE_E(0, "NO MEDIA PROPOSAL", 0, 0, 0);
        return;
    }

    if (pFramedMediaProposal != IMS_NULL)
    {
        delete pFramedMediaProposal;
        pFramedMediaProposal = IMS_NULL;
    }

    pFramedMediaProposal = new FramedMediaProposalImpl(pMediaProposal);

    if (pFramedMediaProposal == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO MEMORY", 0, 0, 0);
        return;
    }
}

PRIVATE VIRTUAL void FramedMediaImpl::OnMedia_FictitiousMediaDestroyed(IN Media* pMedia)
{
    //---------------------------------------------------------------------------------------------

    if (pFramedMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    if (pFramedMediaProposal != IMS_NULL)
    {
        delete pFramedMediaProposal;
        pFramedMediaProposal = IMS_NULL;
    }
}
