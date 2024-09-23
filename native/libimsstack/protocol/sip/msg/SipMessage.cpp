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
#include "SipDatatypes.h"
#include "SipDebug.h"
#include "msg/SipMessage.h"
#include "msg/SipMsgBody.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipMemory.h"
#include "platform/SipString.h"

SipMessage::SipMessage() :
        m_eSipMsgType(SipMessage::TYPE_INVALID),
        m_pReqLine(SIP_NULL),
        m_pStatusLine(SIP_NULL),
        m_objHdrs(SIP_NULL),
        m_pMsgBodyList(SIP_NULL)
#ifdef SIP_BADMESSAGE_PARSING
        ,
        mbitMask(MANDATORY_HDR_NONE),
        m_pBadHdrList(SIP_NULL)
#endif
{
    m_objHdrs = new SipHeaders();
}

SipMessage::SipMessage(SIP_INT32 eSipMsgType) :
        m_eSipMsgType(eSipMsgType),
        m_pReqLine(SIP_NULL),
        m_pStatusLine(SIP_NULL),
        m_objHdrs(SIP_NULL),
        m_pMsgBodyList(SIP_NULL)
#ifdef SIP_BADMESSAGE_PARSING
        ,
        mbitMask(MANDATORY_HDR_NONE),
        m_pBadHdrList(SIP_NULL)
#endif
{
    m_objHdrs = new SipHeaders();
}

SipMessage::SipMessage(const SipMessage& objSipMsg) :
        m_eSipMsgType(objSipMsg.m_eSipMsgType),
        m_pReqLine(SIP_NULL),
        m_pStatusLine(SIP_NULL),
        m_objHdrs(SIP_NULL),
        m_pMsgBodyList(SIP_NULL)
#ifdef SIP_BADMESSAGE_PARSING
        ,
        mbitMask(objSipMsg.mbitMask),
        m_pBadHdrList(SIP_NULL)
#endif
{
    m_objHdrs = new SipHeaders();

    m_objHdrs->CopyHdrs(objSipMsg.m_objHdrs);

    if (objSipMsg.m_pReqLine != SIP_NULL)
    {
        m_pReqLine = new SipRequestLine(*(objSipMsg.m_pReqLine));
    }

    if (objSipMsg.m_pStatusLine != SIP_NULL)
    {
        m_pStatusLine = new SipStatusLine(*(objSipMsg.m_pStatusLine));
    }

    if (objSipMsg.m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList = new SipMsgBodyList(*(objSipMsg.m_pMsgBodyList));
    }

#ifdef SIP_BADMESSAGE_PARSING
    if (objSipMsg.m_pBadHdrList != SIP_NULL)
    {
        m_pBadHdrList = new SipHeaderList(SipHeaderBase::TYPE_INVALID);

        SIP_UINT32 nSize = objSipMsg.m_pBadHdrList->GetSize();

        for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
        {
            SipHeaderBase* pHdr = objSipMsg.m_pBadHdrList->GetObj(nCount);

            if (pHdr != SIP_NULL)
            {
                SipBadHeader* pBadHdr = new SipBadHeader(*(static_cast<SipBadHeader*>(pHdr)));
                m_pBadHdrList->AddHeader(pBadHdr);
                pBadHdr->SipDelete();
                pHdr->SipDelete();
            }
        }
    }
#endif
}

SipMessage::~SipMessage()
{
    if (m_pReqLine != SIP_NULL)
    {
        m_pReqLine->SipDelete();
    }
    if (m_pStatusLine != SIP_NULL)
    {
        m_pStatusLine->SipDelete();
    }
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
    }
    if (m_objHdrs != SIP_NULL)
    {
        delete m_objHdrs;
    }
#ifdef SIP_BADMESSAGE_PARSING
    if (m_pBadHdrList != SIP_NULL)
    {
        m_pBadHdrList->SipDelete();
    }
#endif
}

SIP_VOID SipMessage::SetRequestline(SipRequestLine* pReqLine)
{
    if (m_pReqLine != SIP_NULL)
    {
        m_pReqLine->SipDelete();
    }

    m_pReqLine = pReqLine;
}

SIP_BOOL SipMessage::RemoveHdr(SIP_INT32 eHdrType)
{
    return m_objHdrs->RemoveHdr(eHdrType);
}

