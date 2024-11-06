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
#include "SipDebug.h"
#include "msg/SipMessage.h"
#include "msg/SipMsgBody.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipMemory.h"
#include "platform/SipString.h"

extern SIP_CHAR gaszSipHdr[][SIP_MAX_HDR_LEN];

SipMIMEHdrs::SipMIMEHdrs() :
        m_pContentType(SIP_NULL),
        m_pContentEncoding(SIP_NULL),
        m_pContentDisposition(SIP_NULL),
        m_pUnKnownHdrList(SIP_NULL)
{
}

SipMIMEHdrs::SipMIMEHdrs(const SipMIMEHdrs& objMimeHdr) :
        m_pContentType(SIP_NULL),
        m_pContentEncoding(SIP_NULL),
        m_pContentDisposition(SIP_NULL),
        m_pUnKnownHdrList(SIP_NULL)
{
    if (objMimeHdr.m_pContentType != SIP_NULL)
    {
        m_pContentType = new SipContentTypeHeader(*(objMimeHdr.m_pContentType));
    }

    if (objMimeHdr.m_pContentDisposition != SIP_NULL)
    {
        m_pContentDisposition = new SipHeaderBase(*(objMimeHdr.m_pContentDisposition));
    }

    if (objMimeHdr.m_pContentEncoding != SIP_NULL)
    {
        m_pContentEncoding = new SipHeaderBase(*(objMimeHdr.m_pContentEncoding));
    }

    if (objMimeHdr.m_pUnKnownHdrList != SIP_NULL)
    {
        m_pUnKnownHdrList = new SipHeaderList(*(objMimeHdr.m_pUnKnownHdrList));
    }
}

SipMIMEHdrs::~SipMIMEHdrs()
{
    if (m_pContentType != SIP_NULL)
    {
        m_pContentType->SipDelete();
    }
    if (m_pContentEncoding != SIP_NULL)
    {
        m_pContentEncoding->SipDelete();
    }
    if (m_pContentDisposition != SIP_NULL)
    {
        m_pContentDisposition->SipDelete();
    }
    if (m_pUnKnownHdrList != SIP_NULL)
    {
        m_pUnKnownHdrList->SipDelete();
    }
}

