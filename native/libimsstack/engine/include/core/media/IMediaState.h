/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100512  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_MEDIA_STATE_H_
#define _INTERFACE_MEDIA_STATE_H_

#include "AString.h"

class SdpMediaParameter;

class IMediaState
{
public:
    virtual const AString& GetConnectionAddress() const = 0;
    virtual IMS_SINT32 GetMediaState() const = 0;
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_SINT32 nMid) const = 0;
    virtual const AString& GetPeerConnectionAddress() const = 0;
    virtual SdpMediaParameter* GetPeerMediaParameter(IN IMS_SINT32 nMid) const = 0;
    virtual SdpMediaParameter* GetProposalMediaParameter(IN IMS_SINT32 nMid) = 0;

public:
    // Types of main media state
    enum
    {
        MEDIA_STATE_INACTIVE = 1,
        MEDIA_STATE_INACTIVE_PROPOSAL,
        MEDIA_STATE_PENDING,
        MEDIA_STATE_ACTIVE,
        MEDIA_STATE_DELETED,
        MEDIA_STATE_PROPOSAL
    };
};

#endif  // _INTERFACE_MEDIA_STATE_H_
