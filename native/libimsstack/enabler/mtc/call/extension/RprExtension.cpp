#include "IMessage.h"
#include "ServiceTrace.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/RprExtension.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
RprExtension::RprExtension() :
        MtcExtension(MtcExtensionSet::OPTION_TAG_RPR)
{
}

PUBLIC
RprExtension::RprExtension(IN const RprExtension& objRhs) :
        MtcExtension(objRhs)
{
}

PUBLIC VIRTUAL
RprExtension::~RprExtension()
{
}

PUBLIC VIRTUAL
IMtcExtension* RprExtension::Clone() const
{
    return new RprExtension(*this);
}

PUBLIC VIRTUAL
void RprExtension::HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest)
{
    if (nMethod != IMessage::SESSION_START)
    {
        return;
    }

    MtcExtension::HandleRequest(nMethod, objRequest);
}

PUBLIC VIRTUAL
void RprExtension::HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse)
{
    if (nMethod != IMessage::SESSION_START)
    {
        return;
    }

    MtcExtension::HandleResponse(nMethod, objResponse);
}