SIP_BOOL SipMIMEHdrs::SetMimeHdrs(SipHeaderBase* pHdr)
{
    if (pHdr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODACCESSOR, "SetHdrs Failed, pHdr is NULL", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_INT32 eHdrType = pHdr->GetHdrType();
    switch (eHdrType)
    {
        case SipHeaderBase::CONTENT_TYPE:
            if (m_pContentType != SIP_NULL)
            {
                m_pContentType->SipDelete();
            }
            m_pContentType = static_cast<SipContentTypeHeader*>(pHdr);
            pHdr->Increment();
            break;

        case SipHeaderBase::CONTENT_DISPOSITION:
            if (m_pContentDisposition != SIP_NULL)
            {
                m_pContentDisposition->SipDelete();
            }
            m_pContentDisposition = pHdr;
            pHdr->Increment();
            break;

        case SipHeaderBase::CONTENT_ENCODING:
            if (m_pContentEncoding != SIP_NULL)
            {
                m_pContentEncoding->SipDelete();
            }
            m_pContentEncoding = pHdr;
            pHdr->Increment();
            break;
        case SipHeaderBase::UNKNOWN:
        {
            if (m_pUnKnownHdrList == SIP_NULL)
            {
                m_pUnKnownHdrList = new SipHeaderList(SipHeaderBase::UNKNOWN);
            }
            m_pUnKnownHdrList->AddHeader(pHdr);
        }
        break;
        default:
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODACCESSOR, "SetMimeHdr Failed, Type = %d", eHdrType, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

SipHeaderBase* SipMIMEHdrs::GetUnknownHdr(SIP_UINT32 nIndex)
{
    return (m_pUnKnownHdrList != SIP_NULL) ? m_pUnKnownHdrList->GetObj(nIndex) : SIP_NULL;
}

SipHeaderBase* SipMIMEHdrs::GetMimeHdrObj(SIP_INT32 eIndex)
{
    switch (eIndex)
    {
        case CONTENT_TYPE:
            if (m_pContentType != SIP_NULL)
            {
                m_pContentType->Increment();
            }
            return m_pContentType;

        case CONTENT_ENCODING:
            if (m_pContentEncoding != SIP_NULL)
            {
                m_pContentEncoding->Increment();
            }
            return m_pContentEncoding;

        case CONTENT_DISPOSITION:
            if (m_pContentDisposition != SIP_NULL)
            {
                m_pContentDisposition->Increment();
            }
            return m_pContentDisposition;

        case UNKNOWN:
            if (m_pUnKnownHdrList != SIP_NULL)
            {
                m_pUnKnownHdrList->Increment();
            }
            return m_pUnKnownHdrList;

        default:
            return SIP_NULL;
    }
}

SipHeaderBase* SipMIMEHdrs::GetNewMIMEHdrObj(SIP_INT32 eHdrType)
{
    switch (eHdrType)
    {
        case SipHeaderBase::CONTENT_TYPE:
            if (m_pContentType != SIP_NULL)
            {
                m_pContentType->SipDelete();
            }
            m_pContentType = new SipContentTypeHeader();
            return m_pContentType;

        case SipHeaderBase::CONTENT_ENCODING:
            if (m_pContentEncoding != SIP_NULL)
            {
                m_pContentEncoding->SipDelete();
            }
            m_pContentEncoding = new SipHeaderBase(SipHeaderBase::CONTENT_ENCODING);
            return m_pContentEncoding;

        case SipHeaderBase::CONTENT_DISPOSITION:
            if (m_pContentDisposition != SIP_NULL)
            {
                m_pContentDisposition->SipDelete();
            }
            m_pContentDisposition = new SipHeaderBase(SipHeaderBase::CONTENT_DISPOSITION);
            return m_pContentDisposition;

        default:
            if (m_pUnKnownHdrList == SIP_NULL)
            {
                m_pUnKnownHdrList = new SipHeaderList(SipHeaderBase::UNKNOWN);
            }
            return m_pUnKnownHdrList;
    }
}

SIP_BOOL SipMIMEHdrs::EncodeMIMEHdrs(SIP_CHAR** ppCurrPos)
{
    SIP_INT32 nHdr = SipMIMEHdrs::CONTENT_TYPE;

    while (nHdr < SipMIMEHdrs::END)
    {
        SipHeaderBase* pTemp = GetMimeHdrObj(nHdr);
        if (pTemp != SIP_NULL)
        {
            if (pTemp->GetHdrType() != SipHeaderBase::UNKNOWN)
            {
                SipHeaders::SipEncodeHdrName(
                        pTemp->GetHdrType(), ppCurrPos, SipConfiguration::MSG_OPT_ENCODE_NONE);
            }

            if (pTemp->EncodeHdr(ppCurrPos) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeHdr Failed", SIP_ZERO, SIP_ZERO);
                pTemp->SipDelete();
                return SIP_FALSE;
            }
            pTemp->SipDelete();
            SIP_ENC_CRLF(*ppCurrPos);
        }
        nHdr++;
    }

    return SIP_TRUE;
}

SIP_BOOL SipMIMEHdrs::DecodeMIMEHdrs(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pTempPos = SIP_NULL;
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    /*Get the position previous to ":"*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: colon not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pTempNext = pTempPos + SIP_TWO;
    pTempNext = SipSkipFwLWS(pTempNext, pEndPt);

    /*skip the WSP form back*/
    pTempPos = SipSkipRwWSP(pStartPt, pTempPos);

    /*Create  the header name*/
    SIP_CHAR* pszHdrName = SipCreateString(pStartPt, pTempPos);
    if (pszHdrName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*this will return the type of header on passing name*/
    SIP_INT32 eHdrType = SipGetMimeHdrType(pszHdrName);

    /*Free the header name*/
    /*Get the header object*/
    SipHeaderBase* pHeader = GetNewMIMEHdrObj(eHdrType);
    if (pHeader == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: GetNewMIMEHdrObj failed", SIP_ZERO,
                SIP_ZERO);
        delete[] pszHdrName;
        return SIP_FALSE;
    }

    if (pHeader->GetHdrType() == SipHeaderBase::UNKNOWN)
    {
        SipUnknownHeader* pUnknown = new SipUnknownHeader();
        if (pUnknown == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            delete[] pszHdrName;
            return SIP_FALSE;
        }

        pUnknown->SetHeaderName(pszHdrName);
        delete[] pszHdrName;

        SIP_CHAR* pszHdrValue = SipCreateString(pTempNext, pEndPt);
        if (pszHdrValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            pUnknown->SipDelete();
            return SIP_FALSE;
        }

        pUnknown->SetHeaderValue(pszHdrValue);
        delete[] pszHdrValue;

        if (m_pUnKnownHdrList == SIP_NULL)
        {
            m_pUnKnownHdrList = new SipHeaderList(SipHeaderBase::UNKNOWN);
        }

        /*Add the header into the unknown list*/
        if (m_pUnKnownHdrList->AddHeader(pUnknown) == SIP_FALSE)
        {
            pUnknown->SipDelete();
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Add to list Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pUnknown->SipDelete();
    }
    else
    {
        delete[] pszHdrName;
        /*Update the Start Point to the start of hdr value*/
        pStartPt = pTempNext;
        /*Update the length for decoding*/
        nDecLen = pEndPt - pStartPt + SIP_ONE;
        /*Call the Decoder function*/
        if (pHeader->DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipMsgBody::SipMsgBody() :
        m_eBodyType(SINGLE_BODY),
        m_pMIMEHdrs(SIP_NULL),
        m_pBuffer(SIP_NULL),
        m_nBufLen(SIP_ZERO),
        m_pBodyList(SIP_NULL),
        m_bEncodeMime(SIP_TRUE)
{
}

SipMsgBody::SipMsgBody(SIP_INT32 eBodyType) :
        m_eBodyType(SINGLE_BODY),
        m_pMIMEHdrs(SIP_NULL),
        m_pBuffer(SIP_NULL),
        m_nBufLen(SIP_ZERO),
        m_pBodyList(SIP_NULL),
        m_bEncodeMime(SIP_TRUE)
{
    m_eBodyType = eBodyType;
    m_pMIMEHdrs = new SipMIMEHdrs();
    if (eBodyType == MULTI_PART_BODY)
    {
        m_pBodyList = new SipMsgBodyList();
    }
}

SipMsgBody::SipMsgBody(const SipMsgBody& objMsgBody) :
        m_eBodyType(objMsgBody.m_eBodyType),
        m_pMIMEHdrs(SIP_NULL),
        m_pBuffer(SIP_NULL),
        m_nBufLen(SIP_ZERO),
        m_pBodyList(SIP_NULL),
        m_bEncodeMime(objMsgBody.m_bEncodeMime)
{
    if ((objMsgBody.m_pBuffer != SIP_NULL) && (objMsgBody.m_nBufLen > SIP_ZERO))
    {
        m_pBuffer = new SIP_CHAR[objMsgBody.m_nBufLen];
        SipPf_Memcpy(m_pBuffer, objMsgBody.m_pBuffer, objMsgBody.m_nBufLen);
        m_nBufLen = objMsgBody.m_nBufLen;
    }

    if (objMsgBody.m_pMIMEHdrs != SIP_NULL)
    {
        m_pMIMEHdrs = new SipMIMEHdrs(*(objMsgBody.m_pMIMEHdrs));
    }

    if (objMsgBody.m_pBodyList != SIP_NULL)
    {
        m_pBodyList = new SipMsgBodyList(*(objMsgBody.m_pBodyList));
    }
}

SipMsgBody::~SipMsgBody()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        m_pMIMEHdrs->SipDelete();
        m_pMIMEHdrs = SIP_NULL;
    }
    if (m_pBuffer != SIP_NULL)
    {
        delete[] m_pBuffer;
        m_pBuffer = SIP_NULL;
    }
    if (m_pBodyList != SIP_NULL)
    {
        m_pBodyList->SipDelete();
    }
}

SIP_BOOL SipMsgBody::EncodeSingleMsgBody(SIP_CHAR** ppCurrPos)
{
    if (m_pBuffer == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeMsgBody Failed - No body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    SIP_CHAR* pBody = *ppCurrPos;
    SipPf_Memcpy(pBody, m_pBuffer, m_nBufLen);

    /*Update the current position*/
    *ppCurrPos += m_nBufLen;

    return SIP_TRUE;
}

SIP_BOOL SipMsgBody::EncodeMIMEMsgBody(SIP_CHAR** ppCurrPos)
{
    /*Check for message body list*/
    if (m_pBodyList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeMIMEMsgBody: No body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get The content type to get the boundary*/
    SipContentTypeHeader* pContentType = static_cast<SipContentTypeHeader*>(GetContentType());
    if (pContentType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Content Type Not Present", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszBoundary = pContentType->GetBoundary();

    if (m_pBodyList->EncodeBody(ppCurrPos, pszBoundary) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Msg Body Enc Failed", SIP_ZERO, SIP_ZERO);
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
    pContentType->SipDelete();

    return SIP_TRUE;
}

SIP_BOOL SipMsgBody::EncodeBody(SIP_CHAR** ppCurrPos)
{
    /*Encode the  MIME headers*/
    if (IsMimeEncoding() == SIP_TRUE)
    {
        SIP_ENC_CRLF(*ppCurrPos);

        if (m_pMIMEHdrs != SIP_NULL)
        {
            if (m_pMIMEHdrs->EncodeMIMEHdrs(ppCurrPos) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODENCODER, "EncodeBody Failed - No body", SIP_ZERO, SIP_ZERO);
            }
        }

        /*Put the second CRLF*/
        SIP_ENC_CRLF(*ppCurrPos);
    }

    return (GetBodyType() == SipMsgBody::SINGLE_BODY) ? EncodeSingleMsgBody(ppCurrPos)
                                                      : EncodeMIMEMsgBody(ppCurrPos);
}

SIP_BOOL SipMsgBody::DecodeSingleMsgBody(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    SIP_UINT32 nSize = pEndPt - pStartPt;
    SIP_CHAR* pData = new SIP_CHAR[nSize + SIP_ONE];

    if (pData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeSingleMsgBody:Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Memcpy(pData, pStartPt, nSize);
    pData[nSize] = SIP_NULL_CHAR;

    m_pBuffer = pData;
    m_nBufLen = nSize;
    m_eBodyType = SINGLE_BODY;

    return SIP_TRUE;
}

SIP_BOOL SipMsgBody::DecodeMIMEMsgBody(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    if (pStartPt == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody: Illegal argument", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pNext1Pt = pStartPt + SIP_ONE;
    /*Case when No Header is present before the start of body*/
    if (IS_CRLF(*pStartPt, *pNext1Pt))
    {
        return DecodeSingleMsgBody(pStartPt, pEndPt);
    }

    /*Header Decoding*/
    m_pMIMEHdrs = new SipMIMEHdrs();
    if (m_pMIMEHdrs == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody:Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_BOOL bHdrEnd = SIP_FALSE;
    while ((pStartPt <= pEndPt) && (bHdrEnd == SIP_FALSE))
    {
        /*find next terminating CRLF*/
        SIP_UINT32 nDecLen = SIP_ZERO;
        const SIP_CHAR* pTempPos = SIP_NULL;
        // Fail condition to be added
        SipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd);
        nDecLen = pTempPos - pStartPt + SIP_ONE;

        if (m_pMIMEHdrs->DecodeMIMEHdrs(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody:Hdr Decoding failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pStartPt = pTempPos + SIP_THREE;
    }
    /*Check for Header end completion*/
    if (bHdrEnd != SIP_TRUE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of Actual Message body Buffer*/
    pStartPt = pStartPt + SIP_TWO;
    /*Now Get Type of Body*/
    SipContentTypeHeader* pContentType = static_cast<SipContentTypeHeader*>(
            m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE));
    if (pContentType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody: Body in not valid", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    const SIP_CHAR* pszMType = pContentType->GetMediaType();
    if (pszMType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Wronge Content Type", SIP_ZERO, SIP_ZERO);
        pContentType->SipDelete();
        return SIP_FALSE;
    }
    SIP_BOOL bSingleBody = (SIP_BOOL)SipPf_Stricmp(pszMType, MULTIPART);

    /*Case of mime body*/
    if (bSingleBody == SIP_FALSE)
    {
        SIP_CHAR* pszBoundary = pContentType->GetBoundary();
        if (pszBoundary == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "No boundary in Content Type Hdr", SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            return SIP_FALSE;
        }

        m_pBodyList = new SipMsgBodyList();
        if (m_pBodyList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            delete[] pszBoundary;
            return SIP_FALSE;
        }
        m_eBodyType = MULTI_PART_BODY;

        if (m_pBodyList->DecodeMIMEBody(pStartPt, pEndPt, pszBoundary) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEBody: failed", SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            delete[] pszBoundary;
            return SIP_FALSE;
        }
        delete[] pszBoundary;
    }
    /*Case of Single Body*/
    else
    {
        if (DecodeSingleMsgBody(pStartPt, pEndPt) == SIP_FALSE)
        {
            return SIP_FALSE;
        }
    }

    pContentType->SipDelete();
    return SIP_TRUE;
}

SIP_BOOL SipMsgBody::SetMimeHdr(SipHeaderBase* pHdrBase)
{
    if (m_pMIMEHdrs == SIP_NULL)
    {
        m_pMIMEHdrs = new SipMIMEHdrs();
    }
    return m_pMIMEHdrs->SetMimeHdrs(pHdrBase);
}

SIP_BOOL SipMsgBody::SetMsgBuffer(const SIP_CHAR* pMsgBuffer, SIP_UINT32 nBufLen)
{
    if (pMsgBuffer != SIP_NULL)
    {
        SIP_CHAR* pData = new SIP_CHAR[nBufLen + SIP_ONE];
        if (pData == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SetMsgBuffer:Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        SipPf_Memcpy(pData, pMsgBuffer, nBufLen);
        pData[nBufLen] = SIP_NULL_CHAR;
        m_pBuffer = pData;
        m_nBufLen = nBufLen;
        return SIP_TRUE;
    }

    return SIP_FALSE;
}

SipMsgBodyList* SipMsgBody::GetMessageBodyList()
{
    if (m_pBodyList != SIP_NULL)
    {
        m_pBodyList->Increment();
    }
    return m_pBodyList;
}

SipContentTypeHeader* SipMsgBody::GetContentType()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        return static_cast<SipContentTypeHeader*>(
                m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE));
    }

    return SIP_NULL;
}

SipHeaderBase* SipMsgBody::GetContentEncoding()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        return m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING);
    }

    return SIP_NULL;
}

SipHeaderBase* SipMsgBody::GetContentDisposition()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        return m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION);
    }

    return SIP_NULL;
}

SipHeaderBase* SipMsgBody::GetMimeHdr(SIP_INT32 eHdrType, SIP_UINT32 nIndex)
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        switch (eHdrType)
        {
            case SipHeaderBase::CONTENT_TYPE:
                return m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE);

            case SipHeaderBase::CONTENT_DISPOSITION:
                return m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION);

            case SipHeaderBase::CONTENT_ENCODING:
                return m_pMIMEHdrs->GetMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING);

            case SipHeaderBase::UNKNOWN:
                return m_pMIMEHdrs->GetUnknownHdr(nIndex);

            default:
                return SIP_NULL;
        }
    }

    return SIP_NULL;
}

