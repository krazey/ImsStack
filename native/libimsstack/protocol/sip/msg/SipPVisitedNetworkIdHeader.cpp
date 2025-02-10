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
#include "msg/SipPVisitedNetworkIdHeader.h"
#include "platform/SipString.h"

SipPVisitedNetworkIdHeader::SipPVisitedNetworkIdHeader() :
        SipHeaderBase(SipHeaderBase::P_VISITED_NETWORK_ID)
{
}

SipPVisitedNetworkIdHeader::SipPVisitedNetworkIdHeader(
        const SipPVisitedNetworkIdHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipPVisitedNetworkIdHeader::~SipPVisitedNetworkIdHeader() {}

SIP_BOOL SipPVisitedNetworkIdHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Case of nothing is present*/
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*First Check the presence of Header Prm i.e. ";"
      And decode if present*/
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;
    if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, SIP_SEMI) ==
            SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Header parameter decoding failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Update the End point for Previous Decoding*/
        pEndPt = pTempPre;
    }

    /* value can be token or quoted string
       P-Visited-Network-ID   = "P-Visited-Network-ID" HCOLON vnetwork-spec *(COMMA vnetwork-spec)
       vnetwork-spec  = (token / quoted-string) *(SEMI vnetwork-param)
       vnetwork-param = generic-param
     */
    //--------------------------------------------------------------------------------
    if ((DQUOTE == *pStartPt) &&
            (pEndPt != SIP_NULL && (DQUOTE == *pEndPt) && BACKSLASH != *(pEndPt - 1)))
    {
        pStartPt = pStartPt + SIP_ONE;
        pEndPt = pEndPt - SIP_ONE;
    }
    //-----------------------------------------------------------------------------
    SIP_CHAR* pszValue = SipAbnfUtil::CreateString(pStartPt, pEndPt);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    return SIP_TRUE;
}

SipHeaderBase* SipPVisitedNetworkIdHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPVisitedNetworkIdHeader(
                *reinterpret_cast<SipPVisitedNetworkIdHeader*>(pHeader));
    }
    return new SipPVisitedNetworkIdHeader();
}
