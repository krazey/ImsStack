/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20201023  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "Sip.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipParsingHelper.h"
#include "util/OperatorFeatureResolver.h"

PUBLIC GLOBAL
IMS_BOOL OperatorFeatureResolver::IsMessageForEarlySessionModel(IN const ISIPMessage* piSIPMsg)
{
    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const AString strEarlySession(SIP::STR_EARLY_SESSION);

    if (!piSIPMsg->IsOptionRequired(strEarlySession))
    {
        return IMS_FALSE;
    }

    AString strHeaderBody = piSIPMsg->GetHeader(ISIPHeader::CONTENT_DISPOSITION);

    if (strHeaderBody.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    ISIPHeader* piHeader = SIPParsingHelper::CreateHeader(
            ISIPHeader::CONTENT_DISPOSITION, strHeaderBody);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strHeaderValue = piHeader->GetValue();
    piHeader->Destroy();

    if (!strHeaderValue.EqualsIgnoreCase(strEarlySession))
    {
        return IMS_FALSE;
    }

    return (piSIPMsg->GetSDPBodyPart() != IMS_NULL);
}
