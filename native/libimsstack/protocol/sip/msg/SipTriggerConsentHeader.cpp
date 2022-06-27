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
#include "msg/SipTriggerConsentHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

SipTriggerConsentHeader::SipTriggerConsentHeader() :
        SipHeaderBase(SipHeaderBase::TRIGGER_CONSENT),
        m_pSipUri(SIP_NULL)
{
}

SipTriggerConsentHeader::SipTriggerConsentHeader(const SipTriggerConsentHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pSipUri(SIP_NULL)
{
    if (objHeader.m_pSipUri != SIP_NULL)
    {
        m_pSipUri = new SipUri(*(objHeader.m_pSipUri));
    }
}

/*destructor*/
SipTriggerConsentHeader::~SipTriggerConsentHeader()
{
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->SipDelete();
    }
}

/*virtual methods*/
SIP_BOOL SipTriggerConsentHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pSipUri == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing SipUri", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pSipUri->Encode(objBuffer, SIP_TRUE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "Encode: Encoding SipUri failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

/*Function for encoding of headers*/
SIP_BOOL SipTriggerConsentHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pSipUri == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeHdr: SIP Uri missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pSipUri->EncodeSipUri(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeHdr: SIP Uri Encoding Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SipUri* SipTriggerConsentHeader::GetSipUri()
{
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->increment();
        return m_pSipUri;
    }
    return SIP_NULL;
}

SIP_BOOL SipTriggerConsentHeader::SetSipUri(SipUri* pSipUri)
{
    if (pSipUri == SIP_NULL)
    {
        return SIP_FALSE;
    }
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->SipDelete();
    }
    pSipUri->increment();
    m_pSipUri = pSipUri;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipTriggerConsentHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipTriggerConsentHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    if (*pStartPt == LEFT_ANGLE)
    {
        pStartPt++;
    }

    if (*pEndPt == RIGHT_ANGLE)
    {
        pEndPt--;
    }

    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr: Hdr Prm Decoding Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }
    /*Update the decode length*/
    nDecLen = pEndPt - pStartPt + SIP_ONE;
    /*Now Decode the SIP URI*/
    m_pSipUri = new SipUri();
    if (m_pSipUri == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // skip "sip:" from the user name
    SIP_CHAR* pszSIPScheme = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pszSIPScheme, COLON) == SIP_TRUE)
    {
        SIP_CHAR* pszTempScheme = sipCreateString(pStartPt, pszSIPScheme);
        if (SipPf_Stricmp(pszTempScheme, "sip") == SIP_ZERO)
        {
            pStartPt = pszSIPScheme + 2;
            nDecLen -= SipPf_Strlen("sip:");
        }
        if (pszTempScheme != SIP_NULL)
        {
            delete[] pszTempScheme;
        }
    }

    if (m_pSipUri->DecodeSipUri(pStartPt, nDecLen) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: SIP URI Decoding Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipTriggerConsentHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipTriggerConsentHeader(*reinterpret_cast<SipTriggerConsentHeader*>(pHeader));
    }
    return new SipTriggerConsentHeader();
}
