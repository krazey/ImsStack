#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipAddress.h"

#include "call/IMtcCallManager.h"

#include "conferencecall/ConferenceUtils.h"

__IMS_TRACE_TAG_COM_MTC__;


PUBLIC GLOBAL
const AString& ConferenceUtils::GetUserPart(IN const AString& strUri, OUT AString& strUserPart)
{
    if (strUri.Contains("sip") || strUri.Contains("tel"))
    {
        SIPAddress objSIPAddress(strUri);

        if (objSIPAddress.GetUserInfoPart() != IMS_NULL)
        {
            strUserPart = objSIPAddress.GetUserInfoPart()->GetUser();
        }
    }
    else
    {
        strUserPart = strUri;
    }

    return strUserPart;
}
