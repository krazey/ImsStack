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
#include "IpAddress.h"
#include "SipDebug.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipUri::SipUri() :
        m_pszUser(SIP_NULL),
        m_pszPassword(SIP_NULL),
        m_pszHost(SIP_NULL),
        m_nPort(SIP_ZERO),
        m_eHostType(SipAddrSpec::HOST_NAME),
        m_pUriParamList(SIP_NULL),
        m_pUriHdrParamList(SIP_NULL)
{
}

SipUri::SipUri(const SipUri& objSipUri) :
        IParameterComponent(objSipUri),
        m_pszUser(SipPf_Strdup(objSipUri.m_pszUser)),
        m_pszPassword(SipPf_Strdup(objSipUri.m_pszPassword)),
        m_pszHost(SipPf_Strdup(objSipUri.m_pszHost)),
        m_nPort(objSipUri.m_nPort),
        m_eHostType(objSipUri.m_eHostType),
        m_pUriParamList(SIP_NULL),
        m_pUriHdrParamList(SIP_NULL)
{
    if (objSipUri.m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList = new SipParameterList(*(objSipUri.m_pUriParamList));
    }

    if (objSipUri.m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList = new SipParameterList(*(objSipUri.m_pUriHdrParamList));
    }
}

SipUri::~SipUri()
{
    if (m_pszUser != SIP_NULL)
    {
        delete[] m_pszUser;
    }
    if (m_pszPassword != SIP_NULL)
    {
        delete[] m_pszPassword;
    }
    /*Host Port contains host(can be domain name or IP) and port*/
    if (m_pszHost != SIP_NULL)
    {
        delete[] m_pszHost;
    }
    /*List of obj of CSipNameValue*/
    if (m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList->SipDelete();
    }
    /*List of obj of CSipNameValue*/
    if (m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList->SipDelete();
    }
}

SIP_BOOL SipUri::IsValidComponent(const SIP_CHAR* pszComponent) const
{
    if ((SipPf_Stricmp(pszComponent, SIP_USER) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_PASSWORD) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_HOST) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_PORT) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_USER_PRM) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_METHOD) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_MADDR_PRM) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_TTL_PRM) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_TRNSPORT_PRM) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_LR_PRM) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_OTHER_PRM) == 0) ||
            (SipPf_Stricmp(pszComponent, SIP_HEADERS) == 0))
    {
        return SIP_TRUE;
    }

    return SIP_FALSE;
}

SIP_BOOL SipUri::SetUser(const SIP_CHAR* pszUser)
{
    return SetCharVar(pszUser, m_pszUser);
}

SIP_BOOL SipUri::SetPassword(const SIP_CHAR* pszPass)
{
    return SetCharVar(pszPass, m_pszPassword);
}

SipParameterList* SipUri::GetUriParamList()
{
    if (m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList->Increment();
    }
    return m_pUriParamList;
}

SipParameterList* SipUri::GetHdrParamList()
{
    if (m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList->Increment();
    }
    return m_pUriHdrParamList;
}

SIP_VOID SipUri::RemoveHdrParam(const SIP_CHAR* pszName)
{
    if (m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList->RemoveParam(pszName);
    }
}

