/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _STREAM_MEDIA_PROPOSAL_IMPL_H_
#define _STREAM_MEDIA_PROPOSAL_IMPL_H_

#include "media/IMedia.h"
#include "media/StreamMediaProposal.h"

class StreamMediaProposalImpl : public IMedia
{
public:
    StreamMediaProposalImpl(IN StreamMediaProposal* pMediaProposal_);
    virtual ~StreamMediaProposalImpl();

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
    StreamMediaProposal* pMediaProposal;
};

#endif  // _STREAM_MEDIA_PROPOSAL_IMPL_H_
