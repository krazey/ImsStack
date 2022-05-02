/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class contains constants representing SIP response codes as defined in RFC 3261
    and extensions.
*/

#include "ServiceMemory.h"
#include "SipStatusCode.h"



PRIVATE GLOBAL
const SIPStatusCode::CodeTable SIPStatusCode::CODE_TABLE[] =
{
    { SIPStatusCode::SC_100, "Trying" },
    { SIPStatusCode::SC_180, "Ringing" },
    { SIPStatusCode::SC_181, "Call Is Being Forwarded" },
    { SIPStatusCode::SC_182, "Queued" },
    { SIPStatusCode::SC_183, "Session Progress" },
    { SIPStatusCode::SC_199, "Early Dialog Terminated" },
    { SIPStatusCode::SC_200, "OK" },
    { SIPStatusCode::SC_202, "Accepted" },
    { SIPStatusCode::SC_204, "No Notification" },
    { SIPStatusCode::SC_300, "Multiple Choices" },
    { SIPStatusCode::SC_301, "Moved Permanently" },
    { SIPStatusCode::SC_302, "Moved Temporarily" },
    { SIPStatusCode::SC_305, "Use Proxy" },
    { SIPStatusCode::SC_380, "Alternative Service" },
    { SIPStatusCode::SC_400, "Bad Request" },
    { SIPStatusCode::SC_401, "Unauthorized" },
    { SIPStatusCode::SC_402, "Payment Required" },
    { SIPStatusCode::SC_403, "Forbidden" },
    { SIPStatusCode::SC_404, "Not Found" },
    { SIPStatusCode::SC_405, "Method Not Allowed" },
    { SIPStatusCode::SC_406, "Not Acceptable" },
    { SIPStatusCode::SC_407, "Proxy Authentication Required" },
    { SIPStatusCode::SC_408, "Request Timeout" },
    { SIPStatusCode::SC_410, "Gone" },
    { SIPStatusCode::SC_412, "Conditional Request Failed" },
    { SIPStatusCode::SC_413, "Request Entity Too Large" },
    { SIPStatusCode::SC_414, "Request-URI Too Long" },
    { SIPStatusCode::SC_415, "Unsupported Media Type" },
    { SIPStatusCode::SC_416, "Unsupported URI Scheme" },
    { SIPStatusCode::SC_417, "Unknown Resource-Priority" },
    { SIPStatusCode::SC_420, "Bad Extension" },
    { SIPStatusCode::SC_421, "Extension Required" },
    { SIPStatusCode::SC_422, "Session Interval Too Small" },
    { SIPStatusCode::SC_423, "Interval Too Brief" },
    { SIPStatusCode::SC_424, "Bad Location Information" },
    { SIPStatusCode::SC_428, "Use Identity Header" },
    { SIPStatusCode::SC_429, "Provide Referrer Identity" },
    { SIPStatusCode::SC_430, "Flow Failed" },
    { SIPStatusCode::SC_433, "Anonymity Disallowed" },
    { SIPStatusCode::SC_436, "Bad Identity-Info" },
    { SIPStatusCode::SC_437, "Unsupported Certificate" },
    { SIPStatusCode::SC_438, "Invalid Identity Header" },
    { SIPStatusCode::SC_439, "First Hop Lacks Outbound Support" },
    { SIPStatusCode::SC_440, "Max-Breadth Exceeded" },
    { SIPStatusCode::SC_469, "Bad Info Package" },
    { SIPStatusCode::SC_470, "Consent Needed" },
    { SIPStatusCode::SC_480, "Temporarily Unavailable" },
    { SIPStatusCode::SC_481, "Call/Transaction Does Not Exist" },
    { SIPStatusCode::SC_482, "Loop Detect" },
    { SIPStatusCode::SC_483, "Too Many Hops" },
    { SIPStatusCode::SC_484, "Address Incomplete" },
    { SIPStatusCode::SC_485, "Ambiguous" },
    { SIPStatusCode::SC_486, "Busy Here" },
    { SIPStatusCode::SC_487, "Request Terminated" },
    { SIPStatusCode::SC_488, "Not Acceptable Here" },
    { SIPStatusCode::SC_489, "Bad Event" },
    { SIPStatusCode::SC_491, "Request Pending" },
    { SIPStatusCode::SC_493, "Undecipherable" },
    { SIPStatusCode::SC_494, "Security Agreement Required" },
    { SIPStatusCode::SC_499, "Not Reachable" },
    { SIPStatusCode::SC_500, "Server Internal Error" },
    { SIPStatusCode::SC_501, "Not Implemented" },
    { SIPStatusCode::SC_502, "Bad Gateway" },
    { SIPStatusCode::SC_503, "Service Unavailable" },
    { SIPStatusCode::SC_504, "Server Time-out" },
    { SIPStatusCode::SC_505, "Version Not Supported" },
    { SIPStatusCode::SC_513, "Message Too Large" },
    { SIPStatusCode::SC_580, "Precondition Failure" },
    { SIPStatusCode::SC_600, "Busy Everywhere" },
    { SIPStatusCode::SC_603, "Decline" },
    { SIPStatusCode::SC_604, "Does Not Exist Everywhere" },
    { SIPStatusCode::SC_606, "Not Acceptable" },

    { SIPStatusCode::SC_MAX, IMS_NULL }
};



