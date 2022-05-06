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
#include "TextParser.h"
#include "base/IMS.h"
#include "IMSCore.h"
#include "media/MediaDescriptor.h"
#include "media/StreamMediaProposal.h"
#include "media/StreamMedia.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
StreamMedia::StreamMedia(IN Service* pService_, IN ISDPOAState* piOAState_) :
        Media(pService_, piOAState_)
{
    SetInitializationDone(IMS_TRUE);
}

PUBLIC VIRTUAL StreamMedia::~StreamMedia()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: StreamMedia", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_SINT32 StreamMedia::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return IMSCore::MEDIA_TYPE_STREAM;
}

PROTECTED VIRTUAL MediaProposal* StreamMedia::CreateMediaProposal(IN ISDPOAState* piOAState)
{
    StreamMediaProposal* pMediaProposal = new StreamMediaProposal(piOAState);

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
