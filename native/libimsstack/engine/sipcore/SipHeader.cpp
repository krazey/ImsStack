/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceMemory.h"
#include "AStringBuffer.h"

#include "SipAddress.h"
#include "SipHeader.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipPrivate.h"
#include "SipStack.h"

// clang-format off
// Constant Header Names
PUBLIC GLOBAL const IMS_CHAR* SipHeader::NAME[] = {
        SipHeaderName::ALLOW,
        SipHeaderName::ALLOW_EVENTS,
        SipHeaderName::AUTHORIZATION,
        SipHeaderName::CALL_ID,
        SipHeaderName::CONTACT,
        SipHeaderName::CONTACT,
        SipHeaderName::CONTACT,
        SipHeaderName::CONTENT_DISPOSITION,
        SipHeaderName::CONTENT_ENCODING,
        SipHeaderName::CONTENT_LENGTH,
        SipHeaderName::CONTENT_TYPE,
        SipHeaderName::CSEQ,
        SipHeaderName::EVENT,
        SipHeaderName::EXPIRES,
        SipHeaderName::EXPIRES,
        SipHeaderName::EXPIRES,
        SipHeaderName::ACCEPT,
        SipHeaderName::MIN_EXPIRES,
        SipHeaderName::FROM,
        SipHeaderName::MAX_FORWARDS,
        SipHeaderName::MIME_VERSION,
        SipHeaderName::PRIVACY,
        SipHeaderName::P_PREFERRED_IDENTITY,
        SipHeaderName::P_ASSERTED_IDENTITY,
        SipHeaderName::MIN_SE,
        SipHeaderName::PATH,
        SipHeaderName::P_ASSOCIATED_URI,
        SipHeaderName::P_CALLED_PARTY_ID,
        SipHeaderName::P_VISITED_NETWORK_ID,
        SipHeaderName::P_CHARGING_FUNCTION_ADDRESSES,
        SipHeaderName::P_ACCESS_NETWORK_INFO,
        SipHeaderName::P_CHARGING_VECTOR,
        SipHeaderName::SERVICE_ROUTE,
        SipHeaderName::HISTORY_INFO,
        SipHeaderName::REQUEST_DISPOSITION,
        SipHeaderName::ACCEPT_CONTACT,
        SipHeaderName::REJECT_CONTACT,
        SipHeaderName::JOIN,
        SipHeaderName::SIP_IF_MATCH,
        SipHeaderName::SIP_ETAG,
        SipHeaderName::PROXY_AUTHENTICATE,
        SipHeaderName::PROXY_AUTHORIZATION,
        SipHeaderName::RACK,
        SipHeaderName::RECORD_ROUTE,
        SipHeaderName::REFERRED_BY,
        SipHeaderName::REFER_TO,
        SipHeaderName::REPLACES,
        SipHeaderName::REQUIRE,
        SipHeaderName::ROUTE,
        SipHeaderName::RSEQ,
        SipHeaderName::SECURITY_CLIENT,
        SipHeaderName::SECURITY_VERIFY,
        SipHeaderName::SECURITY_SERVER,
        SipHeaderName::SESSION_EXPIRES,
        SipHeaderName::SUBSCRIPTION_STATE,
        SipHeaderName::SUPPORTED,
        SipHeaderName::TIMESTAMP,
        SipHeaderName::TO,
        SipHeaderName::UNSUPPORTED,
        SipHeaderName::VIA,
        SipHeaderName::WARNING,
        SipHeaderName::WWW_AUTHENTICATE,
        IMS_NULL,
        SipHeaderName::RETRY_AFTER,
        SipHeaderName::RETRY_AFTER,
        SipHeaderName::RETRY_AFTER,
        SipHeaderName::P_EARLY_MEDIA,
        SipHeaderName::RESOURCE_PRIORITY,
        SipHeaderName::ACCEPT_RESOURCE_PRIORITY,
        SipHeaderName::DATE,
        SipHeaderName::ACCEPT_ENCODING,
        SipHeaderName::ACCEPT_LANGUAGE,
        SipHeaderName::ALERT_INFO,
        SipHeaderName::ANSWER_MODE,
        SipHeaderName::AUTHENTICATION_INFO,
        SipHeaderName::CALL_INFO,
        SipHeaderName::CONTENT_LANGUAGE,
        SipHeaderName::ERROR_INFO,
        SipHeaderName::FLOW_TIMER,
        SipHeaderName::IDENTITY,
        SipHeaderName::IDENTITY_INFO,
        SipHeaderName::IN_REPLY_TO,
        SipHeaderName::ORGANIZATION,
        SipHeaderName::P_ANSWER_STATE,
        SipHeaderName::PERMISSION_MISSING,
        SipHeaderName::P_MEDIA_AUTHORIZATION,
        SipHeaderName::P_PROFILE_KEY,
        SipHeaderName::P_REFUSED_URI_LIST,
        SipHeaderName::PRIORITY,
        SipHeaderName::PRIV_ANSWER_MODE,
        SipHeaderName::PROXY_REQUIRE,
        SipHeaderName::P_SERVED_USER,
        SipHeaderName::P_USER_DATABASE,
        SipHeaderName::REASON,
        SipHeaderName::REFER_SUB,
        SipHeaderName::REPLY_TO,
        SipHeaderName::RESPONSE_KEY,
        SipHeaderName::SERVER,
        SipHeaderName::SUBJECT,
        SipHeaderName::SUPPRESS_IF_MATCH,
        SipHeaderName::TARGET_DIALOG,
        SipHeaderName::TRIGGER_CONSENT,
        SipHeaderName::USER_AGENT,
        SipHeaderName::FEATURE_CAPS,
        SipHeaderName::GEOLOCATION,
        SipHeaderName::GEOLOCATION_ERROR,
        SipHeaderName::GEOLOCATION_ROUTING,
        SipHeaderName::INFO_PACKAGE,
        SipHeaderName::MAX_BREADTH,
        SipHeaderName::P_ASSERTED_SERVICE,
        SipHeaderName::POLICY_CONTACT,
        SipHeaderName::POLICY_ID,
        SipHeaderName::P_PREFERRED_SERVICE,
        SipHeaderName::RECV_INFO,
        SipHeaderName::SESSION_ID,
        IMS_NULL,
};
// clang-format on

