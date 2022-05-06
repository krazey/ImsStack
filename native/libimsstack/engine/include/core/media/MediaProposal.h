/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _MEDIA_PROPOSAL_H_
#define _MEDIA_PROPOSAL_H_

#include "media/IMediaState.h"

class ISDPOAState;
class MediaDescriptor;

class MediaProposal : public IMediaState
{
public:
    MediaProposal(IN ISDPOAState* piOAState_);
    virtual ~MediaProposal();

private:
    MediaProposal(IN CONST MediaProposal& objRHS);
    MediaProposal& operator=(IN CONST MediaProposal& objRHS);

public:
    // IMedia interface
    virtual const IMSList<MediaDescriptor*>& GetMediaDescriptors() const;
    virtual IMS_SINT32 GetType() const = 0;

    IMS_BOOL CreateDescriptor(IN CONST IMSList<MediaDescriptor*>& objDescriptors);
    IMS_SINT32 GetDirection() const;
    MediaDescriptor* GetMediaDescriptor() const;
    MediaDescriptor* GetMediaDescriptor(IN IMS_SINT32 nMid) const;

protected:
    // IMediaState interface
    virtual const AString& GetConnectionAddress() const;
    virtual IMS_SINT32 GetMediaState() const;
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_SINT32 nMid) const;
    virtual const AString& GetPeerConnectionAddress() const;
    virtual SdpMediaParameter* GetPeerMediaParameter(IN IMS_SINT32 nMid) const;
    virtual SdpMediaParameter* GetProposalMediaParameter(IN IMS_SINT32 nMid);

private:
    ISDPOAState* piOAState;
    IMSList<MediaDescriptor*> objDescriptors;
};

#endif  // _MEDIA_PROPOSAL_H_
