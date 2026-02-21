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
#include "AStringBuffer.h"
#include "ImsLib.h"
#include "IpAddress.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "SipAddress.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipError.h"
#include "SipHeader.h"
#include "SipParameter.h"
#include "SipPrivate.h"
#include "SipStack.h"
#include "SipUtils.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC GLOBAL const IMS_CHAR SipAddress::PARAM_MADDR[] = "maddr";
PUBLIC GLOBAL const IMS_CHAR SipAddress::PARAM_METHOD[] = "method";
PUBLIC GLOBAL const IMS_CHAR SipAddress::PARAM_PHONE_CONTEXT[] = "phone-context";
PUBLIC GLOBAL const IMS_CHAR SipAddress::PARAM_TRANSPORT[] = "transport";
PUBLIC GLOBAL const IMS_CHAR SipAddress::PARAM_TTL[] = "ttl";
PUBLIC GLOBAL const IMS_CHAR SipAddress::PARAM_USER[] = "user";

PUBLIC
SipAddress::UserInfoPart::UserInfoPart() :
        m_strUser(AString::ConstNull()),
        m_strPassword(AString::ConstNull())
{
}

PUBLIC
SipAddress::UserInfoPart::UserInfoPart(IN const SipAddress::UserInfoPart& other) :
        m_strUser(other.m_strUser),
        m_strPassword(other.m_strPassword)
{
    for (IMS_UINT32 i = 0; i < other.m_objParameters.GetSize(); ++i)
    {
        const SipParameter* pParameter = other.m_objParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            m_objParameters.Append(new SipParameter(*pParameter));
        }
    }
}

PUBLIC
SipAddress::UserInfoPart::~UserInfoPart()
{
    RemoveAllParameters();
}

PUBLIC
SipAddress::UserInfoPart& SipAddress::UserInfoPart::operator=(
        IN const SipAddress::UserInfoPart& other)
{
    if (this != &other)
    {
        RemoveAllParameters();

        m_strUser = other.m_strUser;
        m_strPassword = other.m_strPassword;

        for (IMS_UINT32 i = 0; i < other.m_objParameters.GetSize(); ++i)
        {
            const SipParameter* pParameter = other.m_objParameters.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                m_objParameters.Append(new SipParameter(*pParameter));
            }
        }
    }

    return (*this);
}