PUBLIC
SipHeader::SipHeader() :
        m_nType(ISipHeader::ANY),
        m_strName(AString::ConstNull()),
        m_strBody(AString::ConstNull()),
        m_pAddress(IMS_NULL)
{
}

PUBLIC
SipHeader::SipHeader(IN IMS_SINT32 nType) :
        m_nType(nType),
        m_strName(SipStack::GetHeaderName(nType)),
        m_strBody(AString::ConstNull()),
        m_pAddress(IMS_NULL)
{
}

PUBLIC
SipHeader::SipHeader(IN const AString& strName) :
        m_nType(ISipHeader::ANY),
        m_strName(strName),
        m_strBody(AString::ConstNull()),
        m_pAddress(IMS_NULL)
{
    m_nType = SipStack::GetHeaderTypeFromName(strName);
}

PUBLIC
SipHeader::SipHeader(IN const SipHeaderBase* pSipHdr) :
        m_nType(ISipHeader::ANY),
        m_strName(AString::ConstNull()),
        m_strBody(AString::ConstNull()),
        m_pAddress(IMS_NULL)
{
    m_nType = SipStack::GetHeaderType(pSipHdr);

    if (m_nType == ISipHeader::UNKNOWN)
    {
        m_strName = SipStack::GetUnknownHeaderName(pSipHdr);
    }
    else
    {
        m_strName = SipStack::GetHeaderName(m_nType);
    }

    SipStack::EncodeHeaderBody(pSipHdr, IMS_FALSE, m_strBody);

    // If the header type is unknown, decode the body according to the general syntax rule...
    if (m_nType == ISipHeader::UNKNOWN)
    {
        ParseUnknownBody(m_strBody);
    }

    if (SipStack::IsAddressFormatHeader(m_nType, m_strName))
    {
        m_pAddress = new SipAddress(m_strBody);
    }

    // P-Preferred-Identity & P-Asserted-Identity
    //    : name-addr / addr-spec -> no header parameters
    if ((m_nType != ISipHeader::P_PREFERRED_IDENTITY) &&
            (m_nType != ISipHeader::P_ASSERTED_IDENTITY) && (m_nType != ISipHeader::UNKNOWN))
    {
        m_objParams = SipStack::ExtractParameters(pSipHdr);
    }
}

PUBLIC VIRTUAL SipHeader::~SipHeader()
{
    if (m_pAddress != IMS_NULL)
    {
        delete m_pAddress;
    }

    if (!m_objParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
        {
            SipParameter* pParameter = m_objParams.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                delete pParameter;
            }
        }

        m_objParams.Clear();
    }
}

PUBLIC VIRTUAL void SipHeader::Destroy()
{
    delete this;
}