SIP_BOOL SipUri::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pszUser != SIP_NULL)
    {
        SIP_CHAR* pszTempUser = SipPercentEncoding::DoPerEnc_UserAndHeader(m_pszUser, SIP_USER);
        objBuffer += pszTempUser;
        delete[] pszTempUser;

        if (m_pszPassword != SIP_NULL)
        {
            objBuffer += COLON;

            SIP_CHAR* pszTempPassword = SipPercentEncoding::DoPerEnc_Password(m_pszPassword);
            objBuffer += pszTempPassword;
            delete[] pszTempPassword;
        }

        objBuffer += ATRATE;
    }

    if (m_pszHost != SIP_NULL)
    {
        if (m_eHostType == SipAddrSpec::HOST_IPV6)
        {
            objBuffer += LEFT_SQUARE;
            objBuffer += m_pszHost;
            objBuffer += RIGHT_SQUARE;
        }
        else
        {
            SIP_CHAR* pszTempHost = SipPercentEncoding::DoPerEnc_Host(m_pszHost);
            objBuffer += pszTempHost;
            delete[] pszTempHost;
        }

        if ((m_nPort != SIP_ZERO) && (m_nPort != SIP_UNSPECIFIED_PORT))
        {
            objBuffer += COLON;
            objBuffer += m_nPort;
        }
    }
    else
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Encode: Host value is missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if ((bParams == SIP_TRUE) && (m_pUriParamList != SIP_NULL))
    {
        const_cast<SipUri*>(this)->SetComponentType(IParameterComponent::URI);
        m_pUriParamList->Encode(objBuffer, SIP_SEMI,
                const_cast<IParameterComponent*>(static_cast<const IParameterComponent*>(this)));
    }

    // "?"   header   *( "&"   header )
    if (m_pUriHdrParamList != SIP_NULL)
    {
        SipVector<SipNameValue*>& objHeaders = m_pUriHdrParamList->GetList();

        if (objHeaders.IsEmpty() == SIP_TRUE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encode: No URI header parameters", SIP_ZERO, SIP_ZERO);
            return SIP_TRUE;
        }

        objBuffer += QMARK;

        SIP_UINT32 nSize = objHeaders.GetSize();

        for (SIP_UINT32 i = SIP_ZERO; i < nSize; i++)
        {
            SipNameValue* pNameValue = objHeaders.GetAt(i);

            if (pNameValue != SIP_NULL)
            {
                if (i != SIP_ZERO)
                {
                    objBuffer += AMPERSAND;
                }

                const_cast<SipUri*>(this)->SetComponentType(IParameterComponent::HEADER);
                pNameValue->Encode(objBuffer,
                        const_cast<IParameterComponent*>(
                                static_cast<const IParameterComponent*>(this)));
            }
        }
    }
    const_cast<SipUri*>(this)->SetComponentType(IParameterComponent::NORMAL);

    return SIP_TRUE;
}

