/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20160523  hwangoo.park@             Created
    </table>

    Description

*/

#include "SipConfigProxy.h"
#include "SipFeatures.h"

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsEventHeaderApplicableForRefer(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsHeaderSessionIdRequired(IN IMS_SINT32 nSlotId)
{
    return SIPConfigProxy::IsSessionIdHeaderSupported(nSlotId);
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsHostPartValidationRequiredForIncomingRequestRouting(
        IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsPANIHeaderForAckRequired(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsSocketOptionRequiredForTcpMaxSeg(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsReferSubHeaderSupported(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsStandard2XXRetransmissionIntervalRequired(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsTransportParameterIgnoredForIncomingRequestRouting(
        IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsTransportParameterUdpIgnoredForOutgoingRequest(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SIPFeatures::IsTransportParameterIgnoredForRegBinding(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}
