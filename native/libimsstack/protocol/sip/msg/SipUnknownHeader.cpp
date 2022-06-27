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
#include "msg/SipUnknownHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipUnknownHeader::SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUnknownHeader::SipUnknownHeader() :
        SipHeaderBase(SipHeaderBase::UNKNOWN),
        m_pszHdrName(SIP_NULL),
        m_pszHdrValue(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipUnknownHeader::SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUnknownHeader::SipUnknownHeader(const SipUnknownHeader& objHeader) :
        SipHeaderBase(SipHeaderBase::UNKNOWN),
        m_pszHdrName(SipPf_Strdup(objHeader.m_pszHdrName)),
        m_pszHdrValue(SipPf_Strdup(objHeader.m_pszHdrValue))
{
}

/******************************************************************************
 * Function name      : SipUnknownHeader::~SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUnknownHeader::~SipUnknownHeader()
{
    if (m_pszHdrName != SIP_NULL)
    {
        delete[] m_pszHdrName;
    }
    if (m_pszHdrValue != SIP_NULL)
    {
        delete[] m_pszHdrValue;
    }
}

SIP_BOOL SipUnknownHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pszHdrName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Missing header name", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszHdrName;
    objBuffer += COLON;
    objBuffer += SPACE;
    objBuffer += m_pszHdrValue;

    if (bParams == SIP_TRUE)
    {
        // Later, if the unknown header parses the header parameters,
        // enable the following code.
        // return EncodeParameters(objBuffer);
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUnknownHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUnknownHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_pszHdrName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Header name not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszHdrName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_COLON(*ppCurrPos);

    SIP_ENC_SP(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszHdrValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUnknownHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUnknownHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    (void)pStartPt;
    (void)nDecLen;

    return SIP_TRUE;
}

SIP_BOOL SipUnknownHeader::SetHeaderName(const SIP_CHAR* pszHdrName)
{
    return SetCharVar(pszHdrName, m_pszHdrName);
}

SIP_BOOL SipUnknownHeader::SetHeaderValue(const SIP_CHAR* pszHdrValue)
{
    return SetCharVar(pszHdrValue, m_pszHdrValue);
}

SipHeaderBase* SipUnknownHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipUnknownHeader(*reinterpret_cast<SipUnknownHeader*>(pHeader));
    }
    return new SipUnknownHeader();
}
