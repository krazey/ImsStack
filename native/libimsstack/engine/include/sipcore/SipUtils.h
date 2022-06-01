/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_UTIL_H_
#define _SIP_UTIL_H_

#include "AString.h"

class SIPUtil
{
public:
    static AString GenerateBoundary();
    static AString GenerateCallId(IN const AString& strHost);
    // HEADER_REQ_SESSION-ID
    // draft-kaplan-insipid-session-id-04, for Session-ID header field
    static AString GenerateSessionId(IN IMS_SINT32 nSlotId, IN const AString& strCallId);
    static AString GenerateTag(IN const AString& strMagicCookie);
    static AString GenerateViaBranch(IN const IMS_CHAR* pszToTag, IN const IMS_CHAR* pszFromTag,
            IN const IMS_CHAR* pszCallID, IN const IMS_CHAR* pszRequestURI,
            IN const IMS_CHAR* pszTopmostVia, IN IMS_SINT32 nCSeqNum,
            IN const AString& strExtensionToken = AString::ConstNull());
    static AString GenerateViaBranch(IN const AString& strExtensionToken = AString::ConstNull());

    static void Init(IN IMS_SINT32 nSlotId);

private:
    // HEADER_REQ_SESSION-ID
    static AString* pFixedKeyForSessionId;
};

#endif  // _SIP_UTIL_H_