PUBLIC
IMS_BOOL SipAddress::UserInfoPart::Create(IN const AString& strUserInfo)
{
    if (strUserInfo.GetLength() == 0)
    {
        // No user part
        return IMS_FALSE;
    }

    m_strUser = AString::ConstNull();
    m_strPassword = AString::ConstNull();
    RemoveAllParameters();

    strUserInfo.SplitB(TextParser::CHAR_COLON, m_strUser, m_strPassword);

    if (m_strUser.Contains(TextParser::CHAR_SEMICOLON))
    {
        ImsList<AString> objTokens = m_strUser.Split(TextParser::CHAR_SEMICOLON);

        m_strUser = objTokens.GetAt(0);

        for (IMS_UINT32 i = 1; i < objTokens.GetSize(); ++i)
        {
            SipParameter* pParameter = new SipParameter();

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

            if (!m_objParameters.Append(pParameter))
            {
                delete pParameter;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
const SipParameter* SipAddress::UserInfoPart::GetParameter(IN const AString& strName) const
{
    for (IMS_UINT32 i = 0; i < m_objParameters.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParameters.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            return pParameter;
        }
    }

    return IMS_NULL;
}

PRIVATE
void SipAddress::UserInfoPart::RemoveAllParameters()
{
    if (m_objParameters.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objParameters.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            delete pParameter;
        }
    }

    m_objParameters.Clear();
}

PUBLIC
SipAddress::SipAddress() :
        m_bIsWildcard(IMS_FALSE),
        m_bDquotForDisplayName(IMS_FALSE),
        m_bAquotRequired(IMS_FALSE),
        m_strDisplayName(AString::ConstNull()),
        m_strScheme(AString::ConstNull()),
        m_strUserInfo(AString::ConstNull()),
        m_strHostInfo(AString::ConstNull()),
        m_nPort(Sip::PORT_UNSPECIFIED),
        m_pUserInfoPart(IMS_NULL)
{
}

PUBLIC
SipAddress::SipAddress(IN const AString& strAddress) :
        m_bIsWildcard(IMS_FALSE),
        m_bDquotForDisplayName(IMS_FALSE),
        m_bAquotRequired(IMS_FALSE),
        m_strDisplayName(AString::ConstNull()),
        m_strScheme(AString::ConstNull()),
        m_strUserInfo(AString::ConstNull()),
        m_strHostInfo(AString::ConstNull()),
        m_nPort(Sip::PORT_UNSPECIFIED),
        m_pUserInfoPart(IMS_NULL)
{
    Decode(strAddress, IMS_TRUE);
}

PUBLIC
SipAddress::SipAddress(IN const AString& strDisplayName, IN const AString& strUri) :
        m_bIsWildcard(IMS_FALSE),
        m_bDquotForDisplayName(IMS_FALSE),
        m_bAquotRequired(IMS_FALSE),
        m_strDisplayName(strDisplayName),
        m_strScheme(AString::ConstNull()),
        m_strUserInfo(AString::ConstNull()),
        m_strHostInfo(AString::ConstNull()),
        m_nPort(Sip::PORT_UNSPECIFIED),
        m_pUserInfoPart(IMS_NULL)
{
    Decode(strUri, IMS_TRUE, IMS_FALSE);
}

PUBLIC
SipAddress::SipAddress(IN const SipAddress& other) :
        m_bIsWildcard(other.m_bIsWildcard),
        m_bDquotForDisplayName(other.m_bDquotForDisplayName),
        m_bAquotRequired(other.m_bAquotRequired),
        m_strDisplayName(other.m_strDisplayName),
        m_strScheme(other.m_strScheme),
        m_strUserInfo(other.m_strUserInfo),
        m_strHostInfo(other.m_strHostInfo),
        m_nPort(other.m_nPort),
        m_pUserInfoPart(IMS_NULL)
{
    IMS_UINT32 i;

    for (i = 0; i < other.m_objHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = other.m_objHeaders.GetAt(i);
        ISipHeader* piNewHeader = piHeader->Clone();

        if (piNewHeader != IMS_NULL)
        {
            m_objHeaders.Append(piNewHeader);
        }
    }

    for (i = 0; i < other.m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = other.m_objParams.GetAt(i);
        SipParameter* pNewParameter = new SipParameter(*pParameter);

        if (pNewParameter != IMS_NULL)
        {
            m_objParams.Append(pNewParameter);
        }
    }
}

PUBLIC
SipAddress::~SipAddress()
{
    RemoveAllHeaderComponents();
    RemoveAllParameters();

    if (m_pUserInfoPart != IMS_NULL)
    {
        delete m_pUserInfoPart;
    }
}

PUBLIC
SipAddress& SipAddress::operator=(IN const SipAddress& other)
{
    if (this != &other)
    {
        RemoveAllHeaderComponents();
        RemoveAllParameters();

        m_bIsWildcard = other.m_bIsWildcard;
        m_bDquotForDisplayName = other.m_bDquotForDisplayName;
        m_bAquotRequired = other.m_bAquotRequired;
        m_strDisplayName = other.m_strDisplayName;
        m_strScheme = other.m_strScheme;
        m_strUserInfo = other.m_strUserInfo;
        m_strHostInfo = other.m_strHostInfo;
        m_nPort = other.m_nPort;
        m_pUserInfoPart = IMS_NULL;

        IMS_UINT32 i;

        for (i = 0; i < other.m_objHeaders.GetSize(); ++i)
        {
            const ISipHeader* piHeader = other.m_objHeaders.GetAt(i);
            ISipHeader* piNewHeader = piHeader->Clone();

            if (piNewHeader != IMS_NULL)
            {
                m_objHeaders.Append(piNewHeader);
            }
        }

        for (i = 0; i < other.m_objParams.GetSize(); ++i)
        {
            const SipParameter* pParameter = other.m_objParams.GetAt(i);
            SipParameter* pNewParameter = new SipParameter(*pParameter);

            if (pNewParameter != IMS_NULL)
            {
                m_objParams.Append(pNewParameter);
            }
        }
    }

    return (*this);
}

IMS_RESULT SipAddress::AddParameter(IN const AString& strName, IN const AString& strValue)
{
    if (strName.IsNULL() || m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check the grammar validity

    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            pParameter->AddValue(strValue);

            SipPrivate::SetLastError(SipError::NO_ERROR);
            return IMS_SUCCESS;
        }
    }

    // If the parameter with the specified name does not exist, then add new one.
    SipParameter* pParameter = IMS_NULL;

    if (strValue.IsNULL())
    {
        pParameter = new SipParameter(strName);
    }
    else
    {
        pParameter = new SipParameter(strName, strValue);
    }

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

PUBLIC
IMS_BOOL SipAddress::Create(IN const AString& strAddress)
{
    if (strAddress.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    m_bIsWildcard = IMS_FALSE;
    m_bDquotForDisplayName = IMS_FALSE;
    m_strDisplayName = AString::ConstNull();
    m_strScheme = AString::ConstNull();
    m_strUserInfo = AString::ConstNull();
    m_strHostInfo = AString::ConstNull();
    m_nPort = Sip::PORT_UNSPECIFIED;

    RemoveAllParameters();
    RemoveAllHeaderComponents();

    return Decode(strAddress, IMS_TRUE);
}

PUBLIC
IMS_BOOL SipAddress::Equals(IN const SipAddress& objAddress) const
{
    // If both addresses are the special ("*") value, those values are the same.
    if (m_bIsWildcard && objAddress.m_bIsWildcard)
    {
        return IMS_TRUE;
    }

    if (!m_bIsWildcard && objAddress.m_bIsWildcard)
    {
        return IMS_FALSE;
    }

    if (m_bIsWildcard && !objAddress.m_bIsWildcard)
    {
        return IMS_FALSE;
    }

    if (!m_strScheme.EqualsIgnoreCase(objAddress.m_strScheme))
    {
        return IMS_FALSE;
    }

    // "sip" or "sips" URI scheme
    if (IsSchemeSip() || IsSchemeSips())
    {
        // 4 needs to be updated
        return CompareSipUris(objAddress);
    }
    else if (IsSchemeTel())
    {
        return CompareTelUris(objAddress);
    }
    // 4 comparison of an address format with the other URI scheme
    else
    {
        return m_strHostInfo.EqualsIgnoreCase(objAddress.m_strHostInfo);
    }
}

PUBLIC
const ISipHeader* SipAddress::GetHeader(
        IN IMS_SINT32 nType, IN const AString& strName /* = AString::ConstNull()*/) const
{
    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = m_objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        if (nType == piHeader->GetType())
        {
            if (nType != ISipHeader::UNKNOWN)
            {
                return piHeader;
            }
            else
            {
                const IMS_CHAR cCompactName = SipStack::GetCompactHeaderName(nType, strName);
                const IMS_CHAR* pszName = SipStack::GetHeaderName(nType, strName);
                const AString& strHeaderName = piHeader->GetName();

                if (strHeaderName.EqualsIgnoreCase(cCompactName) ||
                        strHeaderName.EqualsIgnoreCase(pszName))
                {
                    return piHeader;
                }
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
const SipParameter* SipAddress::GetParameter(IN const AString& strName) const
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

PUBLIC
IMS_SINT32 SipAddress::GetPort() const
{
    if (m_nPort == Sip::PORT_UNSPECIFIED)
    {
        if (m_bIsWildcard)
        {
            return 0;
        }

        if (m_strScheme.Equals(Sip::STR_SIP))
            return Sip::PORT_5060;
        else if (m_strScheme.Equals(Sip::STR_SIPS))
            return Sip::PORT_5061;
        else
            return 0;
    }

    return m_nPort;
}

PUBLIC
AString SipAddress::GetUri() const
{
    if (m_bIsWildcard)
    {
        return AString(&TextParser::CHAR_ASTERISK, 1);
    }

    if (m_strHostInfo.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objUri(256);

    if (m_strScheme.GetLength() > 0)
    {
        objUri.Append(m_strScheme);
        objUri.Append(TextParser::CHAR_COLON);
    }

    if (m_strUserInfo.GetLength() > 0)
    {
        objUri.Append(m_strUserInfo);
        objUri.Append(TextParser::CHAR_AT);
    }

    IpAddress objHost;

    if (objHost.Parse(m_strHostInfo))
    {
        if (objHost.IsIPv6Address())
        {
            objUri.Append(TextParser::CHAR_LSBRACKET);
            objUri.Append(m_strHostInfo);
            objUri.Append(TextParser::CHAR_RSBRACKET);
        }
        else
        {
            objUri.Append(m_strHostInfo);
        }
    }
    else
    {
        objUri.Append(m_strHostInfo);
    }

    if (m_nPort != Sip::PORT_UNSPECIFIED)
    {
        AString strPort;
        strPort.SetNumber(m_nPort);

        objUri.Append(TextParser::CHAR_COLON);
        objUri.Append(strPort);
    }

    return static_cast<const AStringBuffer&>(objUri).GetString();
}

PUBLIC
const SipAddress::UserInfoPart* SipAddress::GetUserInfoPart() const
{
    if (m_strUserInfo.GetLength() == 0)
    {
        // No userinfo part

        // If SIP address is "tel" URI.
        if (IsSchemeTel())
        {
            return CreateUserInfoPart(m_strHostInfo);
        }
        else if (IsServiceUrn())
        {
            // To remove "service:" prefix
            IMS_SINT32 nIndex = m_strHostInfo.GetIndexOf(TextParser::CHAR_COLON);

            if (nIndex != AString::NPOS)
            {
                AString strUserPart = m_strHostInfo.GetSubStr(nIndex + 1);

                return CreateUserInfoPart(strUserPart);
            }
        }

        return IMS_NULL;
    }

    return CreateUserInfoPart(m_strUserInfo);
}

PUBLIC
const AString& SipAddress::GetUserPart() const
{
    if (m_bIsWildcard)
    {
        return AString::ConstNull();
    }

    if (IsSchemeSip() || IsSchemeSips())
    {
        return m_strUserInfo;
    }

    return m_strHostInfo;
}

PUBLIC
IMS_BOOL SipAddress::IsServiceUrn() const
{
    if (m_bIsWildcard)
    {
        return IMS_FALSE;
    }

    if (!m_strScheme.EqualsIgnoreCase("urn"))
    {
        return IMS_FALSE;
    }

    // Check if "service:" is present in case-insensitively
    return m_strHostInfo.StartsWith("service:");
}

PUBLIC
void SipAddress::RemoveAllHeaderComponents()
{
    if (m_objHeaders.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        ISipHeader* piHeader = m_objHeaders.GetAt(i);

        if (piHeader != IMS_NULL)
        {
            piHeader->Destroy();
        }
    }

    m_objHeaders.Clear();
}

PUBLIC
void SipAddress::RemoveAllParameters()
{
    if (m_objParams.IsEmpty())
    {
        return;
    }

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

PUBLIC
void SipAddress::RemoveParameter(IN const AString& strName)
{
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            m_objParams.RemoveAt(i);
            delete pParameter;
            return;
        }
    }
}

PUBLIC
IMS_RESULT SipAddress::SetDisplayName(IN const AString& strName)
{
    if (m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    m_strDisplayName = strName;

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
        IN const AString& strName /* = AString::ConstNull()*/)
{
    if (m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if ((nType <= ISipHeader::INVALID) || (nType >= ISipHeader::ANY))
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    SipHeader* pHeader = new SipHeader(nType);

    if (pHeader == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_FAILURE;
    }

    if (nType == ISipHeader::UNKNOWN)
    {
        pHeader->SetName(strName);
    }

    if (pHeader->SetHeaderValue(TextParser::DoPercentDecoding(strValue)) != IMS_SUCCESS)
    {
        SipPrivate::SetLastError(SipError::PARSING_ERROR);

        delete pHeader;
        return IMS_FAILURE;
    }

    if (!m_objHeaders.Append(pHeader))
    {
        delete pHeader;
        return IMS_FAILURE;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetHeaders(
        IN const AString& strHeaders, IN IMS_BOOL bRemoveAll /* = IMS_TRUE*/)
{
    if (m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (bRemoveAll)
    {
        RemoveAllHeaderComponents();
    }

    if (!DecodeHeaderComponent(strHeaders))
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetHost(IN const AString& strHost)
{
    if (strHost.IsNULL() || m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;  // Throw exception
    }

    IpAddress objHost;

    if (objHost.Parse(strHost))
    {
        m_strHostInfo = objHost.ToString();
    }
    else
    {
        m_strHostInfo = strHost;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetParameter(IN const AString& strName, IN const AString& strValue)
{
    if (strName.IsNULL() || m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase(strName))
        {
            if (pParameter->SetValue(strValue) != IMS_SUCCESS)
            {
                return IMS_FAILURE;
            }

            SipPrivate::SetLastError(SipError::NO_ERROR);
            return IMS_SUCCESS;
        }
    }

    // If the parameter with the specified name does not exist, then add new one.
    SipParameter* pParameter = IMS_NULL;

    if (strValue.IsNULL())
    {
        pParameter = new SipParameter(strName);
    }
    else
    {
        pParameter = new SipParameter(strName, strValue);
    }

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

PUBLIC
IMS_RESULT SipAddress::SetPort(IN IMS_SINT32 nPort)
{
    if ((nPort < 0) || (nPort > Sip::PORT_UNSPECIFIED) || m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (nPort == 0)
    {
        m_nPort = Sip::PORT_UNSPECIFIED;
    }
    else
    {
        m_nPort = nPort;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetScheme(IN const AString& strScheme)
{
    if (strScheme.IsNULL() || m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    m_strScheme = strScheme;

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetUri(IN const AString& strURI)
{
    if (strURI.IsNULL() || strURI.Equals(TextParser::CHAR_ASTERISK) || m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!Decode(strURI, IMS_FALSE, IMS_FALSE, IMS_FALSE))
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipAddress::SetUser(IN const AString& strUser)
{
    if (m_bIsWildcard)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    m_strUserInfo = strUser;

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
AString SipAddress::ToString() const
{
    if (m_bIsWildcard)
    {
        return AString(&TextParser::CHAR_ASTERISK, 1);
    }

    if (m_strHostInfo.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objSipAddr(512);
    IMS_BOOL bFormAquot = IMS_FALSE;

    if (m_bAquotRequired || (m_strDisplayName.GetLength() > 0) || !m_objParams.IsEmpty() ||
            !m_objHeaders.IsEmpty())
    {
        bFormAquot = IMS_TRUE;
    }

    if (m_strDisplayName.GetLength() > 0)
    {
        if (IsDisplayNameToken(m_strDisplayName))
        {
            IMS_SINT32 nSlotId = IMS_SLOT_0;

            if (!m_bDquotForDisplayName && SystemConfig::IsMultiSimEnabled())
            {
                nSlotId = ThreadService::GetCurrentSlotId();
            }

            if (m_bDquotForDisplayName ||
                    ((nSlotId != IMS_SLOT_ANY) &&
                            SipConfigProxy::IsDisplayNameDquotRequired(nSlotId)))
            {
                objSipAddr.Append(TextParser::CHAR_DQUOT);
                objSipAddr.Append(m_strDisplayName);
                objSipAddr.Append(TextParser::CHAR_DQUOT);
            }
            else
            {
                objSipAddr.Append(m_strDisplayName);
            }
        }
        else
        {
            objSipAddr.Append(TextParser::CHAR_DQUOT);

            // '"' / '\' -> need to be escaped
            if (m_strDisplayName.Contains('"') || m_strDisplayName.Contains('\\'))
            {
                objSipAddr.Append(EscapeDquotAndBackslash(m_strDisplayName));
            }
            else
            {
                objSipAddr.Append(m_strDisplayName);
            }

            objSipAddr.Append(TextParser::CHAR_DQUOT);
        }

        objSipAddr.Append(TextParser::CHAR_SP);
    }

    if (bFormAquot)
    {
        objSipAddr.Append(TextParser::CHAR_LAQUOT);
    }

    if (m_strScheme.GetLength() > 0)
    {
        objSipAddr.Append(m_strScheme);
        objSipAddr.Append(TextParser::CHAR_COLON);
    }

    if (m_strUserInfo.GetLength() > 0)
    {
        objSipAddr.Append(m_strUserInfo);
        objSipAddr.Append(TextParser::CHAR_AT);
    }

    IpAddress objHost;

    if (objHost.Parse(m_strHostInfo))
    {
        if (objHost.IsIPv6Address())
        {
            objSipAddr.Append(TextParser::CHAR_LSBRACKET);
            objSipAddr.Append(m_strHostInfo);
            objSipAddr.Append(TextParser::CHAR_RSBRACKET);
        }
        else
        {
            objSipAddr.Append(m_strHostInfo);
        }
    }
    else
    {
        objSipAddr.Append(m_strHostInfo);
    }

    if (m_nPort != Sip::PORT_UNSPECIFIED)
    {
        AString strPort;

        strPort.SetNumber(m_nPort);

        objSipAddr.Append(TextParser::CHAR_COLON);
        objSipAddr.Append(strPort);
    }

    if (!m_objParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
        {
            const SipParameter* pParameter = m_objParams.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                objSipAddr.Append(TextParser::CHAR_SEMICOLON);
                objSipAddr.Append(pParameter->ToString());
            }
        }
    }

    // If uri-header parameter exists, then inserts those fields
    if (!m_objHeaders.IsEmpty())
    {
        // hnv-unreserved = "[" / "]" / "/" / "?" / ":" / "+" / "$"
        // unreserved (mark) = "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
        // non-reserved characters = "-" / "_" / "." / "~"
        // const AString strHNV("[]/?:+$!*'()");
        // const AString strEscaped("\"<>");
        const AString strHnv("[]/?:+$-_.!~*'()");
        const ISipHeader* piHeader = m_objHeaders.GetAt(0);

        objSipAddr.Append(TextParser::CHAR_QUESTIONMARK);

        objSipAddr.Append(piHeader->GetName());
        objSipAddr.Append(TextParser::CHAR_EQUAL);
        objSipAddr.Append(TextParser::DoPercentEncodingEx(piHeader->GetHeaderValue(), strHnv));

        for (IMS_UINT32 i = 1; i < m_objHeaders.GetSize(); ++i)
        {
            piHeader = m_objHeaders.GetAt(i);

            if (piHeader != IMS_NULL)
            {
                objSipAddr.Append(TextParser::CHAR_AMPERSAND);
                objSipAddr.Append(piHeader->GetName());
                objSipAddr.Append(TextParser::CHAR_EQUAL);
                objSipAddr.Append(
                        TextParser::DoPercentEncodingEx(piHeader->GetHeaderValue(), strHnv));
            }
        }
    }

    if (bFormAquot)
    {
        objSipAddr.Append(TextParser::CHAR_RAQUOT);
    }

    return static_cast<const AStringBuffer&>(objSipAddr).GetString();
}

PUBLIC GLOBAL const ImsList<SipAddress*>& SipAddress::ConstEmptyList()
{
    static const ImsList<SipAddress*> CONST_EMPTY_LIST = ImsList<SipAddress*>();
    return CONST_EMPTY_LIST;
}

PUBLIC GLOBAL const SipAddress& SipAddress::ConstNull()
{
    static const SipAddress CONST_NULL;
    return CONST_NULL;
}

PUBLIC GLOBAL IMS_SINT32 SipAddress::GetTelUriFormat(IN const AString& strResource)
{
    // global-number-digits := "+" *phonedigit DIGIT *phonedigit
    // local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
    // phonedigit := DIGIT / [visual-separator]
    // phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]

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

            if (!IMS_ISDIGIT(c) && !IsVisualSeparator(c) && (c != '*') && (c != '#') &&
                    !((c >= 'A') && (c <= 'F')))
            {
                return TEL_FORMAT_NONE;
            }
        }

        return TEL_FORMAT_LOCAL;
    }
}

PRIVATE
IMS_BOOL SipAddress::CompareSipUris(IN const SipAddress& objAddress) const
{
    // clang-format off
    static struct
    {
        const IMS_CHAR* pszName;
        IMS_SINT32 nSize;
    } SPECIAL_PARAMETER[] = {
            {PARAM_USER,   4},
            {PARAM_TTL,    3},
            {PARAM_METHOD, 6},
            {PARAM_MADDR,  5}
  /*
  Do not include "transport" uri-parameter as a special one.
  { PARAM_TRANSPORT, 9 }
  */
    };
    // clang-format on

    static const IMS_UINT32 MAX_SPECIAL_PARAMETER =
            sizeof(SPECIAL_PARAMETER) / sizeof(SPECIAL_PARAMETER[0]);

    if (m_nPort != objAddress.m_nPort)
    {
        return IMS_FALSE;
    }

    AString strValue;
    AString strOtherValue;

    strValue = TextParser::DoPercentDecoding(m_strHostInfo);
    strOtherValue = TextParser::DoPercentDecoding(objAddress.m_strHostInfo);

    if (!strValue.EqualsIgnoreCase(strOtherValue))
        return IMS_FALSE;

    strValue = TextParser::DoPercentDecoding(m_strUserInfo);
    strOtherValue = TextParser::DoPercentDecoding(objAddress.m_strUserInfo);

    if (!strValue.Equals(strOtherValue))
    {
        return IMS_FALSE;
    }

    // "transport" uri-parameter comparison
    if (!CompareTransportParameters(objAddress))
    {
        IMS_TRACE_D("\"transport\" uri-parameter is not matched", 0, 0, 0);
        return IMS_FALSE;
    }

    // Check if any uri-parameter components are appearing in both URIs.
    // user/ttl/method/maddr[/transport]
    AString strParamName;

    for (IMS_UINT32 i = 0; i < MAX_SPECIAL_PARAMETER; ++i)
    {
        strParamName.Attach(SPECIAL_PARAMETER[i].pszName, SPECIAL_PARAMETER[i].nSize);
        IMS_BOOL bParamPresent = IsParameterPresent(strParamName);
        IMS_BOOL bOtherParamPresent = objAddress.IsParameterPresent(strParamName);

        if (bParamPresent != bOtherParamPresent)
        {
            IMS_TRACE_E(0,
                    "Parameter (%s) is not matched; "
                    "One of them does not contain the parameter",
                    SPECIAL_PARAMETER[i].pszName, 0, 0);
            return IMS_FALSE;
        }
    }

    // Check if any uri-parameter components are equals if and only if it appears in both URIs.
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParam = m_objParams.GetAt(i);
        const AString& strName = pParam->GetName();

        // Check for each uri-parameter fields
        for (IMS_UINT32 j = 0; j < objAddress.m_objParams.GetSize(); ++j)
        {
            const SipParameter* pOtherParam = objAddress.m_objParams.GetAt(j);

            if (strName.EqualsIgnoreCase(pOtherParam->GetName()))
            {
                if (!pParam->Equals(pOtherParam))
                {
                    IMS_TRACE_E(0, "Parameter(%s) is not matched(%s|%s)", strName.GetStr(),
                            pParam->GetValue().GetStr(), pOtherParam->GetValue().GetStr());
                    return IMS_FALSE;
                }

                break;
            }
        }
    }

    // Check if the header components are appearing in both URIs.
    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        IMS_BOOL bHeaderFound = IMS_FALSE;
        const ISipHeader* piHeader = m_objHeaders.GetAt(i);

        for (IMS_UINT32 j = 0; j < objAddress.m_objHeaders.GetSize(); ++j)
        {
            const ISipHeader* piOtherHeader = objAddress.m_objHeaders.GetAt(j);

            if (piHeader->Equals(piOtherHeader))
            {
                bHeaderFound = IMS_TRUE;
                break;
            }
        }

        if (!bHeaderFound)
        {
            IMS_TRACE_E(0, "Header (%s, %s) is not found", piHeader->GetName().GetStr(),
                    piHeader->GetValue().GetStr(), 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipAddress::CompareTelUris(IN const SipAddress& objAddress) const
{
    // Checks if the local-number-digits or global-number-digits
    if (m_strHostInfo.StartsWith(TextParser::CHAR_PLUS) &&
            !objAddress.m_strHostInfo.StartsWith(TextParser::CHAR_PLUS))
    {
        return IMS_FALSE;
    }

    if (!m_strHostInfo.StartsWith(TextParser::CHAR_PLUS) &&
            objAddress.m_strHostInfo.StartsWith(TextParser::CHAR_PLUS))
    {
        return IMS_FALSE;
    }

    // Compares the number digits
    if (!CompareNumberDigits(m_strHostInfo, objAddress.m_strHostInfo))
    {
        return IMS_FALSE;
    }

    // Check the phone-context parameter
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParam = m_objParams.GetAt(i);

        if (pParam->GetName().EqualsIgnoreCase(PARAM_PHONE_CONTEXT))
        {
            IMS_BOOL bFound = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < objAddress.m_objParams.GetSize(); ++j)
            {
                const SipParameter* pOtherParam = objAddress.m_objParams.GetAt(i);

                if (pOtherParam->GetName().EqualsIgnoreCase(PARAM_PHONE_CONTEXT))
                {
                    const AString& strValue = pParam->GetValue();
                    const AString& strOtherValue = pOtherParam->GetValue();

                    if (strValue.StartsWith(TextParser::CHAR_PLUS) &&
                            !strOtherValue.StartsWith(TextParser::CHAR_PLUS))
                    {
                        return IMS_FALSE;
                    }

                    if (!strValue.StartsWith(TextParser::CHAR_PLUS) &&
                            strOtherValue.StartsWith(TextParser::CHAR_PLUS))
                    {
                        return IMS_FALSE;
                    }

                    if (strValue.StartsWith(TextParser::CHAR_PLUS) &&
                            strOtherValue.StartsWith(TextParser::CHAR_PLUS))
                    {
                        // global-digit-number
                        if (!CompareNumberDigits(strValue, strOtherValue))
                        {
                            return IMS_FALSE;
                        }
                    }
                    else
                    {
                        // domainname
                        if (!strValue.EqualsIgnoreCase(strOtherValue))
                        {
                            return IMS_FALSE;
                        }
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
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        IMS_BOOL bParamFound = IMS_FALSE;
        const SipParameter* pParam = m_objParams.GetAt(i);

        for (IMS_UINT32 j = 0; j < objAddress.m_objParams.GetSize(); ++j)
        {
            if (pParam->Equals(objAddress.m_objParams.GetAt(j)))
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

PRIVATE
IMS_BOOL SipAddress::CompareTransportParameters(IN const SipAddress& objAddress) const
{
    const AString strParamName(PARAM_TRANSPORT);
    const SipParameter* pParameter = GetParameter(strParamName);
    const SipParameter* pOtherParameter = objAddress.GetParameter(strParamName);

    if ((pParameter == IMS_NULL) && (pOtherParameter == IMS_NULL))
    {
        // No "transport" uri-parameter
        return IMS_TRUE;
    }

    const AString& strTransport =
            (pParameter != IMS_NULL) ? pParameter->GetValue() : AString::ConstNull();
    const AString& strOtherTransport =
            (pOtherParameter != IMS_NULL) ? pOtherParameter->GetValue() : AString::ConstNull();

    if ((strTransport.GetLength() == 0) && (strOtherTransport.GetLength() == 0))
    {
        // Exceptional case: MUST NOT be reached.
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
    const AString& strValue = (strTransport.GetLength() == 0) ? strOtherTransport : strTransport;

    if ((IsSchemeSip() && strValue.EqualsIgnoreCase(Sip::STR_UDP)) ||
            (IsSchemeSips() && strValue.EqualsIgnoreCase(Sip::STR_TCP)))
    {
        // One default(missing) & one explicitly contain
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
const SipAddress::UserInfoPart* SipAddress::CreateUserInfoPart(IN const AString& strUserPart) const
{
    if (m_pUserInfoPart == IMS_NULL)
    {
        m_pUserInfoPart = new UserInfoPart();
    }

    if (m_pUserInfoPart != IMS_NULL)
    {
        if (!m_pUserInfoPart->Create(strUserPart))
        {
            return IMS_NULL;
        }
    }

    return m_pUserInfoPart;
}

PRIVATE
IMS_BOOL SipAddress::Decode(IN const AString& strAddress, IN IMS_BOOL bParseParameter,
        IN IMS_BOOL bParseDisplayName /* = IMS_TRUE*/, IN IMS_BOOL bParseHeader /* = IMS_TRUE*/)
{
    AString strUri = strAddress.Trim();

    SipPrivate::SetLastError(SipError::NO_ERROR);

    // Checks if the address is the special ("*") value
    if (strUri.Equals(TextParser::CHAR_ASTERISK))
    {
        m_bIsWildcard = IMS_TRUE;
        m_strHostInfo = strUri;
        return IMS_TRUE;
    }

    IMS_SINT32 nLaquot = TextParser::GetIndexOfDelimiter(strUri, TextParser::CHAR_LAQUOT);

    if (nLaquot != AString::NPOS)
    {
        // If LAQUOT is found, then find the display name if it is there
        if (bParseDisplayName == IMS_TRUE)
        {
            m_strDisplayName = strUri.GetSubStr(0, nLaquot);
            m_strDisplayName = m_strDisplayName.Trim();

            // 4 Remove DQUOT and restore the escaped characters
            if (m_strDisplayName.StartsWith(TextParser::CHAR_DQUOT) &&
                    m_strDisplayName.EndsWith(TextParser::CHAR_DQUOT))
            {
                m_strDisplayName = TextParser::TrimDquot(m_strDisplayName);

                // Restore the original characters if it is escaped for DQUOT and backslash
                m_strDisplayName.Replace("\\\"", "\"");
                m_strDisplayName.Replace("\\\\", "\\");

                m_bDquotForDisplayName = IMS_TRUE;
            }
        }

        strUri = strUri.GetSubStr(nLaquot + 1);

        IMS_SINT32 nRaquot = strUri.GetIndexOf(TextParser::CHAR_RAQUOT);

        if (nRaquot != AString::NPOS)
        {
            strUri.Truncate(nRaquot);
        }
    }

    SipAddrSpec* pAddrSpec = SipStack::DecodeAddrSpec(strUri);

    if (pAddrSpec != IMS_NULL)
    {
        if (SipStack::IsUriSchemeSip(pAddrSpec) || SipStack::IsUriSchemeSips(pAddrSpec))
        {
            if (SipStack::IsUriSchemeSip(pAddrSpec))
            {
                m_strScheme = Sip::STR_SIP;
            }
            else
            {
                m_strScheme = Sip::STR_SIPS;
            }

            // user-info (user:password)
            m_strUserInfo = SipStack::AddrSpec_GetUser(pAddrSpec);

            const IMS_CHAR* pszPassword = SipStack::AddrSpec_GetPassword(pAddrSpec);

            if (pszPassword != IMS_NULL)
            {
                m_strUserInfo += TextParser::CHAR_COLON;
                m_strUserInfo += pszPassword;
            }

            m_strHostInfo = SipStack::AddrSpec_GetHost(pAddrSpec);

            IpAddress objHost;

            if (objHost.Parse(m_strHostInfo))
            {
                if (objHost.IsIPv6Address())
                {
                    m_strHostInfo = objHost.ToString();
                }
            }

            IMS_SINT32 nTmpPort = SipStack::AddrSpec_GetPort(pAddrSpec);

            if (nTmpPort != 0)
                m_nPort = nTmpPort;
            else
                m_nPort = Sip::PORT_UNSPECIFIED;

            if (bParseHeader)
            {
                RemoveAllHeaderComponents();

                if (!SipStack::DecodeHeaderComponent(pAddrSpec, m_objHeaders))
                {
                    SipPrivate::SetLastError(SipError::PARSING_ERROR);
                    SipStack::FreeAddrSpec(pAddrSpec);
                    return IMS_FALSE;
                }
            }

            // Process URI parameters
            if (bParseParameter)
            {
                m_objParams = SipStack::ExtractParameters(pAddrSpec);
            }
        }
        else
        {
            // Extract the URI scheme
            IMS_SINT32 nIndex = strUri.GetIndexOf(TextParser::CHAR_COLON);

            if (nIndex == AString::NPOS)
            {
                IMS_TRACE_D("SipAddress (%s) does not have a scheme",
                        SipDebug::GetUri1(strAddress).GetStr(), 0, 0);
            }
            else
            {
                m_strScheme = strUri.GetSubStr(0, nIndex);
            }

            IMS_SINT32 nParamIndex = strUri.GetIndexOf(TextParser::CHAR_SEMICOLON, nIndex + 1);
            IMS_SINT32 nHeaderIndex = strUri.GetIndexOf(TextParser::CHAR_QUESTIONMARK, nIndex + 1);

            if (nHeaderIndex != AString::NPOS)
            {
                if (bParseHeader)
                {
                    AString strHeaders = strUri.GetSubStr(nHeaderIndex + 1);

                    RemoveAllHeaderComponents();

                    if (!DecodeHeaderComponent(strHeaders))
                    {
                        IMS_TRACE_E(0, "Parsing URI header parameter (%s) failed",
                                strHeaders.GetStr(), 0, 0);
                    }
                }

                strUri = strUri.GetSubStr(0, nHeaderIndex);

                if (nParamIndex > nHeaderIndex)
                {
                    // No uri-parameters
                    nParamIndex = AString::NPOS;
                }
            }

            m_strHostInfo = strUri.GetSubStr(nIndex + 1, nParamIndex - (nIndex + 1));
            m_strHostInfo = m_strHostInfo.Trim();

            // Extract the uri-parameters if present
            // Process URI parameters
            if (bParseParameter && (nParamIndex != AString::NPOS))
            {
                AString strParams = strUri.GetSubStr(nParamIndex + 1);

                // Parsing the uri-parameters
                ImsList<AString> objTokens = strParams.Split(TextParser::CHAR_SEMICOLON);

                for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
                {
                    SipParameter* pParameter = new SipParameter();

                    if (pParameter == IMS_NULL)
                    {
                        continue;
                    }

                    const AString& strToken = objTokens.GetAt(i);

                    if (!pParameter->Create(strToken))
                    {
                        delete pParameter;

                        IMS_TRACE_E(
                                0, "Parsing SIP Parameter (%s) failed", strToken.GetStr(), 0, 0);
                        continue;
                    }

                    if (!m_objParams.Append(pParameter))
                    {
                        delete pParameter;
                        IMS_TRACE_E(0, "Adding SIP Parameter (%s) failed", strToken.GetStr(), 0, 0);
                    }
                }
            }
        }

        SipStack::FreeAddrSpec(pAddrSpec);
    }
    else
    {
        SipPrivate::SetLastError(SipError::PARSING_ERROR);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipAddress::DecodeHeaderComponent(IN const AString& strHeaders)
{
    return SipStack::DecodeHeaderComponent(strHeaders, m_objHeaders);
}

PRIVATE
IMS_BOOL SipAddress::IsParameterPresent(IN const AString& strName) const
{
    for (IMS_UINT32 i = 0; i < m_objParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objParams.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (strName.EqualsIgnoreCase(pParameter->GetName()))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL SipAddress::CompareNumberDigits(
        IN const AString& strDigits1, IN const AString& strDigits2)
{
    AString strTmpDigits1;
    AString strTmpDigits2;

    for (IMS_SINT32 i = 0; i < strDigits1.GetLength(); ++i)
    {
        IMS_CHAR cDigit = strDigits1[i];

        if (IsVisualSeparator(cDigit))
        {
            continue;
        }

        strTmpDigits1.Append(cDigit);
    }

    for (IMS_SINT32 i = 0; i < strDigits2.GetLength(); ++i)
    {
        IMS_CHAR cDigit = strDigits2[i];

        if (IsVisualSeparator(cDigit))
        {
            continue;
        }

        strTmpDigits2.Append(cDigit);
    }

    return strTmpDigits1.EqualsIgnoreCase(strTmpDigits2);
}

PRIVATE GLOBAL AString SipAddress::EscapeDquotAndBackslash(IN const AString& strValue)
{
    AString strEscapedValue = strValue;

    // strEscapedValue.Replace("\\\"", "\"");
    // strEscapedValue.Replace("\\\\", "\\");
    strEscapedValue.Replace("\\", "\\\\");
    strEscapedValue.Replace("\"", "\\\"");

    return strEscapedValue;
}

PRIVATE GLOBAL IMS_BOOL SipAddress::IsDisplayNameToken(IN const AString& strDisplayName)
{
    IMS_SINT32 nIndex = 0;

    while (nIndex < strDisplayName.GetLength())
    {
        const IMS_CHAR ch = strDisplayName[nIndex];

        // token / HTAB / LF / CR / SP
        if (!IsToken(ch) && (ch != 0x09) && (ch != 0x0A) && (ch != 0x0D) && (ch != 0x20))
        {
            return IMS_FALSE;
        }

        ++nIndex;
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL SipAddress::IsToken(IN const IMS_CHAR ch)
{
    // alphanum / "-" / "." / "!" / "%" / "*" / "_" / "+" / "'" / "`" / "~"

    if (IMS_ISDIGIT(ch) || IMS_ISALPHA(ch) || (ch == '-') || (ch == '.') || (ch == '!') ||
            (ch == '%') || (ch == '*') || (ch == '_') || (ch == '+') || (ch == '\'') ||
            (ch == '`') || (ch == '~'))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL SipAddress::IsVisualSeparator(IN const IMS_CHAR ch)
{
    // "-", ".", "(", ")"
    return (ch == '-') || (ch == '.') || (ch == '(') || (ch == ')');
}
