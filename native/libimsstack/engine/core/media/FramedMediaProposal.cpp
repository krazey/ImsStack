/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100507  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IMSCore.h"
#include "media/FramedMediaProposal.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
FramedMediaProposal::FramedMediaProposal(IN ISDPOAState* piOAState_) :
        MediaProposal(piOAState_)
{
}

PUBLIC VIRTUAL FramedMediaProposal::~FramedMediaProposal()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: FramedMediaProposal", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_SINT32 FramedMediaProposal::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return IMSCore::MEDIA_TYPE_FRAMED;
}
