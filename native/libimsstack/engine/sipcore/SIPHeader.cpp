/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description
     This class provides generic SIP header parser helper. It can be used to parse base string
    header values that are read from SIP message using e.g. SIPConnection::GetHeader() method.
    It uses generic format to parse the header value and parameters following the syntax given
    in RFC 3261.
    - field-name: field-value *(;parameter-name=parameter-value)
    - auth-header-name: auth-scheme LWS auth-param *(COMMA auth-param)
*/

#include "ServiceMemory.h"
#include "AStringBuffer.h"
#include "SIPPrivate.h"
#include "SIPHeaderName.h"
#include "SIPAddress.h"
#include "SIPParameter.h"
#include "SIPHeader.h"



// Constant Header Names
PUBLIC GLOBAL
const IMS_CHAR* SIPHeader::NAME[] =
{
    SIPHeaderName::ALLOW,
    SIPHeaderName::ALLOW_EVENTS,
    SIPHeaderName::AUTHORIZATION,
    SIPHeaderName::CALL_ID,
    SIPHeaderName::CONTACT,
    SIPHeaderName::CONTACT,
    SIPHeaderName::CONTACT,
    SIPHeaderName::CONTENT_DISPOSITION,
    SIPHeaderName::CONTENT_ENCODING,
    SIPHeaderName::CONTENT_LENGTH,
    SIPHeaderName::CONTENT_TYPE,
    SIPHeaderName::CSEQ,
    SIPHeaderName::EVENT,
    SIPHeaderName::EXPIRES,
    SIPHeaderName::EXPIRES,
    SIPHeaderName::EXPIRES,
    SIPHeaderName::ACCEPT,
    SIPHeaderName::MIN_EXPIRES,
    SIPHeaderName::FROM,
    SIPHeaderName::MAX_FORWARDS,
    SIPHeaderName::MIME_VERSION,
    SIPHeaderName::PRIVACY,
    SIPHeaderName::P_PREFERRED_IDENTITY,
    SIPHeaderName::P_ASSERTED_IDENTITY,
    SIPHeaderName::MIN_SE,
    SIPHeaderName::PATH,
    SIPHeaderName::P_ASSOCIATED_URI,
    SIPHeaderName::P_CALLED_PARTY_ID,
    SIPHeaderName::P_VISITED_NETWORK_ID,
    SIPHeaderName::P_CHARGING_FUNCTION_ADDRESSES,
    SIPHeaderName::P_ACCESS_NETWORK_INFO,
    SIPHeaderName::P_CHARGING_VECTOR,
    SIPHeaderName::SERVICE_ROUTE,
    SIPHeaderName::HISTORY_INFO,
    SIPHeaderName::REQUEST_DISPOSITION,
    SIPHeaderName::ACCEPT_CONTACT,
    SIPHeaderName::REJECT_CONTACT,
    SIPHeaderName::JOIN,
    SIPHeaderName::SIP_IF_MATCH,
    SIPHeaderName::SIP_ETAG,
    SIPHeaderName::PROXY_AUTHENTICATE,
    SIPHeaderName::PROXY_AUTHORIZATION,
    SIPHeaderName::RACK,
    SIPHeaderName::RECORD_ROUTE,
    SIPHeaderName::REFERRED_BY,
    SIPHeaderName::REFER_TO,
    SIPHeaderName::REPLACES,
    SIPHeaderName::REQUIRE,
    SIPHeaderName::ROUTE,
    SIPHeaderName::RSEQ,
    SIPHeaderName::SECURITY_CLIENT,
    SIPHeaderName::SECURITY_VERIFY,
    SIPHeaderName::SECURITY_SERVER,
    SIPHeaderName::SESSION_EXPIRES,
    SIPHeaderName::SUBSCRIPTION_STATE,
    SIPHeaderName::SUPPORTED,
    SIPHeaderName::TIMESTAMP,
    SIPHeaderName::TO,
    SIPHeaderName::UNSUPPORTED,
    SIPHeaderName::VIA,
    SIPHeaderName::WARNING,
    SIPHeaderName::WWW_AUTHENTICATE,
    IMS_NULL,
    SIPHeaderName::RETRY_AFTER,
    SIPHeaderName::RETRY_AFTER,
    SIPHeaderName::RETRY_AFTER,
    SIPHeaderName::P_EARLY_MEDIA,
    SIPHeaderName::RESOURCE_PRIORITY,
    SIPHeaderName::ACCEPT_RESOURCE_PRIORITY,
    SIPHeaderName::DATE,
    IMS_NULL
};