SIP_BOOL SipMsgBody::IsMessageBodySDP()
{
    SipContentTypeHeader* pHdr = GetContentType();
    if (pHdr == SIP_NULL)
    {
        return SIP_FALSE;
    }
    const SIP_CHAR* pszSubMType = pHdr->GetSubMediaType();
    SIP_INT16 nResult = SipPf_Stricmp(pszSubMType, SDP);
    pHdr->SipDelete();
    return (nResult == SIP_ZERO) ? SIP_TRUE : SIP_FALSE;
}

SIP_BOOL SipMsgBody::GetMsgBuffer(SIP_CHAR** ppMsgBuffer)
{
    if (m_pBuffer != SIP_NULL)
    {
        SIP_UINT32 nSize = m_nBufLen;
        SIP_CHAR* pData = new SIP_CHAR[nSize + SIP_ONE];
        if (pData == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "GetMsgBuffer:Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        SipPf_Memcpy(pData, m_pBuffer, nSize);
        pData[nSize] = SIP_NULL_CHAR;
        *ppMsgBuffer = pData;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SipMsgBodyList::SipMsgBodyList() :
        m_objBodyList(SipVector<SipMsgBody*>())
{
}

SipMsgBodyList::~SipMsgBodyList()
{
    while (m_objBodyList.IsEmpty() != SIP_TRUE)
    {
        SipMsgBody* pMsgBody = m_objBodyList.Top();
        pMsgBody->SipDelete();
        m_objBodyList.Pop();
    }
}

SipMsgBodyList::SipMsgBodyList(const SipMsgBodyList& objMsgBodyList) :
        m_objBodyList(SipVector<SipMsgBody*>())
{
    SIP_UINT32 nSize = objMsgBodyList.m_objBodyList.GetSize();

    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SipMsgBody* pTempBody = objMsgBodyList.m_objBodyList.GetAt(nCount);
        if (pTempBody != SIP_NULL)
        {
            SipMsgBody* pBody = new SipMsgBody(*pTempBody);
            if (pBody == SIP_NULL)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODDECODER, "Copy Constructor Malloc failed", SIP_ZERO, SIP_ZERO);
            }
            if ((m_objBodyList.Add(pBody)) < 0)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODDECODER, "Copy Constructor Append Failed", SIP_ZERO, SIP_ZERO);
            }
        }
    }
}

