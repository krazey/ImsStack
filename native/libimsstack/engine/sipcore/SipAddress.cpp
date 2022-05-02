/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class provides a generic SIP address parser.
    It can be used to parse either full SIP name addresses like:
        - IMSer <sip:user@ims.com>
        - sip:+821075323421@ims.com;user=phone
        - sips:user@ims.com;transport=tcp

    Remarks
    - It does not escape address strings.
    - It ignores header part of SIP URI.
    - Its valid scheme format is the same as defined in SIP BNF for absolute URI.
    - It allows "*" as the valid address of Contact header.
    In this case, ToString() & GetURI() return "*", all other accessor methods return NULL or 0.
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "SystemConfig.h"
#include "IMSLib.h"
#include "IPAddress.h"
#include "AStringBuffer.h"
#include "SIPHeader.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SipConfigProxy.h"
#include "SIPStackHeaders.h"
#include "SIPUtil.h"
#include "SipAddress.h"

__IMS_TRACE_TAG_SIP__;



PUBLIC GLOBAL
const IMS_CHAR SIPAddress::PARAM_MADDR[] = "maddr";
PUBLIC GLOBAL
const IMS_CHAR SIPAddress::PARAM_METHOD[] = "method";
PUBLIC GLOBAL
const IMS_CHAR SIPAddress::PARAM_PHONE_CONTEXT[] = "phone-context";
PUBLIC GLOBAL
const IMS_CHAR SIPAddress::PARAM_TRANSPORT[] = "transport";
PUBLIC GLOBAL
const IMS_CHAR SIPAddress::PARAM_TTL[] = "ttl";
PUBLIC GLOBAL
const IMS_CHAR SIPAddress::PARAM_USER[] = "user";



/*
 Constructs a new UserInfoPart.

Remarks

*/
PUBLIC
SIPAddress::UserInfoPart::UserInfoPart()
    : strUser(AString::ConstNull())
    , strPassword(AString::ConstNull())
{
}

/*
 Constructs a new UserInfoPart.

Remarks

*/
PUBLIC
SIPAddress::UserInfoPart::UserInfoPart(IN CONST SIPAddress::UserInfoPart &objRHS)
    : strUser(objRHS.strUser)
    , strPassword(objRHS.strPassword)
{
    for (IMS_UINT32 i = 0; i < objRHS.objParameters.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objRHS.objParameters.GetAt(i);

        if (pParameter == IMS_NULL)
            continue;

        objParameters.Append(new SIPParameter(*pParameter));
    }
}

/*
 Destructs this UserInfoPart.

Remarks

*/
PUBLIC
SIPAddress::UserInfoPart::~UserInfoPart()
{
    RemoveAllParameters();
}

/*
 Override the default assignment operator.

Remarks

*/
PUBLIC
SIPAddress::UserInfoPart& SIPAddress::UserInfoPart::operator=(
        IN CONST SIPAddress::UserInfoPart &objRHS)
{
    if (this != &objRHS)
    {
        RemoveAllParameters();

        strUser = objRHS.strUser;
        strPassword = objRHS.strPassword;

        for (IMS_UINT32 i = 0; i < objRHS.objParameters.GetSize(); ++i)
        {
            const SIPParameter *pParameter = objRHS.objParameters.GetAt(i);

            if (pParameter == IMS_NULL)
                continue;

            objParameters.Append(new SIPParameter(*pParameter));
        }
    }

    return (*this);
}

/*
 Parses the userinfo part of SIP/SIPS URI.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strUserInfo             Userinfo part string
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The userinfo part is successfully parsed
IMS_FALSE               An error occurs during parsing
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::UserInfoPart::Create(IN CONST AString &strUserInfo)
{
    //---------------------------------------------------------------------------------------------

    if (strUserInfo.GetLength() == 0)
    {
        // No user part
        return IMS_FALSE;
    }

    strUser = AString::ConstNull();
    strPassword = AString::ConstNull();
    RemoveAllParameters();

    strUserInfo.SplitB(TextParser::CHAR_COLON, strUser, strPassword);

    if (strUser.Contains(TextParser::CHAR_SEMICOLON))
    {
        IMSList<AString> objTokens = strUser.Split(TextParser::CHAR_SEMICOLON);

        strUser = objTokens.GetAt(0);

        for (IMS_UINT32 i = 1; i < objTokens.GetSize(); ++i)
        {
            SIPParameter *pParameter = new SIPParameter();

            if (pParameter == IMS_NULL)
            {
                continue;
            }

            if (!pParameter->Create(objTokens.GetAt(i)))
            {
                IMS_TRACE_E(0, "Parsing the parameter of userinfo part is failed", 0, 0, 0);
                delete pParameter;
                continue;
            }

            if (!objParameters.Append(pParameter))
            {
                delete pParameter;
            }
        }
    }

    return IMS_TRUE;
}

/*
 Returns the parameter of user-info part.
All the parameters are separated by the semi-colon.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
SIPParameter*           SIPParameter to the named userinfo parameter
NULL pointer            The named userinfo parameter is not found
</table>
*/
PUBLIC
const SIPParameter* SIPAddress::UserInfoPart::GetParameter(IN CONST AString &strName) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParameters.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            return pParameter;
        }
    }

    return IMS_NULL;
}

