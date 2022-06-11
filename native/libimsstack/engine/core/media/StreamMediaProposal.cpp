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
#include "ImsCore.h"
#include "media/StreamMediaProposal.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
StreamMediaProposal::StreamMediaProposal(IN ISdpOaState* piOAState_) :
        MediaProposal(piOAState_)
{
}

PUBLIC VIRTUAL StreamMediaProposal::~StreamMediaProposal()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: StreamMediaProposal", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_SINT32 StreamMediaProposal::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return ImsCore::MEDIA_TYPE_STREAM;
}