SIP_BOOL SipMsgBodyList::EncodeBody(SIP_CHAR** ppMsgBuffCurrPos, const SIP_CHAR* pszBoundary)
{
    SIP_UINT32 nNumBodies = m_objBodyList.GetSize();

    /*Encoding Of Boundary*/
    SIP_ENC_DASH(*ppMsgBuffCurrPos);
    SipPf_Strcpy(*ppMsgBuffCurrPos, pszBoundary);
    SipEnc_UpdateCurrPos(ppMsgBuffCurrPos);

    /*Get the message bodies and encode them*/
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nNumBodies; nCount++)
    {
        SipMsgBody* pMsgbody = m_objBodyList.GetAt(nCount);
        if (pMsgbody == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "EncodeBody Failed - No body", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pMsgbody->EncodeBody(ppMsgBuffCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encode message Body Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /*Put the Closing Boundary*/
        SIP_ENC_CRLF(*ppMsgBuffCurrPos);
        SIP_ENC_DASH(*ppMsgBuffCurrPos);
        SipPf_Strcpy(*ppMsgBuffCurrPos, pszBoundary);
        SipEnc_UpdateCurrPos(ppMsgBuffCurrPos);
    }

    /*End boundary*/
    SIP_ENC_DASH(*ppMsgBuffCurrPos);
    SIP_ENC_CRLF(*ppMsgBuffCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipMsgBodyList::GetEncodedMessageBody(
        SIP_CHAR** ppMsgBufer, SIP_UINT32& nMsgLen, const SIP_CHAR* pszBoundary)
{
    SIP_UINT32 nCount = m_objBodyList.GetSize();
    if (nCount == SIP_ZERO)
    {
        return SIP_TRUE;
    }

    SIP_CHAR* pBufferCurrPos = *ppMsgBufer;

    SIP_UINT32 nLen = SIP_ZERO;
    /*Now check for length calculation with boundary or without boundary*/
    if (pszBoundary == SIP_NULL)
    {
        SipMsgBody* pBody = m_objBodyList.GetAt(SIP_ZERO);
        pBody->EncodeSingleMsgBody(&pBufferCurrPos);
        pBody->GetMsgBuffLen(&nLen);
    }
    else
    {
        if (EncodeBody(&pBufferCurrPos, pszBoundary) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode msg body fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        nLen = pBufferCurrPos - *ppMsgBufer;
    }

    nMsgLen = nLen;

    return SIP_TRUE;
}

SIP_BOOL SipMsgBodyList::AddBody(SipMsgBody* pBody)
{
    if ((m_objBodyList.Add(pBody)) < 0)
    {
        return SIP_FALSE;
    }
    pBody->Increment();
    return SIP_TRUE;
}

SipMsgBody* SipMsgBodyList::GetBodyByIndex(SIP_UINT32 nIndex)
{
    if (nIndex < m_objBodyList.GetSize())
    {
        SipMsgBody* pBody = m_objBodyList.GetAt(nIndex);
        pBody->Increment();
        return pBody;
    }
    return SIP_NULL;
}

SIP_BOOL SipMsgBodyList::DecodeSingleBody(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*single body support*/

    SipMsgBody* pMsgBody = new SipMsgBody();
    if (pMsgBody == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeSingleBody:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pMsgBody->DecodeSingleMsgBody(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeSingleBody:Body Decoding failed", SIP_ZERO, SIP_ZERO);
        pMsgBody->SipDelete();
        return SIP_FALSE;
    }
    if (m_objBodyList.Add(pMsgBody) < 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeSingleBody:Body Decoding failed", SIP_ZERO, SIP_ZERO);
        pMsgBody->SipDelete();
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipMsgBodyList::DecodeMIMEBody(
        const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR* pszBoundary)
{
    /*Boundary check*/
    if (pszBoundary == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeMIMEBody: Boundary unavailable", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Remove CRLF before boundary*/
    pStartPt = SkipConsecutiveCRLFs(pStartPt);

    /*Get the boundrary (Find start of Transport -padding)*/
    /*Update start pt aftr  "--" */
    /*Get the boundrary (In case of No Transport -padding)*/
    /*Update the start point to the sart of boundary*/
    pStartPt = pStartPt + SIP_TWO;
    SIP_CHAR* pszTempBoundary = SIP_NULL;
    const SIP_CHAR* pTempPos = SIP_NULL;

    if (SipFindCrlf(pStartPt, pEndPt, &pTempPos) == SIP_TRUE)
    {
        pszTempBoundary = SipCreateString(pStartPt, pTempPos);
        if (pszTempBoundary == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMsgBodyList::DecodeMIMEBody:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipMsgBodyList::DecodeMIMEBody:Boundary Invalid",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nBoundaryLen = pTempPos - pStartPt;
    if (SipPf_Stricmp(pszBoundary, pszTempBoundary) != SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeMIMEBody:Boundary Not Matching", SIP_ZERO, SIP_ZERO);
        delete[] pszTempBoundary;
        return SIP_FALSE;
    }
    delete[] pszTempBoundary;

    /*Update the start point till the start of boundary*/
    pStartPt = pStartPt + nBoundaryLen + SIP_ONE;
    /*Update the Start point to the Start Of headers*/
    pStartPt = SipSkipFwLWS(pStartPt, pEndPt);

    SIP_BOOL bBodyEnd = SIP_FALSE;
    while ((pStartPt <= pEndPt) && (bBodyEnd == SIP_FALSE))
    {
        SipMsgBody* pMsgBody = new SipMsgBody();
        if (pMsgBody == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMsgBodyList::DecodeMIMEBody:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        const SIP_CHAR* pTempEnd = SipFindBodyEnd(pStartPt, pEndPt, pszBoundary, bBodyEnd);

        if (pMsgBody->DecodeMIMEMsgBody(pStartPt, pTempEnd) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMsgBodyList::DecodeMIMEBody:Body Decoding failed", SIP_ZERO, SIP_ZERO);
            pMsgBody->SipDelete();
            return SIP_FALSE;
        }
        if (m_objBodyList.Add(pMsgBody) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMsgBodyList::DecodeMIMEBody:Body Decoding failed", SIP_ZERO, SIP_ZERO);
            pMsgBody->SipDelete();
            return SIP_FALSE;
        }

        /*Update the start point to the start of next body*/
        /* 2 for -- 1 for len and 2 for CRLF*/
        // pTempEnd doesn't include CRLF before boundary information
        pStartPt = pTempEnd + SIP_TWO + nBoundaryLen + SIP_FIVE;
    }
    return SIP_TRUE;
}