/*
 Removes all the parameters.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PRIVATE
void SIPAddress::UserInfoPart::RemoveAllParameters()
{
    //---------------------------------------------------------------------------------------------

    if (objParameters.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        SIPParameter *pParameter = objParameters.GetAt(i);

        if (pParameter == IMS_NULL)
            continue;

        delete pParameter;
    }

    objParameters.Clear();
}



/*
 Constructs a new SIPAddress without any values.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPAddress::SIPAddress()
    : bIsWildcard(IMS_FALSE)
    , bDQUOTForDisplayName(IMS_FALSE)
    , bAQUOTRequired(IMS_FALSE)
    , strDisplayName(AString::ConstNull())
    , strScheme(AString::ConstNull())
    , strUserInfo(AString::ConstNull())
    , strHostInfo(AString::ConstNull())
    , nPort(SIP::PORT_UNSPECIFIED)
    , pUserInfoPart(IMS_NULL)
{
}

/*
 Constructs a new SIPAddress from string.
The string can be either following:
    - Name address: DisplayName <sip:user:password@host:port;uri-parameters>
    - Plain SIP URI: sip:user:password@host:port;uri-parameters
    - Special address: "*"

Remarks

Parameters
<table>
parameter               description
----------              ----------
strAddress              SIP address format as string
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPAddress::SIPAddress(IN CONST AString &strAddress)
    : bIsWildcard(IMS_FALSE)
    , bDQUOTForDisplayName(IMS_FALSE)
    , bAQUOTRequired(IMS_FALSE)
    , strDisplayName(AString::ConstNull())
    , strScheme(AString::ConstNull())
    , strUserInfo(AString::ConstNull())
    , strHostInfo(AString::ConstNull())
    , nPort(SIP::PORT_UNSPECIFIED)
    , pUserInfoPart(IMS_NULL)
{
    //---------------------------------------------------------------------------------------------

    Decode(strAddress, IMS_TRUE);
}

/*
 Constructs a new SIPAddress from the display name and URI string.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strDisplayName_         Display name
strURI                  SIP address format string as addr-spec
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPAddress::SIPAddress(IN CONST AString &strDisplayName_, IN CONST AString &strURI)
    : bIsWildcard(IMS_FALSE)
    , bDQUOTForDisplayName(IMS_FALSE)
    , bAQUOTRequired(IMS_FALSE)
    , strDisplayName(strDisplayName_)
    , strScheme(AString::ConstNull())
    , strUserInfo(AString::ConstNull())
    , strHostInfo(AString::ConstNull())
    , nPort(SIP::PORT_UNSPECIFIED)
    , pUserInfoPart(IMS_NULL)
{
    //---------------------------------------------------------------------------------------------

    Decode(strURI, IMS_TRUE, IMS_FALSE);
}

/*
 Constructs a new SIPAddress from the SIPAddress object.

Remarks

Parameters
<table>
parameter               description
----------              ----------
objRHS                  Reference to SIPAddress
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPAddress::SIPAddress(IN CONST SIPAddress &objRHS)
    : bIsWildcard(objRHS.bIsWildcard)
    , bDQUOTForDisplayName(objRHS.bDQUOTForDisplayName)
    , bAQUOTRequired(objRHS.bAQUOTRequired)
    , strDisplayName(objRHS.strDisplayName)
    , strScheme(objRHS.strScheme)
    , strUserInfo(objRHS.strUserInfo)
    , strHostInfo(objRHS.strHostInfo)
    , nPort(objRHS.nPort)
    , pUserInfoPart(IMS_NULL)
{
    IMS_UINT32 i;

    //---------------------------------------------------------------------------------------------

    for (i = 0; i < objRHS.objHeaders.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objRHS.objHeaders.GetAt(i);
        ISIPHeader *piNewHeader = piHeader->Clone();

        if (piNewHeader != IMS_NULL)
            objHeaders.Append(piNewHeader);
    }

    for (i = 0; i < objRHS.objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objRHS.objParams.GetAt(i);
        SIPParameter *pNewParameter = new SIPParameter(*pParameter);

        if (pNewParameter != IMS_NULL)
            objParams.Append(pNewParameter);
    }
}

/*
 Destructs the SIPAddress instance.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPAddress::~SIPAddress()
{
    //---------------------------------------------------------------------------------------------

    RemoveAllHeaderComponents();
    RemoveAllParameters();

    if (pUserInfoPart != IMS_NULL)
    {
        delete pUserInfoPart;
    }
}

/*
 Copys the given SIPAddress's values to its own variables.
It overrides a default assignment operation for SIPAddress object.

Remarks

Parameters
<table>
parameter               description
----------              ----------
objRHS                  Reference to SIPAddress
</table>

Returns
<table>
return                  description
----------              ----------
SIPAddress              Reference to own SIPAddress
</table>
*/
PUBLIC
SIPAddress& SIPAddress::operator=(IN CONST SIPAddress &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        RemoveAllHeaderComponents();
        RemoveAllParameters();

        bIsWildcard = objRHS.bIsWildcard;

        bDQUOTForDisplayName = objRHS.bDQUOTForDisplayName;
        bAQUOTRequired = objRHS.bAQUOTRequired;
        strDisplayName = objRHS.strDisplayName;
        strScheme = objRHS.strScheme;
        strUserInfo = objRHS.strUserInfo;
        strHostInfo = objRHS.strHostInfo;
        nPort = objRHS.nPort;

        IMS_UINT32 i;

        for (i = 0; i < objRHS.objHeaders.GetSize(); ++i)
        {
            const ISIPHeader *piHeader = objRHS.objHeaders.GetAt(i);
            ISIPHeader *piNewHeader = piHeader->Clone();

            if (piNewHeader != IMS_NULL)
                objHeaders.Append(piNewHeader);
        }

        for (i = 0; i < objRHS.objParams.GetSize(); ++i)
        {
            const SIPParameter *pParameter = objRHS.objParams.GetAt(i);
            SIPParameter *pNewParameter = new SIPParameter(*pParameter);

            if (pNewParameter != IMS_NULL)
                objParams.Append(pNewParameter);
        }
    }

    return (*this);
}

/*
 Adds the named URI parameter to the specified value. If the value is NULL, the parameter
is interpreted as a parameter without value. The parameter value will be added.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 The named URI parameter
strValue                The URI parameter value to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The URI parameter is successfully set
IMS_FAILURE             The name is NULL, invalid or the address represents the immutable "*" value
</table>
*/
IMS_RESULT SIPAddress::AddParameter(IN CONST AString &strName, IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (strName.IsNULL() || bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check the grammar validity

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            pParameter->AddValue(strValue);

            SIPPrivate::SetLastError(SIPError::NO_ERROR);
            return IMS_SUCCESS;
        }
    }

    // If the parameter with the specified name does not exist, then add new one.
    SIPParameter *pParameter = IMS_NULL;

    if (strValue.IsNULL())
        pParameter = new SIPParameter(strName);
    else
        pParameter = new SIPParameter(strName, strValue);

    if (pParameter == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::NO_MEMORY);
        return IMS_FAILURE;
    }

    if (!objParams.Append(pParameter))
    {
        delete pParameter;
        SIPPrivate::SetLastError(SIPError::LIST_OPERATION_FAILED);
        return IMS_FAILURE;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Parses the SIP address format string.
The string can be either following:
    - Name address: DisplayName <sip:user:password@host:port;uri-parameters>
    - Plain SIP URI: sip:user:password@host:port;uri-parameters
    - Special address: "*"

Remarks

Parameters
<table>
parameter               description
----------              ----------
strAddress              SIP address format string
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The address is successfully parsed
IMS_FALSE               An error occurs during parsing
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::Create(IN CONST AString &strAddress)
{
    //---------------------------------------------------------------------------------------------

    if (strAddress.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    bIsWildcard = IMS_FALSE;
    bDQUOTForDisplayName = IMS_FALSE;
    strDisplayName = AString::ConstNull();
    strScheme = AString::ConstNull();
    strUserInfo = AString::ConstNull();
    strHostInfo = AString::ConstNull();
    nPort = SIP::PORT_UNSPECIFIED;

    RemoveAllParameters();
    RemoveAllHeaderComponents();

    if (!Decode(strAddress, IMS_TRUE))
        return IMS_FALSE;

    return IMS_TRUE;
}

/*
 Checks if the given SIPAddress is the same.

Remarks

Parameters
<table>
parameter               description
----------              ----------
objAddress              Object for SIPAddress which is compared
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                If both SIP addresses matched, this value returns
IMS_FALSE               If both SIP addresses are not matched, this value returns
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::Equals(IN CONST SIPAddress &objAddress) const
{
    //---------------------------------------------------------------------------------------------

    // If both addresses are the special ("*") value, those values are the same.
    if (bIsWildcard && objAddress.bIsWildcard)
        return IMS_TRUE;

    if (!bIsWildcard && objAddress.bIsWildcard)
        return IMS_FALSE;

    if (bIsWildcard && !objAddress.bIsWildcard)
        return IMS_FALSE;

    if (!strScheme.EqualsIgnoreCase(objAddress.strScheme))
        return IMS_FALSE;

    // "sip" or "sips" URI scheme
    if (IsSchemeSIP() || IsSchemeSIPS())
    {
        //4 needs to be updated
        return CompareSIPURIs(objAddress);
    }
    else if (IsSchemeTEL())
    {
        return CompareTelURIs(objAddress);
    }
    //4 comparison of an address format with the other URI scheme
    else
    {
        return strHostInfo.EqualsIgnoreCase(objAddress.strHostInfo);
    }
}

/*
 Returns the display name of SIP addresss.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Non-NULL string         The display name if it is available
NULL string             NULL string if it is not available or the address is a special ("*") value
</table>
*/
PUBLIC
const AString& SIPAddress::GetDisplayName() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return AString::ConstNull();

    return strDisplayName;
}

/*
 Returns a header of SIP address which matches with the specified header type.

Remarks

Parameters
<table>
parameter               description
----------              ----------
nType                   Header type
</table>

Returns
<table>
return                  description
----------              ----------
ISIPHeader*             Pointer to uri-header parameter
</table>
*/
PUBLIC
const ISIPHeader* SIPAddress::GetHeader(IN IMS_SINT32 nType,
        IN CONST AString &strName /* = AString::ConstNull() */) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        if (nType == piHeader->GetType())
        {
            if (nType != ISIPHeader::UNKNOWN)
            {
                return piHeader;
            }
            else
            {
                const IMS_CHAR cCompactName = SIPStack::GetCompactHeaderName(nType, strName);
                const IMS_CHAR *pszName = SIPStack::GetHeaderName(nType, strName);
                const AString &strHeaderName = piHeader->GetName();

                if (strHeaderName.EqualsIgnoreCase(cCompactName)
                        || strHeaderName.EqualsIgnoreCase(pszName))
                {
                    return piHeader;
                }
            }
        }
    }

    return IMS_NULL;
}