/*
 Constructs a new SIPHeader without any values.

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
SIPHeader::SIPHeader()
    : nType(ISIPHeader::ANY)
    , strName(AString::ConstNull())
    , strBody(AString::ConstNull())
    , pAddress(IMS_NULL)
{
}

/*
 Constructs a new SIPHeaader with the type only.

Remarks

Parameters
<table>
parameter               description
----------              ----------
nType_                  The header type
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPHeader::SIPHeader(IN IMS_SINT32 nType_)
    : nType(nType_)
    , strName(SIPStack::GetHeaderNameFromType(nType_))
    , strBody(AString::ConstNull())
    , pAddress(IMS_NULL)
{
}

/*
 Constructs a new SIPHeaader with the name only.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName_                The header name
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPHeader::SIPHeader(IN CONST AString &strName_)
    : nType(ISIPHeader::ANY)
    , strName(strName_)
    , strBody(AString::ConstNull())
    , pAddress(IMS_NULL)
{
    //---------------------------------------------------------------------------------------------

    nType = SIPStack::GetHeaderTypeFromName(strName);
}

/*
 Constructs a new SIPHeader from the pointer to SipHeaderBase structure.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pstHeader               Pointer to SipHeaderBase (which is defined by SIP stack)
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC
SIPHeader::SIPHeader(IN CONST SipHeaderBase *pstHeader)
    : nType(ISIPHeader::ANY)
    , strName(AString::ConstNull())
    , strBody(AString::ConstNull())
    , pAddress(IMS_NULL)
{
    //---------------------------------------------------------------------------------------------

    nType = SIPStack::GetHeaderType(pstHeader);

    if (nType == ISIPHeader::UNKNOWN)
    {
        strName = SIPStack::GetUnknownHeaderName(const_cast<SipHeaderBase*>(pstHeader));
    }
    else
    {
        strName = SIPStack::GetHeaderNameFromType(nType);
    }

    SIPStack::EncodeHeaderBody(pstHeader, IMS_FALSE, strBody);

    // If the header type is unknown, decode the body according to the general syntax rule...
    if (nType == ISIPHeader::UNKNOWN)
    {
        ParseUnknownBody(strBody);
    }

    if (SIPStack::IsAddressFormatHeader(nType, strName))
    {
        pAddress = new SIPAddress(strBody);
    }

    // P-Preferred-Identity & P-Asserted-Identity
    //    : name-addr / addr-spec -> no header parameters
    if ((nType != ISIPHeader::P_PREFERRED_IDENTITY)
            && (nType != ISIPHeader::P_ASSERTED_IDENTITY)
            && (nType != ISIPHeader::UNKNOWN))
    {
        objParams = SIPStack::ExtractParameters(const_cast<SipHeaderBase*>(pstHeader));
    }
}

/*
 Destructs a SIPHeader instance.

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
PUBLIC VIRTUAL
SIPHeader::~SIPHeader()
{
    //---------------------------------------------------------------------------------------------

    if (pAddress != IMS_NULL)
        delete pAddress;

    if (!objParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
        {
            SIPParameter *pParameter = objParams.GetAt(i);

            if (pParameter != IMS_NULL)
                delete pParameter;
        }

        objParams.Clear();
    }
}

/*
 Destroys the SIPHeader instance.

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
PUBLIC VIRTUAL
void SIPHeader::Destroy()
{
    //---------------------------------------------------------------------------------------------

    delete this;
}

/*
 Clones and returns a SIPHeader.

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
new SIPHeader           Pointer to a new SIPHeader
</table>
*/
PUBLIC VIRTUAL
ISIPHeader* SIPHeader::Clone() const
{
    SIPHeader *pHeader = new SIPHeader(nType);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    pHeader->strName = strName;
    pHeader->strBody = strBody;

    if (pAddress != IMS_NULL)
    {
        pHeader->pAddress = new SIPAddress(*pAddress);
    }

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);
        SIPParameter *pNewParameter = new SIPParameter(*pParameter);

        if (pNewParameter != IMS_NULL)
            pHeader->objParams.Append(pNewParameter);
    }

    return pHeader;
}

