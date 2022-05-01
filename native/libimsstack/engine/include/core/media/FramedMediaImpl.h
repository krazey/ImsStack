/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100503  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FRAMED_MEDIA_IMPL_H_
#define _FRAMED_MEDIA_IMPL_H_

#include "media/MediaImpl.h"
#include "media/IOnMediaListener.h"
#include "media/FramedMedia.h"

class FramedMediaProposalImpl;



class FramedMediaImpl
    : public MediaImpl
    , public IMedia
    , public IOnMediaListener
{
public:
    FramedMediaImpl(IN FramedMedia *pFramedMedia_);
    virtual ~FramedMediaImpl();

private:
    FramedMediaImpl(IN CONST FramedMediaImpl &objRHS);
    FramedMediaImpl& operator=(IN CONST FramedMediaImpl &objRHS);

private:
    // MediaImpl class
    virtual IMS_BOOL Equals(IN CONST IMedia *piMedia) const;
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

    virtual void OnMedia_FictitiousMediaCreated(IN Media *pMedia);
    virtual void OnMedia_FictitiousMediaDestroyed(IN Media *pMedia);

private:
    FramedMediaProposalImpl *pFramedMediaProposal;

    FramedMedia *pFramedMedia;
};

#endif // _FRAMED_MEDIA_IMPL_H_
