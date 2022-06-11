/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100507  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FRAMED_MEDIA_PROPOSAL_H_
#define _FRAMED_MEDIA_PROPOSAL_H_

#include "media/MediaProposal.h"

class FramedMediaProposal : public MediaProposal
{
public:
    FramedMediaProposal(IN ISdpOaState* piOAState_);
    virtual ~FramedMediaProposal();

private:
    FramedMediaProposal(IN CONST FramedMediaProposal& objRHS);
    FramedMediaProposal& operator=(IN CONST FramedMediaProposal& objRHS);

public:
    // MediaProposal class
    virtual IMS_SINT32 GetType() const;
};

#endif  // _FRAMED_MEDIA_PROPOSAL_H_