PUBLIC VIRTUAL ISipHeader* SipHeader::Clone() const
{
    SipHeader* pHeader = new SipHeader(m_nType);

    if (pHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    pHeader->m_strName = m_strName;
    pHeader->m_strBody = m_strBody;

    if (m_pAddress != IMS_NULL)
    {
        pHeader->m_pAddress = new SipAddress(*m_pAddress);
    }

    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);
        SipParameter* pNewParameter = new SipParameter(*pParameter);

        if (pNewParameter != IMS_NULL)
        {
            pHeader->m_objParams.Append(pNewParameter);
        }
    }

    return pHeader;
}

PUBLIC VIRTUAL IMS_BOOL SipHeader::Equals(IN const ISipHeader* piHeader) const
{
    const SipHeader* pHeader = DYNAMIC_CAST(const SipHeader*, piHeader);

    if (pHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nType != pHeader->m_nType)
    {
        return IMS_FALSE;
    }

    if (m_nType == ISipHeader::UNKNOWN)
    {
        const IMS_CHAR* pszHdrName = SipStack::GetHeaderName(m_nType, m_strName);
        const IMS_CHAR* pszOtherHdrName =
                SipStack::GetHeaderName(pHeader->m_nType, pHeader->m_strName);

        if (AString::CompareIgnoreCase(pszHdrName, pszOtherHdrName) != 0)
        {
            return IMS_FALSE;
        }
    }

    // Compare the header value field
    if (SipStack::IsAddressFormatHeader(m_nType, m_strName))
    {
        SipAddress objAddress(m_strBody);
        SipAddress objOtherAddress(pHeader->m_strBody);

        if (!objAddress.Equals(objOtherAddress))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!m_strBody.EqualsIgnoreCase(pHeader->m_strBody))
        {
            return IMS_FALSE;
        }
    }

    // Compare the header parameters
    if (m_objParams.GetSize() != pHeader->m_objParams.GetSize())
    {
        return IMS_FALSE;
    }

    // TODO:: comparison of parameter fields

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SipHeader::GetHeaderValue() const
{
    if (m_objParams.IsEmpty())
    {
        return m_strBody;
    }

    AString strHeaderValue(m_strBody);

    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            strHeaderValue += TextParser::CHAR_SEMICOLON + pParameter->ToString();
        }
    }

    return strHeaderValue;
}

PUBLIC VIRTUAL const SipParameter* SipHeader::GetParameter(IN const AString& strName) const
{
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            return pParameter;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL IMS_RESULT SipHeader::GetParameterNames(OUT ImsList<AString>& objParamNames) const
{
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);

        if (!objParamNames.Append(pParameter->GetName()))
        {
            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_SINT32 SipHeader::GetValueInt() const
{
    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nValue = INVALID_INT;

    if (IsHeaderBodyDigitFormat(m_nType))
    {
        nValue = m_strBody.ToInt32(&bOk);
    }

    return bOk ? nValue : INVALID_INT;
}

PUBLIC VIRTUAL void SipHeader::RemoveParameter(IN const AString& strName)
{
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (pParameter->GetName().Equals(strName))
            {
                m_objParams.RemoveAt(i);
                delete pParameter;
                return;
            }
        }
    }
}

PUBLIC VIRTUAL void SipHeader::SetName(IN const AString& strName)
{
    m_strName = strName.Trim();
    m_nType = SipStack::GetHeaderTypeFromName(m_strName);
}

