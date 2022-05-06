/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _STREAM_MEDIA_IMPL_H_
#define _STREAM_MEDIA_IMPL_H_

#include "media/MediaImpl.h"
#include "media/IOnMediaListener.h"
#include "media/StreamMedia.h"

class StreamMediaProposalImpl;

class StreamMediaImpl : public MediaImpl, public IMedia, public IOnMediaListener
{
public:
    StreamMediaImpl(IN StreamMedia* pStreamMedia_);
    virtual ~StreamMediaImpl();

private:
    StreamMediaImpl(IN CONST StreamMediaImpl& objRHS);
    StreamMediaImpl& operator=(IN CONST StreamMediaImpl& objRHS);

private:
    // MediaImpl class
    virtual IMS_BOOL Equals(IN CONST IMedia* piMedia) const;
    virtual IMedia* GetInterface();
    virtual Media* GetMedia() const;

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

    virtual void OnMedia_FictitiousMediaCreated(IN Media* pMedia);
    virtual void OnMedia_FictitiousMediaDestroyed(IN Media* pMedia);

private:
    StreamMediaProposalImpl* pStreamMediaProposal;

    StreamMedia* pStreamMedia;
};

#endif  // _STREAM_MEDIA_IMPL_H_
