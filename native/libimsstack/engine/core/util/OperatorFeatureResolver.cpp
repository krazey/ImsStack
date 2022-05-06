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

PUBLIC GLOBAL IMS_BOOL OperatorFeatureResolver::IsMessageForEarlySessionModel(
        IN const ISipMessage* piSIPMsg)
{
    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const AString strEarlySession(Sip::STR_EARLY_SESSION);

    if (!piSIPMsg->IsOptionRequired(strEarlySession))
    {
        return IMS_FALSE;
    }

    AString strHeaderBody = piSIPMsg->GetHeader(ISipHeader::CONTENT_DISPOSITION);

    if (strHeaderBody.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::CONTENT_DISPOSITION, strHeaderBody);

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

    return (piSIPMsg->GetSdpBodyPart() != IMS_NULL);
}