/*
 Checks if the given SIPHeader is the same.

Remarks

Parameters
<table>
parameter               description
----------              ----------
piHeader                Pointer to ISIPHeader which is compared
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                If both SIP headers matched, this value returns
IMS_FALSE               If both SIP headers are not matched, this value returns
</table>
*/
PUBLIC VIRTUAL
IMS_BOOL SIPHeader::Equals(IN CONST ISIPHeader *piHeader) const
{
    const SIPHeader *pHeader = DYNAMIC_CAST(const SIPHeader*, piHeader);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
        return IMS_FALSE;

    if (nType != pHeader->nType)
        return IMS_FALSE;

    if ((nType == ISIPHeader::UNKNOWN)
            && (pHeader->nType == ISIPHeader::UNKNOWN))
    {
        const IMS_CHAR *pszFName = SIPStack::GetHeaderName(nType, strName);
        const IMS_CHAR *pszOtherFName = SIPStack::GetHeaderName(pHeader->nType, pHeader->strName);

        if (AString::CompareIgnoreCase(pszFName, pszOtherFName) != 0)
            return IMS_FALSE;
    }

    // Compare the header value field
    if (SIPStack::IsAddressFormatHeader(nType, strName))
    {
        SIPAddress objAddress(strBody);
        SIPAddress objOtherAddress(pHeader->strBody);

        if (!objAddress.Equals(objOtherAddress))
            return IMS_FALSE;
    }
    else
    {
        if (!strBody.EqualsIgnoreCase(pHeader->strBody))
            return IMS_FALSE;
    }

    // Compare the header parameters
    if (objParams.GetSize() != pHeader->objParams.GetSize())
        return IMS_FALSE;

    // TODO:: comparison of parameter fields

    return IMS_TRUE;
}

/*
 Returns the pointer to SIPAddress if this header is a format of SIP address.

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
SIPAddress*             If the header is a format of SIP address, it returns this value
NULL pointer            If the header is not a format of SIP address, it returns NULL pointer
</table>
*/
PUBLIC VIRTUAL
const SIPAddress* SIPHeader::GetSIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    return pAddress;
}

/*
 Returns the full header value including the header parameters.
For example,
    Bruce <sip:bruce@ims.com>;tag=IMS_1928301774

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
Non-NULL                The full header value including the header parameters
Empty or NULL           If the header has only a header name
</table>
*/
PUBLIC VIRTUAL
AString SIPHeader::GetHeaderValue() const
{
    //---------------------------------------------------------------------------------------------

    if (objParams.IsEmpty())
        return strBody;

    AString strHeaderValue(strBody);

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            strHeaderValue += TextParser::CHAR_SEMICOLON + pParameter->ToString();
        }
    }

    return strHeaderValue;
}

/*
 Returns the header name of this SIPHeader.

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
String                  The header name of this SIPHeader
</table>
*/
PUBLIC VIRTUAL
const AString& SIPHeader::GetName() const
{
    //---------------------------------------------------------------------------------------------

    return strName;
}

/*
 Returns the pointer to SIPParameter of one header parameter.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 The parameter name to be returned
</table>

Returns
<table>
return                  description
----------              ----------
SIPParameter*           Parameter to the specified parameter name
</table>
*/
PUBLIC VIRTUAL
const SIPParameter* SIPHeader::GetParameter(IN CONST AString &strName) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
            return pParameter;
    }

    return IMS_NULL;
}

