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
#include "msg/SipRequestDispositionHeader.h"
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "msg/SipAddrSpec.h"
#include "msg/sip_msgutil.h"

SIP_CHAR gaszDirectivesArray[SIP_DIRECTIVE_SIZE][SIP_DIRECTIVE_LEN] = {"proxy", "redirect",
        "cancel", "no-cancel", "fork", "no-fork", "recurse", "no-recurse", "parallel", "sequential",
        "queue", "no-queue"};

SipRequestDispositionHeader::SipRequestDispositionHeader() :
        SipHeaderBase(SipHeaderBase::REQUEST_DISPOSITION)
{
}

SipRequestDispositionHeader::SipRequestDispositionHeader(
        const SipRequestDispositionHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipRequestDispositionHeader::~SipRequestDispositionHeader() {}

SIP_BOOL SipRequestDispositionHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Null value in header", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    for (SIP_UINT16 nCnt = 0; nCnt < SIP_DIRECTIVE_SIZE; nCnt++)
    {
        if (SipPf_Strcmp(gaszDirectivesArray[nCnt], pszValue) == 0)
        {
            return SIP_TRUE;
        }
    }

    return SIP_FALSE;
}

SipHeaderBase* SipRequestDispositionHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipRequestDispositionHeader(
                *reinterpret_cast<SipRequestDispositionHeader*>(pHeader));
    }
    return new SipRequestDispositionHeader();
}
