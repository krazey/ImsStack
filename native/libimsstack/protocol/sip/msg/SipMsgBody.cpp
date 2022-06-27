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
#include "msg/sip_msgutil.h"
#include "msg/SipMsgBody.h"
#include "msg/SipMessage.h"
#include "platform/sip_pf_memory.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"

#define MAX_BODY_SIZE 1500

extern SIP_CHAR gaszSipHdr[][SIP_MAX_HDR_LEN];

/******************************************************************************
 * Function name      : SipMIMEHdrs::SipMIMEHdrs()
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMIMEHdrs::SipMIMEHdrs() :
        m_pContentType(SIP_NULL),
        m_pContentEncoding(SIP_NULL),
        m_pContentDisposition(SIP_NULL),
        m_pUnKnownHdrList(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipMIMEHdrs::SipMIMEHdrs()
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipMIMEHdrs::~SipMIMEHdrs
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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
            m_pContentType = (SipContentTypeHeader*)pHdr;
            pHdr->increment();
            break;

        case SipHeaderBase::CONTENT_DISPOSITION:
            if (m_pContentDisposition != SIP_NULL)
            {
                m_pContentDisposition->SipDelete();
            }
            m_pContentDisposition = pHdr;
            pHdr->increment();
            break;

        case SipHeaderBase::CONTENT_ENCODING:
            if (m_pContentEncoding != SIP_NULL)
            {
                m_pContentEncoding->SipDelete();
            }
            m_pContentEncoding = pHdr;
            pHdr->increment();
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

/******************************************************************************
 * Function name      : SipMIMEHdrs::getNewMIMEHdrObj
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderBase* SipMIMEHdrs::getMimeHdrObj(SIP_INT32 eIndex)
{
    switch (eIndex)
    {
        case CONTENT_TYPE:
            if (m_pContentType != SIP_NULL)
            {
                m_pContentType->increment();
            }
            return m_pContentType;

        case CONTENT_ENCODING:
            if (m_pContentEncoding != SIP_NULL)
            {
                m_pContentEncoding->increment();
            }
            return m_pContentEncoding;

        case CONTENT_DISPOSITION:
            if (m_pContentDisposition != SIP_NULL)
            {
                m_pContentDisposition->increment();
            }
            return m_pContentDisposition;

        case UNKNOWN:
            if (m_pUnKnownHdrList != SIP_NULL)
            {
                m_pUnKnownHdrList->increment();
            }
            return m_pUnKnownHdrList;

        default:
            return SIP_NULL;
    }
}

/******************************************************************************
 * Function name      : SipMIMEHdrs::getNewMIMEHdrObj
 *
 * Description     : To be used only in decoder for creating MIME headers
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderBase* SipMIMEHdrs::getNewMIMEHdrObj(SIP_INT32 eHdrType)
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
            // break;

        case SipHeaderBase::CONTENT_ENCODING:
            if (m_pContentEncoding != SIP_NULL)
            {
                m_pContentEncoding->SipDelete();
            }
            m_pContentEncoding = new SipHeaderBase(SipHeaderBase::CONTENT_ENCODING);
            return m_pContentEncoding;

            // break;

        case SipHeaderBase::CONTENT_DISPOSITION:
            if (m_pContentDisposition != SIP_NULL)
            {
                m_pContentDisposition->SipDelete();
            }
            m_pContentDisposition = new SipHeaderBase(SipHeaderBase::CONTENT_DISPOSITION);
            return m_pContentDisposition;
            // break;

        default:
            if (m_pUnKnownHdrList == SIP_NULL)
            {
                m_pUnKnownHdrList = new SipHeaderList(SipHeaderBase::UNKNOWN);
            }
            return m_pUnKnownHdrList;
    }

    return SIP_NULL;
}

/******************************************************************************
 * Function name      :SipMIMEHdrs::EncodeMIMEHdrs
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMIMEHdrs::EncodeMIMEHdrs(SIP_CHAR** ppCurrPos)
{
    SIP_INT32 nHdr = SipMIMEHdrs::CONTENT_TYPE;

    while (nHdr < SipMIMEHdrs::END)
    {
        SipHeaderBase* pTemp = getMimeHdrObj(nHdr);
        if (pTemp != SIP_NULL)
        {
            if (pTemp->GetHdrType() != SipHeaderBase::UNKNOWN)
            {
                sipEncodeHdrName(pTemp->GetHdrType(), ppCurrPos, ESIPMSGOPT_NONE);
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

/******************************************************************************
 * Function name      : SipMIMEHdrs::DecodeMIMEHdrs
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMIMEHdrs::DecodeMIMEHdrs(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempPos = SIP_NULL;
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    /*Get the position previous to ":"*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: colon not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempNext = pTempPos + SIP_TWO;
    pTempNext = sipSkipFwLWS(pTempNext, pEndPt);

    /*skip the WSP form back*/
    pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

    /*Create  the header name*/
    SIP_CHAR* pszHdrName = sipCreateString(pStartPt, pTempPos);
    if (pszHdrName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*this will return the type of header on passing name*/
    SIP_INT32 eHdrType = sipGetMimeHdrType(pszHdrName);

    /*Free the header name*/
    /*Get the header object*/
    SipHeaderBase* pHeader = getNewMIMEHdrObj(eHdrType);
    if (pHeader == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEHdrs: getNewMIMEHdrObj failed", SIP_ZERO,
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
        SIP_CHAR* pszHdrValue = SIP_NULL;
        if (pUnknown->SetHeaderName(pszHdrName) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SetHeaderName fail", SIP_ZERO, SIP_ZERO);
            pUnknown->SipDelete();
            delete[] pszHdrName;
            return SIP_FALSE;
        }
        delete[] pszHdrName;

        pszHdrValue = sipCreateString(pTempNext, pEndPt);
        if (pszHdrValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            pUnknown->SipDelete();
            return SIP_FALSE;
        }
        if (pUnknown->SetHeaderValue(pszHdrValue) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SetHeaderValue Fail", SIP_ZERO, SIP_ZERO);
            pUnknown->SipDelete();
            delete[] pszHdrValue;
            return SIP_FALSE;
        }
        delete[] pszHdrValue;

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
/******************************************************************************
 * Function name      : SipMsgBody::SipMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMsgBody::SipMsgBody() :
        m_eBodyType(SINGLE_BODY),
        m_pMIMEHdrs(SIP_NULL),
        m_pMessageSummary(SIP_NULL),
        m_pBuffer(SIP_NULL),
        m_nBufLen(SIP_ZERO),
        m_pBodyList(SIP_NULL),
        m_bEncodeMime(SIP_TRUE)
{
}

/******************************************************************************
 * Function name      : SipMsgBody::SipMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMsgBody::SipMsgBody(SIP_INT32 eBodyType) :
        m_eBodyType(SINGLE_BODY),
        m_pMIMEHdrs(SIP_NULL),
        m_pMessageSummary(SIP_NULL),
        m_pBuffer(SIP_NULL),
        m_nBufLen(SIP_ZERO),
        m_pBodyList(SIP_NULL),
        m_bEncodeMime(SIP_TRUE)
{
    m_eBodyType = eBodyType;
    m_pMIMEHdrs = new SipMIMEHdrs();
    if ((eBodyType == MULTI_PART_MIXED_BODY) || (eBodyType == MULTI_PART_ALT_BODY))
    {
        m_pBodyList = new SipMsgBodyList();
    }
    else if (eBodyType == MESSAGE_SUMMARY_BODY)
    {
        m_pMessageSummary = new SipMessageSummary();
    }
}

/******************************************************************************
 * Function name      : SipMsgBody::SipUri
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMsgBody::SipMsgBody(const SipMsgBody& objMsgBody) :
        m_eBodyType(objMsgBody.m_eBodyType),
        m_pMIMEHdrs(SIP_NULL),
        m_pMessageSummary(SIP_NULL),
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
    if (objMsgBody.m_pMessageSummary != SIP_NULL)
    {
        m_pMessageSummary = new SipMessageSummary(*(objMsgBody.m_pMessageSummary));
    }
}

/******************************************************************************
 * Function name      : SipMsgBody::~SipMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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
    if (m_pMessageSummary != SIP_NULL)
    {
        m_pMessageSummary->SipDelete();
    }
}

/******************************************************************************
 * Function name      : SipMsgBody::EncodeMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipMsgBody::EncodeMIMEMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBody::EncodeMIMEMsgBody(SIP_CHAR** ppCurrPos)
{
    /*Check for message body list*/
    if (m_pBodyList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeMIMEMsgBody: No body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get The content type to get the boundary*/
    SipContentTypeHeader* pContentType = SIP_STATIC_CAST(SipContentTypeHeader*, GetContentType());
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

/******************************************************************************
 * Function name      : SipMsgBody::DecodeMessageSummaryMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBody::EncodeMessageSummaryMsgBody(SIP_CHAR** ppCurrPos)
{
    if (m_pMessageSummary == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBody::EncodeMessageSummaryMsgBody:Memory Allocation failed", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pMessageSummary->EncodeMessageSummary(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "EncodeMessageSummaryMsgBody:EncodeMessageSummary failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

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

/******************************************************************************
 * Function name      : SipMsgBody::DecodeSingleMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBody::DecodeSingleMsgBody(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
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

/******************************************************************************
 * Function name      : SipMsgBody::DecodeMIMEMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBody::DecodeMIMEMsgBody(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    if (pStartPt == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody: Illegal argument", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pNext1Pt = pStartPt + SIP_ONE;
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
        SIP_CHAR* pTempPos = SIP_NULL;
        // Fail condition to be added
        sipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd);
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
    SipContentTypeHeader* pContentType = SIP_STATIC_CAST(
            SipContentTypeHeader*, m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE));
    if (pContentType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeMIMEMsgBody: Body in not valid", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    const SIP_CHAR* pszMType = pContentType->GetMediaType();
    if (pszMType == SIP_NULL)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "Wronge Content Type", SIP_ZERO, SIP_ZERO);
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
            SIP_TRACE_NORMAL(
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
        m_eBodyType = MULTI_PART_MIXED_BODY;

        if (m_pBodyList->DecodeMIMEBody(pStartPt, pEndPt, pszBoundary) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEBody: failed", SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            delete[] pszBoundary;
            return SIP_FALSE;
        }
        delete[] pszBoundary;
        pszBoundary = SIP_NULL;
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

/******************************************************************************
 * Function name      : SipMsgBody::DecodeMessageSummaryMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBody::DecodeMessageSummaryMsgBody(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    if (m_pMessageSummary == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "DecodeMessageSummaryMsgBody:message-summary object null", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pMessageSummary->DecodeMessageSummary(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMessageSummaryMsgBody:Body Decoding failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_eBodyType = MESSAGE_SUMMARY_BODY;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMsgBody::SetContentType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBody::SetMimeHdr(SipHeaderBase* pHdrBase)
{
    if (m_pMIMEHdrs == SIP_NULL)
    {
        m_pMIMEHdrs = new SipMIMEHdrs();
    }
    if (m_pMIMEHdrs->SetMimeHdrs(pHdrBase))
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipMsgBody::SetMsgBuffer
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipMsgBody::GetMessageBodyList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMsgBodyList* SipMsgBody::GetMessageBodyList()
{
    if (m_pBodyList != SIP_NULL)
    {
        m_pBodyList->increment();
    }
    return m_pBodyList;
}

/******************************************************************************
 * Function name      : SipMsgBody::GetContentType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipContentTypeHeader* SipMsgBody::GetContentType()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        return SIP_DYNAMIC_CAST(
                SipContentTypeHeader*, m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE));
    }

    return SIP_NULL;
}

/******************************************************************************
 * Function name      : SipMsgBody::GetContentEncoding
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderBase* SipMsgBody::GetContentEncoding()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        return m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING);
    }

    return SIP_NULL;
}

/******************************************************************************
 * Function name      : SipMsgBody::GetContentDisposition
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderBase* SipMsgBody::GetContentDisposition()
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        return m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION);
    }

    return SIP_NULL;
}

/******************************************************************************
 * Function name      : SipMsgBody::GetMimeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderBase* SipMsgBody::GetMimeHdr(SIP_INT32 eHdrType, SIP_UINT32 nIndex)
{
    if (m_pMIMEHdrs != SIP_NULL)
    {
        switch (eHdrType)
        {
            case SipHeaderBase::CONTENT_TYPE:
                return m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE);
                break;

            case SipHeaderBase::CONTENT_DISPOSITION:
                return m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION);
                break;

            case SipHeaderBase::CONTENT_ENCODING:
                return m_pMIMEHdrs->getMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING);
                break;

            case SipHeaderBase::UNKNOWN:
                return m_pMIMEHdrs->GetUnknownHdr(nIndex);
                break;

            default:
                return SIP_NULL;
        }
    }

    return SIP_NULL;
}

/******************************************************************************
 * Function name      :SipMsgBody::IsMessageBodySDP();
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipMsgBody::GetMsgBuffer
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipMsgBodyList::SipMsgBodyList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipMsgBodyList::SipMsgBodyList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name  : SipMsgBodyList::EncodeBody
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SIP_BOOL SipMsgBodyList::EncodeBody(SIP_CHAR** ppMsgBuffCurrPos, SIP_CHAR* pszBoundary)
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
        SIP_CHAR** ppMsgBufer, SIP_UINT32& nMsgLen, SIP_CHAR* pszBoundary)
{
    SIP_UINT32 nCount = m_objBodyList.GetSize();
    if (nCount == SIP_ZERO)
    {
        return SIP_TRUE;
    }

    SIP_CHAR* pBufferStart = *ppMsgBufer;

    SIP_UINT32 nLen = SIP_ZERO;
    /*Now check for length calculation with boundary or without boundary*/
    if (pszBoundary == SIP_NULL)
    {
        SipMsgBody* pBody = m_objBodyList.GetAt(SIP_ZERO);
        pBody->EncodeSingleMsgBody(&pBufferStart);
        pBody->GetMsgBuffLen(&nLen);
    }
    else
    {
        if (EncodeBody(&pBufferStart, pszBoundary) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode msg body fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        nLen = SipPf_Strlen(*ppMsgBufer);
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
    pBody->increment();
    return SIP_TRUE;
}

SipMsgBody* SipMsgBodyList::GetBodyByIndex(SIP_UINT32 nIndex)
{
    if (nIndex < m_objBodyList.GetSize())
    {
        SipMsgBody* pBody = m_objBodyList.GetAt(nIndex);
        pBody->increment();
        return pBody;
    }
    return SIP_NULL;
}

/******************************************************************************
 * Function name      : SipMsgBodyList::DecodeSingleBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBodyList::DecodeSingleBody(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
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

/******************************************************************************
 * Function name      : SipMsgBodyList::DecodeMIMEBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMsgBodyList::DecodeMIMEBody(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR* pszBoundary)
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
    SIP_CHAR* pTempPos = SIP_NULL;

    if (sipFindCrlf(pStartPt, pEndPt, &pTempPos) == SIP_TRUE)
    {
        pszTempBoundary = sipCreateString(pStartPt, pTempPos);
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
    pStartPt = sipSkipFwLWS(pStartPt, pEndPt);

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
        SIP_CHAR* pTempEnd = sipFindBodyEnd(pStartPt, pEndPt, pszBoundary, bBodyEnd);

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

SIP_BOOL SipMsgBodyList::DecodeMessageSummaryBody(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    SipMsgBody* pMsgBody = new SipMsgBody(SipMsgBody::MESSAGE_SUMMARY_BODY);
    if (pMsgBody == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeMessageSummaryBody:Memory Allocation failed", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    if (pMsgBody->DecodeMessageSummaryMsgBody(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeMessageSummaryBody:Body Decoding failed", SIP_ZERO,
                SIP_ZERO);
        pMsgBody->SipDelete();
        return SIP_FALSE;
    }

    if (AddBody(pMsgBody) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMsgBodyList::DecodeMessageSummaryBody:Body Decoding failed", SIP_ZERO,
                SIP_ZERO);
        pMsgBody->SipDelete();
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipMessageSummary::SipMessageSummary() :
        m_nStatus(SipMessageSummary::MSG_WAITING_NO),
        m_pAddrSpec(SIP_NULL),
        m_objSummaryLineList(SipVector<SipSummaryLine*>()),
        m_objNameValueList(SipVector<SipNameValue*>())
{
}

SipMessageSummary::SipMessageSummary(const SipMessageSummary& objMessageSummary) :
        m_nStatus(objMessageSummary.m_nStatus),
        m_pAddrSpec(SIP_NULL),
        m_objSummaryLineList(SipVector<SipSummaryLine*>()),
        m_objNameValueList(SipVector<SipNameValue*>())
{
    if (objMessageSummary.m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec = new SipAddrSpec(*(objMessageSummary.m_pAddrSpec));
    }

    SIP_UINT32 nSize = objMessageSummary.m_objSummaryLineList.GetSize();
    for (SIP_INT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SipSummaryLine* pTempLine = objMessageSummary.m_objSummaryLineList.GetAt(nCount);
        if (pTempLine)
        {
            SipSummaryLine* pLine = new SipSummaryLine(*pTempLine);
            if (pLine == SIP_NULL)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipMsgBodyList::SipMsgBodyList:Copy Constructor Malloc failed", SIP_ZERO,
                        SIP_ZERO);
            }
            if ((m_objSummaryLineList.Add(pLine)) < 0)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipMsgBodyList::SipMsgBodyList:Copy Constructor Append Failed", SIP_ZERO,
                        SIP_ZERO);
            }
            pLine = SIP_NULL;
        }
    }
    nSize = objMessageSummary.m_objNameValueList.GetSize();
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SipNameValue* pTempNmVl = objMessageSummary.m_objNameValueList.GetAt(nCount);
        if (pTempNmVl)
        {
            SipNameValue* pNmVl = new SipNameValue(*pTempNmVl);
            if (pNmVl == SIP_NULL)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipMsgBodyList::SipMsgBodyList:Copy Constructor Malloc failed", SIP_ZERO,
                        SIP_ZERO);
            }
            if (m_objNameValueList.Add(pNmVl) < 0)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipMsgBodyList::SipMsgBodyList:Copy Constructor Append Failed", SIP_ZERO,
                        SIP_ZERO);
            }
            pNmVl = SIP_NULL;
        }
    }
}

SipMessageSummary::~SipMessageSummary()
{
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->SipDelete();
    }

    while (m_objSummaryLineList.IsEmpty() != SIP_TRUE)
    {
        SipSummaryLine* pSummaryLine = m_objSummaryLineList.Top();
        pSummaryLine->SipDelete();
        m_objSummaryLineList.Pop();
    }

    while (m_objNameValueList.IsEmpty() != SIP_TRUE)
    {
        SipNameValue* pNameValue = m_objNameValueList.Top();
        pNameValue->SipDelete();
        m_objNameValueList.Pop();
    }
}

/******************************************************************************
 * Function name      : SipMessageSummary::EncodeMessageSummaryMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessageSummary::EncodeMessageSummary(SIP_CHAR** ppCurrPos)
{
    SIP_CHAR* pBody = *ppCurrPos;

    SipPf_Strcpy(pBody, "Messages-Waiting: ");
    SipEnc_UpdateCurrPos(&pBody);

    if (m_nStatus == SipMessageSummary::MSG_WAITING_NO)
    {
        SipPf_Strcpy(pBody, "No");
    }
    else if (m_nStatus == SipMessageSummary::MSG_WAITING_YES)
    {
        SipPf_Strcpy(pBody, "Yes");
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipMessageSummary::EncodeMessageSummaryMsgBody: Missing Message Waiting Status ",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipEnc_UpdateCurrPos(&pBody);
    SIP_ENC_CRLF(pBody);

    if (m_pAddrSpec != SIP_NULL)
    {
        SipPf_Strcpy(pBody, "Message-Account: ");
        SipEnc_UpdateCurrPos(&pBody);

        if (m_pAddrSpec->EncodeAddrSpec(&pBody) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                    "SipMessageSummary::EncodeMessageSummaryMsgBody: Addr Spec failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        SIP_ENC_CRLF(pBody);
    }

    SIP_UINT32 nSize = m_objSummaryLineList.GetSize();
    SIP_UINT16 nIndex = SIP_ZERO;
    while (nIndex < nSize)
    {
        SipSummaryLine* pSummary = m_objSummaryLineList.GetAt(nIndex);

        if (pSummary->EncodeSummaryLine(&pBody) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode summary line fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_ENC_CRLF(pBody);
        nIndex++;
    }

    nIndex = 0;
    nSize = m_objNameValueList.GetSize();

    if (nSize != 0)
    {
        SIP_ENC_CRLF(pBody);
    }

    while (nIndex < nSize)
    {
        SipNameValue* pcNmVl = m_objNameValueList.GetAt(nIndex);

        SipPf_Strcpy(pBody, pcNmVl->m_pszName);  // copy name
        SipEnc_UpdateCurrPos(&pBody);
        SIP_ENC_COLON(pBody);
        SIP_ENC_SP(pBody);

        SIP_UINT32 nSizeVl = pcNmVl->m_valueList.GetSize();
        SIP_UINT32 nIndexVl = SIP_ZERO;
        while (nIndexVl < nSizeVl)
        {
            SIP_CHAR* pszVal = pcNmVl->m_valueList.GetAt(nIndexVl);
            SipPf_Strcpy(pBody, pszVal);
            SipEnc_UpdateCurrPos(&pBody);

            if (nIndexVl < (nSizeVl - SIP_ONE))
            {
                SIP_ENC_COMMA(pBody);
            }
            nIndexVl++;
        }

        SIP_ENC_CRLF(pBody);
        nIndex++;
    }

    *ppCurrPos = pBody;

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessageSummary::DecodeMessageSummaryMsgBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessageSummary::DecodeMessageSummary(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    SIP_UINT32 nSize = pEndPt - pStartPt + SIP_ONE;

    if (nSize <= SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody:InvalidInput", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Message Waiting = yes/no
    SIP_CHAR* pTempPos = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: COLON Not Found", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempNext = pTempPos + SIP_TWO;
    pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
    /*skip the WSP form back*/
    pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

    SIP_CHAR* pTempStr = sipCreateString(pStartPt, pTempPos);
    if (pTempStr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (SipPf_Stricmp(pTempStr, "Messages-Waiting"))
    {
        delete[] pTempStr;
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody:InvalidInput", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    delete[] pTempStr;
    pTempStr = SIP_NULL;
    pStartPt = pTempNext;

    if (sipFindCrlf(pStartPt, pEndPt, &pTempPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: CRLF Not Found", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = sipSkipFwLWS(pTempNext, pEndPt);
    /*skip the WSP form back*/
    pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

    pTempStr = sipCreateString(pStartPt, pTempPos);
    if (pTempStr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (SipPf_Stricmp(pTempStr, "Yes") == SIP_ZERO)
    {
        m_nStatus = SipMessageSummary::MSG_WAITING_YES;
    }
    else if (SipPf_Stricmp(pTempStr, "No") == SIP_ZERO)
    {
        m_nStatus = SipMessageSummary::MSG_WAITING_NO;
    }
    else
    {
        delete[] pTempStr;
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody:InvalidInput", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    delete[] pTempStr;

    pTempStr = SIP_NULL;
    pTempNext = pTempPos + SIP_THREE;
    pStartPt = pTempNext;

    if ((pStartPt < pEndPt) && !(IS_CR(*pStartPt) && IS_LF(*(pStartPt + SIP_ONE))))
    {
        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: COLON Not Found", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }

        pTempNext = pTempPos + SIP_TWO;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
        /*skip the WSP form back*/
        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

        pTempStr = sipCreateString(pStartPt, pTempPos);
        if (pTempStr == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (SipPf_Stricmp(pTempStr, "Message-Account"))
        {
            delete[] pTempStr;
            pTempStr = SIP_NULL;
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody:InvalidInput", SIP_ZERO,
                    SIP_ZERO);
            goto label;
        }
        delete[] pTempStr;
        pStartPt = pTempNext;

        if (sipFindCrlf(pStartPt, pEndPt, &pTempPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: CRLF Not Found", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);
        m_pAddrSpec = new SipAddrSpec();
        if (m_pAddrSpec == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_UINT32 nDecLen = pTempPos - pStartPt + SIP_ONE;
        if (m_pAddrSpec->DecodeAddrSpec(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: URI Wrong", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTempNext = pTempPos + SIP_THREE;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
        pStartPt = pTempNext;
    }
label:
    // Summary line

    while (pStartPt < pEndPt)
    {
        if (IS_CR(*pStartPt) && IS_LF(*(pStartPt + SIP_ONE)))
        {
            break;
        }

        if (sipFindCrlf(pStartPt, pEndPt, &pTempPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: CRLF Not Found", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);
        SipSummaryLine* pSummary = new SipSummaryLine();
        if (pSummary == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pSummary->DecodeSummaryLine(pStartPt, pTempPos + SIP_TWO) == SIP_FALSE)
        {
            delete pSummary;
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody:InvalidInput", SIP_ZERO,
                    SIP_ZERO);
            break;
        }

        if (m_objSummaryLineList.Add(pSummary) < 0)
        {
            delete pSummary;
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Node not added", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTempNext = pTempPos + SIP_THREE;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
        pStartPt = pTempNext;
    }

    if (IS_CR(*pStartPt) && IS_LF(*(pStartPt + SIP_ONE)))
    {
        pStartPt = pStartPt + SIP_TWO;
    }

    // Name Value Validation
    while (pStartPt < pEndPt)
    {
        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: CRLF Not Found", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }

        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);
        pTempNext = pTempPos + SIP_TWO;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);

        pTempStr = sipCreateString(pStartPt, pTempPos);
        if (pTempStr == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        SipNameValue* pNmVl = new SipNameValue();
        if (pNmVl == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            delete[] pTempStr;
            return SIP_FALSE;
        }
        pNmVl->m_pszName = pTempStr;
        pStartPt = pTempNext;

        while (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COMMA))
        {
            if (sipFindCrlf(pStartPt, pTempPos, &pTempPos))
            {
                break;
            }
            pTempPos = sipSkipRwWSP(pStartPt, pTempPos);
            pTempNext = pTempPos + SIP_TWO;
            pTempNext = sipSkipFwLWS(pTempNext, pEndPt);

            pTempStr = sipCreateString(pStartPt, pTempPos);
            if (pTempStr == SIP_NULL)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                        SIP_ZERO, SIP_ZERO);
                delete pNmVl;
                return SIP_FALSE;
            }
            if (pNmVl->m_valueList.Add(pTempStr) < 0)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipMessageSummary::DecodeMessageSummaryMsgBody: Node not added", SIP_ZERO,
                        SIP_ZERO);
                delete pNmVl;
                return SIP_FALSE;
            }
            pStartPt = pTempNext;
        }

        if (sipFindCrlf(pStartPt, pEndPt, &pTempPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: CRLF Not Found", SIP_ZERO,
                    SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }
        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);
        pTempNext = pTempPos + SIP_THREE;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
        pTempStr = sipCreateString(pStartPt, pTempPos);
        if (pTempStr == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }
        if (pNmVl->m_valueList.Add(pTempStr) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Node not added", SIP_ZERO,
                    SIP_ZERO);
            delete[] pTempStr;
            delete pNmVl;
            return SIP_FALSE;
        }
        pStartPt = pTempNext;
        if (m_objNameValueList.Add(pNmVl) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Node not added", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipSummaryLine::SipSummaryLine() :
        pMedia(SIP_NULL),
        newMessages(SIP_ZERO),
        oldMessages(SIP_ZERO),
        newUrgentMessages(SIP_ZERO),
        oldUrgentMessages(SIP_ZERO)
{
}

SipSummaryLine::SipSummaryLine(const SipSummaryLine& objSummaryLine) :
        pMedia(SipPf_Strdup(objSummaryLine.GetMedia())),
        newMessages(objSummaryLine.GetNewMessages()),
        oldMessages(objSummaryLine.GetOldMessages()),
        newUrgentMessages(objSummaryLine.GetNewUrgentMessages()),
        oldUrgentMessages(objSummaryLine.GetOldUrgentMessages())
{
}

SipSummaryLine::~SipSummaryLine()
{
    if (pMedia != SIP_NULL)
    {
        delete[] pMedia;
    }
}

SIP_BOOL SipSummaryLine::EncodeSummaryLine(SIP_CHAR** ppCurrPos)
{
    SipPf_Strcpy(*ppCurrPos, GetMedia());

    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_COLON(*ppCurrPos);

    SIP_ENC_SP(*ppCurrPos);

    SIP_CHAR newmsg[SIP_MAX_INT];
    SipPf_Sprintf(newmsg, (SIP_CHAR*)"%d", GetNewMessages());
    SipPf_Strcpy(*ppCurrPos, newmsg);

    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_SLASH(*ppCurrPos);

    SIP_CHAR oldmsg[SIP_MAX_INT];
    SipPf_Sprintf(oldmsg, (SIP_CHAR*)"%d", GetOldMessages());
    SipPf_Strcpy(*ppCurrPos, oldmsg);

    SipEnc_UpdateCurrPos(ppCurrPos);

    if ((GetOldUrgentMessages() != SIP_ZERO) || (GetNewUrgentMessages() != SIP_ZERO))
    /* if there are urgent messages */
    {
        SIP_ENC_SP(*ppCurrPos);

        SIP_ENC_LPAREN(*ppCurrPos);

        SIP_CHAR newurgentmsg[SIP_MAX_INT];
        SipPf_Sprintf(newurgentmsg, (SIP_CHAR*)"%d", GetNewUrgentMessages());
        SipPf_Strcpy(*ppCurrPos, newurgentmsg);

        SipEnc_UpdateCurrPos(ppCurrPos);

        SIP_ENC_SLASH(*ppCurrPos);

        SIP_CHAR oldurgentmsg[SIP_MAX_INT];
        SipPf_Sprintf(oldurgentmsg, (SIP_CHAR*)"%d", GetOldUrgentMessages());
        SipPf_Strcpy(*ppCurrPos, oldurgentmsg);

        SipEnc_UpdateCurrPos(ppCurrPos);

        SIP_ENC_RPAREN(*ppCurrPos);
    }

    return SIP_TRUE;
}

SIP_BOOL SipSummaryLine::DecodeSummaryLine(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    if (pStartPt >= pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipSummaryLine :: DecodeSummaryLine:InvalidInput",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempPos = SIP_NULL;
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipSummaryLine :: DecodeSummaryLine: COLON Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempNext = pTempPos + SIP_TWO;
    pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
    /*skip the WSP form back*/
    pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

    SIP_CHAR* pTempStr = sipCreateString(pStartPt, pTempPos);
    if (pTempStr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pMedia = pTempStr;

    pStartPt = pTempNext;
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SLASH) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipSummaryLine :: DecodeSummaryLine: COLON Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pTempNext = pTempPos + SIP_TWO;
    pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
    pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

    pTempStr = sipCreateString(pStartPt, pTempPos);
    if (pTempStr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    newMessages = SipPf_Atoi(pTempStr);
    delete[] pTempStr;

    pStartPt = pTempNext;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, LPARAN) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipSummaryLine :: DecodeSummaryLine: PARAN Not Found", SIP_ZERO, SIP_ZERO);
        pTempPos = pEndPt;
    }

    pTempPos = sipSkipRwWSP(pStartPt, pTempPos);
    pTempStr = sipCreateString(pStartPt, pTempPos);
    if (pTempStr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    oldMessages = SipPf_Atoi(pTempStr);
    delete[] pTempStr;

    if (pTempPos == pEndPt)
    {
        return SIP_TRUE;
    }

    pTempNext = pTempPos + SIP_TWO;
    pTempNext = sipSkipFwLWS(pTempNext, pEndPt);

    pStartPt = pTempNext;
    if (pStartPt < pEndPt)
    {
        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SLASH) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipSummaryLine :: DecodeSummaryLine: COLON Not Found", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

        pTempStr = sipCreateString(pStartPt, pTempPos);
        if (pTempStr == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        newUrgentMessages = SipPf_Atoi(pTempStr);
        delete[] pTempStr;
        pTempNext = pTempPos + SIP_TWO;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
        pStartPt = pTempNext;

        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, RPARAN) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipSummaryLine :: DecodeSummaryLine: COLON Not Found", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTempPos = sipSkipRwWSP(pStartPt, pTempPos);

        pTempStr = sipCreateString(pStartPt, pTempPos);
        if (pTempStr == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipMessageSummary::DecodeMessageSummaryMsgBody: Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        oldUrgentMessages = SipPf_Atoi(pTempStr);
        delete[] pTempStr;
        pTempNext = pTempPos + SIP_THREE;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
    }
    else
    {
        pTempNext = pTempPos + SIP_TWO;
        pTempNext = sipSkipFwLWS(pTempNext, pEndPt);
    }
    if (pTempNext < pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipSummaryLine :: DecodeSummaryLine:InvalidInput",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
