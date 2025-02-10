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
#include "msg/SipMsgUtil.h"
#include "msg/SipRequestDispositionHeader.h"
#include "platform/SipString.h"

const SIP_CHAR* SipRequestDispositionHeader::DIRECTIVE_STRING
        [SipRequestDispositionHeader::MAX_DIRECTIVE_SIZE] = {"proxy", "redirect", "cancel",
                "no-cancel", "fork", "no-fork", "recurse", "no-recurse", "parallel", "sequential",
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

SIP_BOOL SipRequestDispositionHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::Decode(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Null value in header", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    for (SIP_UINT16 nCnt = 0; nCnt < MAX_DIRECTIVE_SIZE; nCnt++)
    {
        if (SipPf_Strcmp(DIRECTIVE_STRING[nCnt], pszValue) == 0)
        {
            return SIP_TRUE;
        }
    }

    return SIP_FALSE;
}

const SIP_CHAR* SipRequestDispositionHeader::GetDirectiveString(SIP_UINT32 nIndex)
{
    if (nIndex < MAX_DIRECTIVE_SIZE)
    {
        return DIRECTIVE_STRING[nIndex];
    }

    return SIP_NULL;
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