PUBLIC
SIPStatusCode::SIPStatusCode(IN IMS_SINT32 nCode_ /* = SC_INVALID */)
    : nCode(nCode_)
    , strReasonPhrase(AString::ConstNull())
{
}

PUBLIC
SIPStatusCode::SIPStatusCode(IN IMS_SINT32 nCode_, IN CONST IMS_CHAR *pszReasonPhrase_)
    : nCode(nCode_)
    , strReasonPhrase(pszReasonPhrase_)
{
}

PUBLIC
SIPStatusCode::SIPStatusCode(IN CONST SIPStatusCode &objRHS)
    : nCode(objRHS.nCode)
    , strReasonPhrase(objRHS.strReasonPhrase)
{
}

PUBLIC
SIPStatusCode::~SIPStatusCode()
{
}

PUBLIC
SIPStatusCode& SIPStatusCode::operator=(IN CONST SIPStatusCode &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nCode = objRHS.nCode;
        strReasonPhrase = objRHS.strReasonPhrase;
    }

    return (*this);
}

/*
Gets a textual representation of the given SIP status code.

Remarks
*/
PUBLIC
SIPStatusCode& SIPStatusCode::operator=(IN IMS_SINT32 nCode)
{
    //---------------------------------------------------------------------------------------------

    this->nCode = nCode;
    return (*this);
}

/*
Gets a textual representation of the given SIP status code.

Remarks
*/
PUBLIC
SIPStatusCode& SIPStatusCode::operator=(IN CONST IMS_CHAR* pszReasonPhrase)
{
    //---------------------------------------------------------------------------------------------

    this->strReasonPhrase = pszReasonPhrase;
    return (*this);
}

/*
Gets a textual representation of the given SIP status code.

Remarks
*/
PUBLIC
SIPStatusCode& SIPStatusCode::operator=(IN CONST AString &strReasonPhrase)
{
    //---------------------------------------------------------------------------------------------

    this->strReasonPhrase = strReasonPhrase;
    return (*this);
}

/*
Gets a textual representation of the given SIP status code.

Remarks
*/
PUBLIC
IMS_SINT32 SIPStatusCode::Compare(IN IMS_SINT32 nCode_) const
{
    //---------------------------------------------------------------------------------------------

    return nCode - nCode_;
}

/*
Gets a textual representation of the given SIP status code.

Remarks
*/
PUBLIC GLOBAL
const IMS_CHAR* SIPStatusCode::GetReasonPhrase(IN IMS_SINT32 nCode)
{
    IMS_SINT32 nIndex = 0;

    //---------------------------------------------------------------------------------------------

    while (CODE_TABLE[nIndex].nCode != SC_MAX)
    {
        if (CODE_TABLE[nIndex].nCode == nCode)
            return CODE_TABLE[nIndex].pszReasonPhrase;

        nIndex++;
    }

    return "";
}