SIP_BOOL SipMessage::SetHeader(SipHeaderBase* pHdr)
{
    if (pHdr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "set header null", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_BOOL bStatus = SIP_FALSE;
    SIP_INT32 nType = pHdr->GetHdrType();

    if (nType == SipHeaderBase::UNKNOWN)
    {
        bStatus = m_objHdrs->AppendHdr(pHdr);
    }
    else
    {
        bStatus = m_objHdrs->SetHdr(pHdr);
    }

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "set header type %d Fail", nType, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipMessage::SetMessageBody(SipMsgBody* pMsgBody)
{
    if (pMsgBody == SIP_NULL)
    {
        return SIP_FALSE;
    }

    if (m_pMsgBodyList == SIP_NULL)
    {
        m_pMsgBodyList = new SipMsgBodyList();
    }
    return m_pMsgBodyList->AddBody(pMsgBody);
}

SIP_VOID SipMessage::SetStatusLine(SipStatusLine* pStatusLine)
{
    if (m_pStatusLine != SIP_NULL)
    {
        m_pStatusLine->SipDelete();
    }

    m_pStatusLine = pStatusLine;
}

SIP_BOOL SipMessage::AppendHeader(SipHeaderBase* pHdr)
{
    if (m_objHdrs->AppendHdr(pHdr) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "AppendHeader Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SIP_BOOL SipMessage::InsertHeader(SipHeaderBase* pHdr, SIP_UINT32 nIndex)
{
    if (m_objHdrs->InsertHdr(pHdr, nIndex) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "AppendHeader Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SIP_INT32 SipMessage::GetMethodType()
{
    const SIP_CHAR* pszMethod = GetMethod();

    if (pszMethod == SIP_NULL)
    {
        return SipMessage::METHOD_INVALID;
    }

    SIP_CHAR SipMethodArray[][SIP_MAX_HDR_LEN] = {
            "INVITE",
            "ACK",
            "OPTIONS",
            "BYE",
            "CANCEL",
            "REGISTER",
            "INFO",
            "PRACK",
            "SUBSCRIBE",
            "NOTIFY",
            "UPDATE",
            "MESSAGE",
            "REFER",
            "PUBLISH",
    };

    for (SIP_INT16 nIndex = SIP_ZERO; nIndex < SipMessage::METHOD_END; nIndex++)
    {
        if (SipPf_Stricmp(pszMethod, SipMethodArray[nIndex]) == SIP_ZERO)
        {
            return nIndex;
        }
    }
    return SipMessage::METHOD_UNKNOWN;
}

const SIP_CHAR* SipMessage::GetMethod()
{
    if (SipMessage::REQ_TYPE == m_eSipMsgType)
    {
        if (m_pReqLine == SIP_NULL)
        {
            return SIP_NULL;
        }
        return m_pReqLine->GetMethod();
    }

    if (m_pStatusLine == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipCSeqHeader* pCseqHdr = static_cast<SipCSeqHeader*>(GetHdrObj(SipHeaderBase::CSEQ));
    if (pCseqHdr == SIP_NULL)
    {
        return SIP_NULL;
    }

    const SIP_CHAR* pszMethod = pCseqHdr->GetMethod();
    pCseqHdr->SipDelete();

    return pszMethod;
}

SipHeaderBase* SipMessage::GetHdrObj(SIP_INT32 eHdrType)
{
    return m_objHdrs->GetHdrObj(eHdrType, SIP_ZERO);
}

SIP_BOOL SipMessage::HasHeader(SIP_INT32 eHdrType) const
{
    SipHeaderBase* pHdrBase = m_objHdrs->GetHdrObj(eHdrType);
    if (pHdrBase != SIP_NULL)
    {
        pHdrBase->SipDelete();
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipMessage::IsReqLineExists()
{
    SipRequestLine* pReqLine = GetReqLine();
    if (pReqLine != SIP_NULL)
    {
        pReqLine->SipDelete();
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipMessage::IsStatusLineExists()
{
    SipStatusLine* pStatusLine = GetStatusLine();
    if (pStatusLine != SIP_NULL)
    {
        pStatusLine->SipDelete();
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

#ifdef SIP_BADMESSAGE_PARSING
SIP_BOOL SipMessage::HasMandatoryHdrs()
{
    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "MANDATORY_HDR_MASK = %x", mbitMask, SIP_ZERO);

    if (mbitMask != MANDATORY_HDR_MASK)
    {
        return SIP_FALSE;
    }

    if (IsReqLineExists() == SIP_TRUE)
    {
        const SIP_CHAR* pszMethod = GetMethod();

        if ((SipPf_Stricmp(pszMethod, "PRACK") == 0) &&
                ((HasHeader(SipHeaderBase::RACK) == SIP_FALSE)))
        {
            return SIP_FALSE;
        }

        SipRequestLine* pReqLine = GetReqLine();
        if (pReqLine == SIP_NULL)
        {
            return SIP_FALSE;
        }

        const SIP_CHAR* pSipVer = pReqLine->GetSipVersion();
        SipAddrSpec* pReqUri = pReqLine->GetReqUri();
        if (((pReqUri != SIP_NULL) && (pszMethod != SIP_NULL) && (pSipVer != SIP_NULL)) ==
                SIP_FALSE)
        {
            pReqLine->SipDelete();
            if (pReqUri != SIP_NULL)
            {
                pReqUri->SipDelete();
            }
            return SIP_FALSE;
        }
        pReqLine->SipDelete();
        pReqUri->SipDelete();
    }
    return SIP_TRUE;
}

SIP_INT32 SipMessage::GetBadHeaderCount() const
{
    if (m_pBadHdrList != SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Bad header count = %d", m_pBadHdrList->GetSize(), SIP_ZERO);
        return m_pBadHdrList->GetSize();
    }
    return 0;
}

SipHeaderList* SipMessage::GetBadHdrs()
{
    return m_pBadHdrList;
}

SIP_VOID SipMessage::DeleteBadHdrList()
{
    if (m_pBadHdrList != SIP_NULL)
    {
        m_pBadHdrList->SipDelete();
        m_pBadHdrList = SIP_NULL;
    }
}
#endif

SIP_BOOL SipMessage::HasMIMEMessageBody()
{
    SipContentTypeHeader* pHdr =
            static_cast<SipContentTypeHeader*>(GetHdrObj(SipHeaderBase::CONTENT_TYPE));

    if (pHdr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszMType = pHdr->GetMediaType();
    SIP_INT16 nResult = SipPf_Stricmp(pszMType, MULTIPART);
    pHdr->SipDelete();
    return (nResult == SIP_ZERO) ? SIP_TRUE : SIP_FALSE;
}

SIP_BOOL SipMessage::HasSDPMessageBody()
{
    SipContentTypeHeader* pHdr =
            static_cast<SipContentTypeHeader*>(GetHdrObj(SipHeaderBase::CONTENT_TYPE));

    if (pHdr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszSubMType = pHdr->GetSubMediaType();
    SIP_INT16 nResult = SipPf_Stricmp(pszSubMType, SDP);
    pHdr->SipDelete();
    return (nResult == SIP_ZERO) ? SIP_TRUE : SIP_FALSE;
}

SipHeaderList* SipMessage::GetHdrList(SIP_INT32 eHdrType)
{
    return (SipHeaders::IsListHdr(eHdrType) == SIP_TRUE)
            ? static_cast<SipHeaderList*>(m_objHdrs->GetHdrObj(eHdrType))
            : SIP_NULL;
}

SIP_BOOL SipMessage::SetHdrList(SipHeaderList* pHdrList)
{
    SIP_UINT32 nListSize = pHdrList->GetSize();
    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nListSize; nIndex++)
    {
        SipHeaderBase* pHdrbase = pHdrList->GetObj(nIndex);
        if (pHdrbase != SIP_NULL)
        {
            SIP_BOOL bStatus = m_objHdrs->AppendHdr(pHdrbase);
            pHdrbase->SipDelete();
            if (bStatus == SIP_FALSE)
            {
                return SIP_FALSE;
            }
        }
    }
    return SIP_TRUE;
}

SIP_BOOL SipMessage::AppendMessageBody(SipMsgBody* pMsgBody)
{
    return SetMessageBody(pMsgBody);
}

SipMsgBody* SipMessage::GetMsgBody(SIP_UINT32 nPos)
{
    return (m_pMsgBodyList != SIP_NULL) ? m_pMsgBodyList->GetBodyByIndex(nPos) : SIP_NULL;
}

SIP_BOOL SipMessage::Encode(SIP_CHAR** ppSipMsgBuffer, /* in-out parameter*/
        SIP_UINT32* pSipMsgLength, /* in-out parameter*/ SIP_UINT32 nMsgOptions)
{
    SIP_CHAR* pCurrPos = *ppSipMsgBuffer;

    if ((m_eSipMsgType == SipMessage::REQ_TYPE) && (m_pReqLine != SIP_NULL))
    {
        if (m_pReqLine->EncodeRequestLine(&pCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encoding Request line Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if ((m_eSipMsgType == SipMessage::RESP_TYPE) && (m_pStatusLine != SIP_NULL))
    {
        if (m_pStatusLine->EncodeStatusLine(&pCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encoding status line Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Start Line Missing or invalid message type",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Put CRLF at the end of Start Line
    SIP_ENC_CRLF(pCurrPos);

    SIP_CHAR aMsgBody[SIP_MAX_MSG_SIZE] = {
            0,
    };
    SIP_CHAR* pMsgBody = &(aMsgBody[0]);
    SIP_UINT32 nMsgLen = 0;

    if (this->m_pMsgBodyList != SIP_NULL)
    {
        /*Set the content type and content length headers before header encoding*/
        if (EncodeMsgBodyAndUpdateContentHdrs(nMsgOptions, &pMsgBody, nMsgLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "updating content headers fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        SipUnknownHeader* pUnknown = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
        if (pUnknown == SIP_NULL)
        {
            SetContentLengthHdr(SIP_ZERO, nMsgOptions);
        }
        else
        {
            // Content-Length header is present in message, reset the value to ZERO as no
            // body in the message.
            SIP_CHAR szLen[SIP_CONTLEN_LEN] = {0};

            SipPf_Sprintf(szLen, "%d", 0);
            pUnknown->SetHeaderValue(szLen);

            pUnknown->SipDelete();
            AdjustContentLengthHdr();
        }
    }

    // Encoding of headers
    if (m_objHdrs->EncodeHdrs(&pCurrPos, nMsgOptions) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Headers Encoding Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Put CRLF at the end of List of Headers
    SIP_ENC_CRLF(pCurrPos);

    // Append Message body if present
    if (m_pMsgBodyList != SIP_NULL)
    {
        pMsgBody = &(aMsgBody[0]);
        SipPf_Memcpy(pCurrPos, pMsgBody, nMsgLen);
        pCurrPos += nMsgLen;
    }

    // Calculate the length of the buffer
    SIP_UINT32 nMsgLength = pCurrPos - *ppSipMsgBuffer;

    *pSipMsgLength = nMsgLength;

    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encoded Message of length(%d)", nMsgLength, SIP_ZERO);

    return SIP_TRUE;
}

SIP_BOOL SipMessage::EncodeMsgBodyAndUpdateContentHdrs(
        SIP_UINT32 nMsgOptions, SIP_CHAR** ppMsgBody, SIP_UINT32& nMsgLen)
{
    /*check for content type header */
    if (HasHeader(SipHeaderBase::CONTENT_TYPE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    /*Check the content type from message*/
    SipContentTypeHeader* pContentType =
            static_cast<SipContentTypeHeader*>(GetHdrObj(SipHeaderBase::CONTENT_TYPE));

    if (pContentType != SIP_NULL)
    {
        /*Get the boundary*/
        SIP_CHAR* pszBoundary = pContentType->GetBoundary();
        pContentType->SipDelete();

        if (m_pMsgBodyList->GetEncodedMessageBody(ppMsgBody, nMsgLen, pszBoundary) == SIP_FALSE)
        {
            if (pszBoundary != SIP_NULL)
            {
                delete[] pszBoundary;
            }
            return SIP_FALSE;
        }

        if (pszBoundary != SIP_NULL)
        {
            delete[] pszBoundary;
        }
    }

    /*Is content-length already present*/
    SipUnknownHeader* pUnknown = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
    if (pUnknown == SIP_NULL)
    { /*if not present add new*/
        SetContentLengthHdr(nMsgLen, nMsgOptions);
    }
    else
    { /*If present validate and update*/
        SIP_INT32 nCurLen = 0;
        const SIP_CHAR* pCurLen = pUnknown->GetHeaderValue();

        if (pCurLen != SIP_NULL)
        {
            nCurLen = SipPf_Atoi(pCurLen);
        }

        if (nMsgLen != nCurLen)
        { /*Mismatch in content length & actual body size*/
            SIP_CHAR szLen[SIP_CONTLEN_LEN] = {0};
            SipPf_Sprintf(szLen, "%u", nMsgLen);
            pUnknown->SetHeaderValue(szLen);
        }

        pUnknown->SipDelete();

        AdjustContentLengthHdr();
    }

    return SIP_TRUE;
}

SIP_BOOL SipMessage::DecodeMultiPartBody(
        const SIP_CHAR* pBuffStart, const SIP_CHAR* pBuffEnd, SIP_UINT32 nMsgBuffLen)
{
    (void)nMsgBuffLen;  // unused as of now

    // If body list already present, free it
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
    }

    m_pMsgBodyList = new SipMsgBodyList();
    if (m_pMsgBodyList == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipContentTypeHeader* pContentType =
            static_cast<SipContentTypeHeader*>(m_objHdrs->GetHdrObj(SipHeaderBase::CONTENT_TYPE));
    if (pContentType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Content Type Not present", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszBoundary = pContentType->GetBoundary();
    pContentType->SipDelete();
    if (pszBoundary == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "No boundary in Content Type Hdr", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pMsgBodyList->DecodeMIMEBody(pBuffStart, pBuffEnd, pszBoundary) != SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEBody Fail", SIP_ZERO, SIP_ZERO);
        delete[] pszBoundary;
        return SIP_FALSE;
    }

    delete[] pszBoundary;

    return SIP_TRUE;
}

SIP_BOOL SipMessage::DecodeFragmentMsg(const SIP_CHAR* pMsgBuff, SIP_UINT32 nMsgBuffLen)
{
    if ((pMsgBuff == SIP_NULL) || (nMsgBuffLen == SIP_ZERO))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeFragmentMsg:empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pStartPt = pMsgBuff;
    const SIP_CHAR* pEndPt = pMsgBuff + nMsgBuffLen;

    // Remove additional CRLF if present at the starting of incoming message.
    while ((pStartPt < pEndPt) && (IS_CR(*pStartPt) && IS_LF(*(pStartPt + SIP_ONE))))
    {
        pStartPt = pStartPt + SIP_TWO;
    }

    if (pStartPt >= pEndPt)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Invalid SIP message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = SipSkipFwLWS(pStartPt, pEndPt);

    if (pStartPt >= pEndPt)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Invalid SIP message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get the buffer till first terminating CRLF
      In case of Request  it should be request line
      In case of Response it should be status line*/
    SIP_BOOL bHdrEnd = SIP_FALSE;
    const SIP_CHAR* pTempPos = SIP_NULL;

    if (SipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeFragmentMsg:no CRLF found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nDecLen = pTempPos - pStartPt + SIP_ONE;

    // STATUS LINE : sip version present at the start.
    if (SipPf_Strncmp(SIP_SIPVER, pStartPt, SIP_FOUR) == 0)
    {
        m_pStatusLine = new SipStatusLine();
        if (m_pStatusLine == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Memory Allocation fail",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pStatusLine->DecodeStatusLine(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Status Line Decoding fail",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        this->m_eSipMsgType = SipMessage::RESP_TYPE;
    }
    else
    {  // REQUEST LINE : If sip version present at the last.
        const SIP_CHAR* pSipVersion = pTempPos;

        while ((*pSipVersion != SPACE) && (pStartPt <= pSipVersion))
        {
            pSipVersion = pSipVersion - SIP_ONE;
        }

        if (*pSipVersion == SPACE)
        {
            pSipVersion = pSipVersion + SIP_ONE;

            if (SipPf_Strncmp(SIP_SIPVER, pSipVersion, SIP_FOUR) == 0)
            {
                m_pReqLine = new SipRequestLine();
                if (m_pReqLine == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "DecodeFragmentMsg:Memory Allocation fail", SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }

                if (m_pReqLine->DecodeRequestLine(pStartPt, nDecLen) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "DecodeFragmentMsg:Req Line Decoding fail", SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }

                this->m_eSipMsgType = SipMessage::REQ_TYPE;
            }
        }
    }

    /*Update the start point with header start*/
    if ((m_pStatusLine != SIP_NULL) || (m_pReqLine != SIP_NULL))
    {
        if (bHdrEnd == SIP_TRUE)
        {
            return SIP_TRUE;
        }
        pStartPt = pTempPos + SIP_THREE;
    }

    if (pStartPt >= pEndPt)
    {
        return SIP_TRUE;
    }

    while (pStartPt < pEndPt)
    {
        pTempPos = SIP_NULL;

        /*find next terminating CRLF*/
        if (SipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeFragmentMsg:no CRLF found", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        nDecLen = pTempPos - pStartPt + SIP_ONE;

        SIP_CHAR* pszHdrName = SIP_NULL;
        SIP_CHAR* pszHdrBody = SIP_NULL;

        if (m_objHdrs->DecodeHdrs(pStartPt, nDecLen, (SIP_CHAR**)&pszHdrName,
                    (SIP_CHAR**)&pszHdrBody) == SIP_FALSE)
        {
            if (pszHdrName != SIP_NULL)
            {
                delete[] pszHdrName;
            }
            if (pszHdrBody != SIP_NULL)
            {
                delete[] pszHdrBody;
            }

            return SIP_FALSE;
        }

        if (pszHdrName != SIP_NULL)
        {
            // check IS_TOKEN for pszHdrName
            delete[] pszHdrName;
        }

        if (pszHdrBody != SIP_NULL)
        {
            delete[] pszHdrBody;
        }

        pStartPt = pTempPos + SIP_THREE;
        pTempPos = SIP_NULL;

        if (bHdrEnd == SIP_TRUE)
        {
            break;
        }
    }

    /*Check for Header end completion*/
    if (bHdrEnd != SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:No header end in message",
                SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    /*Body decoding*/
    SipUnknownHeader* pContentLen = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
    if (pContentLen == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:No Body present in message",
                SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    SIP_INT32 nContentLen = 0;
    const SIP_CHAR* pszHdrValue = pContentLen->GetHeaderValue();
    if (pszHdrValue != SIP_NULL)
    {
        nContentLen = SipPf_Atoi(pszHdrValue);
    }

    pContentLen->SipDelete();

    if (nContentLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "No message body", SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    /*Update the start point to the start of Message body*/
    pStartPt = pStartPt + SIP_TWO;

    if (nContentLen > (pEndPt - pStartPt))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (HasHeader(SipHeaderBase::CONTENT_TYPE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "DecodeFragmentMsg:Message body present without content type", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Now Check for MIME or Single Body*/
    // If body list already present, free it
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
        m_pMsgBodyList = SIP_NULL;
    }

    m_pMsgBodyList = new SipMsgBodyList();
    if (m_pMsgBodyList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Check Content-Encoding header
    SipHeaderBase* pContEnc = m_objHdrs->GetHdrObj(SipHeaderBase::CONTENT_ENCODING, SIP_ZERO);
    if (pContEnc != SIP_NULL)
    {
        const SIP_CHAR* encType = pContEnc->GetValue();
        pContEnc->SipDelete();

        if ((encType != SIP_NULL) && ((SipPf_Stricmp(encType, "gzip") == SIP_ZERO)))
        {
            if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "DecodeFragmentMsg, GZIP Decoding single body failed", SIP_ZERO, SIP_ZERO);
                m_pMsgBodyList->SipDelete();
                m_pMsgBodyList = SIP_NULL;
                return SIP_FALSE;
            }
            return SIP_TRUE;
        }
    }

    SIP_BOOL bSingleBody = SIP_TRUE;
    SipContentTypeHeader* pContentType =
            static_cast<SipContentTypeHeader*>(m_objHdrs->GetHdrObj(SipHeaderBase::CONTENT_TYPE));

    if (pContentType != SIP_NULL)
    {
        const SIP_CHAR* pszMType = pContentType->GetMediaType();
        if (pszMType == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Wrong Content Type",
                    SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }

        bSingleBody = (SipPf_Stricmp(pszMType, MULTIPART) == SIP_ZERO) ? SIP_FALSE : SIP_TRUE;
        if (bSingleBody == SIP_TRUE)
        {
            pContentType->SipDelete();
        }
    }

    if (bSingleBody == SIP_TRUE)
    {
        if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Decode single body fail",
                    SIP_ZERO, SIP_ZERO);
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }
    }
    else
    {
        SIP_CHAR* pszBoundary = pContentType->GetBoundary();
        pContentType->SipDelete();

        if (pszBoundary == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecodeFragmentMsg:No boundary in Content Type Hdr", SIP_ZERO, SIP_ZERO);
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }

        if (m_pMsgBodyList->DecodeMIMEBody(pStartPt, pEndPt, pszBoundary) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeFragmentMsg:Decode MIME body fail",
                    SIP_ZERO, SIP_ZERO);
            delete[] pszBoundary;
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }
        delete[] pszBoundary;
    }

    return SIP_TRUE;
}

SIP_BOOL SipMessage::Decode(const SIP_CHAR* pMsgBuff, SIP_UINT32 nMsgBuffLen)
{
    if ((pMsgBuff == SIP_NULL) || (nMsgBuffLen == SIP_ZERO))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Invalid input", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pStartPt = pMsgBuff;
    const SIP_CHAR* pEndPt = pMsgBuff + nMsgBuffLen;

    // Remove additional CRLF if present at the starting of incoming message.
    while ((pStartPt < pEndPt) && (IS_CR(*pStartPt) && IS_LF(*(pStartPt + SIP_ONE))))
    {
        pStartPt = pStartPt + SIP_TWO;
    }

    if (pStartPt >= pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Invalid SIP message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = SipSkipFwLWS(pStartPt, pEndPt);

    if (pStartPt >= pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Invalid SIP message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get the buffer till first terminating CRLF
      In case of Request  it should be request line
      In case of Response it should be status line*/
    SIP_BOOL bHdrEnd = SIP_FALSE;
    const SIP_CHAR* pTempPos = SIP_NULL;

    if (SipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the End Point to the End of Start line*/
    pEndPt = pTempPos;
    pTempPos = SIP_NULL;

    /*Now determine for request line or status line*/
    /*Find the first token*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Invalid Start Line", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_INT32 eMsgType = SipGetMsgType(pStartPt);
    SIP_UINT32 nDecLen = pEndPt - pStartPt + SIP_ONE;

    if (eMsgType == SipMessage::REQ_TYPE)
    {
        m_pReqLine = new SipRequestLine();
        if (m_pReqLine == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pReqLine->DecodeRequestLine(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Req Line Decoding fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        m_pStatusLine = new SipStatusLine();
        if (m_pStatusLine == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pStatusLine->DecodeStatusLine(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Status Line Decoding fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    /*set the message type in message object*/
    this->m_eSipMsgType = eMsgType;

    /*Update the start point with header start*/
    pStartPt = pEndPt + SIP_THREE;
    pTempPos = SIP_NULL;

    /*update the end point of the message*/
    pEndPt = pMsgBuff + nMsgBuffLen;

    /*Header Decoding*/

    // sipfrag message decoding failure as it may not have headers.
    if (pStartPt >= pEndPt)
    {
        return SIP_TRUE;
    }

#ifdef SIP_BADMESSAGE_PARSING
    SIP_INT32 nBadMandatoryHdrMask = 0;
#endif

    while ((pStartPt < pEndPt) && (bHdrEnd == SIP_FALSE))
    {
        /*find next terminating CRLF*/
        if (SipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Incomplete Message", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        nDecLen = pTempPos - pStartPt + SIP_ONE;

        SIP_CHAR* pszHdrName = SIP_NULL;
        SIP_CHAR* pszHdrBody = SIP_NULL;

        if (m_objHdrs->DecodeHdrs(pStartPt, nDecLen, (SIP_CHAR**)&pszHdrName,
                    (SIP_CHAR**)&pszHdrBody) == SIP_FALSE)
        {
#ifdef SIP_BADMESSAGE_PARSING
            SipBadHeader* pBadHdr = new SipBadHeader();
            if (pBadHdr != SIP_NULL)
            {
                pBadHdr->SetHeaderName(pszHdrName);
                pBadHdr->SetValue(pszHdrBody);
                if (m_pBadHdrList == SIP_NULL)
                {
                    m_pBadHdrList = new SipHeaderList(SipHeaderBase::TYPE_INVALID);
                }
                m_pBadHdrList->AddHeader(pBadHdr);
                pBadHdr->SipDelete();
            }

            SIP_INT32 eHdrType = SipGetHdrType(pszHdrName);

            switch (eHdrType)
            {
                case SipHeaderBase::FROM:
                {
                    nBadMandatoryHdrMask = (nBadMandatoryHdrMask | MANDATORY_HDR_FROM);
                    break;
                }
                case SipHeaderBase::TO:
                {
                    nBadMandatoryHdrMask = (nBadMandatoryHdrMask | MANDATORY_HDR_TO);
                    break;
                }
                case SipHeaderBase::CALL_ID:
                {
                    nBadMandatoryHdrMask = (nBadMandatoryHdrMask | MANDATORY_HDR_CALL_ID);
                    break;
                }
                case SipHeaderBase::CSEQ:
                {
                    nBadMandatoryHdrMask = (nBadMandatoryHdrMask | MANDATORY_HDR_CSEQ);
                    break;
                }
                case SipHeaderBase::VIA:
                {
                    nBadMandatoryHdrMask = (nBadMandatoryHdrMask | MANDATORY_HDR_VIA);
                    break;
                }
                default:
                    break;
            }
#else
            if (pszHdrName != SIP_NULL)
            {
                delete[] pszHdrName;
            }
            if (pszHdrBody != SIP_NULL)
            {
                delete[] pszHdrBody;
            }

            return SIP_FALSE;
#endif
        }
        else
        {
#ifdef SIP_BADMESSAGE_PARSING
            SIP_INT32 eHdrType = SipGetHdrType(pszHdrName);

            switch (eHdrType)
            {
                case SipHeaderBase::FROM:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_FROM);
                    break;
                }
                case SipHeaderBase::TO:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_TO);
                    break;
                }
                case SipHeaderBase::CALL_ID:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_CALL_ID);
                    break;
                }
                case SipHeaderBase::CSEQ:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_CSEQ);
                    break;
                }
                case SipHeaderBase::VIA:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_VIA);
                    break;
                }
                default:
                    break;
            }
#endif
        }

        if (pszHdrName != SIP_NULL)
        {
            delete[] pszHdrName;
        }
        if (pszHdrBody != SIP_NULL)
        {
            delete[] pszHdrBody;
        }

        pStartPt = pTempPos + SIP_THREE;
        pTempPos = SIP_NULL;
    }

#ifdef SIP_BADMESSAGE_PARSING
    mbitMask = (mbitMask & ~nBadMandatoryHdrMask);

    if ((mbitMask & MANDATORY_HDR_FROM) == 0)
    {
        m_objHdrs->RemoveHdr(SipHeaderBase::FROM);
    }
    if ((mbitMask & MANDATORY_HDR_TO) == 0)
    {
        m_objHdrs->RemoveHdr(SipHeaderBase::TO);
    }
    if ((mbitMask & MANDATORY_HDR_CALL_ID) == 0)
    {
        m_objHdrs->RemoveHdr(SipHeaderBase::CALL_ID);
    }
    if ((mbitMask & MANDATORY_HDR_CSEQ) == 0)
    {
        m_objHdrs->RemoveHdr(SipHeaderBase::CSEQ);
    }
    if ((mbitMask & MANDATORY_HDR_VIA) == 0)
    {
        m_objHdrs->RemoveHdr(SipHeaderBase::VIA);
    }
#endif

    /*Check for Header end completion*/
    if (bHdrEnd != SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Check for CSeq Method Match*/
    if (m_pReqLine != SIP_NULL)
    {
        SipCSeqHeader* pCSeq =
                static_cast<SipCSeqHeader*>(m_objHdrs->GetHdrObj(SipHeaderBase::CSEQ));
        if (pCSeq != SIP_NULL)
        {
            const SIP_CHAR* pszMethod_ReqLine = m_pReqLine->GetMethod();
            const SIP_CHAR* pszMethod_CSeq = pCSeq->GetMethod();
            /*Compare Both the methods*/
            if ((SipPf_Stricmp(pszMethod_ReqLine, pszMethod_CSeq)))
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODDECODER, "Decode:Cseq Method Mismatch", SIP_ZERO, SIP_ZERO);

                pCSeq->SipDelete();
                return SIP_FALSE;
            }
            pCSeq->SipDelete();
        }
    }

    /*Body decoding*/
    SipUnknownHeader* pContentLen = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
    if (pContentLen == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "No Body present in message", SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    SIP_INT32 nContentLen = 0;
    const SIP_CHAR* pszHdrValue = pContentLen->GetHeaderValue();
    if (pszHdrValue != SIP_NULL)
    {
        nContentLen = SipPf_Atoi(pszHdrValue);
    }

    pContentLen->SipDelete();

    if (nContentLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "No message body", SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    /*Update the start point to the start of Message body*/
    pStartPt = pStartPt + SIP_TWO;

    if (nContentLen > (pEndPt - pStartPt))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (HasHeader(SipHeaderBase::CONTENT_TYPE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode:Message body present without content type",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Now Check for MIME or Single Body*/
    // If body list already present, free it
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
        m_pMsgBodyList = SIP_NULL;
    }

    m_pMsgBodyList = new SipMsgBodyList();
    if (m_pMsgBodyList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Decode:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Check Content-Encoding header
    SipHeaderBase* pContEnc = m_objHdrs->GetHdrObj(SipHeaderBase::CONTENT_ENCODING, SIP_ZERO);
    if (pContEnc != SIP_NULL)
    {
        const SIP_CHAR* encType = pContEnc->GetValue();
        pContEnc->SipDelete();
        if (encType != SIP_NULL)
        {
            if (SipPf_Stricmp(encType, "gzip") == SIP_ZERO)
            {
                if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "Decode, GZIP Decoding single body failed", SIP_ZERO, SIP_ZERO);
                    m_pMsgBodyList->SipDelete();
                    m_pMsgBodyList = SIP_NULL;
                    return SIP_FALSE;
                }
                return SIP_TRUE;
            }
        }
    }

    SIP_BOOL bSingleBody = SIP_TRUE;
    SipContentTypeHeader* pContentType =
            static_cast<SipContentTypeHeader*>(m_objHdrs->GetHdrObj(SipHeaderBase::CONTENT_TYPE));

    if (pContentType != SIP_NULL)
    {
        const SIP_CHAR* pszMType = pContentType->GetMediaType();
        if (pszMType == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Wrong Content Type", SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }

        bSingleBody = (SipPf_Stricmp(pszMType, MULTIPART) == SIP_ZERO) ? SIP_FALSE : SIP_TRUE;
        if (bSingleBody == SIP_TRUE)
        {
            pContentType->SipDelete();
        }
    }

    if (bSingleBody == SIP_TRUE)
    {
        if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Decode single body fail", SIP_ZERO, SIP_ZERO);
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }
    }
    else
    {
        SIP_CHAR* pszBoundary = pContentType->GetBoundary();
        pContentType->SipDelete();

        if (pszBoundary == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "No boundary in Content Type Hdr", SIP_ZERO, SIP_ZERO);
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }

        if (m_pMsgBodyList->DecodeMIMEBody(pStartPt, pEndPt, pszBoundary) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode:Decode MIME body fail", SIP_ZERO, SIP_ZERO);
            delete[] pszBoundary;
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }
        delete[] pszBoundary;
    }

    return SIP_TRUE;
}

/*Set Content-length Header*/
SIP_VOID SipMessage::SetContentLengthHdr(SIP_UINT32 nLen, SIP_UINT32 nMsgOptions)
{
    SipUnknownHeader* pUnknown = new SipUnknownHeader();
    if (pUnknown != SIP_NULL)
    {
        SIP_CHAR szLen[SIP_CONTLEN_LEN] = {0};
        SipPf_Sprintf(szLen, "%u", nLen);

        const SIP_CHAR* pszHdrName = ((nMsgOptions & SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM) ==
                                             SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM)
                ? "l"
                : "Content-Length";

        pUnknown->SetHeaderName(pszHdrName);
        pUnknown->SetHeaderValue(szLen);
        SetHeader(pUnknown);

        pUnknown->SipDelete();
    }
}

/*Get Unknown header using name of the header*/
SipUnknownHeader* SipMessage::GetUnknownHdrObj(const SIP_CHAR* pszHdrName)
{
    if (pszHdrName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipHeaderList* pList = GetHdrList(SipHeaderBase::UNKNOWN);
    if (pList != SIP_NULL)
    {
        for (SIP_UINT32 i = 0; i < pList->GetSize(); i++)
        {
            SipUnknownHeader* pUnknown = static_cast<SipUnknownHeader*>(pList->GetObj(i));
            if (pUnknown != SIP_NULL)
            {
                const SIP_CHAR* pszUKHdrName = pUnknown->GetHeaderName();
                if ((pszUKHdrName != SIP_NULL) &&
                        (SipPf_Stricmp(pszUKHdrName, pszHdrName) == SIP_ZERO))
                {
                    pList->SipDelete();
                    return pUnknown;
                }
                pUnknown->SipDelete();
            }
        }
        pList->SipDelete();
    }
    return SIP_NULL;
}

/*Get Unknown header using type of the header, Only Content-Length/l supported as of now*/
SipUnknownHeader* SipMessage::GetUnknownHdrObj(SIP_INT32 eType)
{
    SipHeaderList* pList = GetHdrList(SipHeaderBase::UNKNOWN);
    if (pList != SIP_NULL)
    {
        for (SIP_UINT32 i = 0; i < pList->GetSize(); i++)
        {
            SipUnknownHeader* pUnknown = static_cast<SipUnknownHeader*>(pList->GetObj(i));
            if (pUnknown != SIP_NULL)
            {
                const SIP_CHAR* pszUKHdrName = pUnknown->GetHeaderName();
                switch (eType)
                {
                    case SipHeaderBase::CONTENT_LENGTH:
                    {
                        if (pszUKHdrName != SIP_NULL)
                        {
                            if ((!SipPf_Stricmp(pszUKHdrName, "Content-Length")) ||
                                    (!SipPf_Stricmp(pszUKHdrName, "l")))
                            {
                                pList->SipDelete();
                                return pUnknown;
                            }
                        }
                    }
                    break;

                        /*TODO - All required headers to be added on need basis.*/
                        /*other option - Use SipMessage::GetUnknownHdrObj(SIP_CHAR* strHdrName)*/
                    default:
                        break;
                }
                pUnknown->SipDelete();
            }
        }
        pList->SipDelete();
    }
    return SIP_NULL;
}

/* Adjust Content-Length header : move it to the last unknown header's position */
void SipMessage::AdjustContentLengthHdr()
{
    SipHeaderList* pList = GetHdrList(SipHeaderBase::UNKNOWN);

    if (pList == SIP_NULL)
    {
        return;
    }

    SIP_UINT32 nCount = pList->GetSize();

    if (nCount <= 1)
    {
        pList->SipDelete();
        return;
    }

    /* Content-Length header is the last one, then no action */
    nCount -= 1;

    SipUnknownHeader* pContentLength = SIP_NULL;

    for (SIP_UINT32 i = 0; i < nCount; i++)
    {
        SipUnknownHeader* pUnknown = static_cast<SipUnknownHeader*>(pList->GetObj(i));

        if (pUnknown != SIP_NULL)
        {
            const SIP_CHAR* pszName = pUnknown->GetHeaderName();

            if (pszName != SIP_NULL)
            {
                if (SipPf_Stricmp(pszName, "Content-Length") == 0 ||
                        SipPf_Stricmp(pszName, "l") == 0)
                {
                    pContentLength = pUnknown;

                    pList->RemoveHdr(i);
                    break;
                }
            }

            pUnknown->SipDelete();
        }
    }

    if (pContentLength != SIP_NULL)
    {
        pList->AddHeader(pContentLength);

        pContentLength->SipDelete();
    }

    pList->SipDelete();
}

void SipMessage::RemoveAllMessageBodies()
{
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
        m_pMsgBodyList = SIP_NULL;
    }
}

SIP_UINT16 SipMessage::GetStatusCode() const
{
    if (m_eSipMsgType != SipMessage::RESP_TYPE)
    {
        return SIP_SC_INVALID;
    }

    if (m_pStatusLine == SIP_NULL)
    {
        return SIP_SC_INVALID;
    }

    return m_pStatusLine->GetStatusCodeAsInt();
}

SipHeaderBase* SipMessage::GetHeader(SIP_INT32 nType, SIP_UINT32 nIndex)
{
    SipHeaderList* pHdrList = GetHdrList(nType);
    SipHeaderBase* pHdr = SIP_NULL;

    if (pHdrList == SIP_NULL)
    {
        pHdr = GetHdrObj(nType);
    }
    else
    {
        pHdr = pHdrList->GetObj(nIndex);
        pHdrList->SipDelete();
    }
    return pHdr;
}

SIP_BOOL SipMessage::CheckTxnMandatoryParams(
        SipMessage* pSipMsg, SIP_INT32* peMsgType, SIP_INT32* peMethodType)
{
    SIP_INT32 eMsgType = pSipMsg->GetMsgType();
    if (eMsgType == SipMessage::TYPE_INVALID)
    {
        return SIP_FALSE;
    }

    SIP_INT32 eMethodType = pSipMsg->GetMethodType();

    if (eMethodType == SipMessage::METHOD_INVALID)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "Invalid Method Type(%d != %d)", eMethodType,
                pSipMsg->GetMethodType());
        return SIP_FALSE;
    }

    /* Step 1: Check for Mandatory Headers for Transaction key */
    /* Check for To Hdr */
    if (pSipMsg->HasHeader(SipHeaderBase::TO) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'To' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::FROM) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'From' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::CSEQ) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'CSeq' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::CALL_ID) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'CallID' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::VIA) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'Via' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Step 2: Check for Request line */
    if (eMsgType == SipMessage::REQ_TYPE)
    {
        if (pSipMsg->IsReqLineExists() == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "IsReqLineExists Fails", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        if (pSipMsg->IsStatusLineExists() == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "IsStatusLineExists Fails", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    *peMsgType = eMsgType;
    *peMethodType = eMethodType;

    return SIP_TRUE;
}

SIP_UINT32 SipMessage::GetRSeqNum(SipMessage* pMessage, SIP_INT32 eHdrType)
{
    SIP_UINT32 nRSeqNum = 0;

    if (pMessage != SIP_NULL &&
            (eHdrType == SipHeaderBase::RSEQ || eHdrType == SipHeaderBase::RACK))
    {
        SipHeaderBase* pHeader = pMessage->GetHeader(eHdrType, 0);

        if (pHeader != SIP_NULL)
        {
            if (eHdrType == SipHeaderBase::RSEQ)
            {
                SipIntegerHeader* pRSeq = static_cast<SipIntegerHeader*>(pHeader);
                nRSeqNum = pRSeq->GetValueInt();
            }
            else
            {
                SipRAcKHeader* pRAck = static_cast<SipRAcKHeader*>(pHeader);
                nRSeqNum = pRAck->GetResponseNum();
            }

            pHeader->SipDelete();
        }
    }

    return nRSeqNum;
}
