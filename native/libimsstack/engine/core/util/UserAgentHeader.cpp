/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120508  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceUtil.h"
#include "AStringBuffer.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipConfigProxy.h"
#include "util/UserAgentHeader.h"


__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC GLOBAL
void UserAgentHeader::SetHeader(IN CONST AString &strName, IN CONST SIPProfile *pSIPProfile,
        IN CONST AString& /*strServiceId*/, IN CONST IPAddress& /*objIP*/, IN IMS_SINT32 nSlotId,
        IN_OUT ISIPMessage *&piSIPMsg)
{
    if (strName.GetLength() == 0)
    {
        return;
    }

    if (!SIPConfigProxy::IsUserAgentConfigured(nSlotId, pSIPProfile))
    {
        return;
    }

    AString strUaString;

    if (piSIPMsg->GetMethod().Equals(SIPMethod::REGISTER))
    {
        strUaString = SIPConfigProxy::GetRegUaString(nSlotId, pSIPProfile);
    }
    else
    {
        strUaString = SIPConfigProxy::GetUaString(nSlotId, pSIPProfile);
    }

    if (strUaString.GetLength() == 0)
    {
        IMS_TRACE_D("UA version is empty", 0, 0, 0);
        return;
    }

    if (piSIPMsg->SetHeader(ISIPHeader::UNKNOWN, strUaString, strName) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting %s header failed", strName.GetStr(), 0, 0);
        return;
    }
}