PUBLIC VIRTUAL IMS_RESULT SipHeader::SetParameter(
        IN const AString& strName, IN const AString& strValue)
{
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            return pParameter->SetValue(strValue);
        }
    }

    // Now, no existing parameter in the parameter list

    SipParameter* pParameter = new SipParameter(strName, strValue);

    if (pParameter == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_FAILURE;
    }

    if (!m_objParams.Append(pParameter))
    {
        delete pParameter;

        SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_FAILURE;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SipHeader::SetValueInt(IN IMS_SINT32 nValue)
{
    if (IsHeaderBodyDigitFormat(m_nType))
    {
        if (nValue >= 0)
        {
            m_strBody.SetNumber(nValue);
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

PUBLIC VIRTUAL AString SipHeader::ToString() const
{
    AString strHeader;

    strHeader.Append(m_strName);
    strHeader.Append(TextParser::CHAR_COLON);
    strHeader.Append(TextParser::CHAR_SP);
    strHeader.Append(ToStringWithoutName());

    return strHeader;
}

PUBLIC VIRTUAL AString SipHeader::ToStringWithoutName() const
{
    AStringBuffer objHeaderValue(512);

    if (m_pAddress != IMS_NULL)
    {
        objHeaderValue.Append(m_pAddress->ToString());

        if (!m_objParams.IsEmpty())
        {
            const AString& strTemp = static_cast<const AStringBuffer&>(objHeaderValue).GetString();

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
        objHeaderValue.Append(m_strBody);
    }

    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            objHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            // Do not add the white space...
            // strHeader.Append(TextParser::CHAR_SP);
            objHeaderValue.Append(pParameter->ToString());
        }
    }

    if (m_nType == PRIVACY)
    {
        // Remove the semi-colon; for the microsip stack consistency
        objHeaderValue.Erase(0, 1);
    }

    return static_cast<const AStringBuffer&>(objHeaderValue).GetString();
}

PUBLIC GLOBAL IMS_BOOL SipHeader::IsValidType(IN IMS_SINT32 nType)
{
    if ((nType > ISipHeader::INVALID) && (nType < ISipHeader::ANY))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL SipHeader::Decode(IN const AString& strBody, IN IMS_BOOL bParseParameter /*= IMS_TRUE*/)
{
    AString strTrimmedBody = strBody.Trim();

    if (strTrimmedBody.IsEmpty() || strTrimmedBody.IsNULL())
    {
        m_strBody = strTrimmedBody;

        SipPrivate::SetLastError(SipError::NO_ERROR);
        return IMS_TRUE;
    }

    SipHeaderBase* pSipHdr = SipStack::DecodeHeader(m_nType, m_strName, strTrimmedBody);

    if (pSipHdr == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::PARSING_ERROR);
        return IMS_FALSE;
    }

    // Process the header type which has an ANY type: Contact, Expires, Retry-After
    m_nType = SipStack::GetHeaderType(pSipHdr);

    SipStack::EncodeHeaderBody(pSipHdr, IMS_FALSE, m_strBody);

    // If the header type is unknown, decode the body according to the general syntax rule...
    if (m_nType == ISipHeader::UNKNOWN)
    {
        ParseUnknownBody(m_strBody);
    }

    if (m_pAddress != IMS_NULL)
    {
        delete m_pAddress;
        m_pAddress = IMS_NULL;
    }

    if (SipStack::IsAddressFormatHeader(m_nType, m_strName))
    {
        m_pAddress = new SipAddress(m_strBody);

        if (SipStack::IsAquotRequiredForAddressFormat(m_nType, m_strName))
        {
            m_pAddress->SetAquotRequired(IMS_TRUE);
        }
    }

    // P-Preferred-Identity & P-Asserted-Identity
    //    : name-addr / addr-spec -> no header parameters
    if ((bParseParameter == IMS_TRUE) && (m_nType != ISipHeader::P_PREFERRED_IDENTITY) &&
            (m_nType != ISipHeader::P_ASSERTED_IDENTITY) && (m_nType != ISipHeader::UNKNOWN))
    {
        m_objParams = SipStack::ExtractParameters(pSipHdr);
    }

    SipStack::FreeHeaderEx(pSipHdr);

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipHeader::ParseUnknownBody(IN const AString& strBody)
{
    // Find ';'
    IMS_SINT32 nSemiColon = TextParser::GetIndexOfDelimiter(strBody, TextParser::CHAR_SEMICOLON);

    // No parameters
    if (nSemiColon == AString::NPOS)
    {
        m_strBody = strBody;
        return IMS_TRUE;
    }

    m_objParams = SipStack::ExtractParameters(
            strBody.GetSubStr(nSemiColon + 1), TextParser::CHAR_SEMICOLON);

    m_strBody = strBody.GetSubStr(0, nSemiColon).Trim();

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL SipHeader::IsHeaderBodyDigitFormat(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case ISipHeader::CONTENT_LENGTH:   // FALL-THROUGH
        case ISipHeader::EXPIRES_SEC:      // FALL-THROUGH
        case ISipHeader::EXPIRES_ANY:      // FALL-THROUGH
        case ISipHeader::MIN_EXPIRES:      // FALL-THROUGH
        case ISipHeader::MAX_FORWARDS:     // FALL-THROUGH
        case ISipHeader::MIN_SE:           // FALL-THROUGH
        case ISipHeader::RETRY_AFTER_SEC:  // FALL-THROUGH
        case ISipHeader::RETRY_AFTER_ANY:  // FALL-THROUGH
        case ISipHeader::RSEQ:             // FALL-THROUGH
        case ISipHeader::SESSION_EXPIRES:  // FALL-THROUGH
        case ISipHeader::FLOW_TIMER:       // FALL-THROUGH
        case ISipHeader::MAX_BREADTH:
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}
