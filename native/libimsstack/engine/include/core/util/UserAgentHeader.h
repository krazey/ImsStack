/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120508  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _USER_AGENT_HEADER_H_
#define _USER_AGENT_HEADER_H_

#include "AString.h"

class ISipMessage;
class SipProfile;

class UserAgentHeader
{
private:
    UserAgentHeader();

public:
    static void SetHeader(IN CONST AString& strName, IN CONST SipProfile* pSIPProfile,
            IN CONST AString& strServiceId, IN CONST IPAddress& objIP, IN IMS_SINT32 nSlotId,
            IN_OUT ISipMessage*& piSIPMsg);
};

#endif  // _USER_AGENT_HEADER_H_