/*
 Returns the names of all the header parameters. If there are no header parameters,
the method returns an empty list.

Remarks

Parameters
<table>
parameter               description
----------              ----------
objPNames               Names of the header parameters
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The operation is successfully done
IMS_FAILURE             An error occurs
</table>
*/
PUBLIC VIRTUAL
IMS_RESULT SIPHeader::GetParameterNames(OUT IMSList<AString> &objPNames) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (!objPNames.Append(pParameter->GetName()))
        {
            SIPPrivate::SetLastError(SIPError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_SUCCESS;
}

/*
 Returns the list of all the header parameters in this SIPHeader.

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
List of SIPParameter    The list of SIPParameter for this SIPHeader
</table>
*/
PUBLIC VIRTUAL
const IMSList<SIPParameter*>& SIPHeader::GetParameters() const
{
    //---------------------------------------------------------------------------------------------

    return objParams;
}

/*
 Returns the enumeration type of this SIPHeader which corresponds to the header name.

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
Type of header          Enumeration type of this SIPHeader
</table>
*/
PUBLIC VIRTUAL
IMS_SINT32 SIPHeader::GetType() const
{
    //---------------------------------------------------------------------------------------------

    return nType;
}

/*
 Returns the header value without header parameters. For example,
for header <sip:user@192.168.200.201>;expires=3600,
this method returns <sip:user@192.168.200.201>.
In the case of an authorization or authentication header,
it returns only the authentication scheme e.g. "Digest".

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
Header body             The header value without header parameters
                        It is an empty string if the value was set to be NULL or empty string
</table>
*/
PUBLIC VIRTUAL
const AString& SIPHeader::GetValue() const
{
    //---------------------------------------------------------------------------------------------

    return strBody;
}

/*
 Returns the header value without header parameters. It is the same method with GetValue()
except for returning an integer value for the header value.

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
Header body             The header value without header parameters
                        It is -1 if the value cannot have an integer value
</table>
*/
PUBLIC VIRTUAL
IMS_SINT32 SIPHeader::GetValueInt() const
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case ISIPHeader::CONTENT_LENGTH:
    case ISIPHeader::EXPIRES_SEC:
    case ISIPHeader::EXPIRES_ANY:
    case ISIPHeader::MIN_EXPIRES:
    case ISIPHeader::MAX_FORWARDS:
    case ISIPHeader::MIN_SE:
    case ISIPHeader::RETRY_AFTER_SEC:
    case ISIPHeader::RETRY_AFTER_ANY:
    case ISIPHeader::RSEQ:
    case ISIPHeader::SESSION_EXPIRES:
    {
        IMS_BOOL bOK = IMS_FALSE;
        IMS_SINT32 nValue = strBody.ToInt32(&bOK);

        if (!bOK)
        {
            break;
        }

        return nValue;
    }
    default:
        break;
    }

    return (-1);
}

/*
 Removes the header parameter if it is found in this SIPHeader.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 Name of the header parameter
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC VIRTUAL
void SIPHeader::RemoveParameter(IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (pParameter->GetName().Equals(strName))
            {
                objParams.RemoveAt(i);
                delete pParameter;
                return;
            }
        }
    }
}

/*
 Sets the header name, for example "Contact". If the argument has a leading
and trailing white spaces, those are ignored.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 Name of the header
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
PUBLIC VIRTUAL
void SIPHeader::SetName(IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    this->strName = strName.Trim();

    nType = SIPStack::GetHeaderTypeFromName(this->strName);
}

/*
 Sets the value of header parameter. If the parameter does not exist, it will be added.
For example, for header value "<sip:user@192.168.200.201>" calling
SetParameter("expires", "3600") will construct header value
"<sip:user@192.168.200.201>;expires=3600".
If the value is NULL or empty string, the parameter is interpreted
as a parameter without value.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strName                 Name of the header parameter
strValue                Value of the header parameter (NULL or empty string is allowed)
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The header parameter is successfully set
IMS_FAILURE             An error occurs
</table>
*/
PUBLIC VIRTUAL
IMS_RESULT SIPHeader::SetParameter(IN CONST AString &strName, IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            return pParameter->SetValue(strValue);
        }
    }

    // Now, no existing parameter in the parameter list

    SIPParameter *pParameter = new SIPParameter(strName, strValue);

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
 Sets the header value as string with any header parameters.
If the header value is an empty string or NULL, it means a header with no value.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strHeaderValue          The full header value
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The full header value is successfully set
IMS_FAILURE             An error occurs
</table>
*/
PUBLIC VIRTUAL
IMS_RESULT SIPHeader::SetHeaderValue(IN CONST AString &strHeaderValue)
{
    //---------------------------------------------------------------------------------------------

    if (!Decode(strHeaderValue))
        return IMS_FAILURE;

    return IMS_SUCCESS;
}

