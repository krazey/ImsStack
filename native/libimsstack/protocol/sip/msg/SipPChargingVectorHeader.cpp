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
#include "msg/SipMsgUtil.h"
#include "msg/SipPChargingVectorHeader.h"
#include "platform/SipString.h"

SipPChargingVectorHeader::SipPChargingVectorHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_pChargingVectorList(SIP_NULL)
{
}

SipPChargingVectorHeader::SipPChargingVectorHeader(const SipPChargingVectorHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pChargingVectorList(SIP_NULL)
{
    if (objHeader.m_pChargingVectorList != SIP_NULL)
    {
        m_pChargingVectorList = new SipNameValue(*(objHeader.m_pChargingVectorList));
    }
}

SipPChargingVectorHeader::~SipPChargingVectorHeader()
{
    if (m_pChargingVectorList != SIP_NULL)
    {
        m_pChargingVectorList->SipDelete();
    }
}

SIP_BOOL SipPChargingVectorHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pChargingVectorList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargingVectorList->Encode(objBuffer) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipPChargingVectorHeader::Encode(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pChargingVectorList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargingVectorList->Encode(ppCurrPos) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipPChargingVectorHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;
    /*Header value is the first node of SipParameterList
      and the other Node will contain SIP parameter*/
    if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, SIP_SEMI) ==
            SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Header parameters decoding failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }

    /*Decode the Header Value*/
    m_pChargingVectorList = new SipNameValue();
    if (m_pChargingVectorList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargingVectorList->Decode(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Name value decoding failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    // charge vector should be icid-value
    if ((GetHdrType() == SipHeaderBase::P_CHARGING_VECTOR) &&
            (SipPf_Stricmp("icid-value", m_pChargingVectorList->m_pszName) != SIP_ZERO))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid header value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPChargingVectorHeader::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPChargingVectorHeader(*reinterpret_cast<SipPChargingVectorHeader*>(pHeader));
    }
    return new SipPChargingVectorHeader(eHeaderType);
}