/*
 Returns the headers of SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMSList<ISIPHeader*>    List of uri-header parameter
</table>
*/
PUBLIC
const IMSList<ISIPHeader*>& SIPAddress::GetHeaders() const
{
    //---------------------------------------------------------------------------------------------

    return objHeaders;
}

/*
 Returns the host part of SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Non-NULL string         The host part of this address
NULL string             NULL string if the address is the special ("*") value
</table>
*/
PUBLIC
const AString& SIPAddress::GetHost() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return AString::ConstNull();

    return strHostInfo;
}

/*
 Returns the value associated with the named URI parameter.

Remarks

Parameters
<table>
parameter                   description
----------                  ----------
strName                     The name of URI parameter
</table>

Returns
<table>
return                      description
----------                  ----------
Pointer to SIPParameter     SIPParameter to the named URI parameter
NULL pointer                The named URI parameter is not found
</table>
*/
PUBLIC
const SIPParameter* SIPAddress::GetParameter(IN CONST AString &strName) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            return pParameter;
        }
    }

    return IMS_NULL;
}

/*
 Returns all the uri-parameters of SIP address.

Remarks

Parameters
<table>
parameter                   description
----------                  ----------
</table>

Returns
<table>
return                      description
----------                  ----------
IMSList<SIPParameter*>      List of uri-parameter
</table>
*/
PUBLIC
const IMSList<SIPParameter*>& SIPAddress::GetParameters() const
{
    //---------------------------------------------------------------------------------------------

    return objParams;
}

/*
 Returns the port number of the SIP address.
 If the port number is not set, the function returns 5060 for "sip" scheme and
 5061 for "sips" scheme. If the address is wildcard ("*"), it returns 0.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Port number             The value which is explicitly set or default value (5060 or 5061)
0                       Zero if the schems is not SIP or SIPS
                        or the address is the special ("*") value
</table>
*/
PUBLIC
IMS_SINT32 SIPAddress::GetPort() const
{
    //---------------------------------------------------------------------------------------------

    if (nPort == SIP::PORT_UNSPECIFIED)
    {
        if (bIsWildcard)
            return 0;

        if (strScheme.Equals(SIP::STR_SIP))
            return SIP::PORT_5060;
        else if (strScheme.Equals(SIP::STR_SIPS))
            return SIP::PORT_5061;
        else
            return 0;
    }

    return nPort;
}

/*
 Returns the scheme of SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Non-NULL string         The scheme of this SIP address (sip/sips/tel/pres/im/...)
NULL string             NULL string if the address is the special ("*") value
</table>
*/
PUBLIC
const AString& SIPAddress::GetScheme() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return AString::ConstNull();

    return strScheme;
}

/*
 Returns the URI part of SIP address (without parameters).
The URI part of the address is of the form "scheme:user@host:port".

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Non-NULL string         The URI part of this SIP address
"*"                     If the address is the special ("*") value
</table>
*/
PUBLIC
AString SIPAddress::GetURI() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
    {
        return AString(&TextParser::CHAR_ASTERISK, 1);
    }

    if (strHostInfo.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AString strURI = AString::ConstNull();

    if (strScheme.GetLength() > 0)
    {
        strURI += strScheme;
        strURI += TextParser::CHAR_COLON;
    }

    if (strUserInfo.GetLength() > 0)
    {
        strURI += strUserInfo;
        strURI += TextParser::CHAR_AT;
    }

    IPAddress objHost;

    if (objHost.Parse(strHostInfo))
    {
        if (objHost.IsIPv6Address())
        {
            strURI += TextParser::CHAR_LSBRACKET;
            strURI += strHostInfo;
            strURI += TextParser::CHAR_RSBRACKET;
        }
        else
        {
            strURI += strHostInfo;
        }
    }
    else
    {
        strURI += strHostInfo;
    }

    if (nPort != SIP::PORT_UNSPECIFIED)
    {
        AString strPort;
        strPort.SetNumber(nPort);

        strURI += TextParser::CHAR_COLON;
        strURI += strPort;
    }

    return strURI;
}

/*
 Returns the user part of SIP address. There is no separate method for getting the password field
so that if the password field is present in the address, then the function returns the value of
"user:password" (instead of only "user").

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Non-NULL string         The user part of this SIP address
NULL string             If the user part is missing or the address is the special ("*") value
</table>
*/
PUBLIC
const AString& SIPAddress::GetUser() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return AString::ConstNull();

    return strUserInfo;
}

/*
 Returns the pointer of UserInfoPart.
(userinfo := (user / telephon-number) [ ":" password ])

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
UserInfoPart*           If userinfo part is present, returns the pointer of UserInfoPart.
                        Otherwise, returns IMS_NULL.
</table>
*/
PUBLIC
const SIPAddress::UserInfoPart* SIPAddress::GetUserInfoPart() const
{
    //---------------------------------------------------------------------------------------------

    if (strUserInfo.GetLength() == 0)
    {
        // No userinfo part

        // If SIP address is "tel" URI...
        if (IsSchemeTEL())
        {
            return CreateUserInfoPart(strHostInfo);
        }
        else if (IsServiceURN())
        {
            // To remove "service:" prefix
            IMS_SINT32 nIndex = strHostInfo.GetIndexOf(TextParser::CHAR_COLON);

            if (nIndex != AString::NPOS)
            {
                AString strUserPart = strHostInfo.GetSubStr(nIndex + 1);

                return CreateUserInfoPart(strUserPart);
            }
        }

        return IMS_NULL;
    }

    return CreateUserInfoPart(strUserInfo);
}

/*
 Returns the user part of SIP address.
 For SIP or SIPS, there is no separate method for getting the password field
so that if the password field is present in the address, then the function returns the value of
"user:password" (instead of only "user").
 For the others, the funcation returns the host part of SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Non-NULL string         The user part of this SIP address
NULL string             If the user part is missing or the address is the special ("*") value
</table>
*/
PUBLIC
const AString& SIPAddress::GetUserPart() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
    {
        return AString::ConstNull();
    }

    if (IsSchemeSIP() || IsSchemeSIPS())
    {
        return strUserInfo;
    }

    return strHostInfo;
}