/*
 Sets the header value as string without parameters. For example,
for the header "<sip:user@192.168.200.201>", the existing (if any) header parameter values
are not modified.
If the header value is an empty string or NULL, it means a header with no value.

For an authorization and authentication header, this method sets the authentication scheme
e.g. "Digest".

Remarks

Parameters
<table>
parameter               description
----------              ----------
strValue                The value of the header
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The header value is successfully set
IMS_FAILURE             An error occurs
</table>
*/
PUBLIC VIRTUAL
IMS_RESULT SIPHeader::SetValue(IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (!Decode(strValue, IMS_FALSE))
        return IMS_FAILURE;

    return IMS_SUCCESS;
}

/*
 Sets the header value as string without parameters. It is the same method with SetValue(...)
except for setting an integer value for the header value.
If the header value cannot have an integer value, it fails.

Remarks

Parameters
<table>
parameter               description
----------              ----------
nValue                  The value of the header
</table>

Returns
<table>
return                  description
----------              ----------
IMS_SUCCESS             The header value is successfully set
IMS_FAILURE             An error occurs
</table>
*/
PUBLIC VIRTUAL
IMS_RESULT SIPHeader::SetValueInt(IN IMS_SINT32 nValue)
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case ISIPHeader::CONTENT_LENGTH:
    case ISIPHeader::EXPIRES_SEC:
    case ISIPHeader::EXPIRES_ANY:
    case ISIPHeader::MIN_EXPIRES:
    case ISIPHeader::MAX_FORWARDS:
    case ISIPHeader::MIN_SE:
    case ISIPHeader::RSEQ:
    case ISIPHeader::SESSION_EXPIRES:
    {
        if (nValue >= 0)
            strBody.SetNumber(nValue);

        return IMS_SUCCESS;
    }
    default:
        break;
    }

    return IMS_FAILURE;
}

/*
 Returns the string representation of the header according to header type.
For example,
    - From: Bruce < sip:bruce@ims.com>;tag=192168200201
    - WWW-Authenticate: Digest realm="ims.com", domain="sip:ims.com", auth="auth",
         nonce="f84f1cec41e6cbe5aea9c8e88d359", opaque ="", stale=FALSE, algorithm=MD5

 The value part of the header may be missing if the header was created with empty string
or NULL as strValue and has not been set using SetValue().

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
String                  String representation of this SIPHeader
</table>
*/
PUBLIC VIRTUAL
AString SIPHeader::ToString() const
{
    AString strHeader;

    //---------------------------------------------------------------------------------------------

    strHeader.Append(strName);
    strHeader.Append(TextParser::CHAR_COLON);
    strHeader.Append(TextParser::CHAR_SP);
    strHeader.Append(ToStringWithoutName());

    return strHeader;
}

/*
 Returns the string representation of the header according to header type without header name.
For example,
    - From: Bruce < sip:bruce@ims.com>;tag=192168200201
       => Bruce < sip:bruce@ims.com>;tag=192168200201
    - WWW-Authenticate: Digest realm="ims.com", domain="sip:ims.com", auth="auth",
        nonce="f84f1cec41e6cbe5aea9c8e88d359", opaque ="", stale=FALSE, algorithm=MD5
       => Digest realm="ims.com", domain="sip:ims.com", auth="auth",
        nonce="f84f1cec41e6cbe5aea9c8e88d359", opaque ="", stale=FALSE, algorithm=MD5

 The value part of the header may be missing if the header was created with empty string
or NULL as strValue and has not been set using SetValue().

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
String                  String representation of this SIPHeader without header name
</table>
*/
PUBLIC VIRTUAL
AString SIPHeader::ToStringWithoutName() const
{
    AStringBuffer objHeaderValue(512);

    //---------------------------------------------------------------------------------------------

    if (pAddress != IMS_NULL)
    {
        objHeaderValue.Append(pAddress->ToString());

        if (!objParams.IsEmpty())
        {
            const AString &strTemp
                    = static_cast<const AStringBuffer&>(objHeaderValue).GetString();

            if (strTemp.Contains('<') && strTemp.Contains('>'))
            {
                // no-op
            }
            else
            {
                objHeaderValue.Prepend('<');
                objHeaderValue.Append('>');
            }
        }
    }
    else
    {
        objHeaderValue.Append(strBody);
    }

    for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            objHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            // Do not add the white space...
            //strHeader.Append(TextParser::CHAR_SP);
            objHeaderValue.Append(pParameter->ToString());
        }
    }

    if (nType == PRIVACY)
    {
        // Remove the semi-colon; for the microsip stack consistency
        objHeaderValue.Erase(0, 1);
    }

    return static_cast<const AStringBuffer&>(objHeaderValue).GetString();
}

