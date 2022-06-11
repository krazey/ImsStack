/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _STREAM_MEDIA_PROPOSAL_H_
#define _STREAM_MEDIA_PROPOSAL_H_

#include "media/MediaProposal.h"

class StreamMediaProposal : public MediaProposal
{
public:
    StreamMediaProposal(IN ISdpOaState* piOAState_);
    virtual ~StreamMediaProposal();

private:
    StreamMediaProposal(IN CONST StreamMediaProposal& objRHS);
    StreamMediaProposal& operator=(IN CONST StreamMediaProposal& objRHS);

public:
    // MediaProposal class
    virtual IMS_SINT32 GetType() const;
};

#endif  // _STREAM_MEDIA_PROPOSAL_H_
