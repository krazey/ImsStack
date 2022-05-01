/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100506  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FRAMED_MEDIA_PROPOSAL_IMPL_H_
#define _FRAMED_MEDIA_PROPOSAL_IMPL_H_

#include "media/IMedia.h"
#include "media/FramedMediaProposal.h"



class FramedMediaProposalImpl
    : public IMedia
{
public:
    FramedMediaProposalImpl(IN FramedMediaProposal *pMediaProposal_);
    virtual ~FramedMediaProposalImpl();

private:
    // IMedia interface
    virtual IMS_SINT32 GetDirection() const;
    virtual IMSList<IMediaDescriptor*> GetMediaDescriptors() const;
    virtual IMedia* GetProposal(IN IMS_BOOL bIMSExtension = IMS_TRUE) const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_SINT32 GetUpdateState() const;
    virtual IMS_RESULT SetDirection(IN IMS_SINT32 nDirection);
    //// IMS extensions
    virtual IMediaDescriptor* GetMediaDescriptor() const;
    virtual IMS_SINT32 GetType() const;
    virtual void RemoveMediaDescriptor(IN IMS_UINT32 nPosition);

private:
    FramedMediaProposal *pMediaProposal;
};

#endif // _FRAMED_MEDIA_PROPOSAL_IMPL_H_