SIP_BOOL SipUri::EncodeSipUri(SIP_CHAR** ppCurrPos)
{
    /* encoding of user info
       userinfo = ( user / telephone-subscriber ) [ ":" password ] "@"  */
    if (m_pszUser != SIP_NULL)
    {
        SIP_CHAR* pszTempUser = SipPercentEncoding::DoPerEnc_UserAndHeader(m_pszUser, SIP_USER);
        SipPf_Strcpy(*ppCurrPos, pszTempUser);
        delete[] pszTempUser;

        SipEnc_UpdateCurrPos(ppCurrPos);

        if (m_pszPassword != SIP_NULL)
        {
            /*encode the password*/
            SIP_ENC_COLON(*ppCurrPos);

            SIP_CHAR* pszTempPassword = SipPercentEncoding::DoPerEnc_Password(m_pszPassword);
            SipPf_Strcpy(*ppCurrPos, pszTempPassword);
            delete[] pszTempPassword;

            SipEnc_UpdateCurrPos(ppCurrPos);
        }

        SIP_ENC_ATTHERATE(*ppCurrPos);
    }

    /* encoding of host port
       hostport = host [ ":" port ] */
    if (m_pszHost != SIP_NULL)
    {
        /*Check for the IPV6 and IPV4 */
        if (m_eHostType == SipAddrSpec::HOST_IPV6)
        {
            **ppCurrPos = LEFT_SQUARE;
            (*ppCurrPos)++;

            SipPf_Strcpy(*ppCurrPos, m_pszHost);
            SipEnc_UpdateCurrPos(ppCurrPos);

            **ppCurrPos = RIGHT_SQUARE;
            (*ppCurrPos)++;
        }
        /*Do Percent Encoding if Required*/
        else
        {
            SIP_CHAR* pszTempHost = SipPercentEncoding::DoPerEnc_Host(m_pszHost);
            SipPf_Strcpy(*ppCurrPos, pszTempHost);
            SipEnc_UpdateCurrPos(ppCurrPos);
            delete[] pszTempHost;
        }

        /*Encoding of Port*/
        if ((m_nPort != SIP_ZERO) && (m_nPort != SIP_UNSPECIFIED_PORT))
        {
            const SIP_UINT16 MAX_PORT_LEN = 6;
            SIP_CHAR szTmp[MAX_PORT_LEN] = {'\0'};

            SipPf_Sprintf(szTmp, "%u", m_nPort);
            SIP_ENC_COLON(*ppCurrPos);
            SipPf_Strcpy(*ppCurrPos, szTmp);
            SipEnc_UpdateCurrPos(ppCurrPos);
        }
    }
    else
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeSipUri: Host value Missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*encoding of URI params*/
    if (m_pUriParamList != SIP_NULL)
    {
        SetComponentType(IParameterComponent::URI);
        m_pUriParamList->Encode(ppCurrPos, SIP_SEMI, static_cast<IParameterComponent*>(this));
    }

    /*encoding of Hdr params*/
    // "?"   header   *( "&"   header )
    if (m_pUriHdrParamList != SIP_NULL)
    {
        SetComponentType(IParameterComponent::HEADER);

        SipVector<SipNameValue*>& sipList = m_pUriHdrParamList->GetList();
        if (sipList.IsEmpty() == SIP_TRUE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeSipUri: Empty list", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /*Put a QUESTION MARK*/
        SIP_ENC_QMARK(*ppCurrPos);

        SIP_UINT32 nSize = sipList.GetSize();

        for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
        {
            SipNameValue* pParamNamValue = sipList.GetAt(nCount);

            if (pParamNamValue != SIP_NULL)
            {
                if (nCount != SIP_ZERO)
                {
                    SIP_ENC_AMPERSAND(*ppCurrPos);
                }
                pParamNamValue->Encode(ppCurrPos,
                        const_cast<IParameterComponent*>(
                                static_cast<const IParameterComponent*>(this)));
            }
        }
    }
    SetComponentType(IParameterComponent::NORMAL);

    return SIP_TRUE;
}

SIP_BOOL SipUri::DecUserInfo(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /* check for userinfo = ( user / telephone-subscriber ) [ ":" password ] "@" */

    const SIP_CHAR* pTempPos = SIP_NULL;
    /* Decode password part in userinfo */
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_TRUE)
    {
        const SIP_CHAR* pPasswordStart = pTempPos + SIP_TWO;
        SIP_CHAR* pszPassword = SipCreateString(pPasswordStart, pEndPt);
        if (pszPassword == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /*Do percentage Encoding*/
        m_pszPassword = SipPercentEncoding::DoPercentDecoding(pszPassword);

        pEndPt = pTempPos;
    }
    /* Decode ( user   /   telephone-subscriber ) part in userinfo */
    SIP_CHAR* pszUser = SipCreateString(pStartPt, pEndPt);
    if (pszUser == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    /*Do percentage Encoding*/
    m_pszUser = SipPercentEncoding::DoPercentDecoding(pszUser);

    return SIP_TRUE;
}

SIP_BOOL SipUri::DecHostPort(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*hostport = host [ ":" port ]
      host = hostname   /   IPv4address   /   IPv6reference
      hostname = *( domainlabel   "." )   toplabel   [ "." ]
      domainlabel = alphanum   /   alphanum   *( alphanum   /   "-" )   alphanum
      toplabel = ALPHA   /   ALPHA   *( alphanum   /   "-" )   alphanum
      IPv4address = 1*3DIGIT   "."   1*3DIGIT   "."   1*3DIGIT   "."   1*3DIGIT
      IPv6reference = "["   IPv6address   "]"
      IPv6address = hexpart   [ ":"   IPv4address ]
      hexpart = hexseq   /   hexseq   "::"   [ hexseq ]   /   "::"   [ hexseq ]
      hexseq = hex4   *( ":"   hex4 )
      hex4 = 1*4HEXDIG
      port = 1*DIGIT */

    const SIP_CHAR* pTempPos = SIP_NULL;

    /* IPV6 is enclosed in between '[' and ']', get start and end point of Ipv6 address*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, LEFT_SQUARE) == SIP_TRUE)
    {
        m_eHostType = SipAddrSpec::HOST_IPV6;
        pStartPt = pTempPos + SIP_TWO;
        pTempPos = SIP_NULL;
        if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, RIGHT_SQUARE) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Host[IPV6]", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }
    /* Host : Hostname or IPv4 or IPv6 */
    m_pszHost = SipCreateString(pStartPt, pTempPos);
    if (m_pszHost == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    IpAddress objIpAddr;
    if (objIpAddr.Parse(AString(m_pszHost)) == SIP_TRUE)
    {
        m_eHostType = objIpAddr.IsIPv6Address() ? SipAddrSpec::HOST_IPV6 : SipAddrSpec::HOST_IPV4;
    }
    else if (m_eHostType == SipAddrSpec::HOST_IPV6)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Host[IPV6]", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_eHostType == SipAddrSpec::HOST_NAME)
    {
        m_pszHost = SipPercentEncoding::DoPercentDecoding(m_pszHost);
    }

    pStartPt = pTempPos + SIP_ONE;
    /* Port number */
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_TRUE)
    {
        pTempPos = pTempPos + SIP_TWO;
        SIP_CHAR* pszPort = SipCreateString(pTempPos, pEndPt);
        if (pszPort == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_INT32 nLength = pEndPt - pTempPos + SIP_ONE;

        for (SIP_INT32 nCount = SIP_ZERO; nCount < nLength; nCount++)
        {
            if ((pszPort[nCount] < '0') || (pszPort[nCount] > '9'))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "non numeric port number received",
                        SIP_ZERO, SIP_ZERO);
                delete[] pszPort;
                return SIP_FALSE;
            }
        }

        m_nPort = SipPf_Atoi(pszPort);
        delete[] pszPort;
    }
    else
    {
        m_nPort = SIP_UNSPECIFIED_PORT;
    }
    return SIP_TRUE;
}

SIP_BOOL SipUri::DecodeSipUri(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* SIP URI : sip(s):user:password@host:port;uri-parameters?headers
     * uri-parameters (parameter-name "=" parameter-value pairs) are separated by semi-colons and
     * headers (hname = hvalue pairs) are separated by Ampersand
     */
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPos = SIP_NULL;

    /* Decode user:password part in SIP URI */
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, ATRATE) == SIP_TRUE)
    {
        if (DecUserInfo(pStartPt, pTempPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "User Info Decode Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pStartPt = pTempPos + SIP_TWO;
    }
    /* Decode headers part in SIP URI */
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, QMARK) == SIP_TRUE)
    {
        const SIP_CHAR* pHeaderStart = pTempPos + SIP_TWO;

        m_pUriHdrParamList = new SipParameterList();
        if (m_pUriHdrParamList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SetComponentType(IParameterComponent::HEADER);

        if (m_pUriHdrParamList->Decode(pHeaderStart, pEndPt, AMPERSAND,
                    static_cast<IParameterComponent*>(this)) == SIP_FALSE)
        {
            SetComponentType(IParameterComponent::NORMAL);
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Hdr prm Decode Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        pEndPt = pTempPos;
        pTempPos = SIP_NULL;
    }
    /* Decode uri-parameters part in SIP URI */
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SIP_SEMI) == SIP_TRUE)
    {
        const SIP_CHAR* pUriParamStart = pTempPos + SIP_TWO;

        m_pUriParamList = new SipParameterList();
        if (m_pUriParamList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SetComponentType(IParameterComponent::URI);
        if (m_pUriParamList->Decode(pUriParamStart, pEndPt, SIP_SEMI,
                    static_cast<IParameterComponent*>(this)) == SIP_FALSE)
        {
            SetComponentType(IParameterComponent::NORMAL);
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Uri Prm Decode Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        pEndPt = pTempPos;
    }

    SetComponentType(IParameterComponent::NORMAL);
    /* Decode host:port part in SIP URI */
    if (DecHostPort(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Host port Decode Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

const SIP_CHAR* SipUri::GetSchemeString(SipUri::UriType eUriType)
{
    switch (eUriType)
    {
        case SipUri::SCHEME_SIP:
            return "sip";
        case SipUri::SCHEME_SIPS:
            return "sips";
        default:
            return SIP_NULL;
    }
}

SipAddrSpec::SipAddrSpec() :
        m_eUriType(SipUri::SCHEME_SIP),
        m_pSipUri(SIP_NULL),
        m_pszAbsUri(SIP_NULL)
{
}

SipAddrSpec::SipAddrSpec(const SipAddrSpec& objAddressSpec) :
        m_eUriType(objAddressSpec.m_eUriType),
        m_pSipUri(SIP_NULL),
        m_pszAbsUri(SipPf_Strdup(objAddressSpec.m_pszAbsUri))
{
    if (objAddressSpec.m_pSipUri != SIP_NULL)
    {
        m_pSipUri = new SipUri(*(objAddressSpec.m_pSipUri));
    }
}

SipAddrSpec::~SipAddrSpec()
{
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->SipDelete();
    }
    if (m_pszAbsUri != SIP_NULL)
    {
        delete[] m_pszAbsUri;
    }
}

SIP_BOOL SipAddrSpec::SetAbsUri(const SIP_CHAR* pszSipUri)
{
    return SetCharVar(pszSipUri, m_pszAbsUri);
}

SipUri* SipAddrSpec::GetSipUri()
{
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->Increment();
    }
    return m_pSipUri;
}

SIP_BOOL SipAddrSpec::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pSipUri != SIP_NULL)
    {
        const SIP_CHAR* pStrUri = SipUri::GetSchemeString(m_eUriType);

        if (pStrUri != SIP_NULL)
        {
            objBuffer += pStrUri;
            objBuffer += COLON;
        }

        if (m_pSipUri->Encode(objBuffer, bParams) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "SipUri: Encoding error", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (m_pszAbsUri != SIP_NULL)
    {
        objBuffer += m_pszAbsUri;
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No URI for encoding", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipAddrSpec::EncodeAddrSpec(SIP_CHAR** ppCurrPos) const
{
    if (m_pSipUri != SIP_NULL)
    {
        /*encoding of uri name*/
        const SIP_CHAR* pStrUri = SipUri::GetSchemeString(m_eUriType);

        if (pStrUri != SIP_NULL)
        {
            SipPf_Strcpy(*ppCurrPos, pStrUri);
            SipEnc_UpdateCurrPos(ppCurrPos);
            SIP_ENC_COLON(*ppCurrPos);
        }

        SipEnc_UpdateCurrPos(ppCurrPos);

        if (m_pSipUri->EncodeSipUri(ppCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Uri Encoding error", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (m_pszAbsUri != SIP_NULL)
    {
        SipPf_Strcpy(*ppCurrPos, m_pszAbsUri);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No Uri set for encoding", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SIP_BOOL SipAddrSpec::DecodeAddrSpec(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Validate the input prm*/
    if ((nDecLen == SIP_ZERO) || (pStartPt == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalide Input prm", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "No URI scheme", SIP_ZERO, SIP_ZERO);
        m_eUriType = SipUri::SCHEME_ABS;
    }
    else
    {
        m_eUriType = SipGetUriType(pStartPt, pTempPos);
    }

    /*CAse of Sip or Sips URI*/
    if ((m_eUriType == SipUri::SCHEME_SIP) || (m_eUriType == SipUri::SCHEME_SIPS))
    {
        /*Set the Start point after COLON*/
        pStartPt = pTempPos + SIP_TWO;
        /*Update the length of buffer*/
        nDecLen = pEndPt - pStartPt + SIP_ONE;

        SipUri* pSipUri = new SipUri();
        if (pSipUri == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipAddrSpec::DecodeAddrSpec: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Set PercentEncoding Value*/

        /*Decode the sip uri*/
        if (pSipUri->DecodeSipUri(pStartPt, nDecLen) == SIP_FALSE)
        {
            pSipUri->SipDelete();
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipAddrSpec::DecodeAddrSpec: Sip Uri Decode Failed", SIP_ZERO, SIP_ZERO);

            return SIP_FALSE;
        }
        m_pSipUri = pSipUri;
    }
    else
    {
        m_pszAbsUri = SipCreateString(pStartPt, pEndPt);
        if (m_pszAbsUri == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipNameAddr::SipNameAddr() :
        m_pszDispName(SIP_NULL),
        m_pAddrSpec(SIP_NULL)
{
}

SipNameAddr::SipNameAddr(const SipNameAddr& objNameAddr) :
        m_pszDispName(SipPf_Strdup(objNameAddr.m_pszDispName)),
        m_pAddrSpec(SIP_NULL)
{
    if (objNameAddr.m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec = new SipAddrSpec(*(objNameAddr.m_pAddrSpec));
    }
}

SipNameAddr::~SipNameAddr()
{
    if (m_pszDispName != SIP_NULL)
    {
        delete[] m_pszDispName;
    }
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->SipDelete();
    }
}

SipAddrSpec* SipNameAddr::GetAddrSpec()
{
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->Increment();
    }
    return m_pAddrSpec;
}
SIP_BOOL SipNameAddr::SetAddrSpec(SipAddrSpec* pSipAddrSpec)
{
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->SipDelete();
    }
    m_pAddrSpec = pSipAddrSpec;
    return SIP_TRUE;
}

SIP_BOOL SipNameAddr::SetDisplayName(const SIP_CHAR* pszDisplayName)
{
    return SetCharVar(pszDisplayName, m_pszDispName);
}

SIP_BOOL SipNameAddr::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pAddrSpec == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No addr-spec", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pszDispName != SIP_NULL)
    {
        objBuffer += m_pszDispName;

        // FIX_MESSAGE_ENCODING_OPERATION
        //  Add LWS between the display name and left angle quote ('<').
        //  First, check the display name if it has a double quotation.
        //  If present, just do normal procedure. Else, add the display name and space.
        //  But, we will always add the space after the display name.
        objBuffer += SPACE;
    }

    objBuffer += LEFT_ANGLE;

    if (m_pAddrSpec->Encode(objBuffer, bParams) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encoding addr-spec failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += RIGHT_ANGLE;

    return SIP_TRUE;
}

SIP_BOOL SipNameAddr::EncodeNameAddr(SIP_CHAR** ppCurrPos)
{
    if (m_pAddrSpec == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No Addr Spec", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pszDispName != SIP_NULL)
    {
        SipPf_Strcpy(*ppCurrPos, m_pszDispName);
        SipEnc_UpdateCurrPos(ppCurrPos);

        // FIX_MESSAGE_ENCODING_OPERATION
        //  Add LWS between the display name and left angle quote ('<').
        //  First, check the display name if it has a double quotation.
        //  If present, just do normal procedure. Else, add the display name and space.
        //  But, we will always add the space after the display name.
        SIP_ENC_SP(*ppCurrPos);
    }

    SIP_ENC_LAQUOT(*ppCurrPos);

    if (m_pAddrSpec->EncodeAddrSpec(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Addr Spec failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_ENC_RAQUOT(*ppCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipNameAddr::DecodeNameAddr(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    if (pStartPt == pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "NameAddr missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, LEFT_ANGLE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Left Angle Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    /*Case of display Name present*/
    if (pStartPt <= pTempPre)
    {
        m_pszDispName = SipCreateString(pStartPt, pTempPre);
        if (m_pszDispName == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    /*Now Decode the Addr Spec*/
    m_pAddrSpec = new SipAddrSpec();
    if (m_pAddrSpec == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get the length of address spec*/
    SIP_UINT32 nDecLen = pEndPt - pTempNext + SIP_ONE;

    if (m_pAddrSpec->DecodeAddrSpec(pTempNext, nDecLen) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Addr Spec decoding failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
