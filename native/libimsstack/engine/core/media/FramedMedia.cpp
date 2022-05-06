/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100503  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "base/IMS.h"
#include "IMSCore.h"
#include "media/MediaDescriptor.h"
#include "media/FramedMediaProposal.h"
#include "media/FramedMedia.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
FramedMedia::FramedMedia(IN Service* pService_, IN ISDPOAState* piOAState_) :
        Media(pService_, piOAState_)
{
    SetInitializationDone(IMS_TRUE);
}

PUBLIC VIRTUAL FramedMedia::~FramedMedia()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: FramedMedia", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_SINT32 FramedMedia::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return IMSCore::MEDIA_TYPE_FRAMED;
}

PROTECTED VIRTUAL MediaProposal* FramedMedia::CreateMediaProposal(IN ISDPOAState* piOAState)
{
    FramedMediaProposal* pMediaProposal = new FramedMediaProposal(piOAState);

    //---------------------------------------------------------------------------------------------

    if (pMediaProposal == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pMediaProposal->CreateDescriptor(GetMediaDescriptors()))
    {
        delete pMediaProposal;
        return IMS_NULL;
    }

    return pMediaProposal;
}
