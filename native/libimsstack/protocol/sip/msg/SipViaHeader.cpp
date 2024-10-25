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
#include "msg/SipViaHeader.h"
#include "platform/SipString.h"

SipViaHeader::SipViaHeader() :
        SipHeaderBase(SipHeaderBase::VIA),
        m_pszProtocolName(SIP_NULL),
        m_pszProtocolVer(SIP_NULL),
        m_pszTransport(SIP_NULL),
        m_pszHost(SIP_NULL),
        m_nPort(SIP_ZERO),
        m_eHostType(SipAddrSpec::HOST_NAME)
{
}

SipViaHeader::SipViaHeader(const SipViaHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszProtocolName(SipPf_Strdup(objHeader.GetProtocolName())),
        m_pszProtocolVer(SipPf_Strdup(objHeader.GetProtocolVer())),
        m_pszTransport(SipPf_Strdup(objHeader.GetTransport())),
        m_pszHost(SipPf_Strdup(objHeader.GetHost())),
        m_nPort(objHeader.GetPort()),
        m_eHostType(objHeader.m_eHostType)
{
}

SipViaHeader::~SipViaHeader()
{
    if (m_pszProtocolName != SIP_NULL)
    {
        delete[] m_pszProtocolName;
    }
    if (m_pszProtocolVer != SIP_NULL)
    {
        delete[] m_pszProtocolVer;
    }
    if (m_pszTransport != SIP_NULL)
    {
        delete[] m_pszTransport;
    }
    if (m_pszHost != SIP_NULL)
    {
        delete[] m_pszHost;
    }
}

SIP_BOOL SipViaHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pszProtocolName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing protocol", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszProtocolName;
    objBuffer += SLASH;

    if (m_pszProtocolVer == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing protocol version", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszProtocolVer;
    objBuffer += SLASH;

    if (m_pszTransport == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing transport protocol", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszTransport;
    objBuffer += SPACE;

    // sent-by = host [ COLON port ]
    if (m_pszHost == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing host", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // IPV6: m_pszHost already includes the square brackets.
    objBuffer += m_pszHost;

    if (m_nPort != SIP_ZERO)
    {
        objBuffer += COLON;
        objBuffer += m_nPort;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipViaHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    /* Encoding of header value*/
    /*Encode sent protocol
      sent-protocol = protocol-name SLASH protocol-version SLASH transport */
    /*protocol name*/
    if (m_pszProtocolName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Protocol ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszProtocolName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SipMsgUtil::Encode(*ppCurrPos, SLASH);

    /*protocol-version*/
    if (m_pszProtocolVer == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Protocol Version ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    SipPf_Strcpy(*ppCurrPos, m_pszProtocolVer);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SipMsgUtil::Encode(*ppCurrPos, SLASH);

    /*transport*/
    if (m_pszTransport == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Transport ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    SipPf_Strcpy(*ppCurrPos, m_pszTransport);
    SipEnc_UpdateCurrPos(ppCurrPos);

    /*put a space for LWS*/
    SipMsgUtil::Encode(*ppCurrPos, SPACE);

    /*Encode sent by*/
    /*sent-by = host [ COLON port ] */
    if (m_pszHost == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Host ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // In case of IPv6 - Left Square and Right Square Bracket already included in Host
    SipPf_Strcpy(*ppCurrPos, m_pszHost);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_nPort != SIP_ZERO)
    {
        const SIP_UINT16 MAX_PORT_LEN = 6;
        SIP_CHAR szTmp[MAX_PORT_LEN];
        SipPf_Sprintf(szTmp, "%u", m_nPort);

        SipMsgUtil::Encode(*ppCurrPos, COLON);

        SipPf_Strcpy(*ppCurrPos, szTmp);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_VOID SipViaHeader::SetProtocolName(const SIP_CHAR* pszProtocolName)
{
    SipMsgUtil::SetValue(pszProtocolName, m_pszProtocolName);
}

SIP_VOID SipViaHeader::SetProtocolVer(const SIP_CHAR* pszProtocolVer)
{
    SipMsgUtil::SetValue(pszProtocolVer, m_pszProtocolVer);
}

SIP_VOID SipViaHeader::SetTransport(const SIP_CHAR* pszTransport)
{
    SipMsgUtil::SetValue(pszTransport, m_pszTransport);
}

SIP_VOID SipViaHeader::SetHost(const SIP_CHAR* pszHost)
{
    SipMsgUtil::SetValue(pszHost, m_pszHost);
}

const SIP_CHAR* SipViaHeader::GetBranch() const
{
    return GetParamValue("branch");
}

SIP_BOOL SipViaHeader::SetBranchParam(const SIP_CHAR* pszBranch)
{
    return AddParam("branch", pszBranch);
}

SIP_BOOL SipViaHeader::DecodeHostPort(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*hostport = host [ COLON port ]
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

    const SIP_CHAR* pTempPre = SIP_NULL;
    /*check for IPV6 address*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, LEFT_SQUARE) == SIP_TRUE)
    {
        m_eHostType = SipAddrSpec::HOST_IPV6;
        pStartPt = pTempPre + SIP_ONE;
        pTempPre = SIP_NULL;
        if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, RIGHT_SQUARE) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Host[IPV6]", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTempPre = pTempPre + SIP_ONE;
    }
    else if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, COLON) == SIP_FALSE)
    {
        pTempPre = pEndPt;
    }

    m_pszHost = SipCreateString(pStartPt, pTempPre);
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

    pStartPt = pTempPre;
    pTempPre = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, COLON) == SIP_TRUE)
    {
        pTempPre = pTempPre + SIP_TWO;
        SIP_CHAR* pszPort = SipCreateString(pTempPre, pEndPt);
        if (pszPort == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        m_nPort = SipPf_Atoi(pszPort);
        delete[] pszPort;
    }

    return SIP_TRUE;
}

SIP_BOOL SipViaHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

    /*Search for the Protocol Name End*/
    /*sent-protocol = protocol-name SLASH protocol-version SLASH transport */
    /*Find First SLASH with Skipped LWS from both side*/
    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SLASH) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Protocol Name Missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    m_pszProtocolName = SipCreateString(pStartPt, pTempPre);
    if (m_pszProtocolName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the Start Point to the Start of protocol version
      i.e. to the right of "/" (LWS Skipped) */
    pStartPt = pTempNext;
    pTempPre = SIP_NULL;
    pTempNext = SIP_NULL;

    /*Find Next SLASH with Skipped LWS from both side*/
    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SLASH) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Protocol Version Missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    m_pszProtocolVer = SipCreateString(pStartPt, pTempPre);
    if (m_pszProtocolVer == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the Start Point to the Start of Transport*/
    pStartPt = pTempNext;
    pTempPre = SIP_NULL;
    pTempNext = SIP_NULL;

    /*Find the LWS i.e. End of Transport*/
    if (SipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: LWS missing in Via", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    m_pszTransport = SipCreateString(pStartPt, pTempPre);
    if (m_pszTransport == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipViaHeader::DecodeHdr: Memory Allocation Failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Skip Fw LWS And Get the Start of Sent by
      i.e. sent-by = host [ COLON port ]  */
    pTempPre = pTempPre + SIP_ONE;
    pStartPt = SipSkipFwLWS(pTempPre, pEndPt);
    pTempPre = SIP_NULL;

    /*Now check for the Via Prm*/
    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_FALSE)
    {
        pTempPre = pEndPt;
    }

    if (DecodeHostPort(pStartPt, pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr: Host Port Decoding Fail in via",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTempNext != SIP_NULL)
    {
        return DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI);
    }

    return SIP_TRUE;
}

SIP_BOOL SipViaHeader::IsValidHeader() const
{
    if ((m_pszProtocolName == SIP_NULL) || (m_pszProtocolVer == SIP_NULL) ||
            (m_pszTransport == SIP_NULL) || (m_pszHost == SIP_NULL))
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipViaHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipViaHeader(*reinterpret_cast<SipViaHeader*>(pHeader));
    }
    return new SipViaHeader();
}