/*
 Checks if the port number is explicitly set or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The port number is not set explicitly
IMS_FALSE               The port number is set explicitly
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::IsPortUnspecified() const
{
    //---------------------------------------------------------------------------------------------

    return (nPort == SIP::PORT_UNSPECIFIED);
}

/*
 Checks if the scheme is SIP URI scheme or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The scheme is SIP URI scheme
IMS_FALSE               The scheme is not SIP URI scheme
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::IsSchemeSIP() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return IMS_FALSE;

    return strScheme.EqualsIgnoreCase(SIP::STR_SIP);
}

/*
 Checks if the scheme is SIPS URI scheme or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The scheme is SIPS URI scheme
IMS_FALSE               The scheme is not SIPS URI scheme
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::IsSchemeSIPS() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return IMS_FALSE;

    return strScheme.EqualsIgnoreCase(SIP::STR_SIPS);
}

/*
 Checks if the scheme is TEL URI scheme or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The scheme is TEL URI scheme
IMS_FALSE               The scheme is not TEL URI scheme
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::IsSchemeTEL() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
        return IMS_FALSE;

    return strScheme.EqualsIgnoreCase(SIP::STR_TEL);
}

/*
 Checks if the SIP address is for a service URN or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The SIP address is a service URN
IMS_FALSE               The SIP address is not a service URN
</table>
*/
PUBLIC
IMS_BOOL SIPAddress::IsServiceURN() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
    {
        return IMS_FALSE;
    }

    if (!strScheme.EqualsIgnoreCase("urn"))
    {
        return IMS_FALSE;
    }

    // Check if "service:" is present in case-insensitively
    return strHostInfo.StartsWith("service:");
}

/*
 Removes all the header components.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
void SIPAddress::RemoveAllHeaderComponents()
{
    //---------------------------------------------------------------------------------------------

    if (objHeaders.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        ISIPHeader *piHeader = objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        piHeader->Destroy();
    }

    objHeaders.Clear();
}

/*
 Removes all the uri parameters.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
void SIPAddress::RemoveAllParameters()
{
    //---------------------------------------------------------------------------------------------

    if (objParams.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter == IMS_NULL)
            continue;

        delete pParameter;
    }

    objParams.Clear();
}

/*
 Removes the named URI parameter. The method returns without any action
if the named parameter is not defined or if the address is the special ("*") value.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 The name of the parameter to remove
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
void SIPAddress::RemoveParameter(IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            objParams.RemoveAt(i);
            delete pParameter;
            return;
        }
    }
}

/*
 Sets the flag to indicate that AQUOT is required when forming URI
even though any uri parameter doesn't exist.

Remarks

Parameters
<table>
parameter               description
----------              ----------
bAQUOTRequired          Flag for AQUOT requirement
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
void SIPAddress::SetAQUOTRequired(IN IMS_BOOL bAQUOTRequired)
{
    this->bAQUOTRequired = bAQUOTRequired;
}

/*
 Sets the display name. Empty string or NULL removes the display name.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 The display name to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The display name is successfully set
IMS_FAILURE             The display name is invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetDisplayName(IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    // Check the grammar validity

    if (bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    strDisplayName = strName;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the flag to indicate if DQUOT is required when forming display-name field.

Remarks

Parameters
<table>
parameter               description
----------              ----------
bDQUOTRequired          Flag to indicate if DQUOTE should be included in display-name field
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
void SIPAddress::SetDQUOTRequiredForDisplayName(IN IMS_BOOL bDQUOTRequired)
{
    bDQUOTForDisplayName = bDQUOTRequired;
}

/*
 Sets the header field.

Remarks

Parameters
<table>
parameter               description
----------              ----------
nType                   Header type as a enumeration
strValue                Header value
strName                 Header name; it is valid if nType is UNKNOWN
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The headers is successfully set
IMS_FAILURE             The headers is invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
        IN CONST AString &strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    // Check the grammar validity

    if (bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if ((nType <= ISIPHeader::INVALID) || (nType >= ISIPHeader::ANY))
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    SIPHeader *pHeader = new SIPHeader(nType);

    if (pHeader == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::NO_MEMORY);
        return IMS_FAILURE;
    }

    if (nType == ISIPHeader::UNKNOWN)
    {
        pHeader->SetName(strName);
    }

    if (pHeader->SetHeaderValue(TextParser::DoPercentDecoding(strValue)) != IMS_SUCCESS)
    {
        SIPPrivate::SetLastError(SIPError::PARSING_ERROR);

        delete pHeader;
        return IMS_FAILURE;
    }

    if (!objHeaders.Append(pHeader))
    {
        delete pHeader;
        return IMS_FAILURE;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the headers field.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strHeaders              The headers field to set
                        If the multiple headers set, each header will be identified by '&'.
bRemoveAll              Flag to indicate if the existing header parameters are removed
                        or not before setting the header parameters
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The headers is successfully set
IMS_FAILURE             The headers is invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetHeaders(IN CONST AString &strHeaders,
        IN IMS_BOOL bRemoveAll /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    // Check the grammar validity

    if (bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (bRemoveAll)
    {
        RemoveAllHeaderComponents();
    }

    if (!DecodeHeaderComponent(strHeaders))
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the host part of the SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strHost                 The host part to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The host part is successfully set
IMS_FAILURE             The host part is NULL, invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetHost(IN CONST AString &strHost)
{
    //---------------------------------------------------------------------------------------------

    // Check the grammar validity

    if (strHost.IsNULL() || bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE; // Throw exception
    }

    IPAddress objHost;

    if (objHost.Parse(strHost))
    {
        strHostInfo = objHost.ToString();
    }
    else
    {
        strHostInfo = strHost;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the named URI parameter to the specified value. If the value is NULL, the parameter
is interpreted as a parameter without value. Existing parameter will be overwritten,
otherwise the parameter is added.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 The named URI parameter
strValue                The URI parameter value to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The URI parameter is successfully set
IMS_FAILURE             The name is NULL, invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetParameter(IN CONST AString &strName, IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (strName.IsNULL() || bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check the grammar validity

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            if (pParameter->SetValue(strValue) != IMS_SUCCESS)
            {
                return IMS_FAILURE;
            }

            SIPPrivate::SetLastError(SIPError::NO_ERROR);
            return IMS_SUCCESS;
        }
    }

    // If the parameter with the specified name does not exist, then add new one.
    SIPParameter *pParameter = IMS_NULL;

    if (strValue.IsNULL())
        pParameter = new SIPParameter(strName);
    else
        pParameter = new SIPParameter(strName, strValue);

    if (pParameter == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::NO_MEMORY);
        return IMS_FAILURE;
    }

    if (!objParams.Append(pParameter))
    {
        delete pParameter;
        SIPPrivate::SetLastError(SIPError::LIST_OPERATION_FAILED);
        return IMS_FAILURE;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the port number of the SIP address. The valid range is 0 ~ 65535.
After setting the port to 0, the port number is removed from the address URI,
GetPort() returns the default 5060 value for "sip" and 5061 for "sips" scheme.

Remarks

Parameters
<table>
parameter               description
----------              ----------
nPort                   Port number (0 ~ 65535) to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The port number is successfully set
IMS_FAILURE             The port number is invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetPort(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    if ((nPort < 0) || (nPort > 65535) || bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (nPort == 0)
        this->nPort = SIP::PORT_UNSPECIFIED;
    else
        this->nPort = nPort;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the scheme of the SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strScheme               The scheme format to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The scheme is successfully set
IMS_FAILURE             The scheme is invalid
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetScheme(IN CONST AString &strScheme)
{
    //---------------------------------------------------------------------------------------------

    if (strScheme.IsNULL() || bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    this->strScheme = strScheme;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Sets the URI part of the SIP address (without parameter). The URI part of the address is of the
form "scheme:user@host:port". If the parameter of the method contains URI parameters,
they don't overwrite existing URI parameters, they are simply ignored if present.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strURI                  The URI part of the address to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The URI part is successfully set
IMS_FAILURE             The URI part is invalid, "*"
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetURI(IN CONST AString &strURI)
{
    //---------------------------------------------------------------------------------------------

    if (strURI.IsNULL() || strURI.Equals(TextParser::CHAR_ASTERISK) || bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!Decode(strURI, IMS_FALSE, IMS_FALSE, IMS_FALSE))
        return IMS_FAILURE;

    return IMS_SUCCESS;
}

/*
 Sets the user part of the SIP address. Empty string or NULL removes the user part.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strUser                 The user part of the address to set
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The user part is successfully set
IMS_FAILURE             The user part is invalid, "*"
                        or the address represents the immutable "*" value
</table>
*/
PUBLIC
IMS_RESULT SIPAddress::SetUser(IN CONST AString &strUser)
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check the grammar rules

    this->strUserInfo = strUser;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Returns a fully qualified SIP address, with display name, URI and URI parameters.
