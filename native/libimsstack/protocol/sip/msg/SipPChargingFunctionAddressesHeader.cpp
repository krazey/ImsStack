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
#include "msg/SipPChargingFunctionAddressesHeader.h"
#include "SipDebug.h"
#include "platform/SipString.h"
#include "msg/SipMsgUtil.h"

SipPChargingFunctionAddressesHeader::SipPChargingFunctionAddressesHeader() :
        SipHeaderBase(SipHeaderBase::P_CHRG_FUN_ADDR),
        m_pChargeAddr(SIP_NULL)
{
}

SipPChargingFunctionAddressesHeader::SipPChargingFunctionAddressesHeader(
        const SipPChargingFunctionAddressesHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pChargeAddr(SIP_NULL)
{
    if (objHeader.m_pChargeAddr != SIP_NULL)
    {
        m_pChargeAddr = new SipNameValue(*(objHeader.m_pChargeAddr));
    }
}

SipPChargingFunctionAddressesHeader::~SipPChargingFunctionAddressesHeader()
{
    if (m_pChargeAddr != SIP_NULL)
    {
        m_pChargeAddr->SipDelete();
    }
}

SIP_BOOL SipPChargingFunctionAddressesHeader::Encode(
        AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pChargeAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargeAddr->Encode(objBuffer) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipPChargingFunctionAddressesHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pChargeAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipPChargingFunctionAddressesHeader::EncodeHdr:m_pChargeAddr missing", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargeAddr->Encode(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipPChargingFunctionAddressesHeader::EncodeHdr: Name Value Encoding failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipPChargingFunctionAddressesHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipPChargingFunctionAddressesHeader::DecodeHdr",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;
    /*Header value is the first node of SipParameterList
      and the other Node will contain SIP parameter*/
    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipPChargingFunctionAddressesHeader::DecodeHdr: Hdr Prm Decoding Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }

    /*Decode the Header Value*/
    m_pChargeAddr = new SipNameValue();
    if (m_pChargeAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pChargeAddr->Decode(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipAuthInfoHeader::DecodeHdr: Name Value Decoding Successful", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPChargingFunctionAddressesHeader::GetNewObj(
        SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPChargingFunctionAddressesHeader(
                *reinterpret_cast<SipPChargingFunctionAddressesHeader*>(pHeader));
    }
    return new SipPChargingFunctionAddressesHeader();
}