/*
 Checks if the header type is valid.

Remarks

Parameters
<table>
parameter               description
----------              ----------
nType                   The header type
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The header type is valid
IMS_FALSE               The header type is not valid
</table>
*/
PUBLIC GLOBAL
IMS_BOOL SIPHeader::IsValidType(IN IMS_SINT32 nType)
{
    //---------------------------------------------------------------------------------------------

    if ((nType > ISIPHeader::INVALID) && (nType < ISIPHeader::ANY))
        return IMS_TRUE;

    return IMS_FALSE;
}

/*
 Parses the header body field.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strBody_                The header body of SIP header
bParseParameter         Flag to indicate if the header parameter needs to be parsed
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The SIP header is successfully parsed
IMS_FALSE               An error occurs during parsing
</table>
*/
PRIVATE
IMS_BOOL SIPHeader::Decode(IN CONST AString &strBody_,
        IN IMS_BOOL bParseParameter /* = IMS_TRUE */)
{
    AString strTrimmedBody = strBody_.Trim();

    //---------------------------------------------------------------------------------------------

    if (strTrimmedBody.IsEmpty() || strTrimmedBody.IsNULL())
    {
        strBody = strTrimmedBody;

        SIPPrivate::SetLastError(SIPError::NO_ERROR);
        return IMS_TRUE;
    }

    SipHeaderBase *pstHeader = SIPStack::DecodeHeader(nType, strName, strTrimmedBody);

    if (pstHeader == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::PARSING_ERROR);
        return IMS_FALSE;
    }

    // Process the header type which has an ANY type: Contact, Expires, Retry-After
    nType = SIPStack::GetHeaderType(pstHeader);

    SIPStack::EncodeHeaderBody(pstHeader, IMS_FALSE, strBody);

    // If the header type is unknown, decode the body according to the general syntax rule...
    if (nType == ISIPHeader::UNKNOWN)
    {
        ParseUnknownBody(strBody);
    }

    if (pAddress != IMS_NULL)
    {
        delete pAddress;
        pAddress = IMS_NULL;
    }

    if (SIPStack::IsAddressFormatHeader(nType, strName))
    {
        pAddress = new SIPAddress(strBody);

        if (SIPStack::IsAQUOTRequiredForAddressFormat(nType, strName))
        {
            pAddress->SetAQUOTRequired(IMS_TRUE);
        }
    }

    // P-Preferred-Identity & P-Asserted-Identity
    //    : name-addr / addr-spec -> no header parameters
    if ((bParseParameter == IMS_TRUE)
            && (nType != ISIPHeader::P_PREFERRED_IDENTITY)
            && (nType != ISIPHeader::P_ASSERTED_IDENTITY)
            && (nType != ISIPHeader::UNKNOWN))
    {
        objParams = SIPStack::ExtractParameters(pstHeader);
    }

    SIPStack::FreeHeaderEx(pstHeader);

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return IMS_TRUE;
}

/*
 Parses the unknown header body field.

Remarks

Parameters
<table>
parameter               description
----------              ----------
strBody_                The header body of SIP header
</table>

Returns
<table>
return                  description
----------              ----------
IMS_TRUE                The SIP header is successfully parsed
IMS_FALSE               An error occurs during parsing
</table>
*/
PRIVATE
IMS_BOOL SIPHeader::ParseUnknownBody(IN CONST AString &strBody_)
{
    // Find ';'
    IMS_SINT32 nSemiColon = TextParser::GetIndexOfDelimiter(strBody_, TextParser::CHAR_SEMICOLON);

    //---------------------------------------------------------------------------------------------

    // No parameters
    if (nSemiColon == AString::NPOS)
    {
        strBody = strBody_;
        return IMS_TRUE;
    }

    objParams = SIPStack::ExtractParameters(
                    strBody_.GetSubStr(nSemiColon+1), TextParser::CHAR_SEMICOLON);

    strBody = strBody_.GetSubStr(0, nSemiColon).Trim();

    return IMS_TRUE;
}