If display name is not specified only a SIP URI is returned.
If the port is not explicitly set (to 5060 or other value), it will be omitted from the address
URI in returned string.

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
Fully qualified SIP address    SIP/SIPS/TEL/... or "*" if the address is the special ("*") value
</table>
*/
PUBLIC
AString SIPAddress::ToString() const
{
    //---------------------------------------------------------------------------------------------

    if (bIsWildcard)
    {
        return AString(&TextParser::CHAR_ASTERISK, 1);
    }

    if (strHostInfo.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objStringBuffer(512);
    IMS_BOOL bFormAQUOT = IMS_FALSE;

    if (bAQUOTRequired || (strDisplayName.GetLength() > 0)
            || !objParams.IsEmpty() || !objHeaders.IsEmpty())
    {
        bFormAQUOT = IMS_TRUE;
    }

    if (strDisplayName.GetLength() > 0)
    {
        if (IsDisplayNameToken(strDisplayName))
        {
            IMS_SINT32 nSlotId = IMS_SLOT_0;

            if (!bDQUOTForDisplayName && SystemConfig::IsMultiSimEnabled())
            {
                nSlotId = ThreadService::GetCurrentSlotId();
            }

            if (bDQUOTForDisplayName
                    || ((nSlotId != IMS_SLOT_ANY)
                        && SIPConfigProxy::IsDisplayNameDQUOTRequired(nSlotId)))
            {
                objStringBuffer.Append(TextParser::CHAR_DQUOT);
                objStringBuffer.Append(strDisplayName);
                objStringBuffer.Append(TextParser::CHAR_DQUOT);
            }
            else
            {
                objStringBuffer.Append(strDisplayName);
            }
        }
        else
        {
            objStringBuffer.Append(TextParser::CHAR_DQUOT);

            // '"' / '\' -> need to be escaped
            if (strDisplayName.Contains('"') || strDisplayName.Contains('\\'))
            {
                objStringBuffer.Append(EscapeDQUOTAndBackslash(strDisplayName));
            }
            else
            {
                objStringBuffer.Append(strDisplayName);
            }

            objStringBuffer.Append(TextParser::CHAR_DQUOT);
        }

        objStringBuffer.Append(TextParser::CHAR_SP);
    }

    if (bFormAQUOT)
        objStringBuffer.Append(TextParser::CHAR_LAQUOT);

    if (strScheme.GetLength() > 0)
    {
        objStringBuffer.Append(strScheme);
        objStringBuffer.Append(TextParser::CHAR_COLON);
    }

    if (strUserInfo.GetLength() > 0)
    {
        objStringBuffer.Append(strUserInfo);
        objStringBuffer.Append(TextParser::CHAR_AT);
    }

    IPAddress objHost;

    if (objHost.Parse(strHostInfo))
    {
        if (objHost.IsIPv6Address())
        {
            objStringBuffer.Append(TextParser::CHAR_LSBRACKET);
            objStringBuffer.Append(strHostInfo);
            objStringBuffer.Append(TextParser::CHAR_RSBRACKET);
        }
        else
        {
            objStringBuffer.Append(strHostInfo);
        }
    }
    else
    {
        objStringBuffer.Append(strHostInfo);
    }

    if (nPort != SIP::PORT_UNSPECIFIED)
    {
        AString strPort;

        strPort.SetNumber(nPort);

        objStringBuffer.Append(TextParser::CHAR_COLON);
        objStringBuffer.Append(strPort);
    }

    if (!objParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
        {
            const SIPParameter *pParameter = objParams.GetAt(i);

            if (pParameter == IMS_NULL)
                continue;

            objStringBuffer.Append(TextParser::CHAR_SEMICOLON);
            objStringBuffer.Append(pParameter->ToString());
        }
    }

    // If uri-header parameter exists, then inserts those fields
    if (!objHeaders.IsEmpty())
    {
        // hnv-unreserved = "[" / "]" / "/" / "?" / ":" / "+" / "$"
        // unreserved (mark) = "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
        // non-reserved characters = "-" / "_" / "." / "~"
        //const AString strHNV("[]/?:+$!*'()");
        //const AString strEscaped("\"<>");
        const AString strHNV("[]/?:+$-_.!~*'()");
        const ISIPHeader *piHeader = objHeaders.GetAt(0);

        objStringBuffer.Append(TextParser::CHAR_QUESTIONMARK);

        objStringBuffer.Append(piHeader->GetName());
        objStringBuffer.Append(TextParser::CHAR_EQUAL);
        objStringBuffer.Append(
                TextParser::DoPercentEncodingEx(piHeader->GetHeaderValue(), strHNV));

        for (IMS_UINT32 i = 1; i < objHeaders.GetSize(); ++i)
        {
            const ISIPHeader *piHeader = objHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            objStringBuffer.Append(TextParser::CHAR_AMPERSAND);
            objStringBuffer.Append(piHeader->GetName());
            objStringBuffer.Append(TextParser::CHAR_EQUAL);
            objStringBuffer.Append(
                    TextParser::DoPercentEncodingEx(piHeader->GetHeaderValue(), strHNV));
        }
    }

    if (bFormAQUOT)
        objStringBuffer.Append(TextParser::CHAR_RAQUOT);

    return static_cast<const AStringBuffer&>(objStringBuffer).GetString();
}

/*
 Returns the default list of SIPAddresses (empty list).

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
IMSList<SIPAddress*>&   Reference to the list of SIPAddresses (empty list)
</table>
*/
PUBLIC GLOBAL
const IMSList<SIPAddress*>& SIPAddress::ConstEmptyList()
{
    static const IMSList<SIPAddress*> CONST_EMPTY_LIST = IMSList<SIPAddress*>();

    //---------------------------------------------------------------------------------------------

    return CONST_EMPTY_LIST;
}

/*
 Returns the default SIPAddress (null).

Remarks

Parameters
<table>
parameter               description
----------              ----------
</table>

Returns
<table>
return                  description
----------              ----------
const SIPAddress&       Reference to SIPAddress (null)
</table>
*/
PUBLIC GLOBAL
const SIPAddress& SIPAddress::ConstNull()
{
    static const SIPAddress CONST_NULL;

    //---------------------------------------------------------------------------------------------

    return CONST_NULL;
}

/*
 Returns the tel URI format for the specified resource string.
This method assumes that strResource does not contain any URI scheme and
leading / trailing WSP.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strResource             Resource string to be checked
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SINT32              TEL_FORMAT_XXX (XXX := NONE / GLOBAL / LOCAL)
</table>
*/
PUBLIC GLOBAL
IMS_SINT32 SIPAddress::GetTelURIFormat(IN CONST AString &strResource)
{
    // global-number-digits := "+" *phonedigit DIGIT *phonedigit
    // local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
    // phonedigit := DIGIT / [visual-separator]
    // phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]

    //---------------------------------------------------------------------------------------------

    if (strResource.Equals('+'))
    {
        return TEL_FORMAT_NONE;
    }

    if (strResource.StartsWith('+'))
    {
        for (IMS_SINT32 i = 1; i < strResource.GetLength(); ++i)
        {
            const IMS_CHAR c = strResource[i];

            if (!IMS_ISDIGIT(c) && !IsVisualSeparator(c))
            {
                return TEL_FORMAT_NONE;
            }
        }

        return TEL_FORMAT_GLOBAL;
    }
    else
    {
        for (IMS_SINT32 i = 0; i < strResource.GetLength(); ++i)
        {
            const IMS_CHAR c = strResource[i];

            if (!IMS_ISDIGIT(c)
                    && !IsVisualSeparator(c)
                    && (c != '*')
                    && (c != '#')
                    && !((c >= 'A') && (c <= 'F')))
            {
                return TEL_FORMAT_NONE;
            }
        }

        return TEL_FORMAT_LOCAL;
    }
}

/*
 Compares if the specified SIP address with SIP URI format is equal or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
objAddress              Object for SIPAddress which is compared
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                If both SIP addresses match, this value will be returned
IMS_FALSE               If both SIP addresses do not match, this value will be returned
</table>
*/
PRIVATE
IMS_BOOL SIPAddress::CompareSIPURIs(IN CONST SIPAddress &objAddress) const
{
    static struct
    {
        const IMS_CHAR *pszName;
        IMS_SINT32 nSize;
    } SPECIAL_PARAMETER[] =
        {
            { PARAM_USER, 4 },
            { PARAM_TTL, 3 },
            { PARAM_METHOD, 6 },
            { PARAM_MADDR, 5 }
            /*
            Do not include "transport" uri-parameter as a special one.
            { PARAM_TRANSPORT, 9 }
            */
        };

    static const IMS_UINT32 MAX_SPECIAL_PARAMETER
            = sizeof(SPECIAL_PARAMETER)/sizeof(SPECIAL_PARAMETER[0]);

    //---------------------------------------------------------------------------------------------

    if (nPort != objAddress.nPort)
        return IMS_FALSE;

    AString strValue, strOtherValue;

    strValue = TextParser::DoPercentDecoding(strHostInfo);
    strOtherValue = TextParser::DoPercentDecoding(objAddress.strHostInfo);

    if (!strValue.EqualsIgnoreCase(strOtherValue))
        return IMS_FALSE;

    strValue = TextParser::DoPercentDecoding(strUserInfo);
    strOtherValue = TextParser::DoPercentDecoding(objAddress.strUserInfo);

    if (!strValue.Equals(strOtherValue))
        return IMS_FALSE;

    // "transport" uri-parameter comparison
    if (!CompareTransportParameters(objAddress))
    {
        IMS_TRACE_D("\"transport\" uri-parameter is not matched", 0, 0, 0);
        return IMS_FALSE;
    }

    // Check if any uri-parameter components are appearing in both URIs.
    // user/ttl/method/maddr[/transport]
    IMS_BOOL bParamPresent = IMS_FALSE;
    IMS_BOOL bOtherParamPresent = IMS_FALSE;
    AString strParamName;

    for (IMS_UINT32 i = 0; i < MAX_SPECIAL_PARAMETER; ++i)
    {
        strParamName.Attach(SPECIAL_PARAMETER[i].pszName, SPECIAL_PARAMETER[i].nSize);
        bParamPresent = IsParameterPresent(strParamName);
        bOtherParamPresent = objAddress.IsParameterPresent(strParamName);

        if (bParamPresent != bOtherParamPresent)
        {
            IMS_TRACE_E(0, "Parameter (%s) is not matched; " \
                    "One of them does not contain the parameter",
                    SPECIAL_PARAMETER[i].pszName, 0, 0);
            return IMS_FALSE;
        }
    }

    // Check if any uri-parameter components are equals if and only if it appears in both URIs.
    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParam = objParams.GetAt(i);
        const AString &strName = pParam->GetName();

        // Check for each uri-parameter fields
        for (IMS_UINT32 j = 0; j < objAddress.objParams.GetSize(); ++j)
        {
            const SIPParameter *pOtherParam = objAddress.objParams.GetAt(j);

            if (strName.EqualsIgnoreCase(pOtherParam->GetName()))
            {
                if (!pParam->Equals(pOtherParam))
                {
                    IMS_TRACE_E(0, "Parameter (%s) is not matched - %s : %s",
                            strName.GetStr(), pParam->GetValue().GetStr(),
                            pOtherParam->GetValue().GetStr());
                    return IMS_FALSE;
                }

                break;
            }
        }
    }

    // Check if the header components are appearing in both URIs.
    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        IMS_BOOL bHeaderFound = IMS_FALSE;
        const ISIPHeader *piHeader = objHeaders.GetAt(i);

        for (IMS_UINT32 j = 0; j < objAddress.objHeaders.GetSize(); ++j)
        {
            const ISIPHeader *piOtherHeader = objAddress.objHeaders.GetAt(j);

            if (piHeader->Equals(piOtherHeader))
            {
                bHeaderFound = IMS_TRUE;
                break;
            }
        }

        if (!bHeaderFound)
        {
            IMS_TRACE_E(0, "Header (%s, %s) is not found",
                    piHeader->GetName().GetStr(), piHeader->GetValue().GetStr(), 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*
 Compares if the specified SIP address with Tel URI format is equal or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
objAddress              Object for SIPAddress which is compared
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                If both TEL URIs match, this value will be returned
IMS_FALSE               If both TEL URIs do not match, this value will be returned
</table>
*/
PRIVATE
IMS_BOOL SIPAddress::CompareTelURIs(IN CONST SIPAddress &objAddress) const
{
    //---------------------------------------------------------------------------------------------

    // Checks if the local-number-digits or global-number-digits
    if (strHostInfo.StartsWith(TextParser::CHAR_PLUS)
            && !objAddress.strHostInfo.StartsWith(TextParser::CHAR_PLUS))
    {
        return IMS_FALSE;
    }

    if (!strHostInfo.StartsWith(TextParser::CHAR_PLUS)
            && objAddress.strHostInfo.StartsWith(TextParser::CHAR_PLUS))
    {
        return IMS_FALSE;
    }

    // Compares the number digits
    if (!CompareNumberDigits(strHostInfo, objAddress.strHostInfo))
        return IMS_FALSE;

    // Check the phone-context parameter
    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParam = objParams.GetAt(i);

        if (pParam->GetName().EqualsIgnoreCase(PARAM_PHONE_CONTEXT))
        {
            IMS_BOOL bFound = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < objAddress.objParams.GetSize(); ++j)
            {
                const SIPParameter *pOtherParam = objAddress.objParams.GetAt(i);

                if (pOtherParam->GetName().EqualsIgnoreCase(PARAM_PHONE_CONTEXT))
                {
                    const AString &strValue = pParam->GetValue();
                    const AString &strOtherValue = pOtherParam->GetValue();

                    if (strValue.StartsWith(TextParser::CHAR_PLUS)
                            && !strOtherValue.StartsWith(TextParser::CHAR_PLUS))
                    {
                        return IMS_FALSE;
                    }

                    if (!strValue.StartsWith(TextParser::CHAR_PLUS)
                            && strOtherValue.StartsWith(TextParser::CHAR_PLUS))
                    {
                        return IMS_FALSE;
                    }

                    if (strValue.StartsWith(TextParser::CHAR_PLUS)
                            && strOtherValue.StartsWith(TextParser::CHAR_PLUS))
                    {
                        // global-digit-number
                        if (!CompareNumberDigits(strValue, strOtherValue))
                            return IMS_FALSE;
                    }
                    else
                    {
                        // domainname
                        if (!strValue.EqualsIgnoreCase(strOtherValue))
                            return IMS_FALSE;
                    }

                    bFound = IMS_TRUE;
                    break;
                }
            }

            if (bFound == IMS_FALSE)
            {
                return IMS_FALSE;
            }

            break;
        }
    }

    // Check for each uri-parameter fields
    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        IMS_BOOL bParamFound = IMS_FALSE;
        const SIPParameter *pParam = objParams.GetAt(i);

        for (IMS_UINT32 j = 0; j < objAddress.objParams.GetSize(); ++j)
        {
            if (pParam->Equals(objAddress.objParams.GetAt(j)))
            {
                bParamFound = IMS_TRUE;
                break;
            }
        }

        if (!bParamFound)
        {
            IMS_TRACE_E(0, "Parameter (%s) is not matched", pParam->GetName().GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*
 Compares if "transport" uri-parameter is equal or not.
 This logic is based on RFC 3261, 19.1.4 URI Comparison clause,
 but it doesn't cover the below example case.

   sip:bob@biloxi.com     (can resolve to different port and transports)
   sip:bob@biloxi.com:6000;transport=tcp

Remarks

Parameters
<table>
parameter               description
----------              ----------
objAddress              Object for SIPAddress which is compared
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                If both transport parameters match, this value will be returned
IMS_FALSE               If both transport parameters do not match, this value will be returned
</table>
*/
PRIVATE
IMS_BOOL SIPAddress::CompareTransportParameters(IN CONST SIPAddress &objAddress) const
{
    const AString strParamName(PARAM_TRANSPORT);
    const SIPParameter *pParameter = GetParameter(strParamName);
    const SIPParameter *pOtherParameter = objAddress.GetParameter(strParamName);

    //---------------------------------------------------------------------------------------------

    if ((pParameter == IMS_NULL) && (pOtherParameter == IMS_NULL))
    {
        // No "transport" uri-parameter
        return IMS_TRUE;
    }

    const AString &strTransport = (pParameter != IMS_NULL) ?\
            pParameter->GetValue() : AString::ConstNull();
    const AString &strOtherTransport = (pOtherParameter != IMS_NULL) ?\
            pOtherParameter->GetValue() : AString::ConstNull();

    if ((strTransport.GetLength() == 0) && (strOtherTransport.GetLength() == 0))
    {
        // Exceptional case :: It MUST NOT be reached.
        return IMS_TRUE;
    }
    else if ((strTransport.GetLength() != 0) && (strOtherTransport.GetLength() != 0))
    {
        return strTransport.EqualsIgnoreCase(strOtherTransport);
    }

    /**
     * A URI omitting any component with a default value will not match a URI explicitly
     * containing that component with its default value.
     * For instance, a URI omitting the optional port component will not match a URI
     * explicitly declaring port 5060.
     * The same is true for the transport-parameter, ttl-parameter, user-parameter,
     * and method components.
     */
    const AString &strValue = (strTransport.GetLength() == 0) ? strOtherTransport : strTransport;

    if (IsSchemeSIP() && strValue.EqualsIgnoreCase(SIP::STR_UDP))
    {
        // One default(missing) & one explicitly contain
        return IMS_FALSE;
    }
    else if (IsSchemeSIPS() && strValue.EqualsIgnoreCase(SIP::STR_TCP))
    {
        // One default(missing) & one explicitly contain
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*
 Creates a user info. part from the specified user part.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strUserPart             User part string
</table>

Returns
<table>
return                  description
----------              ----------
UserInfoPart*           If user part is successfully parsed, returns the pointer of UserInfoPart.
                        Otherwise, returns IMS_NULL.
</table>
*/
PRIVATE
const SIPAddress::UserInfoPart* SIPAddress::CreateUserInfoPart(IN CONST AString &strUserPart) const
{
    //---------------------------------------------------------------------------------------------

    if (pUserInfoPart == IMS_NULL)
    {
        pUserInfoPart = new UserInfoPart();
    }

    if (pUserInfoPart != IMS_NULL)
    {
        if (!pUserInfoPart->Create(strUserPart))
        {
            return IMS_NULL;
        }
    }

    return pUserInfoPart;
}

/*
 Parses the fully qualified SIP address or the URI part of SIP address.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strAddress              Fully qualified SIP address or URI part
bParseParameter         Flag to indicate if the URI parameter needs to be parsed
bParseDisplayName       Flag to indicate if the display name needs to be parsed
bParseHeader            Flag to indicate if the header parameter needs to be parsed
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The SIP address is successfully parsed
IMS_FALSE               An error occurs during parsing
</table>
*/
PRIVATE
IMS_BOOL SIPAddress::Decode(IN CONST AString& strAddress, IN IMS_BOOL bParseParameter,
        IN IMS_BOOL bParseDisplayName /* = IMS_TRUE */, IN IMS_BOOL bParseHeader /* = IMS_TRUE */)
{
    AString strURI = strAddress.Trim();

    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    // Checks if the address is the special ("*") value
    if (strURI.Equals(TextParser::CHAR_ASTERISK))
    {
        bIsWildcard = IMS_TRUE;
        strHostInfo = strURI;
        return IMS_TRUE;
    }

    IMS_SINT32 nLAQUOT = TextParser::GetIndexOfDelimiter(strURI, TextParser::CHAR_LAQUOT);

    if (nLAQUOT != AString::NPOS)
    {
        // If LAQUOT is found, then find the display name if it is there
        if (bParseDisplayName == IMS_TRUE)
        {
            strDisplayName = strURI.GetSubStr(0, nLAQUOT);
            strDisplayName = strDisplayName.Trim();

            //4 Remove DQUOT and restore the escaped characters
            if (strDisplayName.StartsWith(TextParser::CHAR_DQUOT)
                    && strDisplayName.EndsWith(TextParser::CHAR_DQUOT))
            {
                strDisplayName = TextParser::TrimDQUOT(strDisplayName);

                // Restore the original characters if it is escaped for DQUOT and backslash
                strDisplayName.Replace("\\\"", "\"");
                strDisplayName.Replace("\\\\", "\\");

                bDQUOTForDisplayName = IMS_TRUE;
            }
        }

        strURI = strURI.GetSubStr(nLAQUOT + 1);

        IMS_SINT32 nRAQUOT = strURI.GetIndexOf(TextParser::CHAR_RAQUOT);

        if (nRAQUOT != AString::NPOS)
        {
            strURI.Truncate(nRAQUOT);
        }
    }

    SipAddrSpec *pAddrSpec = SIPStack::DecodeAddrSpec(strURI);

    if (pAddrSpec != IMS_NULL)
    {
        if (SIPStack::IsUriSchemeSIP(pAddrSpec) || SIPStack::IsUriSchemeSIPS(pAddrSpec))
        {
            if (SIPStack::IsUriSchemeSIP(pAddrSpec))
                strScheme = SIP::STR_SIP;
            else
                strScheme = SIP::STR_SIPS;

            // user-info (user:password)
            strUserInfo = SIPStack::AddrSpec_GetUser(pAddrSpec);

            const IMS_CHAR* pszPassword = SIPStack::AddrSpec_GetPassword(pAddrSpec);

            if (pszPassword != IMS_NULL)
            {
                strUserInfo += TextParser::CHAR_COLON;
                strUserInfo += pszPassword;
            }

            strHostInfo = SIPStack::AddrSpec_GetHost(pAddrSpec);

            IPAddress objHost;

            if (objHost.Parse(strHostInfo))
            {
                if (objHost.IsIPv6Address())
                {
                    strHostInfo = objHost.ToString();
                }
            }

            IMS_SINT32 nTmpPort = SIPStack::AddrSpec_GetPort(pAddrSpec);

            if (nTmpPort != 0)
                nPort = nTmpPort;
            else
                nPort = SIP::PORT_UNSPECIFIED;

            if (bParseHeader)
            {
                RemoveAllHeaderComponents();

                if (!SIPStack::DecodeHeaderComponent(pAddrSpec, objHeaders))
                {
                    SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
                    SIPStack::FreeAddrSpec(pAddrSpec);
                    return IMS_FALSE;
                }
            }

            // Process URI parameters
            if (bParseParameter)
            {
                objParams = SIPStack::ExtractParameters(pAddrSpec);
            }
        }
        else
        {
            // Extract the URI scheme
            IMS_SINT32 nIndex = strURI.GetIndexOf(TextParser::CHAR_COLON);

            if (nIndex == AString::NPOS)
            {
                IMS_TRACE_D("SIPAddress (%s) does not have a scheme",
                        SIPDebug::GetUri1(strAddress).GetStr(), 0, 0);
            }
            else
            {
                strScheme = strURI.GetSubStr(0, nIndex);
            }

            IMS_SINT32 nParamIndex = strURI.GetIndexOf(TextParser::CHAR_SEMICOLON, nIndex + 1);
            IMS_SINT32 nHeaderIndex = strURI.GetIndexOf(TextParser::CHAR_QUESTIONMARK, nIndex + 1);

            if (nHeaderIndex != AString::NPOS)
            {
                if (bParseHeader)
                {
                    AString strHeaders = strURI.GetSubStr(nHeaderIndex + 1);

                    RemoveAllHeaderComponents();

                    if (!DecodeHeaderComponent(strHeaders))
                    {
                        IMS_TRACE_E(0, "Parsing URI header parameter (%s) failed",
                                strHeaders.GetStr(), 0, 0);
                    }
                }

                strURI = strURI.GetSubStr(0, nHeaderIndex);

                if (nParamIndex > nHeaderIndex)
                {
                    // No uri-parameters
                    nParamIndex = AString::NPOS;
                }
            }

            strHostInfo = strURI.GetSubStr(nIndex + 1, nParamIndex - (nIndex + 1));
            strHostInfo = strHostInfo.Trim();

            // Extract the uri-parameters if present
            // Process URI parameters
            if (bParseParameter && (nParamIndex != AString::NPOS))
            {
                AString strParams = strURI.GetSubStr(nParamIndex + 1);

                // Parsing the uri-parameters
                IMSList<AString> objTokens = strParams.Split(TextParser::CHAR_SEMICOLON);

                for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
                {
                    SIPParameter *pParameter = new SIPParameter();

                    if (pParameter == IMS_NULL)
                        continue;

                    const AString &strToken = objTokens.GetAt(i);

                    if (!pParameter->Create(strToken))
                    {
                        delete pParameter;

                        IMS_TRACE_E(0, "Parsing SIP Parameter (%s) failed",
                                strToken.GetStr(), 0, 0);
                        continue;
                    }

                    if (!objParams.Append(pParameter))
                    {
                        delete pParameter;
                        IMS_TRACE_E(0, "Adding SIP Parameter (%s) failed",
                                strToken.GetStr(), 0, 0);
                    }
                }
            }
        }

        SIPStack::FreeAddrSpec(pAddrSpec);
    }
    else
    {
        SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*
 Decodes the URI header components.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strHeaders              The header components
                        headers := header *("&" header)
                        header := hname "=" hvalue
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The header components is successfully parsed
IMS_FALSE               An error occurs during parsing
</table>
*/
PRIVATE
IMS_BOOL SIPAddress::DecodeHeaderComponent(IN CONST AString &strHeaders)
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::DecodeHeaderComponent(strHeaders, objHeaders);
}

/*
 Checks if the specified parameter exists or not among the parameters.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 Parameter name to be checked
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                It returns this value if SIP address contains the parameter
IMS_FALSE               It returns this value if SIP address does not contain the parameter
</table>
*/
PRIVATE
IMS_BOOL SIPAddress::IsParameterPresent(IN CONST AString &strName) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter == IMS_NULL)
            continue;

        if (strName.EqualsIgnoreCase(pParameter->GetName()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*
 Compares if the specified two number digits equals or not. It is only for TEL URI format.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strDigits1              Digits to be compared
strDigits2              Digits to be compared
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The two digits are equal
IMS_FALSE               The two digits are not equal
</table>
*/
PRIVATE GLOBAL
IMS_BOOL SIPAddress::CompareNumberDigits(IN CONST AString &strDigits1,
        IN CONST AString &strDigits2)
{
    AString strTmpDigits1;
    AString strTmpDigits2;

    //---------------------------------------------------------------------------------------------

    for (IMS_SINT32 i = 0; i < strDigits1.GetLength(); ++i)
    {
        IMS_CHAR cDigit = strDigits1[i];

        if (IsVisualSeparator(cDigit))
            continue;

        strTmpDigits1.Append(cDigit);
    }

    for (IMS_SINT32 i = 0; i < strDigits2.GetLength(); ++i)
    {
        IMS_CHAR cDigit = strDigits2[i];

        if (IsVisualSeparator(cDigit))
            continue;

        strTmpDigits2.Append(cDigit);
    }

    return strTmpDigits1.EqualsIgnoreCase(strTmpDigits2);
}

/*
 Escape the special characters (DQUOT / Backslash) if it is required.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strValue                String to be escaped
</table>

Returns
<table>
return                  description
----------              ----------
AString                 Escaped string
</table>
*/
PRIVATE GLOBAL
AString SIPAddress::EscapeDQUOTAndBackslash(IN CONST AString &strValue)
{
    AString strEscapedValue = strValue;

    //---------------------------------------------------------------------------------------------

    //strEscapedValue.Replace("\\\"", "\"");
    //strEscapedValue.Replace("\\\\", "\\");
    strEscapedValue.Replace("\\", "\\\\");
    strEscapedValue.Replace("\"", "\\\"");

    return strEscapedValue;
}

/*
 Checks if the specified display name is composed of token (w/ LWS) or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strDisplayName          String to be tested
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The display name is composed of the token characters
IMS_FALSE               The display name is not composed of the token characters
</table>
*/
PRIVATE GLOBAL
IMS_BOOL SIPAddress::IsDisplayNameToken(IN CONST AString &strDisplayName)
{
    IMS_SINT32 nIndex = 0;

    //---------------------------------------------------------------------------------------------

    while (nIndex < strDisplayName.GetLength())
    {
        const IMS_CHAR ch = strDisplayName[nIndex];

        // token / HTAB / LF / CR / SP
        if (!IsToken(ch)
            && (ch != 0x09) && (ch != 0x0A) && (ch != 0x0D) && (ch != 0x20))
        {
            return IMS_FALSE;
        }

        ++nIndex;
    }

    return IMS_TRUE;
}

/*
 Checks if the specified character is a token character or not.

Remarks

Parameters
<table>
parameter               description
----------              ----------
ch                      Character to be tested
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The character is a token character
IMS_FALSE               The character is not a token character
</table>
*/
PRIVATE GLOBAL
IMS_BOOL SIPAddress::IsToken(IN CONST IMS_CHAR ch)
{
    // alphanum / "-" / "." / "!" / "%" / "*" / "_" / "+" / "'" / "`" / "~"

    //---------------------------------------------------------------------------------------------

    if (IMS_ISDIGIT(ch) || IMS_ISALPHA(ch)
            || (ch == '-') || (ch == '.') || (ch == '!') || (ch == '%') || (ch == '*')
            || (ch == '_') || (ch == '+') || (ch == '\'') || (ch == '`') || (ch == '~'))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*
 Checks if the specified character is a visual separator or not. It is only for TEL URI format.

Remarks

Parameters
<table>
parameter               description
----------              ----------
ch                      Character to be tested
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The character is a visual separator
IMS_FALSE               The character is not a visual separator
</table>
*/
PRIVATE GLOBAL
IMS_BOOL SIPAddress::IsVisualSeparator(IN CONST IMS_CHAR ch)
{
    // "-", ".", "(", ")"

    //---------------------------------------------------------------------------------------------

    if ((ch == '-') || (ch == '.') || (ch == '(') || (ch == ')'))
        return IMS_TRUE;

    return IMS_FALSE;
}
